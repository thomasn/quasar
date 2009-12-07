// $Id: discount_db.cpp,v 1.8 2005/03/01 19:59:42 bpepers Exp $
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

#include "discount.h"
#include "discount_select.h"

// Create a Discount
bool
QuasarDB::create(Discount& discount)
{
    if (!validate(discount)) return false;

    QString cmd = insertCmd("discount", "discount_id", "name,line_disc,"
			    "tx_disc,account_id,method,amount");
    Stmt stmt(_connection, cmd);

    insertData(discount, stmt);
    stmtSetString(stmt, discount.name());
    stmtSetBool(stmt, discount.lineDiscount());
    stmtSetBool(stmt, discount.txDiscount());
    stmtSetId(stmt, discount.accountId());
    stmtSetInt(stmt, discount.method());
    stmtSetFixed(stmt, discount.amount());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, discount);
    return true;
}

// Delete a Discount
bool
QuasarDB::remove(const Discount& discount)
{
    if (discount.id() == INVALID_ID) return false;
    if (!removeData(discount, "discount", "discount_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, discount);
    return true;
}

// Update a Discount
bool
QuasarDB::update(const Discount& orig, Discount& discount)
{
    if (orig.id() == INVALID_ID || discount.id() == INVALID_ID) return false;
    if (!validate(discount)) return false;

    // Update the discount table
    QString cmd = updateCmd("discount", "discount_id", "name,line_disc,"
			    "tx_disc,account_id,method,amount");
    Stmt stmt(_connection, cmd);

    updateData(orig, discount, stmt);
    stmtSetString(stmt, discount.name());
    stmtSetBool(stmt, discount.lineDiscount());
    stmtSetBool(stmt, discount.txDiscount());
    stmtSetId(stmt, discount.accountId());
    stmtSetInt(stmt, discount.method());
    stmtSetFixed(stmt, discount.amount());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Discount.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id discount_id, Discount& discount)
{
    if (discount_id == INVALID_ID) return false;
    DiscountSelect conditions;
    vector<Discount> discounts;

    conditions.id = discount_id;
    if (!select(discounts, conditions)) return false;
    if (discounts.size() != 1) return false;

    discount = discounts[0];
    return true;
}

// Returns a vector of Discount's.  Returns discounts sorted by name.
bool
QuasarDB::select(vector<Discount>& discounts, const DiscountSelect& conditions)
{
    discounts.clear();

    QString cmd = selectCmd("discount", "discount_id", "name,line_disc,"
			    "tx_disc,account_id,method,amount",
			    conditions, "name");
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Discount discount;
	int next = 1;
	selectData(discount, stmt, next);
	discount.setName(stmtGetString(stmt, next++));
	discount.setLineDiscount(stmtGetBool(stmt, next++));
	discount.setTxDiscount(stmtGetBool(stmt, next++));
	discount.setAccountId(stmtGetId(stmt, next++));
	discount.setMethod(Discount::Method(stmtGetInt(stmt, next++)));
	discount.setAmount(stmtGetFixed(stmt, next++));
	discounts.push_back(discount);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Discount& discount)
{
    if (!validate((DataObject&)discount)) return false;

    if (discount.name().stripWhiteSpace().isEmpty())
	return error("Blank department name");

    if (!discount.lineDiscount() && !discount.txDiscount())
	return error("Must have one of line or tx discount set");

    return true;
}
