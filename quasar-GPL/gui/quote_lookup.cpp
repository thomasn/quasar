// $Id: quote_lookup.cpp,v 1.8 2004/02/18 20:51:02 bpepers Exp $
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

#include "quote_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "customer.h"
#include "quote_select.h"
#include "list_view_item.h"
#include "lookup_edit.h"
#include "customer_lookup.h"
#include "quote_master.h"

#include <qlabel.h>
#include <qlayout.h>

QuoteLookup::QuoteLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "QuoteLookup"), activeOnly(true)
{
    _label->setText(tr("Number:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Number"), 20);
    _list->addDateColumn(tr("Date"));
    _list->addTextColumn(tr("Customer"), 30);
    _list->addMoneyColumn(tr("Amount"));
    setCaption(tr("Quote Lookup"));

    QLabel* customerLabel = new QLabel(tr("Customer:"), _search);
    customer = new LookupEdit(new CustomerLookup(main, this), _search);
    customer->setLength(30);
    customerLabel->setBuddy(customer);

    setTabOrder(_text, customer);
    _searchGrid->addWidget(customerLabel, 1, 0);
    _searchGrid->addWidget(customer, 1, 1, AlignLeft | AlignVCenter);
}

QuoteLookup::QuoteLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "QuoteLookup"), activeOnly(true)
{
    _label->setText(tr("Number:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Number"), 20);
    _list->addDateColumn(tr("Date"));
    _list->addTextColumn(tr("Customer"), 30);
    _list->addMoneyColumn(tr("Amount"));
    setCaption(tr("Quote Lookup"));

    QLabel* customerLabel = new QLabel(tr("Customer:"), _search);
    customer = new LookupEdit(new CustomerLookup(quasar, this), _search);
    customer->setLength(30);
    customerLabel->setBuddy(customer);

    setTabOrder(_text, customer);
    _searchGrid->addWidget(customerLabel, 1, 0);
    _searchGrid->addWidget(customer, 1, 1, AlignLeft | AlignVCenter);
}

QuoteLookup::~QuoteLookup()
{
}

QString
QuoteLookup::lookupById(Id quote_id)
{
    Quote quote;
    if (quote_id != INVALID_ID && _quasar->db()->lookup(quote_id, quote))
	return quote.number();
    return "";
}

vector<DataPair>
QuoteLookup::lookupByText(const QString& text)
{
    QuoteSelect conditions;
    if (!text.isEmpty())
	conditions.number = text + "%";
    conditions.customer_id = customer->getId();
    conditions.activeOnly = activeOnly;

    vector<Quote> quotes;
    _quasar->db()->select(quotes, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < quotes.size(); ++i) {
	data.push_back(DataPair(quotes[i].id(), quotes[i].number()));
    }

    return data;
}

void
QuoteLookup::refresh()
{
    QuoteSelect conditions;
    if (!text().isEmpty())
	conditions.number = text() + "%";
    conditions.customer_id = customer->getId();
    conditions.activeOnly = activeOnly;

    vector<Quote> quotes;
    _quasar->db()->select(quotes, conditions);

    _list->clear();
    for (unsigned int i = 0; i < quotes.size(); ++i) {
	Quote& quote = quotes[i];

	Customer customer;
	findCustomer(quote.customerId(), customer);

	ListViewItem* item = new ListViewItem(_list, quote.id());
	item->setValue(0, quote.number());
	item->setValue(1, quote.date());
	item->setValue(2, customer.name());
	item->setValue(3, quote.total());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
QuoteLookup::slotNew()
{
    QuoteMaster* window = new QuoteMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
QuoteLookup::slotEdit(Id id)
{
    QuoteMaster* window = new QuoteMaster(_main, id);
    window->show();
    reject();
    return window;
}

bool
QuoteLookup::findCustomer(Id customer_id, Customer& customer)
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
