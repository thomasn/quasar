// $Id: label_batch_db.cpp,v 1.6 2005/03/01 19:59:42 bpepers Exp $
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

#include "label_batch.h"
#include "label_batch_select.h"
#include "item.h"

// Create a LabelBatch
bool
QuasarDB::create(LabelBatch& batch)
{
    if (!validate(batch)) return false;

    // Auto allocate batch number
    if (batch.number().stripWhiteSpace() == "#") {
	fixed number = uniqueNumber("label_batch", "number");
	batch.setNumber(number.toString());
    }

    QString cmd = insertCmd("label_batch", "batch_id", "number,description,"
			    "label_type,store_id,print_date");
    Stmt stmt(_connection, cmd);

    insertData(batch, stmt);
    stmtSetString(stmt, batch.number());
    stmtSetString(stmt, batch.description());
    stmtSetString(stmt, batch.type());
    stmtSetId(stmt, batch.storeId());
    stmtSetDate(stmt, batch.printedOn());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(batch)) return false;

    commit();
    dataSignal(DataEvent::Insert, batch);
    return true;
}

// Delete a LabelBatch
bool
QuasarDB::remove(const LabelBatch& batch)
{
    if (batch.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(batch)) return false;
    if (!removeData(batch, "label_batch", "batch_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, batch);
    return true;
}

// Update a LabelBatch
bool
QuasarDB::update(const LabelBatch& orig, LabelBatch& batch)
{
    if (orig.id() == INVALID_ID || batch.id() == INVALID_ID) return false;
    if (!validate(batch)) return false;

    if (batch.number().stripWhiteSpace() == "#")
	return error("Can't change number to '#'");

    // Update the batch table
    QString cmd = updateCmd("label_batch", "batch_id", "number,description,"
			    "label_type,store_id,print_date");
    Stmt stmt(_connection, cmd);

    updateData(orig, batch, stmt);
    stmtSetString(stmt, batch.number());
    stmtSetString(stmt, batch.description());
    stmtSetString(stmt, batch.type());
    stmtSetId(stmt, batch.storeId());
    stmtSetDate(stmt, batch.printedOn());
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

// Lookup a LabelBatch.
bool
QuasarDB::lookup(Id batch_id, LabelBatch& batch)
{
    if (batch_id == INVALID_ID) return false;
    LabelBatchSelect conditions;
    vector<LabelBatch> batches;

    conditions.id = batch_id;
    if (!select(batches, conditions)) return false;
    if (batches.size() != 1) return false;

    batch = batches[0];
    return true;
}

// Returns a vector of LabelBatch's.
bool
QuasarDB::select(vector<LabelBatch>& batches, const LabelBatchSelect& conds)
{
    batches.clear();

    QString cmd = selectCmd("label_batch", "batch_id", "number,description,"
			    "label_type,store_id,print_date", conds);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	LabelBatch batch;
	int next = 1;
	selectData(batch, stmt, next);
	batch.setNumber(stmtGetString(stmt, next++));
	batch.setDescription(stmtGetString(stmt, next++));
	batch.setType(stmtGetString(stmt, next++));
	batch.setStoreId(stmtGetId(stmt, next++));
	batch.setPrintedOn(stmtGetDate(stmt, next++));
	batches.push_back(batch);
    }

    QString cmd1 = "select item_id,number,size_name,unit_price,"
	"for_price_qty,for_price,label_cnt from label_batch_item "
	"where batch_id = ? order by seq_num";
    Stmt stmt1(_connection, cmd1);

    for (unsigned int i = 0; i < batches.size(); ++i) {
	Id batch_id = batches[i].id();

	stmtSetId(stmt1, batch_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    LabelBatchItem line;
	    int next = 1;
	    line.item_id = stmtGetId(stmt1, next++);
	    line.number = stmtGetString(stmt1, next++);
	    line.size = stmtGetString(stmt1, next++);
	    line.price.setUnitPrice(stmtGetFixed(stmt1, next++));
	    line.price.setForQty(stmtGetFixed(stmt1, next++));
	    line.price.setForPrice(stmtGetFixed(stmt1, next++));
	    line.count = stmtGetInt(stmt1, next++);
	    batches[i].items().push_back(line);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const LabelBatch& batch)
{
    if (!validate((DataObject&)batch)) return false;

    if (batch.number().stripWhiteSpace().isEmpty())
	return error("Blank batch number");

    if (batch.storeId() == INVALID_ID)
        return error("A store is required");

    for (unsigned int i = 0; i < batch.items().size(); ++i) {
	const LabelBatchItem& line = batch.items()[i];
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
QuasarDB::sqlCreateLines(const LabelBatch& batch)
{
    QString cmd = insertText("label_batch_item", "batch_id", "seq_num,"
			     "item_id,number,size_name,unit_price,"
			     "for_price_qty,for_price,label_cnt");
    Stmt stmt(_connection, cmd);

    const vector<LabelBatchItem>& items = batch.items();
    for (unsigned int i = 0; i < items.size(); ++i) {
	const LabelBatchItem& line = items[i];

	stmtSetId(stmt, batch.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.item_id);
	stmtSetString(stmt, line.number);
	stmtSetString(stmt, line.size);
	stmtSetFixed(stmt, line.price.unitPrice());
	stmtSetFixed(stmt, line.price.forQty());
	stmtSetFixed(stmt, line.price.forPrice());
	stmtSetInt(stmt, line.count);
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const LabelBatch& batch)
{
    return remove(batch, "label_batch_item", "batch_id");
}
