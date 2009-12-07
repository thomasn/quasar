// $Id: item_select.cpp,v 1.18 2004/09/08 15:07:06 bpepers Exp $
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

#include "item_select.h"

ItemSelect::ItemSelect()
{
    number = "";
    description = "";
    stockedOnly = false;
    purchasedOnly = false;
    soldOnly = false;
    inventoriedOnly = false;
    checkOrderNum = false;
}

ItemSelect::~ItemSelect()
{
}

QString
ItemSelect::where() const
{
    QString where = Select::where();

    if (id != INVALID_ID)
	addIdCondition(where, "item_id", id);
    if (number != "") {
	QString table = "item_plu";
	if (checkOrderNum)
	    table = "item_num";

	QString sub;
	addCondition(sub, table + ".item_id = item.item_id");
	addStringCondition(sub, table + ".number", number);
	QString cond = "(exists (select item_id from " + table + sub + "))";
	addCondition(where, cond);
    }
    if (description != "")
	addStringCondition(where, "description", description);
    if (dept_id != INVALID_ID)
	addIdCondition(where, "dept_id", dept_id);
    if (subdept_id != INVALID_ID)
	addIdCondition(where, "subdept_id", subdept_id);
    if (group_id != INVALID_ID) {
	QString sub;
	addCondition(sub, "item_group.item_id = item.item_id");
	addIdCondition(sub, "item_group.group_id", group_id);
	QString cond = "exists (select group_id from item_group" + sub + ")";
	addCondition(where, cond);
    }
    if (vendor_id != INVALID_ID) {
	QString sub;
	addCondition(sub, "item_vendor.item_id = item.item_id");
	addIdCondition(sub, "item_vendor.vendor_id", vendor_id);
	QString cond = "exists (select vendor_id from item_vendor" + sub + ")";
	addCondition(where, cond);
    }
    if (location_id != INVALID_ID) {
	QString sub;
	addCondition(sub, "item_store.item_id = item.item_id");
	addIdCondition(sub, "item_store.location_id", location_id);
	if (store_id != INVALID_ID)
	    addIdCondition(sub, "item_store.store_id", store_id);
	QString cond = "exists (select stocked from item_store" + sub + ")";
	addCondition(where, cond);
    }
    if (stockedOnly) {
	QString sub;
	addCondition(sub, "item_store.item_id = item.item_id");
	if (store_id != INVALID_ID)
	    addIdCondition(sub, "item_store.store_id", store_id);
	addCondition(sub, "item_store.stocked = 'Y'");
	QString cond = "exists (select stocked from item_store" + sub + ")";
	addCondition(where, cond);
    }
    if (purchasedOnly)
	addCondition(where, "purchased = 'Y'");
    if (soldOnly)
	addCondition(where, "sold = 'Y'");
    if (inventoriedOnly)
	addCondition(where, "inventoried = 'Y'");

    return where;
}
