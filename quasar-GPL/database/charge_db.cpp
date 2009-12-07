// $Id: charge_db.cpp,v 1.10 2005/03/01 19:59:42 bpepers Exp $
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

#include "charge.h"
#include "charge_select.h"
#include "account.h"
#include "tax.h"

// Create a Charge
bool
QuasarDB::create(Charge& charge)
{
    if (!validate(charge)) return false;

    QString cmd = insertCmd("charge", "charge_id", "name,account_id,"
			    "tax_id,calc_method,amount,alloc_method,"
			    "inc_tax");
    Stmt stmt(_connection, cmd);

    insertData(charge, stmt);
    stmtSetString(stmt, charge.name());
    stmtSetId(stmt, charge.accountId());
    stmtSetId(stmt, charge.taxId());
    stmtSetInt(stmt, charge.calculateMethod());
    stmtSetFixed(stmt, charge.amount());
    stmtSetInt(stmt, charge.allocateMethod());
    stmtSetBool(stmt, charge.includeTax());
    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, charge);
    return true;
}

// Delete a Charge
bool
QuasarDB::remove(const Charge& charge)
{
    if (charge.id() == INVALID_ID) return false;
    if (!removeData(charge, "charge", "charge_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, charge);
    return true;
}

// Update a Charge
bool
QuasarDB::update(const Charge& orig, Charge& charge)
{
    if (orig.id() == INVALID_ID || charge.id() == INVALID_ID) return false;
    if (!validate(charge)) return false;

    // Update the charge table
    QString cmd = updateCmd("charge", "charge_id", "name,account_id,"
			    "tax_id,calc_method,amount,alloc_method,"
			    "inc_tax");
    Stmt stmt(_connection, cmd);

    updateData(orig, charge, stmt);
    stmtSetString(stmt, charge.name());
    stmtSetId(stmt, charge.accountId());
    stmtSetId(stmt, charge.taxId());
    stmtSetInt(stmt, charge.calculateMethod());
    stmtSetFixed(stmt, charge.amount());
    stmtSetInt(stmt, charge.allocateMethod());
    stmtSetBool(stmt, charge.includeTax());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Charge.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id charge_id, Charge& charge)
{
    if (charge_id == INVALID_ID) return false;
    ChargeSelect conditions;
    vector<Charge> charges;

    conditions.id = charge_id;
    if (!select(charges, conditions)) return false;
    if (charges.size() != 1) return false;

    charge = charges[0];
    return true;
}

// Returns a vector of Charges.  Returns charges sorted by name.
bool
QuasarDB::select(vector<Charge>& charges, const ChargeSelect& conditions)
{
    charges.clear();

    QString cmd = selectCmd("charge", "charge_id", "name,account_id,"
			    "tax_id,calc_method,amount,alloc_method,"
			    "inc_tax", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Charge charge;
	int next = 1;
	selectData(charge, stmt, next);
	charge.setName(stmtGetString(stmt, next++));
	charge.setAccountId(stmtGetId(stmt, next++));
	charge.setTaxId(stmtGetId(stmt, next++));
	charge.setCalculateMethod(Charge::Method(stmtGetInt(stmt, next++)));
	charge.setAmount(stmtGetFixed(stmt, next++));
	charge.setAllocateMethod(Charge::Method(stmtGetInt(stmt, next++)));
	charge.setIncludeTax(stmtGetBool(stmt, next++));
	charges.push_back(charge);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Charge& charge)
{
    if (!validate((DataObject&)charge)) return false;

    if (charge.name().stripWhiteSpace().isEmpty())
	return error("Blank charge name");

    Account account;
    if (!lookup(charge.accountId(), account))
	return error("GL account doesn't exist");
    if (account.isHeader())
	return error("GL account is a header");

    if (charge.taxId() != INVALID_ID) {
	Tax tax;
	if (!lookup(charge.taxId(), tax))
	    return error("Tax doesn't exist");
    }

    return true;
}
