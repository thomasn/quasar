// $Id: item_price_select.cpp,v 1.14 2004/10/15 05:19:46 bpepers Exp $
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

#include "item_price_select.h"

ItemPriceSelect::ItemPriceSelect()
{
    priceOnly = false;
    costOnly = false;
    matchCard = false;
    size = "";
}

ItemPriceSelect::~ItemPriceSelect()
{
}

QString
ItemPriceSelect::where() const
{
    QString where = Select::where();

    if (id != INVALID_ID)
	addIdCondition(where, "price_id", id);
    if (priceOnly)
	addCondition(where, "is_cost = 'N'");
    if (costOnly)
	addCondition(where, "is_cost = 'Y'");
    if (!size.isEmpty())
	addStringCondition(where, "size_name", size);
    if (store_id != INVALID_ID) {
	QString cond = "(store_id = '" + store_id.toString() +
	    "' or store_id is null)";
	addCondition(where, cond);
    }
    if (batch_id != INVALID_ID) {
	QString cond = "item_price.price_id in (select price_id from "
	    "promo_batch_item where promo_batch_item.batch_id = '" +
	    batch_id.toString() + "')";
	addCondition(where, cond);
    }

    if (item.id() != INVALID_ID) {
	QString cond = "(item_id = '" + item.id().toString() + "'";
	if (item.deptId() != INVALID_ID)
	    cond += " or dept_id = '" + item.deptId().toString() + "'";
	if (item.subdeptId() != INVALID_ID)
	    cond += " or subdept_id = '" + item.subdeptId().toString() + "'";
	if (item.groups().size() != 0) {
	    cond += " or item_group in (";
	    for (unsigned int i = 0; i < item.groups().size(); ++i) {
		cond += "'" + item.groups()[i].toString() + "'";
		if (i != item.groups().size() - 1)
		    cond += ",";
	    }
	    cond += ")";
	}
	cond += " or (item_id is null and dept_id is null and "
	    "subdept_id is null and item_group is null))";
	addCondition(where, cond);
    }

    if (matchCard || card.id() != INVALID_ID) {
	QString cond = "(card_id ";
	if (card.id() == INVALID_ID)
	    cond += "is null";
	else
	    cond += "= '" + card.id().toString() + "'";
	if (card.groups().size() != 0) {
	    cond += " or card_group in (";
	    for (unsigned int i = 0; i < card.groups().size(); ++i) {
		cond += "'" + card.groups()[i].toString() + "'";
		if (i != card.groups().size() - 1)
		    cond += ",";
	    }
	    cond += ")";
	}
	cond += ")";
	addCondition(where, cond);
    }

    return where;
}
