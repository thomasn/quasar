// $Id: order_list.cpp,v 1.23 2004/01/31 01:50:30 arandell Exp $
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

#include "order_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "order_select.h"
#include "order_master.h"
#include "vendor.h"
#include "lookup_edit.h"
#include "vendor_lookup.h"
#include "store_lookup.h"
#include "money_valcon.h"
#include "date_valcon.h"
#include "grid.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qpopupmenu.h>

OrderList::OrderList(MainWindow* main)
    : ActiveList(main, "OrderList")
{
    _helpSource = "order_list.html";

    _list->addTextColumn(tr("Number"), 14);
    _list->addDateColumn(tr("Date"));
    _list->addTextColumn(tr("Vendor"), 20);
    _list->addMoneyColumn(tr("Amount"));
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

    setCaption(tr("Purchase Orders"));
    finalize(false);
    _vendor->setFocus();
}

OrderList::~OrderList()
{
}

bool
OrderList::isActive(Id order_id)
{
    Order order;
    _quasar->db()->lookup(order_id, order);
    return order.isActive();
}

void
OrderList::setActive(Id order_id, bool active)
{
    if (order_id == INVALID_ID) return;

    Order order;
    _quasar->db()->lookup(order_id, order);

    Order orig = order;
    _quasar->db()->setActive(order, active);
    _quasar->db()->update(orig, order);
}

void
OrderList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id order_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    OrderSelect conditions;
    conditions.activeOnly = !showInactive;
    conditions.vendor_id = _vendor->getId();
    conditions.store_id = _store->getId();
    vector<Order> orders;
    _quasar->db()->select(orders, conditions);

    for (unsigned int i = 0; i < orders.size(); i++) {
	Order& order = orders[i];

	Vendor vendor;
	_quasar->db()->lookup(order.vendorId(), vendor);

	ListViewItem* lvi = new ListViewItem(_list, order.id());
	lvi->setValue(0, order.number());
	lvi->setValue(1, order.date());
	lvi->setValue(2, vendor.name());
	lvi->setValue(3, order.total());
	if (showInactive) lvi->setValue(4, !order.isActive());
	if (order.id() == order_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
OrderList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Purchase Orders"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
OrderList::performNew()
{
    OrderMaster* master = new OrderMaster(_main, INVALID_ID);
    master->show();
}

void
OrderList::performEdit()
{
    Id order_id = currentId();
    OrderMaster* master = new OrderMaster(_main, order_id);
    master->show();
}
