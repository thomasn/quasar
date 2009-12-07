// $Id: order_lookup.cpp,v 1.17 2004/02/18 20:51:02 bpepers Exp $
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

#include "order_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "order_select.h"
#include "list_view_item.h"
#include "lookup_edit.h"
#include "vendor_lookup.h"
#include "order_master.h"

#include <qlabel.h>
#include <qlayout.h>

OrderLookup::OrderLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "OrderLookup"), activeOnly(true)
{
    _label->setText(tr("Number:"));
    _text->setLength(14);
    _list->addTextColumn(tr("Number"), 14);
    _list->addTextColumn(tr("Vendor"), 30);
    _list->addDateColumn(tr("Date"));
    _list->addMoneyColumn(tr("Amount"));
    setCaption(tr("Purchase Order Lookup"));

    QLabel* vendorLabel = new QLabel(tr("Vendor:"), _search);
    vendor = new LookupEdit(new VendorLookup(main, this), _search);
    vendor->setLength(30);
    vendorLabel->setBuddy(vendor);

    setTabOrder(_text, vendor);
    _searchGrid->addWidget(vendorLabel, 1, 0);
    _searchGrid->addWidget(vendor, 1, 1, AlignLeft | AlignVCenter);
}

OrderLookup::OrderLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "OrderLookup"), activeOnly(true)
{
    _label->setText(tr("Number:"));
    _text->setLength(14);
    _list->addTextColumn(tr("Number"), 14);
    _list->addTextColumn(tr("Vendor"), 30);
    _list->addDateColumn(tr("Date"));
    _list->addMoneyColumn(tr("Amount"));
    setCaption(tr("Purchase Order Lookup"));

    QLabel* vendorLabel = new QLabel(tr("Vendor:"), _search);
    vendor = new LookupEdit(new VendorLookup(quasar, this), _search);
    vendor->setLength(30);
    vendorLabel->setBuddy(vendor);

    setTabOrder(_text, vendor);
    _searchGrid->addWidget(vendorLabel, 1, 0);
    _searchGrid->addWidget(vendor, 1, 1, AlignLeft | AlignVCenter);
}

OrderLookup::~OrderLookup()
{
}

QString
OrderLookup::lookupById(Id order_id)
{
    Order order;
    if (order_id != INVALID_ID && _quasar->db()->lookup(order_id, order))
	return order.number();
    return "";
}

vector<DataPair>
OrderLookup::lookupByText(const QString& text)
{
    OrderSelect conditions;
    if (!text.isEmpty())
	conditions.number = text + "%";
    conditions.vendor_id = vendor->getId();
    conditions.store_id = store_id;
    conditions.activeOnly = activeOnly;

    vector<Order> orders;
    _quasar->db()->select(orders, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < orders.size(); ++i) {
	data.push_back(DataPair(orders[i].id(), orders[i].number()));
    }

    return data;
}

void
OrderLookup::refresh()
{
    OrderSelect conditions;
    if (!text().isEmpty())
	conditions.number = text() + "%";
    conditions.vendor_id = vendor->getId();
    conditions.store_id = store_id;
    conditions.activeOnly = activeOnly;

    vector<Order> orders;
    _quasar->db()->select(orders, conditions);

    _list->clear();
    for (unsigned int i = 0; i < orders.size(); ++i) {
	Order& order = orders[i];

	Vendor vendor;
	findVendor(order.vendorId(), vendor);

	ListViewItem* item = new ListViewItem(_list, order.id());
	item->setValue(0, order.number());
	item->setValue(1, vendor.name());
	item->setValue(2, order.date());
	item->setValue(3, order.total());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
OrderLookup::slotNew()
{
    OrderMaster* window = new OrderMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
OrderLookup::slotEdit(Id id)
{
    OrderMaster* window = new OrderMaster(_main, id);
    window->show();
    reject();
    return window;
}

bool
OrderLookup::findVendor(Id vendor_id, Vendor& vendor)
{
    for (unsigned int i = 0; i < _vendors.size(); ++i) {
	if (_vendors[i].id() == vendor_id) {
	    vendor = _vendors[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(vendor_id, vendor))
	return false;

    _vendors.push_back(vendor);
    return true;
}
