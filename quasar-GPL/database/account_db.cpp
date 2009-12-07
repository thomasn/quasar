// $Id: account_db.cpp,v 1.30 2005/03/01 19:59:42 bpepers Exp $
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

#include "account.h"
#include "account_select.h"

// Create an Account
bool
QuasarDB::create(Account& account)
{
    if (!validate(account)) return false;

    QString cmd = insertCmd("account", "account_id", "name,"
			    "number,account_type,header,parent_id,"
			    "last_reconciled,next_number");
    Stmt stmt(_connection, cmd);

    insertData(account, stmt);
    stmtSetString(stmt, account.name());
    stmtSetString(stmt, account.number());
    stmtSetInt(stmt, account.type());
    stmtSetBool(stmt, account.isHeader());
    stmtSetId(stmt, account.parentId());
    stmtSetDate(stmt, account.lastReconciled());
    stmtSetInt(stmt, account.nextNumber());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(account)) return false;

    commit();
    dataSignal(DataEvent::Insert, account);
    return true;
}

// Delete an Account.
bool
QuasarDB::remove(const Account& account)
{
    if (account.id() == INVALID_ID) return false;

    if (account.isHeader()) {
	AccountSelect conditions;
	conditions.parent_id = account.id();
	vector<Account> accounts;
	select(accounts, conditions);
	if (accounts.size() != 0)
	    return error("Can't delete header account with sub-accounts");
    }

    // TODO: check for transactions and balance == 0.0

    if (!sqlDeleteLines(account)) return false;
    if (!removeData(account, "account", "account_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, account);
    return true;
}

// Update an Account.  Validate needs to make sure the type matches
// the parent and children and that all other rules are enforced.
bool
QuasarDB::update(const Account& orig, Account& account)
{
    if (orig.id() == INVALID_ID || account.id() == INVALID_ID) return false;
    if (!validate(account)) return false;

    if (orig.isActive() && !account.isActive()) {
	fixed balance = accountBalance(orig.id());
	if (balance != 0.0)
	    return error("Account has a balance");

	AccountSelect conditions;
	conditions.parent_id = orig.id();
	vector<Account> accounts;
	select(accounts, conditions);
	for (unsigned int i = 0; i < accounts.size(); ++i) {
	    if (accounts[i].isActive())
		return error("Account has active sub-accounts");
	}
    }

    // Update the account table
    QString cmd = updateCmd("account", "account_id", "name,"
			    "number,account_type,header,parent_id,"
			    "last_reconciled,next_number");
    Stmt stmt(_connection, cmd);

    updateData(orig, account, stmt);
    stmtSetString(stmt, account.name());
    stmtSetString(stmt, account.number());
    stmtSetInt(stmt, account.type());
    stmtSetBool(stmt, account.isHeader());
    stmtSetId(stmt, account.parentId());
    stmtSetDate(stmt, account.lastReconciled());
    stmtSetInt(stmt, account.nextNumber());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(account)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup an Account.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id account_id, Account& account)
{
    if (account_id == INVALID_ID) return false;
    AccountSelect conditions;
    vector<Account> accounts;

    conditions.id = account_id;
    if (!select(accounts, conditions)) return false;
    if (accounts.size() != 1) return false;

    account = accounts[0];
    return true;
}

// Lookup an Account.  This is just a shortcut to using select.
bool
QuasarDB::lookup(const QString& number, Account& account)
{
    if (number.isEmpty()) return false;
    AccountSelect conditions;
    vector<Account> accounts;

    conditions.number = number;
    if (!select(accounts, conditions)) return false;
    if (accounts.size() != 1) return false;

    account = accounts[0];
    return true;
}

// Returns a count of Accounts based on the conditions.
bool
QuasarDB::count(int& count, const AccountSelect& conditions)
{
    count = 0;

    QString cmd = "select count(*) from account " + conditions.where();
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next())
	count = stmtGetInt(stmt, 1);

    commit();
    return true;
}

// Returns a vector of Accounts.  Returns Accounts sorted by name.
bool
QuasarDB::select(vector<Account>& accounts, const AccountSelect& conditions)
{
    accounts.clear();

    QString cmd = selectCmd("account", "account_id", "name,number,"
			    "account_type,header,parent_id,"
			    "last_reconciled,next_number", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Account account;
	int next = 1;
	selectData(account, stmt, next);
	account.setName(stmtGetString(stmt, next++));
	account.setNumber(stmtGetString(stmt, next++));
	account.setType(Account::Type(stmtGetInt(stmt, next++)));
	account.setHeader(stmtGetBool(stmt, next++));
	account.setParentId(stmtGetId(stmt, next++));
	account.setLastReconciled(stmtGetDate(stmt, next++));
	account.setNextNumber(stmtGetInt(stmt, next++));
	accounts.push_back(account);
    }

    cmd = "select group_id from account_group where account_id = ? "
	"order by seq_num";
    stmt.setCommand(cmd);

    for (unsigned int i = 0; i < accounts.size(); ++i) {
	Id account_id = accounts[i].id();

	stmtSetId(stmt, account_id);
	if (!execute(stmt)) return false;
	while (stmt.next()) {
	    Id group_id = stmtGetId(stmt, 1);
	    accounts[i].groups().push_back(group_id);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Account& account)
{
    AccountSelect conditions;
    vector<Account> accounts;

    if (!validate((DataObject&)account)) return false;

    // Check for duplicate name
    conditions.name = account.name();
    select(accounts, conditions);
    unsigned int i;
    for (i = 0; i < accounts.size(); ++i)
	if (accounts[i].id() != account.id())
	    return error("Name used for another account");

    // Check for duplicate number
    if (!account.number().isEmpty()) {
	AccountSelect conditions;
	conditions.number = account.number();
	select(accounts, conditions);
	for (unsigned int i = 0; i < accounts.size(); ++i)
	    if (accounts[i].id() != account.id())
		return error("Number used for another account");
    }

    if (account.name().stripWhiteSpace().isEmpty())
	return error("Blank account name");

    if (account.type() < Account::Bank)
	return error("Invalid account type");
    if (account.type() > Account::OtherExpense)
	return error("Invalid account type");

    // Get list of children
    if (account.id() != INVALID_ID) {
	conditions.name = "";
	conditions.parent_id = account.id();
	select(accounts, conditions);
    } else {
	accounts.clear();
    }

    // Check header
    if (account.isHeader()) {
	int count = 0;
	QString cmd = "select count(*) from gltx_account "
	    "where account_id = ?";
	Stmt stmt(_connection, cmd);
	stmtSetId(stmt, account.id());
	if (!execute(stmt)) return false;
	if (stmt.next())
	    count = stmtGetInt(stmt, 1);
	if (count != 0)
	    return error("Account has transactions");
    } else {
	if (accounts.size() != 0)
	    return error("Account has sub-accounts");
    }

    // Verify types match
    for (i = 0; i < accounts.size(); ++i)
	if (accounts[i].type() != account.type())
	    return error("Type doesn't match sub-accounts");

    // Validate parent
    if (account.parentId() != INVALID_ID) {
	if (account.parentId() == account.id())
	    return error("Account can't be its own parent");

	Account parent;
	if (!lookup(account.parentId(), parent))
	    return error("Account parent doesn't exist");
	if (parent.type() != account.type())
	    return error("Account parent type is different");
	if (!parent.isHeader())
	    return error("Account parent is not a header account");

	// Check for loops
	while (parent.id() != INVALID_ID) {
	    for (unsigned int i = 0; i < accounts.size(); ++i)
		if (parent.id() == accounts[i].id())
		    return error("Loop in parent links");

	    if (parent.parentId() == INVALID_ID) break;
	    lookup(parent.parentId(), parent);
	}
    }

    if (account.nextNumber() < 1)
	return error("Invalid next number");

    return true;
}

bool
QuasarDB::sqlCreateLines(const Account& account)
{
    QString cmd = insertText("account_group", "account_id", "seq_num,"
			     "group_id");
    Stmt stmt(_connection, cmd);
    for (unsigned int line = 0; line < account.groups().size(); ++line) {
	stmtSetId(stmt, account.id());
	stmtSetInt(stmt, line);
	stmtSetId(stmt, account.groups()[line]);
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const Account& account)
{
    return remove(account, "account_group", "account_id");
}
