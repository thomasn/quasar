// $Id: price_batch_db.cpp,v 1.4 2005/03/01 19:59:42 bpepers Exp $
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

#include "price_batch.h"
#include "price_batch_select.h"
#include "item.h"

// Create a PriceBatch
bool
QuasarDB::create(PriceBatch& batch)
{
    if (!validate(batch)) return false;

    // Auto allocate batch number
    if (batch.number().stripWhiteSpace() == "#") {
	fixed number = uniqueNumber("price_batch", "number");
	batch.setNumber(number.toString());
    }

    QString cmd = insertCmd("price_batch", "batch_id", "number,description,"
			    "store_id,exec_date");
    Stmt stmt(_connection, cmd);

    insertData(batch, stmt);
    stmtSetString(stmt, batch.number());
    stmtSetString(stmt, batch.description());
    stmtSetId(stmt, batch.storeId());
    stmtSetDate(stmt, batch.executedOn());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(batch)) return false;

    commit();
    dataSignal(DataEvent::Insert, batch);
    return true;
}

// Delete a PriceBatch
bool
QuasarDB::remove(const PriceBatch& batch)
{
    if (batch.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(batch)) return false;
    if (!removeData(batch, "price_batch", "batch_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, batch);
    return true;
}

// Update a PriceBatch
bool
QuasarDB::update(const PriceBatch& orig, PriceBatch& batch)
{
    if (orig.id() == INVALID_ID || batch.id() == INVALID_ID) return false;
    if (!validate(batch)) return false;

    if (batch.number().stripWhiteSpace() == "#")
	return error("Can't change number to '#'");

    // Update the batch table
    QString cmd = updateCmd("price_batch", "batch_id", "number,description,"
			    "store_id,exec_date");
    Stmt stmt(_connection, cmd);

    updateData(orig, batch, stmt);
    stmtSetString(stmt, batch.number());
    stmtSetString(stmt, batch.description());
    stmtSetId(stmt, batch.storeId());
    stmtSetDate(stmt, batch.executedOn());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");
    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(batch)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a PriceBatch.
bool
QuasarDB::lookup(Id batch_id, PriceBatch& batch)
{
    if (batch_id == INVALID_ID) return false;
    PriceBatchSelect conditions;
    vector<PriceBatch> batches;

    conditions.id = batch_id;
    if (!select(batches, conditions)) return false;
    if (batches.size() != 1) return false;

    batch = batches[0];
    return true;
}

// Returns a vector of PriceBatch's.
bool
QuasarDB::select(vector<PriceBatch>& batches, const PriceBatchSelect& conds)
{
    batches.clear();

    QString cmd = selectCmd("price_batch", "batch_id", "number,description,"
			    "store_id,exec_date", conds);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	PriceBatch batch;
	int next = 1;
	selectData(batch, stmt, next);
	batch.setNumber(stmtGetString(stmt, next++));
	batch.setDescription(stmtGetString(stmt, next++));
	batch.setStoreId(stmtGetId(stmt, next++));
	batch.setExecutedOn(stmtGetDate(stmt, next++));
	batches.push_back(batch);
    }

    QString cmd1 = "select item_id,number,size_name,new_uprice,"
        "new_fprice_qty,new_fprice,old_ucost,old_fcost_qty,"
	"old_fcost,old_uprice,old_fprice_qty,old_fprice from "
	"price_batch_item where batch_id = ? order by seq_num";
    Stmt stmt1(_connection, cmd1);

    for (unsigned int i = 0; i < batches.size(); ++i) {
	Id batch_id = batches[i].id();

	stmtSetId(stmt1, batch_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    PriceBatchItem line;
	    int next = 1;
	    line.item_id = stmtGetId(stmt1, next++);
	    line.number = stmtGetString(stmt1, next++);
	    line.size = stmtGetString(stmt1, next++);
	    line.new_price.setUnitPrice(stmtGetFixed(stmt1, next++));
	    line.new_price.setForQty(stmtGetFixed(stmt1, next++));
	    line.new_price.setForPrice(stmtGetFixed(stmt1, next++));
	    line.old_cost.setUnitPrice(stmtGetFixed(stmt1, next++));
	    line.old_cost.setForQty(stmtGetFixed(stmt1, next++));
	    line.old_cost.setForPrice(stmtGetFixed(stmt1, next++));
	    line.old_price.setUnitPrice(stmtGetFixed(stmt1, next++));
	    line.old_price.setForQty(stmtGetFixed(stmt1, next++));
	    line.old_price.setForPrice(stmtGetFixed(stmt1, next++));
	    batches[i].items().push_back(line);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const PriceBatch& batch)
{
    if (!validate((DataObject&)batch)) return false;

    if (batch.number().stripWhiteSpace().isEmpty())
	return error("Blank batch number");

    if (batch.storeId() == INVALID_ID)
        return error("A store is required");

    for (unsigned int i = 0; i < batch.items().size(); ++i) {
	const PriceBatchItem& line = batch.items()[i];
	if (line.item_id == INVALID_ID) continue;

	Item item;
	if (!lookup(line.item_id, item))
	    return error("Item doesn't exist");
	if (!item.isSold())
	    return error("Item isn't sold");

	if (line.number.isEmpty())
	    return error("Blank item number");
    }

    return true;
}

bool
QuasarDB::sqlCreateLines(const PriceBatch& batch)
{
    QString cmd = insertText("price_batch_item", "batch_id", "seq_num,"
			     "item_id,number,size_name,new_uprice,"
			     "new_fprice_qty,new_fprice,old_ucost,"
			     "old_fcost_qty,old_fcost,old_uprice,"
			     "old_fprice_qty,old_fprice");
    Stmt stmt(_connection, cmd);

    const vector<PriceBatchItem>& items = batch.items();
    for (unsigned int i = 0; i < items.size(); ++i) {
	const PriceBatchItem& line = items[i];

	stmtSetId(stmt, batch.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.item_id);
	stmtSetString(stmt, line.number);
	stmtSetString(stmt, line.size);
	stmtSetFixed(stmt, line.new_price.unitPrice());
	stmtSetFixed(stmt, line.new_price.forQty());
	stmtSetFixed(stmt, line.new_price.forPrice());
	stmtSetFixed(stmt, line.old_cost.unitPrice());
	stmtSetFixed(stmt, line.old_cost.forQty());
	stmtSetFixed(stmt, line.old_cost.forPrice());
	stmtSetFixed(stmt, line.old_price.unitPrice());
	stmtSetFixed(stmt, line.old_price.forQty());
	stmtSetFixed(stmt, line.old_price.forPrice());
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const PriceBatch& batch)
{
    return remove(batch, "price_batch_item", "batch_id");
}
