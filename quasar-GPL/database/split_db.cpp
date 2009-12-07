// $Id: split_db.cpp,v 1.10 2004/09/22 01:57:42 bpepers Exp $
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

#include "split.h"
#include "split_select.h"
#include "account.h"
#include "store.h"

// Create a Split
bool
QuasarDB::create(Split& split)
{
    if (!validate(split)) return false;
    if (!create((Gltx&)split)) return false;

    QString cmd = insertText("split", "split_id", "account_id,"
			     "amount,transfer_id");
    Statement stmt(connection(), cmd);

    stmtSetId(stmt, split.id());
    stmtSetId(stmt, split.accountId());
    stmtSetFixed(stmt, split.amount());
    stmtSetId(stmt, split.transferAccount());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(split)) return false;

    commit();
    dataSignal(DataEvent::Insert, split);
    return true;
}

// Delete a Split
bool
QuasarDB::remove(const Split& split)
{
    if (split.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(split)) return false;
    if (!remove((Gltx&)split)) return false;

    commit();
    dataSignal(DataEvent::Delete, split);
    return true;
}

// Update a Split
bool
QuasarDB::update(const Split& orig, Split& split)
{
    if (orig.id() == INVALID_ID || split.id() == INVALID_ID) return false;
    if (!validate(split)) return false;
    if (!update(orig, (Gltx&)split)) return false;

    // Update the split tables
    QString cmd = updateText("split", "split_id", "account_id,"
			     "amount,transfer_id");
    Statement stmt(connection(), cmd);

    stmtSetId(stmt, split.accountId());
    stmtSetFixed(stmt, split.amount());
    stmtSetId(stmt, split.transferAccount());
    stmtSetId(stmt, split.id());

    if (!execute(stmt)) return false;
    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(split)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Split.
bool
QuasarDB::lookup(Id split_id, Split& split)
{
    if (split_id == INVALID_ID) return false;
    SplitSelect conditions;
    vector<Split> splits;

    conditions.id = split_id;
    if (!select(splits, conditions)) return false;
    if (splits.size() != 1) return false;

    split = splits[0];
    return true;
}

// Returns a vector of Splits.
bool
QuasarDB::select(vector<Split>& splits, const SplitSelect& conditions)
{
    splits.clear();

    QString cmd = gltxCmd("split", "split_id", "account_id,"
			  "split.amount,transfer_id", conditions);
    Statement stmt(connection(), cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Split split;
	int next = 1;
	selectData(split, stmt, next);
	selectGltx(split, stmt, next);
	split.setAccountId(stmtGetId(stmt, next++));
	split.setAmount(stmtGetFixed(stmt, next++));
	split.setTransferAccount(stmtGetId(stmt, next++));
	splits.push_back(split);
    }

    QString cmd1 = "select store_id,amount from split_alloc where "
	"split_id = ? order by seq_num";
    Statement stmt1(connection(), cmd1);

    for (unsigned int i = 0; i < splits.size(); ++i) {
	Id gltx_id = splits[i].id();
	GLTX_ACCOUNTS(splits);

	stmtSetId(stmt1, gltx_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    AllocLine line;
	    line.store_id = stmtGetId(stmt1, 1);
	    line.amount = stmtGetFixed(stmt1, 2);
	    splits[i].allocations().push_back(line);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Split& split)
{
    for (unsigned int i = 0; i < split.allocations().size(); ++i) {
	const AllocLine& line = split.allocations()[i];

	Store store;
	if (!lookup(line.store_id, store))
	    return error("Store doesn't exist");
    }

    return validate((Gltx&)split);
}

bool
QuasarDB::sqlCreateLines(const Split& split)
{
    QString cmd = insertText("split_alloc", "split_id", "seq_num,store_id,"
			     "amount");
    Statement stmt(connection(), cmd);

    const vector<AllocLine>& lines = split.allocations();
    for (unsigned int line = 0; line < lines.size(); ++line) {
	Id store_id = lines[line].store_id;
	fixed amount = lines[line].amount;

	stmtSetId(stmt, split.id());
    	stmtSetInt(stmt, line);
	stmtSetId(stmt, store_id);
	stmtSetFixed(stmt, amount);
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const Split& split)
{
    return remove(split, "split_alloc", "split_id");
}
