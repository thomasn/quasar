// $Id: promo_batch_db.cpp,v 1.2 2005/03/01 19:59:42 bpepers Exp $
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

#include "promo_batch.h"
#include "promo_batch_select.h"
#include "item.h"

// Create a PromoBatch
bool
QuasarDB::create(PromoBatch& batch)
{
    if (!validate(batch)) return false;

    // Auto allocate batch number
    if (batch.number().stripWhiteSpace() == "#") {
	fixed number = uniqueNumber("promo_batch", "number");
	batch.setNumber(number.toString());
    }

    QString cmd = insertCmd("promo_batch", "batch_id", "number,description,"
			    "store_id,from_date,to_date,exec_date");
    Stmt stmt(_connection, cmd);

    insertData(batch, stmt);
    stmtSetString(stmt, batch.number());
    stmtSetString(stmt, batch.description());
    stmtSetId(stmt, batch.storeId());
    stmtSetDate(stmt, batch.fromDate());
    stmtSetDate(stmt, batch.toDate());
    stmtSetDate(stmt, batch.executedOn());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(batch)) return false;

    commit();
    dataSignal(DataEvent::Insert, batch);
    return true;
}

// Delete a PromoBatch
bool
QuasarDB::remove(const PromoBatch& batch)
{
    if (batch.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(batch)) return false;
    if (!removeData(batch, "promo_batch", "batch_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, batch);
    return true;
}

// Update a PromoBatch
bool
QuasarDB::update(const PromoBatch& orig, PromoBatch& batch)
{
    if (orig.id() == INVALID_ID || batch.id() == INVALID_ID) return false;
    if (!validate(batch)) return false;

    if (batch.number().stripWhiteSpace() == "#")
	return error("Can't change number to '#'");

    // Update the batch table
    QString cmd = updateCmd("promo_batch", "batch_id", "number,description,"
			    "store_id,from_date,to_date,exec_date");
    Stmt stmt(_connection, cmd);

    updateData(orig, batch, stmt);
    stmtSetString(stmt, batch.number());
    stmtSetString(stmt, batch.description());
    stmtSetId(stmt, batch.storeId());
    stmtSetDate(stmt, batch.fromDate());
    stmtSetDate(stmt, batch.toDate());
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

// Lookup a PromoBatch.
bool
QuasarDB::lookup(Id batch_id, PromoBatch& batch)
{
    if (batch_id == INVALID_ID) return false;
    PromoBatchSelect conditions;
    vector<PromoBatch> batches;

    conditions.id = batch_id;
    if (!select(batches, conditions)) return false;
    if (batches.size() != 1) return false;

    batch = batches[0];
    return true;
}

// Returns a vector of PromoBatch's.
bool
QuasarDB::select(vector<PromoBatch>& batches, const PromoBatchSelect& conds)
{
    batches.clear();

    QString cmd = selectCmd("promo_batch", "batch_id", "number,description,"
			    "store_id,from_date,to_date,exec_date", conds);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	PromoBatch batch;
	int next = 1;
	selectData(batch, stmt, next);
	batch.setNumber(stmtGetString(stmt, next++));
	batch.setDescription(stmtGetString(stmt, next++));
	batch.setStoreId(stmtGetId(stmt, next++));
	batch.setFromDate(stmtGetDate(stmt, next++));
	batch.setToDate(stmtGetDate(stmt, next++));
	batch.setExecutedOn(stmtGetDate(stmt, next++));
	batches.push_back(batch);
    }

    QString cmd1 = "select item_id,number,size_name,unit_price,"
        "for_qty,for_price,ordered,price_id from promo_batch_item "
	"where batch_id = ? order by seq_num";
    Stmt stmt1(_connection, cmd1);

    for (unsigned int i = 0; i < batches.size(); ++i) {
	Id batch_id = batches[i].id();

	stmtSetId(stmt1, batch_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    PromoBatchItem line;
	    int next = 1;
	    line.item_id = stmtGetId(stmt1, next++);
	    line.number = stmtGetString(stmt1, next++);
	    line.size = stmtGetString(stmt1, next++);
	    line.price.setUnitPrice(stmtGetFixed(stmt1, next++));
	    line.price.setForQty(stmtGetFixed(stmt1, next++));
	    line.price.setForPrice(stmtGetFixed(stmt1, next++));
	    line.ordered = stmtGetFixed(stmt1, next++);
	    line.price_id = stmtGetId(stmt1, next++);
	    batches[i].items().push_back(line);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const PromoBatch& batch)
{
    if (!validate((DataObject&)batch)) return false;

    if (batch.number().stripWhiteSpace().isEmpty())
	return error("Blank batch number");

    if (batch.storeId() == INVALID_ID)
        return error("A store is required");

    for (unsigned int i = 0; i < batch.items().size(); ++i) {
	const PromoBatchItem& line = batch.items()[i];
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
QuasarDB::sqlCreateLines(const PromoBatch& batch)
{
    QString cmd = insertText("promo_batch_item", "batch_id", "seq_num,"
			     "item_id,number,size_name,unit_price,"
			     "for_qty,for_price,ordered,price_id");
    Stmt stmt(_connection, cmd);

    const vector<PromoBatchItem>& items = batch.items();
    for (unsigned int i = 0; i < items.size(); ++i) {
	const PromoBatchItem& line = items[i];

	stmtSetId(stmt, batch.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.item_id);
	stmtSetString(stmt, line.number);
	stmtSetString(stmt, line.size);
	stmtSetFixed(stmt, line.price.unitPrice());
	stmtSetFixed(stmt, line.price.forQty());
	stmtSetFixed(stmt, line.price.forPrice());
	stmtSetFixed(stmt, line.ordered);
	stmtSetId(stmt, line.price_id);
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const PromoBatch& batch)
{
    return remove(batch, "promo_batch_item", "batch_id");
}
