// $Id: recurring_db.cpp,v 1.6 2005/03/01 19:59:42 bpepers Exp $
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

#include "recurring.h"
#include "recurring_select.h"
#include "gltx.h"

// Create a Recurring
bool
QuasarDB::create(Recurring& recurring)
{
    if (!validate(recurring)) return false;

    QString cmd = insertCmd("recurring", "recurring_id", "gltx_id,"
			    "description,frequency,day1,day2,max_post,"
			    "last_posted,post_count,group_id");
    Stmt stmt(_connection, cmd);

    insertData(recurring, stmt);
    stmtSetId(stmt, recurring.gltxId());
    stmtSetString(stmt, recurring.description());
    stmtSetInt(stmt, recurring.frequency());
    stmtSetInt(stmt, recurring.day1());
    stmtSetInt(stmt, recurring.day2());
    stmtSetInt(stmt, recurring.maxPostings());
    stmtSetDate(stmt, recurring.lastPosted());
    stmtSetInt(stmt, recurring.postingCount());
    stmtSetId(stmt, recurring.cardGroup());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, recurring);
    return true;
}

// Delete a Recurring
bool
QuasarDB::remove(const Recurring& recurring)
{
    if (recurring.id() == INVALID_ID) return false;
    if (!removeData(recurring, "recurring", "recurring_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, recurring);
    return true;
}

// Update a Recurring
bool
QuasarDB::update(const Recurring& orig, Recurring& recurring)
{
    if (orig.id() == INVALID_ID || recurring.id() == INVALID_ID) return false;
    if (!validate(recurring)) return false;

    // Update the recurring table
    QString cmd = updateCmd("recurring", "recurring_id", "gltx_id,"
			    "description,frequency,day1,day2,max_post,"
			    "last_posted,post_count,group_id");
    Stmt stmt(_connection, cmd);

    updateData(orig, recurring, stmt);
    stmtSetId(stmt, recurring.gltxId());
    stmtSetString(stmt, recurring.description());
    stmtSetInt(stmt, recurring.frequency());
    stmtSetInt(stmt, recurring.day1());
    stmtSetInt(stmt, recurring.day2());
    stmtSetInt(stmt, recurring.maxPostings());
    stmtSetDate(stmt, recurring.lastPosted());
    stmtSetInt(stmt, recurring.postingCount());
    stmtSetId(stmt, recurring.cardGroup());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Recurring.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id recurring_id, Recurring& recurring)
{
    if (recurring_id == INVALID_ID) return false;
    RecurringSelect conditions;
    vector<Recurring> recurrings;

    conditions.id = recurring_id;
    if (!select(recurrings, conditions)) return false;
    if (recurrings.size() != 1) return false;

    recurring = recurrings[0];
    return true;
}

// Returns a vector of Recurrings.
bool
QuasarDB::select(vector<Recurring>& recurrings, const RecurringSelect& conds)
{
    recurrings.clear();

    QString cmd = selectCmd("recurring", "recurring_id", "gltx_id,"
			    "description,frequency,day1,day2,max_post,"
			    "last_posted,post_count,group_id", conds);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Recurring recurring;
	int next = 1;
	selectData(recurring, stmt, next);
	recurring.setGltxId(stmtGetId(stmt, next++));
	recurring.setDescription(stmtGetString(stmt, next++));
	recurring.setFrequency(stmtGetInt(stmt, next++));
	recurring.setDay1(stmtGetInt(stmt, next++));
	recurring.setDay2(stmtGetInt(stmt, next++));
	recurring.setMaxPostings(stmtGetInt(stmt, next++));
	recurring.setLastPosted(stmtGetDate(stmt, next++));
	recurring.setPostingCount(stmtGetInt(stmt, next++));
	recurring.setCardGroup(stmtGetId(stmt, next++));
	recurrings.push_back(recurring);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Recurring& recurring)
{
    if (!validate((DataObject&)recurring)) return false;

    // Check that gltx exists and is not voided
    Gltx gltx;
    if (!lookup(recurring.gltxId(), gltx))
	return error("Transaction doesn't exist");
    if (!gltx.isActive())
	return error("Transaction has been voided");

    // TODO: if card group, check all cards in group are proper type
    // for the transaction type.

    int day1 = recurring.day1();
    int day2 = recurring.day2();
    if (day1 < 1 || day1 > 31) return error("Invalid day 1 value");
    if (day2 < 1 || day2 > 31) return error("Invalid day 2 value");
    if (day2 <= day1) return error("Invalid day values");

    if (recurring.maxPostings() < 0)
	return error("Negative max postings");

    if (recurring.lastPosted().isNull())
	return error("Blank last posted date");

    if (recurring.postingCount() < 0)
	return error("Negative posting count");

    return true;
}
