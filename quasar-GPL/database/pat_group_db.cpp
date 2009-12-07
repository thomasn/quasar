// $Id: pat_group_db.cpp,v 1.6 2005/03/01 19:59:42 bpepers Exp $
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
#include "pat_group.h"
#include "pat_group_select.h"
#include "card.h"

// Create a PatGroup
bool
QuasarDB::create(PatGroup& group)
{
    if (!validate(group)) return false;

    QString cmd = insertCmd("pat_group", "patgroup_id", "name,number,"
			    "equity_id,credit_id");
    Stmt stmt(_connection, cmd);

    insertData(group, stmt);
    stmtSetString(stmt, group.name());
    stmtSetString(stmt, group.number());
    stmtSetId(stmt, group.equityId());
    stmtSetId(stmt, group.creditId());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(group)) return false;

    commit();
    dataSignal(DataEvent::Insert, group);
    return true;
}

// Delete a PatGroup
bool
QuasarDB::remove(const PatGroup& group)
{
    if (group.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(group)) return false;
    if (!removeData(group, "pat_group", "patgroup_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, group);
    return true;
}

// Update a PatGroup
bool
QuasarDB::update(const PatGroup& orig, PatGroup& group)
{
    if (orig.id() == INVALID_ID || group.id() == INVALID_ID) return false;
    if (!validate(group)) return false;

    // Update the pat_group table
    QString cmd = updateCmd("pat_group", "patgroup_id", "name,number,"
			    "equity_id,credit_id");
    Stmt stmt(_connection, cmd);

    updateData(orig, group, stmt);
    stmtSetString(stmt, group.name());
    stmtSetString(stmt, group.number());
    stmtSetId(stmt, group.equityId());
    stmtSetId(stmt, group.creditId());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");
    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(group)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a PatGroup.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id group_id, PatGroup& group)
{
    if (group_id == INVALID_ID) return false;
    PatGroupSelect conditions;
    vector<PatGroup> groups;

    conditions.id = group_id;
    if (!select(groups, conditions)) return false;
    if (groups.size() != 1) return false;

    group = groups[0];
    return true;
}

// Returns a vector of PatGroups.  Returns PatGroups sorted by name.
bool
QuasarDB::select(vector<PatGroup>& groups, const PatGroupSelect& conditions)
{
    groups.clear();

    QString cmd = selectCmd("pat_group", "patgroup_id", "name,number,"
			    "equity_id,credit_id", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	PatGroup group;
	int next = 1;
	selectData(group, stmt, next);
	group.setName(stmtGetString(stmt, next++));
	group.setNumber(stmtGetString(stmt, next++));
	group.setEquityId(stmtGetId(stmt, next++));
	group.setCreditId(stmtGetId(stmt, next++));
	groups.push_back(group);
    }

    QString cmd1 = "select card_id from card where patgroup_id = ?";
    Stmt stmt1(_connection, cmd1);

    for (unsigned int i = 0; i < groups.size(); ++i) {
	Id group_id = groups[i].id();

	stmtSetId(stmt1, group_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    Id card_id = stmtGetId(stmt1, 1);
	    groups[i].card_ids().push_back(card_id);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const PatGroup& group)
{
    if (!validate((DataObject&)group)) return false;

    // Check for duplicate number
    PatGroupSelect conditions;
    conditions.number = group.number();
    vector<PatGroup> groups;
    select(groups, conditions);
    for (unsigned int i = 0; i < groups.size(); ++i)
	if (groups[i].id() != group.id())
	    return error("Number already used for another group");

    if (group.name().stripWhiteSpace().isEmpty())
	return error("Blank group name");

    if (group.number().stripWhiteSpace().isEmpty())
	return error("Blank group number");

    bool foundEquity = false;
    bool foundCredit = false;
    for (unsigned int i = 0; i < group.card_ids().size(); ++i) {
	Id card_id = group.card_ids()[i];

	Card card;
	if (!lookup(card_id, card))
	    return error("Invalid card");

	if (card_id == group.equityId()) {
	    foundEquity = true;
	    if (card.dataType() != DataObject::CUSTOMER)
		return error("Equity card must be a customer");
	}

	if (card_id == group.creditId()) {
	    foundCredit = true;
	    if (card.dataType() != DataObject::CUSTOMER)
		return error("Credit card must be a customer");
	}

	if (card.dataType() != DataObject::CUSTOMER &&
	    card.dataType() != DataObject::VENDOR)
	    return error("Cards must be customers or vendors");

	if (card.patGroupId() != INVALID_ID && card.patGroupId() != group.id())
	    return error("Card '" + card.name() + "' is already in a group");
    }

    if (group.equityId() == INVALID_ID)
	return error("Equity card is manditory");
    if (!foundEquity)
	return error("Equity card is not in the card list");

    if (group.creditId() == INVALID_ID)
	return error("Credit card is manditory");
    if (!foundCredit)
	return error("Credit card is not in the card list");

    return true;
}

bool
QuasarDB::sqlCreateLines(const PatGroup& group)
{
    QString cmd = "update card set patgroup_id=? where card_id=?";
    Stmt stmt(_connection, cmd);

    for (unsigned int i = 0; i < group.card_ids().size(); ++i) {
	stmtSetId(stmt, group.id());
	stmtSetId(stmt, group.card_ids()[i]);
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const PatGroup& group)
{
    QString cmd = "update card set patgroup_id=NULL where patgroup_id=?";
    Stmt stmt(_connection, cmd);
    stmtSetId(stmt, group.id());
    return execute(stmt);
}
