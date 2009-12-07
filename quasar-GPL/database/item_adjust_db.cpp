// $Id: item_adjust_db.cpp,v 1.40 2005/03/16 15:37:17 bpepers Exp $
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

#include "item_adjust.h"
#include "item_adjust_select.h"
#include "adjust_reason.h"
#include "account.h"
#include "item.h"

// Create an ItemAdjust
bool
QuasarDB::create(ItemAdjust& adjustment)
{
    if (!validate(adjustment)) return false;
    if (!create((Gltx&)adjustment)) return false;

    QString cmd = insertText("item_adjust", "item_adjust_id",
			     "reason_id,account_id");
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, adjustment.id());
    stmtSetId(stmt, adjustment.reasonId());
    stmtSetId(stmt, adjustment.accountId());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(adjustment)) return false;

    commit();
    dataSignal(DataEvent::Insert, adjustment);
    return true;
}

// Delete an ItemAdjust
bool
QuasarDB::remove(const ItemAdjust& adjustment)
{
    if (adjustment.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(adjustment)) return false;
    if (!remove((Gltx&)adjustment)) return false;

    commit();
    dataSignal(DataEvent::Delete, adjustment);
    return true;
}

// Update an ItemAdjust
bool
QuasarDB::update(const ItemAdjust& orig, ItemAdjust& adjustment)
{
    if (orig.id() == INVALID_ID || adjustment.id() == INVALID_ID) return false;
    if (!validate(adjustment)) return false;
    if (!update(orig, (Gltx&)adjustment)) return false;

    // Update the item_adjust table
    QString cmd = updateText("item_adjust", "item_adjust_id",
			     "reason_id,account_id");
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, adjustment.reasonId());
    stmtSetId(stmt, adjustment.accountId());
    stmtSetId(stmt, orig.id());

    if (!execute(stmt)) return false;
    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(adjustment)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup an ItemAdjust.
bool
QuasarDB::lookup(Id adjustment_id, ItemAdjust& adjustment)
{
    if (adjustment_id == INVALID_ID) return false;
    ItemAdjustSelect conditions;
    vector<ItemAdjust> adjustments;

    conditions.id = adjustment_id;
    if (!select(adjustments, conditions)) return false;
    if (adjustments.size() != 1) return false;

    adjustment = adjustments[0];
    return true;
}

// Returns a vector of ItemAdjusts.
bool
QuasarDB::select(vector<ItemAdjust>& adjusts, const ItemAdjustSelect& select)
{
    adjusts.clear();

    QString cmd = gltxCmd("item_adjust", "item_adjust_id",
			  "reason_id,account_id", select);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	ItemAdjust adjustment;
	int next = 1;
	selectData(adjustment, stmt, next);
	selectGltx(adjustment, stmt, next);
	adjustment.setReasonId(stmtGetId(stmt, next++));
	adjustment.setAccountId(stmtGetId(stmt, next++));
	adjusts.push_back(adjustment);
    }

    QString cmd1 = "select item_id,account_id,number,description,"
	"size_name,size_qty,quantity,inv_cost,sale_price,"
	"ext_deposit,voided from gltx_item where gltx_id = ? "
	"order by seq_num";
    Stmt stmt1(_connection, cmd1);

    for (unsigned int i = 0; i < adjusts.size(); ++i) {
	Id gltx_id = adjusts[i].id();
	GLTX_ACCOUNTS(adjusts);

	stmtSetId(stmt1, gltx_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    ItemLine line;
	    int next = 1;
	    line.item_id = stmtGetId(stmt1, next++);
	    line.account_id = stmtGetId(stmt1, next++);
	    line.number = stmtGetString(stmt1, next++);
	    line.description = stmtGetString(stmt1, next++);
	    line.size = stmtGetString(stmt1, next++);
	    line.size_qty = stmtGetFixed(stmt1, next++);
	    line.quantity = stmtGetFixed(stmt1, next++);
	    line.inv_cost = stmtGetFixed(stmt1, next++);
	    line.sale_price = stmtGetFixed(stmt1, next++);
	    line.ext_deposit = stmtGetFixed(stmt1, next++);
	    line.voided = stmtGetBool(stmt1, next++);
	    adjusts[i].items().push_back(line);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const ItemAdjust& adjustment)
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < adjustment.items().size(); ++i) {
	const ItemLine& line = adjustment.items()[i];
	if (line.voided) continue;

	Item item;
	if (!lookup(line.item_id, item))
	    return error("Item doesn't exist");
	if (!item.isInventoried())
	    return error("Item isn't inventoried");

	if (line.number.isEmpty())
	    return error("Blank item number");

	total += line.inv_cost + line.ext_deposit;
    }

    if (adjustment.reasonId() != INVALID_ID) {
	AdjustReason reason;
	if (!lookup(adjustment.reasonId(), reason))
	    return error("Adjustment reason doesn't exist");
    }

    if (adjustment.accountId() != INVALID_ID) {
	Account account;
	if (!lookup(adjustment.accountId(), account))
	    return error("Expense account doesn't exist");
	if (account.isHeader())
	    return error("Expense account is a header");
    } else if (total != 0.0 && adjustment.isActive()) {
	return error("Expense account is required");
    }

    return validate((Gltx&)adjustment);
}

bool
QuasarDB::sqlCreateLines(const ItemAdjust& adjustment)
{
    QString cmd1 = insertText("gltx_item", "gltx_id", "seq_num,item_id,"
			      "account_id,number,description,size_name,"
			      "size_qty,quantity,inv_cost,sale_price,"
			      "ext_deposit,voided");
    Stmt stmt1(_connection, cmd1);

    for (unsigned int i = 0; i < adjustment.items().size(); ++i) {
	const ItemLine& line = adjustment.items()[i];

	stmtSetId(stmt1, adjustment.id());
	stmtSetInt(stmt1, i);
	stmtSetId(stmt1, line.item_id);
	stmtSetId(stmt1, line.account_id);
	stmtSetString(stmt1, line.number);
	stmtSetString(stmt1, line.description);
	stmtSetString(stmt1, line.size);
	stmtSetFixed(stmt1, line.size_qty);
	stmtSetFixed(stmt1, line.quantity);
	stmtSetFixed(stmt1, line.inv_cost);
	stmtSetFixed(stmt1, line.sale_price);
	stmtSetFixed(stmt1, line.ext_deposit);
	stmtSetBool(stmt1, line.voided);
	if (!execute(stmt1)) return false;

	if (line.item_id == INVALID_ID) continue;
	if (!adjustment.isActive()) continue;
	if (line.voided) continue;

	itemAdjustment(line.item_id, line.size, adjustment.storeId(),
		       adjustment.postDate(), line.quantity * line.size_qty,
		       line.inv_cost);
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const ItemAdjust& adjustment)
{
    QString cmd = "select item_id,size_name,size_qty,quantity,"
	"inv_cost,voided from gltx_item where gltx_id = ?";
    Stmt stmt(_connection, cmd);
    stmtSetId(stmt, adjustment.id());

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	int next = 1;
	Id item_id = stmtGetId(stmt, next++);
	QString size = stmtGetString(stmt, next++);
	fixed size_qty = stmtGetFixed(stmt, next++);
	fixed quantity = stmtGetFixed(stmt, next++);
	fixed inv_cost = stmtGetFixed(stmt, next++);
	bool voided = stmtGetBool(stmt, next++);

	if (item_id == INVALID_ID) continue;
	if (!adjustment.isActive()) continue;
	if (voided) continue;

	itemAdjustment(item_id, size, adjustment.storeId(),
		       adjustment.postDate(), -quantity * size_qty,
		       -inv_cost);
    }

    return remove(adjustment, "gltx_item", "gltx_id");
}
