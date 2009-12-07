// $Id: reconcile_db.cpp,v 1.10 2005/03/01 19:59:42 bpepers Exp $
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

#include "reconcile.h"
#include "reconcile_select.h"
#include "account.h"

// Create a Reconcile
bool
QuasarDB::create(Reconcile& reconcile)
{
    if (!validate(reconcile)) return false;

    QString cmd = insertCmd("reconcile", "reconcile_id", "account_id,"
			    "stmt_balance,stmt_date,end_date,reconciled");
    Stmt stmt(_connection, cmd);

    insertData(reconcile, stmt);
    stmtSetId(stmt, reconcile.accountId());
    stmtSetFixed(stmt, reconcile.statementBalance());
    stmtSetDate(stmt, reconcile.statementDate());
    stmtSetDate(stmt, reconcile.endDate());
    stmtSetBool(stmt, reconcile.isReconciled());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(reconcile)) return false;

    commit();
    dataSignal(DataEvent::Insert, reconcile);
    return true;
}

// Delete a Reconcile
bool
QuasarDB::remove(const Reconcile& reconcile)
{
    if (reconcile.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(reconcile)) return false;
    if (!removeData(reconcile, "reconcile", "reconcile_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, reconcile);
    return true;
}

// Update a Reconcile
bool
QuasarDB::update(const Reconcile& orig, Reconcile& reconcile)
{
    if (orig.id() == INVALID_ID || reconcile.id() == INVALID_ID) return false;
    if (!validate(reconcile)) return false;

    // Update the reconcile table
    QString cmd = updateCmd("reconcile", "reconcile_id", "account_id,"
			    "stmt_balance,stmt_date,end_date,reconciled");
    Stmt stmt(_connection, cmd);

    updateData(orig, reconcile, stmt);
    stmtSetId(stmt, reconcile.accountId());
    stmtSetFixed(stmt, reconcile.statementBalance());
    stmtSetDate(stmt, reconcile.statementDate());
    stmtSetDate(stmt, reconcile.endDate());
    stmtSetBool(stmt, reconcile.isReconciled());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");
    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(reconcile)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Reconcile.
bool
QuasarDB::lookup(Id reconcile_id, Reconcile& reconcile)
{
    if (reconcile_id == INVALID_ID) return false;
    ReconcileSelect conditions;
    vector<Reconcile> reconciles;

    conditions.id = reconcile_id;
    if (!select(reconciles, conditions)) return false;
    if (reconciles.size() != 1) return false;

    reconcile = reconciles[0];
    return true;
}

// Returns a vector of Reconciles.  Returns Reconciles sorted by stmt_date.
bool
QuasarDB::select(vector<Reconcile>& reconciles, const ReconcileSelect& conds)
{
    reconciles.clear();

    QString cmd = selectCmd("reconcile", "reconcile_id", "account_id,"
			    "stmt_balance,stmt_date,end_date,reconciled",
			    conds, "account_id,stmt_date");
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Reconcile reconcile;
	int next = 1;
	selectData(reconcile, stmt, next);
	reconcile.setAccountId(stmtGetId(stmt, next++));
	reconcile.setStatementBalance(stmtGetFixed(stmt, next++));
	reconcile.setStatementDate(stmtGetDate(stmt, next++));
	reconcile.setEndDate(stmtGetDate(stmt, next++));
	reconcile.setReconciled(stmtGetBool(stmt, next++));
	reconciles.push_back(reconcile);
    }

    cmd = "select gltx_id,seq_num from gltx_account where "
	"account_id = ? and cleared = ?";
    stmt.setCommand(cmd);

    for (unsigned int i = 0; i < reconciles.size(); ++i) {
	stmtSetId(stmt, reconciles[i].accountId());
	stmtSetDate(stmt, reconciles[i].statementDate());
	if (!execute(stmt)) return false;
	while (stmt.next()) {
	    ReconcileLine line;
	    line.gltx_id = stmtGetId(stmt, 1);
	    line.seq_num = stmtGetInt(stmt, 2);
	    reconciles[i].lines().push_back(line);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Reconcile& reconcile)
{
    if (!validate((DataObject&)reconcile)) return false;

    // Check for duplicate account_id, stmt_date
    ReconcileSelect conditions;
    conditions.account_id = reconcile.accountId();
    conditions.stmt_date = reconcile.statementDate();
    vector<Reconcile> reconciles;
    select(reconciles, conditions);
    for (unsigned int i = 0; i < reconciles.size(); ++i)
	if (reconciles[i].id() != reconcile.id())
	    return error("Account/date already used for another reconcile");

    if (reconcile.accountId() == INVALID_ID)
	return error("Blank account");

    Account account;
    if (!lookup(reconcile.accountId(), account))
	return error("Unknown account");
    if (account.isHeader())
	return error("Can't reconcile header accounts");

    if (reconcile.statementDate().isNull())
	return error("Blank statement date");

    if (reconcile.endDate().isNull())
	return error("Blank end date");

    return true;
}

bool
QuasarDB::sqlCreateLines(const Reconcile& reconcile)
{
    QString cmd = "update gltx_account set cleared = ? where "
	"gltx_id = ? and seq_num = ?";
    Stmt stmt(_connection, cmd);

    for (unsigned int i = 0; i < reconcile.lines().size(); ++i) {
	stmtSetDate(stmt, reconcile.statementDate());
	stmtSetId(stmt, reconcile.lines()[i].gltx_id);
	stmtSetInt(stmt, reconcile.lines()[i].seq_num);
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const Reconcile& reconcile)
{
    QString cmd = "update gltx_account set cleared = null where "
	"account_id = ? and cleared = ?";
    Stmt stmt(_connection, cmd);
    stmtSetId(stmt, reconcile.accountId());
    stmtSetDate(stmt, reconcile.statementDate());
    return execute(stmt);
}
