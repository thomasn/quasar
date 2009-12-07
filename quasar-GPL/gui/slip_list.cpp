// $Id: slip_list.cpp,v 1.19 2005/04/03 22:51:25 bpepers Exp $
//
// Copyright (C) 1998-2004 Linux Canada Inc.  All rights reserved.
//
// This file is part of Quasar Accounting
//
// This file may be distributed and/or modified under the terms of the
// GNU General Public License version 2 as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL included in the
// packaging of this file.
//
// Licensees holding a valid Quasar Commercial License may use this file
// in accordance with the Quasar Commercial License Agreement provided
// with the Software in the LICENSE.COMMERCIAL file.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// See http://www.linuxcanada.com or email sales@linuxcanada.com for
// information about Quasar Accounting support and maintenance options.
//
// Contact sales@linuxcanada.com if any conditions of this licensing are
// not clear to you.

#include "slip_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "slip_select.h"
#include "slip_master.h"
#include "vendor.h"
#include "receive.h"
#include "order.h"
#include "slip.h"
#include "object_cache.h"
#include "receive_select.h"
#include "slip_select.h"
#include "lookup_edit.h"
#include "vendor_lookup.h"
#include "store_lookup.h"
#include "date_valcon.h"
#include "grid.h"
#include "output_window.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qmessagebox.h>

SlipList::SlipList(MainWindow* main)
    : ActiveList(main, "SlipList")
{
    _helpSource = "slip_list.html";

    QPopupMenu* process = new QPopupMenu(this);
    process->insertItem(tr("Batch Posting"), this, SLOT(slotPost()));
    menuBar()->insertItem(tr("&Process"), process, -1, 2);

    _list->addTextColumn(tr("Number"), 14);
    _list->addDateColumn(tr("Ship Date"));
    _list->addTextColumn(tr("Status"), 10);
    _list->addTextColumn(tr("Vendor"), 30);
    _list->setSorting(1);

    QLabel* vendorLabel = new QLabel(tr("&Vendor:"), _search);
    _vendor = new LookupEdit(new VendorLookup(_main, this), _search);
    _vendor->setLength(30);
    vendorLabel->setBuddy(_vendor);

    QLabel* storeLabel = new QLabel(tr("&Store:"), _search);
    _store = new LookupEdit(new StoreLookup(_main, this), _search);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    QGridLayout* grid = new QGridLayout(_search);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(2, 1);
    grid->addWidget(vendorLabel, 0, 0);
    grid->addWidget(_vendor, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(storeLabel, 1, 0);
    grid->addWidget(_store, 1, 1, AlignLeft | AlignVCenter);

    _store->setId(_quasar->defaultStore());
    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    _show_posted = new QCheckBox(tr("Show Posted?"), _extra);
    connect(_show_posted, SIGNAL(toggled(bool)), SLOT(slotRefresh()));
    _grid->addWidget(_show_posted, 0, 1, AlignLeft | AlignVCenter);

    setCaption(tr("Packing Slips"));
    finalize(false);
    _vendor->setFocus();
}

SlipList::~SlipList()
{
}

bool
SlipList::isActive(Id slip_id)
{
    Slip slip;
    _quasar->db()->lookup(slip_id, slip);
    return slip.isActive();
}

void
SlipList::setActive(Id slip_id, bool active)
{
    if (slip_id == INVALID_ID) return;

    Slip slip;
    _quasar->db()->lookup(slip_id, slip);

    Slip orig = slip;
    _quasar->db()->setActive(slip, active);
    _quasar->db()->update(orig, slip);
}

void
SlipList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id slip_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    SlipSelect conditions;
    conditions.activeOnly = !showInactive;
    conditions.vendor_id = _vendor->getId();
    conditions.store_id = _store->getId();
    conditions.inc_posted = _show_posted->isChecked();
    vector<Slip> slips;
    _quasar->db()->select(slips, conditions);

    for (unsigned int i = 0; i < slips.size(); i++) {
	Slip& slip = slips[i];

	Vendor vendor;
	_quasar->db()->lookup(slip.vendorId(), vendor);

	ListViewItem* lvi = new ListViewItem(_list, slip.id());
	lvi->setValue(0, slip.number());
	lvi->setValue(1, slip.shipDate());
	lvi->setValue(2, slip.status());
	lvi->setValue(3, vendor.name());
	if (showInactive) lvi->setValue(4, !slip.isActive());
	if (slip.id() == slip_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
SlipList::slotPost()
{
    SlipSelect conditions;
    conditions.activeOnly = true;
    conditions.status = "Received";

    vector<Slip> slips;
    _quasar->db()->select(slips, conditions);

    if (slips.size() == 0) {
	QString message = tr("There are no received slips to be posted");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    int choice = QMessageBox::warning(this, tr("Post Packing Slips"),
			tr("Are you sure you wish to post the packing\n"
			   "slips and create a vendor invoices?"),
			QMessageBox::Yes, QMessageBox::No);
    if (choice != QMessageBox::Yes)
	return;

    OutputWindow* output = new OutputWindow(this);

    ObjectCache cache(_quasar->db());
    vector<Receive> origInvoices;
    vector<Receive> newInvoices;
    vector<Receive> updatedInvoices;

    // Process each packing slip
    for (unsigned int i = 0; i < slips.size(); ++i) {
	Slip& slip = slips[i];
	Id store_id = slip.storeId();

	Vendor vendor;
	if (!cache.findVendor(slip.vendorId(), vendor)) {
	    output->error("Can't find vendor for slip #" + slip.number());
	    continue;
	}

	// Look for existing orders
	vector<Order> orders;
	vector<Id> order_ids;
	for (unsigned int j = 0; j < slip.orders().size(); ++j) {
	    Order order;
	    if (_quasar->db()->lookup(slip.orders()[j], order)) {
		orders.push_back(order);
		order_ids.push_back(order.id());
	    } else {
		output->warning("Failed to find order #" + slip.orders()[j] +
				" for slip #" + slip.number());
	    }
	}

	// Look for existing invoice
	Receive* invoice = NULL;
	if (!slip.invoiceNumber().isEmpty()) {
	    // First look in existing invoices
	    for (unsigned int j = 0; j < updatedInvoices.size(); ++j) {
		if (updatedInvoices[j].number() != slip.invoiceNumber())
		    continue;
		if (updatedInvoices[j].vendorId() != slip.vendorId())
		    continue;

		invoice = &updatedInvoices[j];
		break;
	    }

	    // Next look in new invoices
	    if (invoice == NULL) {
		for (unsigned int j = 0; j < newInvoices.size(); ++j) {
		    if (newInvoices[j].number() != slip.invoiceNumber())
			continue;
		    if (newInvoices[j].vendorId() != slip.vendorId())
			continue;

		    invoice = &newInvoices[j];
		    break;
		}
	    }

	    // And finally try to select it and add it to orig/updated list
	    if (invoice == NULL) {
		ReceiveSelect conditions;
		conditions.vendor_id = slip.vendorId();
		conditions.number = slip.invoiceNumber();

		vector<Receive> invoices;
		_quasar->db()->select(invoices, conditions);

		if (invoices.size() == 1) {
		    origInvoices.push_back(invoices[0]);
		    updatedInvoices.push_back(invoices[0]);
		    invoice = &updatedInvoices[updatedInvoices.size() - 1];
		} else if (invoices.size() > 1) {
		    output->error("Found multiple matches of invoice #" +
				  slip.invoiceNumber() + " for vendor " +
				  vendor.number());
		    continue;
		}
	    }
	}

	// Initialize new invoice if not using old one
	if (invoice == NULL) {
	    Receive newInvoice;
	    newInvoice.setNumber(slip.invoiceNumber());
	    newInvoice.setPostDate(QDate::currentDate());
	    newInvoice.setPostTime(QTime::currentTime());
	    newInvoice.setStoreId(store_id);
	    newInvoice.setStationId(_quasar->defaultStation());
	    newInvoice.setEmployeeId(_quasar->defaultEmployee());
	    newInvoice.setVendorId(slip.vendorId());
	    newInvoice.setVendorAddress(vendor.address());
	    newInvoice.setInvoiceDate(QDate::currentDate());
	    newInvoice.orders() = order_ids;

	    if (newInvoice.number().isEmpty())
		newInvoice.setNumber("#");

	    newInvoices.push_back(newInvoice);
	    invoice = &newInvoices[newInvoices.size() - 1];
	}

	// Add items from packing slip to invoice
	for (unsigned int j = 0; j < slip.items().size(); ++j) {
	    const SlipItem& line1 = slip.items()[j];

	    Item item;
	    if (!cache.findItem(line1.item_id, item)) {
		output->warning("Can't find item");
		continue;
	    }

	    // Cost in vendor invoice comes from the slip, the purchase order,
	    // or the vendor replacement cost
	    fixed ext_cost = line1.ext_cost;
	    if (ext_cost == 0.0) {
		bool found = false;
		for (unsigned int k = 0; k < orders.size(); ++k) {
		    const Order& order = orders[k];
		    for (unsigned int l = 0; l < order.items().size(); ++l) {
			const OrderItem& line = order.items()[l];
			if (line.item_id != line1.item_id) continue;
			if (line.ordered == 0) continue;

			ext_cost = line.ext_cost * line1.quantity/line.ordered;
			ext_cost.moneyRound();
			found = true;
			break;
		    }

		    if (found) break;
		}

		if (!found) {
		    ItemPrice regular;
		    regular.setPrice(item.cost(store_id, line1.size));
		    ext_cost = regular.calculate(line1.quantity);
		}
	    }

	    ReceiveItem line2;
	    line2.item_id = line1.item_id;
	    line2.number = line1.number;
	    line2.size = line1.size;
	    line2.size_qty = line1.size_qty;
	    line2.quantity = line1.quantity;
	    line2.voided = false;
	    line2.cost = Price(ext_cost / line1.quantity);
	    line2.ext_deposit = item.deposit() * line1.size_qty*line1.quantity;
	    line2.ext_cost = ext_cost;
	    line2.tax_id = item.purchaseTax();
	    line2.item_tax_id = item.purchaseTax();
	    line2.include_tax = item.costIncludesTax();
	    line2.include_deposit = item.costIncludesDeposit();
	    invoice->items().push_back(line2);
	}

	// Set status and posted date
	Slip origSlip = slip;
	slip.setStatus("Posted");
	slip.setPostDate(QDate::currentDate());
	if (!_quasar->db()->update(origSlip, slip)) {
	    output->error("Failed setting slip #" + slip.number()+" to posted");
	} else {
	    output->info("Posted slip #" + slip.number());
	}
    }

    // Create new invoices
    for (unsigned int i = 0; i < newInvoices.size(); ++i) {
	Receive& invoice = newInvoices[i];
	_quasar->db()->prepare(invoice, cache);
	if (!_quasar->db()->create(invoice))
	    output->error("Failed creating invoice #" + invoice.number());
	else
	    output->info("Created invoice #" + invoice.number());
    }

    // Update invoices
    for (unsigned int i = 0; i < origInvoices.size(); ++i) {
	Receive& orig = origInvoices[i];
	Receive& invoice = updatedInvoices[i];
	_quasar->db()->prepare(invoice, cache);
	if (!_quasar->db()->update(orig, invoice))
	    output->error("Failed updating invoice #" + orig.number());
	else
	    output->info("Updated invoice #" + invoice.number());
    }

    output->show();
}

void
SlipList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Receivings"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
SlipList::performNew()
{
    SlipMaster* master = new SlipMaster(_main, INVALID_ID);
    master->show();
}

void
SlipList::performEdit()
{
    Id slip_id = currentId();
    SlipMaster* master = new SlipMaster(_main, slip_id);
    master->show();
}
