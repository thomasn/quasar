// $Id: quasar_account.cpp,v 1.21 2005/03/01 19:59:42 bpepers Exp $
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
#include "quasar_client.h"

#include "account.h"
#include "account_select.h"

// Calculate balance for a account
fixed
QuasarDB::accountBalance(Id account_id, QDate date, Id store_id)
{
    fixed balance = 0.0;
    if (date.isNull()) date = QDate::currentDate();

    Account account;
    if (!lookup(account_id, account)) return balance;

    // If the account is a header, add up the children balances
    if (account.isHeader()) {
	vector<Account> kids;
	AccountSelect conditions;
	conditions.parent_id = account_id;

	select(kids, conditions);
	for (unsigned int i = 0; i < kids.size(); ++i)
	    balance += accountBalance(kids[i].id(), date, store_id);

	return balance;
    }

    QString cmd = "select sum(amount) from account_change where "
	"account_id = ? and change_date <= ?";
    if (store_id != INVALID_ID)
	cmd += " and store_id = ?";
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, account_id);
    stmtSetDate(stmt, date);
    if (store_id != INVALID_ID)
	stmtSetId(stmt, store_id);

    if (!execute(stmt)) return balance;
    if (stmt.next())
	balance = stmtGetFixed(stmt, 1);

    return balance;
}

// Adjust the balance of a account
bool
QuasarDB::accountAdjust(Id account_id, Id store_id, QDate date, fixed amount)
{
    if (amount == 0.0) return true;
    if (date.isNull()) date = QDate::currentDate();

    fixed debit_amt = (amount > 0.0) ? amount : 0.0;
    fixed credit_amt = (amount < 0.0) ? amount : 0.0;

    QString cmd = "update account_change set amount = amount + ?,"
	"debit_amt = debit_amt + ?, credit_amt = credit_amt + ? "
	"where account_id = ? and store_id = ? and change_date = ?";
    Stmt stmt(_connection, cmd);

    stmtSetFixed(stmt, amount);
    stmtSetFixed(stmt, debit_amt);
    stmtSetFixed(stmt, credit_amt);
    stmtSetId(stmt, account_id);
    stmtSetId(stmt, store_id);
    stmtSetDate(stmt, date);

    execute(stmt);
    if (stmt.getUpdateCount() != 1)
	return error("invalid update count");

    return true;
}
