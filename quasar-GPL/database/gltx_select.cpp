// $Id: gltx_select.cpp,v 1.37 2005/02/24 03:06:12 bpepers Exp $
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

#include "gltx_select.h"
#include "gltx.h"

GltxSelect::GltxSelect()
{
    type = -1;
    number = "";
    reference = "";
    unprinted = false;
    unpaid = false;
    unclosed = false;
    unlinked = false;
    card_type = -1;
    blankStation = false;
    blankEmployee = false;
    cashrecOnly = false;
}

GltxSelect::~GltxSelect()
{
}

QString
GltxSelect::where() const
{
    QString where = Select::where();

    if (id != INVALID_ID)
	addIdCondition(where, "gltx_id", id);
    if (type != -1)
	addCondition(where, "data_type = " + QString::number(type));
    if (start_date == end_date && !start_date.isNull()) {
	addCondition(where, "post_date = '" +
		     start_date.toString(Qt::ISODate) + "'");
    } else {
	if (!start_date.isNull())
	    addCondition(where, "post_date >= '" +
			 start_date.toString(Qt::ISODate) +"'");
	if (!end_date.isNull())
	    addCondition(where, "post_date <= '" +
			 end_date.toString(Qt::ISODate) + "'");
    }

    if (!number.isEmpty())
	addStringCondition(where, "number", number);
    if (!reference.isEmpty())
	addStringCondition(where, "reference_str", reference);
    if (station_id != INVALID_ID)
	addIdCondition(where, "station_id", station_id);
    if (employee_id != INVALID_ID)
	addIdCondition(where, "employee_id", employee_id);
    if (card_id != INVALID_ID)
	addIdCondition(where, "card_id", card_id);
    if (card_group_id != INVALID_ID)
	addCondition(where, "exists (select card_group.group_id from "
		     "card_group where gltx.card_id = card_group.card_id "
		     "and card_group.group_id = '" + card_group_id.toString() +
		     "')");
    if (card_patgroup_id != INVALID_ID)
	addCondition(where, "exists (select card.patgroup_id from "
		     "card where gltx.card_id = card.card_id and "
		     "card.patgroup_id='" + card_patgroup_id.toString() + "')");
    if (cust_type_id != INVALID_ID)
	addCondition(where, "exists (select card_customer.type_id from "
		     "card_customer where gltx.card_id = "
		     "card_customer.customer_id and card_customer.type_id='" +
		     cust_type_id.toString() + "')");
    if (store_id != INVALID_ID)
	addIdCondition(where, "store_id", store_id);
    if (shift_id != INVALID_ID)
	addIdCondition(where, "gltx.shift_id", shift_id);
    if (link_id != INVALID_ID)
	addIdCondition(where, "link_id", link_id);
    if (unprinted)
	addCondition(where, "printed = 'N'");
    if (unpaid)
	addCondition(where, "paid = 'N'");
    if (unclosed)
	addCondition(where, "gltx.shift_id is null");
    if (unlinked)
	addCondition(where, "link_id is null");
    if (card_type != -1)
	addCondition(where, "exists (select card.data_type from card where "
		     "gltx.card_id = card.card_id and card.data_type = " +
		     QString::number(card_type) + ")");
    if (blankStation)
	addCondition(where, "station_id is null");
    if (blankEmployee)
	addCondition(where, "employee_id is null");
    if (cashrecOnly)
	addCondition(where, "gltx.data_type in (7,11,20,21,22,23,24,42)");

    return where;
}
