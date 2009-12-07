// $Id: expense_db.cpp,v 1.16 2005/03/01 19:59:42 bpepers Exp $
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

#include "expense.h"
#include "expense_select.h"
#include "account.h"
#include "tax.h"

// Create a Expense
bool
QuasarDB::create(Expense& expense)
{
    if (!validate(expense)) return false;

    QString cmd = insertCmd("expense", "expense_id", "name,number,"
			    "tax_id,account_id");
    Stmt stmt(_connection, cmd);

    insertData(expense, stmt);
    stmtSetString(stmt, expense.name());
    stmtSetString(stmt, expense.number());
    stmtSetId(stmt, expense.taxId());
    stmtSetId(stmt, expense.accountId());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, expense);
    return true;
}

// Delete a Expense
bool
QuasarDB::remove(const Expense& expense)
{
    if (expense.id() == INVALID_ID) return false;
    if (!removeData(expense, "expense", "expense_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, expense);
    return true;
}

// Update a Expense
bool
QuasarDB::update(const Expense& orig, Expense& expense)
{
    if (orig.id() == INVALID_ID || expense.id() == INVALID_ID) return false;
    if (!validate(expense)) return false;

    // Update the expense table
    QString cmd = updateCmd("expense", "expense_id", "name,number,"
			    "tax_id,account_id");
    Stmt stmt(_connection, cmd);

    updateData(orig, expense, stmt);
    stmtSetString(stmt, expense.name());
    stmtSetString(stmt, expense.number());
    stmtSetId(stmt, expense.taxId());
    stmtSetId(stmt, expense.accountId());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Expense.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id expense_id, Expense& expense)
{
    if (expense_id == INVALID_ID) return false;
    ExpenseSelect conditions;
    vector<Expense> expenses;

    conditions.id = expense_id;
    if (!select(expenses, conditions)) return false;
    if (expenses.size() != 1) return false;

    expense = expenses[0];
    return true;
}

// Lookup a Expense by its number
bool
QuasarDB::lookup(const QString& number, Expense& expense)
{
    if (number.isEmpty()) return false;
    ExpenseSelect conditions;
    vector<Expense> expenses;

    conditions.number = number;
    if (!select(expenses, conditions)) return false;
    if (expenses.size() != 1) return false;

    expense = expenses[0];
    return true;
}

// Returns a vector of Expenses.  Returns expenses sorted by name.
bool
QuasarDB::select(vector<Expense>& expenses, const ExpenseSelect& conditions)
{
    expenses.clear();

    QString cmd = selectCmd("expense", "expense_id", "name,number,"
			    "tax_id,account_id", conditions, "name");
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Expense expense;
	int next = 1;
	selectData(expense, stmt, next);
	expense.setName(stmtGetString(stmt, next++));
	expense.setNumber(stmtGetString(stmt, next++));
	expense.setTaxId(stmtGetId(stmt, next++));
	expense.setAccountId(stmtGetId(stmt, next++));
	expenses.push_back(expense);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Expense& expense)
{
    if (!validate((DataObject&)expense)) return false;

    if (expense.name().stripWhiteSpace().isEmpty())
	return error("Blank expense name");

    if (expense.number().stripWhiteSpace().isEmpty())
	return error("Blank expense number");

    if (expense.taxId() != INVALID_ID) {
	Tax tax;
	if (!lookup(expense.taxId(), tax))
	    return error("Tax doesn't exist");
    }

    Account account;
    if (!lookup(expense.accountId(), account))
	return error("GL account doesn't exist");
    if (account.isHeader())
	return error("GL account is a header");

    return true;
}
