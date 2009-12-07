// $Id: tender_count_db.cpp,v 1.7 2005/03/01 19:59:42 bpepers Exp $
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

#include "tender_count.h"
#include "tender_count_select.h"

// Create a TenderCount
bool
QuasarDB::create(TenderCount& count)
{
    if (!validate(count)) return false;

    // Auto allocate count number
    if (count.number().stripWhiteSpace() == "#") {
	fixed number = uniqueNumber("tender_count", "number");
	count.setNumber(number.toString());
    }

    QString cmd = insertCmd("tender_count", "count_id", "number,"
			    "count_date,station_id,employee_id,"
			    "store_id,shift_id");
    Stmt stmt(_connection, cmd);

    insertData(count, stmt);
    stmtSetString(stmt, count.number());
    stmtSetDate(stmt, count.date());
    stmtSetId(stmt, count.stationId());
    stmtSetId(stmt, count.employeeId());
    stmtSetId(stmt, count.storeId());
    stmtSetId(stmt, count.shiftId());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(count)) return false;

    commit();
    dataSignal(DataEvent::Insert, count);
    return true;
}

// Delete a TenderCount
bool
QuasarDB::remove(const TenderCount& count)
{
    if (count.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(count)) return false;
    if (!removeData(count, "tender_count", "count_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, count);
    return true;
}

// Update a TenderCount
bool
QuasarDB::update(const TenderCount& orig, TenderCount& count)
{
    if (orig.id() == INVALID_ID || count.id() == INVALID_ID) return false;
    if (!validate(count)) return false;

    if (count.number().stripWhiteSpace() == "#")
	return error("Can't change number to '#'");

    // Update the count table
    QString cmd = updateCmd("tender_count", "count_id", "number,"
			    "count_date,station_id,employee_id,"
			    "store_id,shift_id");
    Stmt stmt(_connection, cmd);

    updateData(orig, count, stmt);
    stmtSetString(stmt, count.number());
    stmtSetDate(stmt, count.date());
    stmtSetId(stmt, count.stationId());
    stmtSetId(stmt, count.employeeId());
    stmtSetId(stmt, count.storeId());
    stmtSetId(stmt, count.shiftId());
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

// Lookup a TenderCount.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id count_id, TenderCount& count)
{
    if (count_id == INVALID_ID) return false;
    TenderCountSelect conditions;
    vector<TenderCount> counts;

    conditions.id = count_id;
    if (!select(counts, conditions)) return false;
    if (counts.size() != 1) return false;

    count = counts[0];
    return true;
}

// Returns a vector of TenderCounts.
bool
QuasarDB::select(vector<TenderCount>& counts, const TenderCountSelect& conds)
{
    counts.clear();

    QString cmd = selectCmd("tender_count", "count_id", "number,"
			    "count_date,station_id,employee_id,"
			    "store_id,shift_id", conds);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	TenderCount count;
	int next = 1;
	selectData(count, stmt, next);
	count.setNumber(stmtGetString(stmt, next++));
	count.setDate(stmtGetDate(stmt, next++));
	count.setStationId(stmtGetId(stmt, next++));
	count.setEmployeeId(stmtGetId(stmt, next++));
	count.setStoreId(stmtGetId(stmt, next++));
	count.setShiftId(stmtGetId(stmt, next++));
	counts.push_back(count);
    }

    QString cmd1 = "select tender_id,amount from count_tender where "
	"count_id = ? order by seq_num";
    Stmt stmt1(_connection, cmd1);

    for (unsigned int i = 0; i < counts.size(); ++i) {
	Id count_id = counts[i].id();

	stmtSetId(stmt1, count_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    TenderInfo line;
	    int next = 1;
	    line.tender_id = stmtGetId(stmt1, next++);
	    line.amount = stmtGetFixed(stmt1, next++);
	    counts[i].tenders().push_back(line);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const TenderCount& count)
{
    if (!validate((DataObject&)count)) return false;

    if (count.number().stripWhiteSpace().isEmpty())
	return error("Blank count number");

    if (count.date().isNull())
	return error("Blank count date");

    if (count.tenders().size() < 1)
	return error("Count must have at least one line");

    return true;
}

bool
QuasarDB::sqlCreateLines(const TenderCount& count)
{
    QString cmd = insertText("count_tender", "count_id", "seq_num,tender_id,"
			     "amount");
    Stmt stmt(_connection, cmd);

    for (unsigned int i = 0; i < count.tenders().size(); ++i) {
	stmtSetId(stmt, count.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, count.tenders()[i].tender_id);
	stmtSetFixed(stmt, count.tenders()[i].amount);
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const TenderCount& count)
{
    return remove(count, "count_tender", "count_id");
}
