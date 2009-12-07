// $Id: quote_list.cpp,v 1.21 2005/01/30 04:25:31 bpepers Exp $
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

#include "quote_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "quote_select.h"
#include "quote_master.h"
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

QuoteList::QuoteList(MainWindow* main)
    : ActiveList(main, "QuoteList")
{
    _helpSource = "quote_list.html";

    _list->addTextColumn(tr("Quote"), 10, AlignRight);
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

    _store->setId(_quasar->defaultStore());
    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    setCaption(tr("Customer Quotes"));
    finalize(false);
    _customer->setFocus();
}

QuoteList::~QuoteList()
{
}

bool
QuoteList::isActive(Id quote_id)
{
    Quote quote;
    _quasar->db()->lookup(quote_id, quote);
    return quote.isActive();
}

void
QuoteList::setActive(Id quote_id, bool active)
{
    if (quote_id == INVALID_ID) return;

    Quote quote;
    _quasar->db()->lookup(quote_id, quote);

    Quote orig = quote;
    _quasar->db()->setActive(quote, active);
    _quasar->db()->update(orig, quote);
}

void
QuoteList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id quote_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    QuoteSelect conditions;
    conditions.activeOnly = !showInactive;
    conditions.customer_id = _customer->getId();
    conditions.store_id = _store->getId();
    vector<Quote> quotes;
    _quasar->db()->select(quotes, conditions);

    for (unsigned int i = 0; i < quotes.size(); i++) {
	Quote& quote = quotes[i];
	QDate expiryDate = quote.expiryDate();

	if (!expiryDate.isNull() && expiryDate < QDate::currentDate()) {
	    Quote orig = quote;
	    _quasar->db()->setActive(quote, false);
	    _quasar->db()->update(orig, quote);
	    if (!showInactive) continue;
	}

	Customer customer;
	_quasar->db()->lookup(quote.customerId(), customer);

	ListViewItem* lvi = new ListViewItem(_list, quote.id());
	lvi->setValue(0, quote.number());
	lvi->setValue(1, quote.date());
	lvi->setValue(2, customer.name());
	lvi->setValue(3, quote.total());
	if (showInactive) lvi->setValue(4, !quote.isActive());
	if (quote.id() == quote_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
QuoteList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Customer Quotes"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
QuoteList::performNew()
{
    QuoteMaster* master = new QuoteMaster(_main, INVALID_ID);
    master->show();
}

void
QuoteList::performEdit()
{
    Id quote_id = currentId();
    QuoteMaster* master = new QuoteMaster(_main, quote_id);
    master->show();
}
