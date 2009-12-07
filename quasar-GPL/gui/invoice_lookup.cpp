// $Id: invoice_lookup.cpp,v 1.18 2004/09/20 22:48:00 bpepers Exp $
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

#include "invoice_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "invoice_select.h"
#include "list_view_item.h"
#include "lookup_edit.h"
#include "customer_lookup.h"
#include "invoice_master.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>

InvoiceLookup::InvoiceLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "InvoiceLookup"), activeOnly(true)
{
    _label->setText(tr("Number:"));
    _text->setLength(14);
    _list->addTextColumn(tr("Number"), 14, AlignRight);
    _list->addTextColumn(tr("Customer"), 30);
    _list->addDateColumn(tr("Date"));
    _list->addMoneyColumn(tr("Amount"));
    setCaption(tr("Customer Invoice Lookup"));

    QLabel* customerLabel = new QLabel(tr("Customer:"), _search);
    customer = new LookupEdit(new CustomerLookup(main, this), _search);
    customer->setLength(30);
    customerLabel->setBuddy(customer);

    setTabOrder(_text, customer);
    _searchGrid->addWidget(customerLabel, 1, 0);
    _searchGrid->addWidget(customer, 1, 1, AlignLeft | AlignVCenter);
}

InvoiceLookup::InvoiceLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "InvoiceLookup"), activeOnly(true)
{
    _label->setText(tr("Number:"));
    _text->setLength(14);
    _list->addTextColumn(tr("Number"), 14, AlignRight);
    _list->addTextColumn(tr("Customer"), 30);
    _list->addDateColumn(tr("Date"));
    _list->addMoneyColumn(tr("Amount"));
    setCaption(tr("Customer Invoice Lookup"));

    QLabel* customerLabel = new QLabel(tr("Customer:"), _search);
    customer = new LookupEdit(new CustomerLookup(quasar, this), _search);
    customer->setLength(30);
    customerLabel->setBuddy(customer);

    setTabOrder(_text, customer);
    _searchGrid->addWidget(customerLabel, 1, 0);
    _searchGrid->addWidget(customer, 1, 1, AlignLeft | AlignVCenter);
}

InvoiceLookup::~InvoiceLookup()
{
}

QString
InvoiceLookup::lookupById(Id invoice_id)
{
    Invoice invoice;
    if (invoice_id != INVALID_ID && _quasar->db()->lookup(invoice_id,invoice))
	return invoice.number();
    return "";
}

vector<DataPair>
InvoiceLookup::lookupByText(const QString& text)
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    InvoiceSelect conditions;
    if (!text.isEmpty())
	conditions.number = text + "%";
    conditions.card_id = customer->getId();
    conditions.store_id = store_id;
    conditions.activeOnly = activeOnly;

    vector<Invoice> invoices;
    _quasar->db()->select(invoices, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < invoices.size(); ++i) {
	data.push_back(DataPair(invoices[i].id(), invoices[i].number()));
    }

    QApplication::restoreOverrideCursor();
    return data;
}

void
InvoiceLookup::refresh()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    InvoiceSelect conditions;
    if (!text().isEmpty())
	conditions.number = text() + "%";
    conditions.card_id = customer->getId();
    conditions.store_id = store_id;
    conditions.activeOnly = activeOnly;

    int count;
    _quasar->db()->count(count, conditions);
    if (count > 100) {
	QApplication::restoreOverrideCursor();

	QWidget* parent = this;
	if (isHidden() && parentWidget() != NULL)
	    parent = parentWidget();

	QString message = tr("This will select %1 invoices\n"
			     "which may be slow.  Continue?").arg(count);
	int ch = QMessageBox::warning(parent, tr("Are You Sure?"), message,
				      QMessageBox::Yes, QMessageBox::No);
	if (ch != QMessageBox::Yes) return;

	QApplication::setOverrideCursor(waitCursor);
	qApp->processEvents();
    }

    vector<Invoice> invoices;
    _quasar->db()->select(invoices, conditions);

    _list->clear();
    for (unsigned int i = 0; i < invoices.size(); ++i) {
	Invoice& invoice = invoices[i];

	Customer customer;
	findCustomer(invoice.customerId(), customer);

	ListViewItem* item = new ListViewItem(_list, invoice.id());
	item->setValue(0, invoice.number());
	item->setValue(1, customer.name());
	item->setValue(2, invoice.postDate());
	item->setValue(3, invoice.total());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);

    QApplication::restoreOverrideCursor();
}

QWidget*
InvoiceLookup::slotNew()
{
    InvoiceMaster* window = new InvoiceMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
InvoiceLookup::slotEdit(Id id)
{
    InvoiceMaster* window = new InvoiceMaster(_main, id);
    window->show();
    reject();
    return window;
}

bool
InvoiceLookup::findCustomer(Id customer_id, Customer& customer)
{
    for (unsigned int i = 0; i < _customers.size(); ++i) {
	if (_customers[i].id() == customer_id) {
	    customer = _customers[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(customer_id, customer))
	return false;

    _customers.push_back(customer);
    return true;
}
