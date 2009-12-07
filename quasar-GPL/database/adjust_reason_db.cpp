// $Id: adjust_reason_db.cpp,v 1.3 2005/03/01 19:59:42 bpepers Exp $
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

#include "adjust_reason.h"
#include "adjust_reason_select.h"

// Create an AdjustReason
bool
QuasarDB::create(AdjustReason& reason)
{
    if (!validate(reason)) return false;

    QString cmd = insertCmd("adjust_reason", "reason_id", "name,number,"
			    "account_id");
    Stmt stmt(_connection, cmd);

    insertData(reason, stmt);
    stmtSetString(stmt, reason.name());
    stmtSetString(stmt, reason.number());
    stmtSetId(stmt, reason.accountId());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, reason);
    return true;
}

// Delete an AdjustReason
bool
QuasarDB::remove(const AdjustReason& reason)
{
    if (reason.id() == INVALID_ID) return false;
    if (!removeData(reason, "adjust_reason", "reason_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, reason);
    return true;
}

// Update a AdjustReason
bool
QuasarDB::update(const AdjustReason& orig, AdjustReason& reason)
{
    if (orig.id() == INVALID_ID || reason.id() == INVALID_ID) return false;
    if (!validate(reason)) return false;

    // Update the reason table
    QString cmd = updateCmd("adjust_reason", "reason_id", "name,number,"
			    "account_id");
    Stmt stmt(_connection, cmd);

    updateData(orig, reason, stmt);
    stmtSetString(stmt, reason.name());
    stmtSetString(stmt, reason.number());
    stmtSetId(stmt, reason.accountId());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup an AdjustReason.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id reason_id, AdjustReason& reason)
{
    if (reason_id == INVALID_ID) return false;
    AdjustReasonSelect conditions;
    vector<AdjustReason> reasons;

    conditions.id = reason_id;
    if (!select(reasons, conditions)) return false;
    if (reasons.size() != 1) return false;

    reason = reasons[0];
    return true;
}

// Lookup an AdjustReason.  This is just a shortcut to using select.
bool
QuasarDB::lookup(const QString& number, AdjustReason& reason)
{
    if (number.isEmpty()) return false;
    AdjustReasonSelect conditions;
    vector<AdjustReason> reasons;

    conditions.number = number;
    if (!select(reasons, conditions)) return false;
    if (reasons.size() != 1) return false;

    reason = reasons[0];
    return true;
}

// Returns a vector of AdjustReason's.  Returns reasons sorted by name.
bool
QuasarDB::select(vector<AdjustReason>& reasons, const AdjustReasonSelect& cond)
{
    reasons.clear();

    QString cmd = selectCmd("adjust_reason", "reason_id", "name,number,"
			    "account_id", cond);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	AdjustReason reason;
	int next = 1;
	selectData(reason, stmt, next);
	reason.setName(stmtGetString(stmt, next++));
	reason.setNumber(stmtGetString(stmt, next++));
	reason.setAccountId(stmtGetId(stmt, next++));
	reasons.push_back(reason);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const AdjustReason& reason)
{
    if (!validate((DataObject&)reason)) return false;

    if (reason.name().stripWhiteSpace().isEmpty())
	return error("Blank reason name");

    if (reason.accountId() == INVALID_ID)
	return error("A ledger account is required");

    return true;
}
