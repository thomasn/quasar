// $Id: receive_lookup.cpp,v 1.12 2004/02/18 20:51:02 bpepers Exp $
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

#include "receive_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "receive_select.h"
#include "list_view_item.h"
#include "lookup_edit.h"
#include "vendor_lookup.h"
#include "receive_master.h"

#include <qlabel.h>
#include <qlayout.h>

ReceiveLookup::ReceiveLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "ReceiveLookup"), activeOnly(false)
{
    _label->setText(tr("Number:"));
    _text->setLength(14);
    _list->addTextColumn(tr("Number"), 14, AlignRight);
    _list->addTextColumn(tr("Vendor"), 30);
    _list->addDateColumn(tr("Date"));
    _list->addMoneyColumn(tr("Amount"));
    setCaption(tr("Vendor Invoice Lookup"));

    QLabel* vendorLabel = new QLabel(tr("Vendor:"), _search);
    vendor = new LookupEdit(new VendorLookup(main, this), _search);
    vendor->setLength(30);
    vendorLabel->setBuddy(vendor);

    setTabOrder(_text, vendor);
    _searchGrid->addWidget(vendorLabel, 1, 0);
    _searchGrid->addWidget(vendor, 1, 1, AlignLeft | AlignVCenter);
}

ReceiveLookup::ReceiveLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "ReceiveLookup"), activeOnly(false)
{
    _label->setText(tr("Number:"));
    _text->setLength(14);
    _list->addTextColumn(tr("Number"), 14, AlignRight);
    _list->addTextColumn(tr("Vendor"), 30);
    _list->addDateColumn(tr("Date"));
    _list->addMoneyColumn(tr("Amount"));
    setCaption(tr("Vendor Invoice Lookup"));

    QLabel* vendorLabel = new QLabel(tr("Vendor:"), _search);
    vendor = new LookupEdit(new VendorLookup(quasar, this), _search);
    vendor->setLength(30);
    vendorLabel->setBuddy(vendor);

    setTabOrder(_text, vendor);
    _searchGrid->addWidget(vendorLabel, 1, 0);
    _searchGrid->addWidget(vendor, 1, 1, AlignLeft | AlignVCenter);
}

ReceiveLookup::~ReceiveLookup()
{
}

QString
ReceiveLookup::lookupById(Id receive_id)
{
    Receive receive;
    if (receive_id != INVALID_ID && _quasar->db()->lookup(receive_id,receive))
	return receive.number();
    return "";
}

vector<DataPair>
ReceiveLookup::lookupByText(const QString& text)
{
    ReceiveSelect conditions;
    if (!text.isEmpty())
	conditions.number = text + "%";
    conditions.vendor_id = vendor->getId();
    conditions.store_id = store_id;
    conditions.activeOnly = activeOnly;

    vector<Receive> receives;
    _quasar->db()->select(receives, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < receives.size(); ++i) {
	data.push_back(DataPair(receives[i].id(), receives[i].number()));
    }

    return data;
}

void
ReceiveLookup::refresh()
{
    ReceiveSelect conditions;
    if (!text().isEmpty())
	conditions.number = text() + "%";
    conditions.vendor_id = vendor->getId();
    conditions.store_id = store_id;
    conditions.activeOnly = activeOnly;

    vector<Receive> receives;
    _quasar->db()->select(receives, conditions);

    _list->clear();
    for (unsigned int i = 0; i < receives.size(); ++i) {
	Receive& receive = receives[i];

	Vendor vendor;
	findVendor(receive.vendorId(), vendor);

	ListViewItem* item = new ListViewItem(_list, receive.id());
	item->setValue(0, receive.number());
	item->setValue(1, vendor.name());
	item->setValue(2, receive.postDate());
	item->setValue(3, receive.total());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
ReceiveLookup::slotNew()
{
    ReceiveMaster* window = new ReceiveMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
ReceiveLookup::slotEdit(Id id)
{
    ReceiveMaster* window = new ReceiveMaster(_main, id);
    window->show();
    reject();
    return window;
}

bool
ReceiveLookup::findVendor(Id vendor_id, Vendor& vendor)
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
