// $Id: subdept_db.cpp,v 1.28 2005/03/01 19:59:42 bpepers Exp $
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

#include "subdept.h"
#include "subdept_select.h"
#include "dept.h"
#include "account.h"
#include "tax.h"

// Create a Subdept
bool
QuasarDB::create(Subdept& subdept)
{
    if (!validate(subdept)) return false;

    QString cmd = insertCmd("subdept", "subdept_id", "name,number,"
			    "dept_id,purchased,sold,inventoried,"
			    "expense_acct,income_acct,asset_acct,"
			    "sell_tax,purchase_tax,target_gm,"
			    "allowed_var,discountable");
    Stmt stmt(_connection, cmd);

    insertData(subdept, stmt);
    stmtSetString(stmt, subdept.name());
    stmtSetString(stmt, subdept.number());
    stmtSetId(stmt, subdept.deptId());
    stmtSetBool(stmt, subdept.isPurchased());
    stmtSetBool(stmt, subdept.isSold());
    stmtSetBool(stmt, subdept.isInventoried());
    stmtSetId(stmt, subdept.expenseAccount());
    stmtSetId(stmt, subdept.incomeAccount());
    stmtSetId(stmt, subdept.assetAccount());
    stmtSetId(stmt, subdept.sellTax());
    stmtSetId(stmt, subdept.purchaseTax());
    stmtSetFixed(stmt, subdept.targetGM());
    stmtSetFixed(stmt, subdept.allowedVariance());
    stmtSetBool(stmt, subdept.isDiscountable());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, subdept);
    return true;
}

// Delete a Subdept
bool
QuasarDB::remove(const Subdept& subdept)
{
    if (subdept.id() == INVALID_ID) return false;
    if (!removeData(subdept, "subdept", "subdept_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, subdept);
    return true;
}

// Update a Subdept.  Changes to the sold/inventoried/purchased flags are
// not allowed (in case subdept has already been used) but this may prove
// to be too constricting.
bool
QuasarDB::update(const Subdept& orig, Subdept& subdept)
{
    if (orig.id() == INVALID_ID || subdept.id() == INVALID_ID) return false;
    if (!validate(subdept)) return false;

#if 0
    if (subdept.isSold() != orig.isSold())
	return error("Can't change sold");
    if (subdept.isPurchased() != orig.isPurchased())
	return error("Can't change purchased");
    if (subdept.isInventoried() != orig.isInventoried())
	return error("Can't change inventoried");
#endif

    // Update the subdept table
    QString cmd = updateCmd("subdept", "subdept_id", "name,number,"
			    "dept_id,purchased,sold,inventoried,"
			    "expense_acct,income_acct,asset_acct,"
			    "sell_tax,purchase_tax,target_gm,"
			    "allowed_var,discountable");
    Stmt stmt(_connection, cmd);

    updateData(orig, subdept, stmt);
    stmtSetString(stmt, subdept.name());
    stmtSetString(stmt, subdept.number());
    stmtSetId(stmt, subdept.deptId());
    stmtSetBool(stmt, subdept.isPurchased());
    stmtSetBool(stmt, subdept.isSold());
    stmtSetBool(stmt, subdept.isInventoried());
    stmtSetId(stmt, subdept.expenseAccount());
    stmtSetId(stmt, subdept.incomeAccount());
    stmtSetId(stmt, subdept.assetAccount());
    stmtSetId(stmt, subdept.sellTax());
    stmtSetId(stmt, subdept.purchaseTax());
    stmtSetFixed(stmt, subdept.targetGM());
    stmtSetFixed(stmt, subdept.allowedVariance());
    stmtSetBool(stmt, subdept.isDiscountable());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    // If target margin changed, update items
    if (orig.targetGM() != subdept.targetGM()) {
	QString cmd = "update item_cost set target_gm = ? where "
	    "exists (select item.item_id from item where "
	    "item_cost.item_id = item.item_id and item.subdept_id = ?) "
	    "and target_gm = ?";
	Stmt stmt(_connection, cmd);
	stmtSetFixed(stmt, subdept.targetGM());
	stmtSetId(stmt, orig.id());
	stmtSetFixed(stmt, orig.targetGM());
	if (!execute(stmt)) return false;
    }

    // If allowed variance changed, update items
    if (orig.allowedVariance() != subdept.allowedVariance()) {
	QString cmd = "update item_cost set allowed_var = ? where "
	    "exists (select item.item_id from item where "
	    "item_cost.item_id = item.item_id and item.subdept_id = ?) "
	    "and allowed_var = ?";
	Stmt stmt(_connection, cmd);
	stmtSetFixed(stmt, subdept.allowedVariance());
	stmtSetId(stmt, orig.id());
	stmtSetFixed(stmt, orig.allowedVariance());
	if (!execute(stmt)) return false;
    }

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Subdept.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id subdept_id, Subdept& subdept)
{
    if (subdept_id == INVALID_ID) return false;
    SubdeptSelect conditions;
    vector<Subdept> subdepts;

    conditions.id = subdept_id;
    if (!select(subdepts, conditions)) return false;
    if (subdepts.size() != 1) return false;

    subdept = subdepts[0];
    return true;
}

// Lookup a Subdept.  This is just a shortcut to using select.
bool
QuasarDB::lookup(const QString& number, Subdept& subdept)
{
    if (number.isEmpty()) return false;
    SubdeptSelect conditions;
    vector<Subdept> subdepts;

    conditions.number = number;
    if (!select(subdepts, conditions)) return false;
    if (subdepts.size() != 1) return false;

    subdept = subdepts[0];
    return true;
}

// Returns a vector of Subdepts.  Returns Subdepts sorted by name.
bool
QuasarDB::select(vector<Subdept>& subdepts, const SubdeptSelect& conditions)
{
    subdepts.clear();

    QString cmd = selectCmd("subdept", "subdept_id", "name,number,"
			    "dept_id,purchased,sold,inventoried,"
			    "expense_acct,income_acct,asset_acct,"
			    "sell_tax,purchase_tax,target_gm,"
			    "allowed_var,discountable", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Subdept subdept;
	int next = 1;
	selectData(subdept, stmt, next);
	subdept.setName(stmtGetString(stmt, next++));
	subdept.setNumber(stmtGetString(stmt, next++));
	subdept.setDeptId(stmtGetId(stmt, next++));
	subdept.setPurchased(stmtGetBool(stmt, next++));
	subdept.setSold(stmtGetBool(stmt, next++));
	subdept.setInventoried(stmtGetBool(stmt, next++));
	subdept.setExpenseAccount(stmtGetId(stmt, next++));
	subdept.setIncomeAccount(stmtGetId(stmt, next++));
	subdept.setAssetAccount(stmtGetId(stmt, next++));
	subdept.setSellTax(stmtGetId(stmt, next++));
	subdept.setPurchaseTax(stmtGetId(stmt, next++));
	subdept.setTargetGM(stmtGetFixed(stmt, next++));
	subdept.setAllowedVariance(stmtGetFixed(stmt, next++));
	subdept.setDiscountable(stmtGetBool(stmt, next++));
	subdepts.push_back(subdept);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Subdept& subdept)
{
    if (!validate((DataObject&)subdept)) return false;

    if (subdept.name().stripWhiteSpace().isEmpty())
	return error("Blank subdepartment name");

    Dept dept;
    if (!lookup(subdept.deptId(), dept))
	return error("Invalid department");

    bool purchased = subdept.isPurchased();
    bool sold = subdept.isSold();
    bool inventoried = subdept.isInventoried();

    QString expenseName = "Expense";
    if (inventoried && sold) expenseName = "COGS";

    if ((inventoried && sold) || (purchased && !inventoried)) {
	Account account;
	if (!lookup(subdept.expenseAccount(), account))
	    return error(expenseName + " account doesn't exist");
	if (account.isHeader())
	    return error(expenseName + " account is a header");
    } else {
	if (subdept.expenseAccount() != INVALID_ID)
	    return error("Expense account can't be set");
    }

    if (sold) {
	Account account;
	if (!lookup(subdept.incomeAccount(), account))
	    return error("Income account doesn't exist");
	if (account.isHeader())
	    return error("Income account is a header");
    } else {
	if (subdept.incomeAccount() != INVALID_ID)
	    return error("Income account can't be set");
    }

    if (inventoried) {
	Account account;
	if (!lookup(subdept.assetAccount(), account))
	    return error("Asset account doesn't exist");
	if (account.isHeader())
	    return error("Asset account is a header");
    } else {
	if (subdept.assetAccount() != INVALID_ID)
	    return error("Asset account can't be set");
    }

    if (sold && subdept.sellTax() != INVALID_ID) {
	Tax tax;
	if (!lookup(subdept.sellTax(), tax))
	    return error("Selling tax doesn't exist");
    } else if (subdept.sellTax() != INVALID_ID) {
	return error("Selling tax can't be set");
    }

    if (purchased && subdept.purchaseTax() != INVALID_ID) {
	Tax tax;
	if (!lookup(subdept.purchaseTax(), tax))
	    return error("Purchase tax doesn't exist");
    } else if (subdept.purchaseTax() != INVALID_ID) {
	return error("Purchase tax can't be set");
    }

    if (sold && purchased) {
	if (subdept.targetGM() < 0.0)
	    return error("Negative target margin");
	if (subdept.allowedVariance() < 0.0)
	    return error("Negative allowed variance");
    } else {
	if (subdept.targetGM() != 0.0)
	    return error("Target margin can't be set");
	if (subdept.allowedVariance() != 0.0)
	    return error("Allowed variance can't be set");
    }

    return true;
}
