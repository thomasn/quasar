// $Id: count_db.cpp,v 1.18 2005/03/01 19:59:42 bpepers Exp $
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

#include "quasar_db.h"

#include "count.h"
#include "count_select.h"
#include "card.h"
#include "item.h"

// Create a Count
bool
QuasarDB::create(Count& count)
{
    if (!validate(count)) return false;

    // Auto allocate count number
    if (count.number().stripWhiteSpace() == "#") {
	fixed number = uniqueNumber("pcount", "number");
	count.setNumber(number.toString());
    }

    QString cmd = insertCmd("pcount", "count_id", "number,description,"
			    "count_date,store_id,location_id,employee_id");
    Stmt stmt(_connection, cmd);

    insertData(count, stmt);
    stmtSetString(stmt, count.number());
    stmtSetString(stmt, count.description());
    stmtSetDate(stmt, count.date());
    stmtSetId(stmt, count.storeId());
    stmtSetId(stmt, count.locationId());
    stmtSetId(stmt, count.employeeId());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(count)) return false;

    commit();
    dataSignal(DataEvent::Insert, count);
    return true;
}

// Delete a Count
bool
QuasarDB::remove(const Count& count)
{
    if (count.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(count)) return false;
    if (!removeData(count, "pcount", "count_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, count);
    return true;
}

// Update a Count
bool
QuasarDB::update(const Count& orig, Count& count)
{
    if (orig.id() == INVALID_ID || count.id() == INVALID_ID) return false;
    if (!validate(count)) return false;

    if (count.number().stripWhiteSpace() == "#")
	return error("Can't change number to '#'");

    // Update the count table
    QString cmd = updateCmd("pcount", "count_id", "number,description,"
			    "count_date,store_id,location_id,employee_id");
    Stmt stmt(_connection, cmd);

    updateData(orig, count, stmt);
    stmtSetString(stmt, count.number());
    stmtSetString(stmt, count.description());
    stmtSetDate(stmt, count.date());
    stmtSetId(stmt, count.storeId());
    stmtSetId(stmt, count.locationId());
    stmtSetId(stmt, count.employeeId());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");
    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(count)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Count.
bool
QuasarDB::lookup(Id count_id, Count& count)
{
    if (count_id == INVALID_ID) return false;
    CountSelect conditions;
    vector<Count> counts;

    conditions.id = count_id;
    if (!select(counts, conditions)) return false;
    if (counts.size() != 1) return false;

    count = counts[0];
    return true;
}

// Returns a vector of Counts.
bool
QuasarDB::select(vector<Count>& counts, const CountSelect& conditions)
{
    counts.clear();

    QString cmd = selectCmd("pcount", "count_id", "number,description,"
			    "count_date,store_id,location_id,employee_id",
			    conditions, "count_id");
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Count count;
	int next = 1;
	selectData(count, stmt, next);
	count.setNumber(stmtGetString(stmt, next++));
	count.setDescription(stmtGetString(stmt, next++));
	count.setDate(stmtGetDate(stmt, next++));
	count.setStoreId(stmtGetId(stmt, next++));
	count.setLocationId(stmtGetId(stmt, next++));
	count.setEmployeeId(stmtGetId(stmt, next++));
	counts.push_back(count);
    }

    QString cmd1 = "select item_id,number,size_name,size_qty,quantity,"
	"on_hand,oh_set from pcount_item where count_id = ? "
	"order by seq_num";
    Stmt stmt1(_connection, cmd1);

    for (unsigned int i = 0; i < counts.size(); ++i) {
	Id count_id = counts[i].id();

	stmtSetId(stmt1, count_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    CountItem line;
	    int next = 1;
	    line.item_id = stmtGetId(stmt1, next++);
	    line.number = stmtGetString(stmt1, next++);
	    line.size = stmtGetString(stmt1, next++);
	    line.size_qty = stmtGetFixed(stmt1, next++);
	    line.quantity = stmtGetFixed(stmt1, next++);
	    line.on_hand = stmtGetFixed(stmt1, next++);
	    line.oh_set = stmtGetBool(stmt1, next++);
	    counts[i].items().push_back(line);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Count& count)
{
    if (!validate((DataObject&)count)) return false;

    if (count.number().stripWhiteSpace().isEmpty())
	return error("Blank count number");

    if (count.date().isNull())
	return error("Blank count date");

    if (count.items().size() < 1)
	return error("Count must have at least one line");

    for (unsigned int i = 0; i < count.items().size(); ++i) {
	const CountItem& line = count.items()[i];
	if (line.item_id == INVALID_ID) continue;

	Item item;
	if (!lookup(line.item_id, item))
	    return error("Item doesn't exist");
	if (!item.isInventoried())
	    return error("Item isn't inventoried");

	if (line.number.isEmpty())
	    return error("Blank item number");
    }

    return true;
}

bool
QuasarDB::sqlCreateLines(const Count& count)
{
    QString cmd = insertText("pcount_item", "count_id", "seq_num,item_id,"
			     "number,size_name,size_qty,quantity,on_hand,"
			     "oh_set");
    Stmt stmt(_connection, cmd);

    const vector<CountItem>& items = count.items();
    for (unsigned int i = 0; i < items.size(); ++i) {
	const CountItem& line = items[i];

	stmtSetId(stmt, count.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.item_id);
	stmtSetString(stmt, line.number);
	stmtSetString(stmt, line.size);
	stmtSetFixed(stmt, line.size_qty);
	stmtSetFixed(stmt, line.quantity);
	stmtSetFixed(stmt, line.on_hand);
	stmtSetBool(stmt, line.oh_set);
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const Count& count)
{
    return remove(count, "pcount_item", "count_id");
}
