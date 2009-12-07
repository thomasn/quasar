// $Id: invoice_list.cpp,v 1.19 2004/08/04 19:09:31 bpepers Exp $
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

#include "invoice_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "invoice_select.h"
#include "invoice_master.h"
#include "customer.h"
#include "lookup_edit.h"
#include "customer_lookup.h"
#include "store_lookup.h"
#include "money_valcon.h"
#include "date_valcon.h"
#include "grid.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qmessagebox.h>

InvoiceList::InvoiceList(MainWindow* main)
    : ActiveList(main, "InvoiceList")
{
    _helpSource = "invoice_list.html";

    _list->addTextColumn(tr("Invoice"), 10, AlignRight);
    _list->addDateColumn(tr("Date"));
    _list->addTextColumn(tr("Customer"), 20);
    _list->addMoneyColumn(tr("Amount"));
    _list->setSorting(0);

    QLabel* customerLabel = new QLabel(tr("&Customer:"), _search);
    _customer = new LookupEdit(new CustomerLookup(_main, this), _search);
    _customer->setLength(30);
    customerLabel->setBuddy(_customer);

    QLabel* storeLabel = new QLabel(tr("&Store:"), _search);
    _store = new LookupEdit(new StoreLookup(_main, this), _search);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    QGridLayout* grid = new QGridLayout(_search);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(2, 1);
    grid->addWidget(customerLabel, 0, 0);
    grid->addWidget(_customer, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(storeLabel, 1, 0);
    grid->addWidget(_store, 1, 1, AlignLeft | AlignVCenter);

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    _show_paid = new QCheckBox(tr("Show Paid?"), _extra);
    connect(_show_paid, SIGNAL(toggled(bool)), SLOT(slotPaidChanged()));

    _inactive->setText(tr("Show Voided?"));
    _grid->addWidget(_show_paid, 0, 1, AlignLeft | AlignVCenter);

    setCaption(tr("Customer Invoices"));
    finalize(false);
    _customer->setFocus();
}

InvoiceList::~InvoiceList()
{
}

void
InvoiceList::slotPaidChanged()
{
    clearLists();

    int columns = _list->columns();
    if (_show_paid->isChecked()) {
	_list->addCheckColumn(tr("Paid?"));
	resize(width() + _list->columnWidth(columns), height());
    } else {
	int column = _list->findColumn(tr("Paid?"));
	if (column != -1) {
	    resize(width() - _list->columnWidth(column), height());
	    _list->removeColumn(column);
	}
    }

    slotRefresh();
}

bool
InvoiceList::isActive(Id invoice_id)
{
    Gltx invoice;
    _quasar->db()->lookup(invoice_id, invoice);
    return invoice.isActive();
}

void
InvoiceList::setActive(Id invoice_id, bool active)
{
    if (invoice_id == INVALID_ID) return;

    Invoice invoice;
    _quasar->db()->lookup(invoice_id, invoice);

    Invoice orig = invoice;
    _quasar->db()->setActive(invoice, active);
    _quasar->db()->update(orig, invoice);
}

void
InvoiceList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    bool showPaid = _show_paid->isChecked();
    int inactiveColumn = _list->findColumn(tr("Inactive?"));
    int paidColumn = _list->findColumn(tr("Paid?"));
    Id invoice_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    GltxSelect conditions;
    conditions.type = DataObject::INVOICE;
    conditions.activeOnly = !showInactive;
    conditions.unpaid = !showPaid;
    conditions.card_id = _customer->getId();
    conditions.store_id = _store->getId();

    int count1, count2;
    _quasar->db()->count(count1, conditions);
    conditions.type = DataObject::RETURN;
    _quasar->db()->count(count2, conditions);

    int count = count1 + count2;
    if (count > 50) {
	QApplication::restoreOverrideCursor();

	QString message = tr("This will select %1 items\n"
			     "which may be slow.  Continue?").arg(count);
	int ch = QMessageBox::warning(this, tr("Are You Sure?"), message,
				      QMessageBox::Yes, QMessageBox::No);
	if (ch != QMessageBox::Yes) return;

	QApplication::setOverrideCursor(waitCursor);
	qApp->processEvents();
    }

    conditions.type = DataObject::INVOICE;
    vector<Gltx> invoices;
    _quasar->db()->select(invoices, conditions);

    conditions.type = DataObject::RETURN;
    vector<Gltx> returns;
    _quasar->db()->select(returns, conditions);

    for (unsigned int i = 0; i < returns.size(); ++i)
	invoices.push_back(returns[i]);

    for (unsigned int i = 0; i < invoices.size(); i++) {
	Gltx& invoice = invoices[i];

	Customer customer;
	_quasar->db()->lookup(invoice.cardId(), customer);

	ListViewItem* lvi = new ListViewItem(_list, invoice.id());
	lvi->setValue(0, invoice.number());
	lvi->setValue(1, invoice.postDate());
	lvi->setValue(2, customer.name());
	lvi->setValue(3, invoice.amount());
	if (showInactive) lvi->setValue(inactiveColumn, !invoice.isActive());
	if (showPaid) lvi->setValue(paidColumn, invoice.paid());
	if (invoice.id() == invoice_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
InvoiceList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Customer Invoices"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
InvoiceList::performNew()
{
    InvoiceMaster* master = new InvoiceMaster(_main, INVALID_ID);
    master->show();
}

void
InvoiceList::performEdit()
{
    Id invoice_id = currentId();
    InvoiceMaster* master = new InvoiceMaster(_main, invoice_id);
    master->show();
}
