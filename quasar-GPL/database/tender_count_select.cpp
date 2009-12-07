// $Id: tender_count_select.cpp,v 1.6 2005/02/24 03:06:12 bpepers Exp $
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

#include "tender_count_select.h"

TenderCountSelect::TenderCountSelect()
{
    number = "";
    unclosed = false;
    blankStation = false;
    blankEmployee = false;
}

TenderCountSelect::~TenderCountSelect()
{
}

QString
TenderCountSelect::where() const
{
    QString where = Select::where();

    if (id != INVALID_ID)
	addIdCondition(where, "count_id", id);
    if (number != "")
	addStringCondition(where, "number", number);
    if (station_id != INVALID_ID)
	addIdCondition(where, "station_id", station_id);
    if (employee_id != INVALID_ID)
	addIdCondition(where, "employee_id", employee_id);
    if (store_id != INVALID_ID)
	addIdCondition(where, "store_id", store_id);
    if (shift_id != INVALID_ID)
	addIdCondition(where, "shift_id", shift_id);
    if (start_date == end_date && !start_date.isNull()) {
	addCondition(where, "count_date = '" +
		     start_date.toString(Qt::ISODate) + "'");
    } else {
	if (!start_date.isNull())
	    addCondition(where, "count_date >= '" +
			 start_date.toString(Qt::ISODate) + "'");
	if (!end_date.isNull())
	    addCondition(where, "count_date <= '" +
			 end_date.toString(Qt::ISODate) + "'");
    }
    if (unclosed)
	addCondition(where, "shift_id is null");
    if (blankStation)
	addCondition(where, "station_id is null");
    if (blankEmployee)
	addCondition(where, "employee_id is null");

    return where;
}
