// $Id: slip_db.cpp,v 1.19 2005/03/13 22:22:21 bpepers Exp $
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

#include "slip.h"
#include "slip_select.h"
#include "card.h"
#include "item.h"

// Create a Slip
bool
QuasarDB::create(Slip& slip)
{
    if (!validate(slip)) return false;

    // Auto allocate slip number
    if (slip.number().stripWhiteSpace() == "#") {
	fixed number = uniqueNumber("slip", "number");
	slip.setNumber(number.toString());
    }

    QString cmd = insertCmd("slip", "slip_id", "vendor_id,number,"
			    "waybill,carrier,ship_date,store_id,"
			    "inv_num,num_pieces,status,post_date");
    Stmt stmt(_connection, cmd);

    insertData(slip, stmt);
    stmtSetId(stmt, slip.vendorId());
    stmtSetString(stmt, slip.number());
    stmtSetString(stmt, slip.waybill());
    stmtSetString(stmt, slip.carrier());
    stmtSetDate(stmt, slip.shipDate());
    stmtSetId(stmt, slip.storeId());
    stmtSetString(stmt, slip.invoiceNumber());
    stmtSetInt(stmt, slip.numPieces());
    stmtSetString(stmt, slip.status());
    stmtSetDate(stmt, slip.postDate());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(slip)) return false;

    commit();
    dataSignal(DataEvent::Insert, slip);
    return true;
}

// Delete a Slip
bool
QuasarDB::remove(const Slip& slip)
{
    if (slip.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(slip)) return false;
    if (!removeData(slip, "slip", "slip_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, slip);
    return true;
}

// Update a Slip
bool
QuasarDB::update(const Slip& orig, Slip& slip)
{
    if (orig.id() == INVALID_ID || slip.id() == INVALID_ID) return false;
    if (!validate(slip)) return false;

    if (slip.number().stripWhiteSpace() == "#")
	return error("Can't change number to '#'");

    // Update the slip table
    QString cmd = updateCmd("slip", "slip_id", "vendor_id,number,"
			    "waybill,carrier,ship_date,store_id,"
			    "inv_num,num_pieces,status,post_date");
    Stmt stmt(_connection, cmd);

    updateData(orig, slip, stmt);
    stmtSetId(stmt, slip.vendorId());
    stmtSetString(stmt, slip.number());
    stmtSetString(stmt, slip.waybill());
    stmtSetString(stmt, slip.carrier());
    stmtSetDate(stmt, slip.shipDate());
    stmtSetId(stmt, slip.storeId());
    stmtSetString(stmt, slip.invoiceNumber());
    stmtSetInt(stmt, slip.numPieces());
    stmtSetString(stmt, slip.status());
    stmtSetDate(stmt, slip.postDate());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");
    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(slip)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Slip.
bool
QuasarDB::lookup(Id slip_id, Slip& slip)
{
    if (slip_id == INVALID_ID) return false;
    SlipSelect conditions;
    vector<Slip> slips;

    conditions.id = slip_id;
    if (!select(slips, conditions)) return false;
    if (slips.size() != 1) return false;

    slip = slips[0];
    return true;
}

// Returns a vector of Slips.
bool
QuasarDB::select(vector<Slip>& slips, const SlipSelect& conditions)
{
    slips.clear();
    QString cmd = selectCmd("slip", "slip_id", "vendor_id,number,"
			    "waybill,carrier,ship_date,store_id,"
			    "inv_num,num_pieces,status,post_date",
			    conditions, "slip_id");
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Slip slip;
	int next = 1;
	selectData(slip, stmt, next);
	slip.setVendorId(stmtGetId(stmt, next++));
	slip.setNumber(stmtGetString(stmt, next++));
	slip.setWaybill(stmtGetString(stmt, next++));
	slip.setCarrier(stmtGetString(stmt, next++));
	slip.setShipDate(stmtGetDate(stmt, next++));
	slip.setStoreId(stmtGetId(stmt, next++));
	slip.setInvoiceNumber(stmtGetString(stmt, next++));
	slip.setNumPieces(stmtGetInt(stmt, next++));
	slip.setStatus(stmtGetString(stmt, next++));
	slip.setPostDate(stmtGetDate(stmt, next++));
	slips.push_back(slip);
    }

    QString cmd1 = "select item_id,number,size_name,size_qty,quantity,"
	"ext_cost from slip_item where slip_id = ? order by seq_num";
    Stmt stmt1(_connection, cmd1);

    QString cmd2 = "select order_number from slip_orders where "
	"slip_id = ? order by seq_num";
    Stmt stmt2(_connection, cmd2);

    for (unsigned int i = 0; i < slips.size(); ++i) {
	Id slip_id = slips[i].id();

	stmtSetId(stmt1, slip_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    SlipItem line;
	    int next = 1;
	    line.item_id = stmtGetId(stmt1, next++);
	    line.number = stmtGetString(stmt1, next++);
	    line.size = stmtGetString(stmt1, next++);
	    line.size_qty = stmtGetFixed(stmt1, next++);
	    line.quantity = stmtGetFixed(stmt1, next++);
	    line.ext_cost = stmtGetFixed(stmt1, next++);
	    slips[i].items().push_back(line);
	}

	stmtSetId(stmt2, slip_id);
	if (!execute(stmt2)) return false;
	while (stmt2.next()) {
	    slips[i].orders().push_back(stmtGetString(stmt2, 1));
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Slip& slip)
{
    if (!validate((DataObject&)slip)) return false;

    if (slip.number().stripWhiteSpace().isEmpty())
	return error("Blank slip number");

    Card card;
    if (!lookup(slip.vendorId(), card))
	return error("Vendor doesn't exist");
    if (card.dataType() != DataObject::VENDOR)
	return error("Card is not a vendor");

    if (slip.shipDate().isNull())
	return error("Blank ship date");

    if (slip.items().size() < 1)
	return error("Slip must have at least one line");

    unsigned int i;
    for (i = 0; i < slip.items().size(); ++i) {
	const SlipItem& line = slip.items()[i];

	Item item;
	if (!lookup(line.item_id, item))
	    return error("Item doesn't exist");
	if (!item.isPurchased())
	    return error("Item isn't purchased");

	if (line.number.isEmpty())
	    return error("Blank item number");

	if (line.quantity < 0.0)
	    return error("Negative quantity");
	if (line.ext_cost < 0.0)
	    return error("Negative extended cost");
    }

    return true;
}

bool
QuasarDB::sqlCreateLines(const Slip& slip)
{
    QString cmd = insertText("slip_item", "slip_id", "seq_num,item_id,"
			     "number,size_name,size_qty,quantity,ext_cost");
    Stmt stmt(_connection, cmd);

    for (unsigned int i = 0; i < slip.items().size(); ++i) {
	const SlipItem& line = slip.items()[i];

	stmtSetId(stmt, slip.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.item_id);
	stmtSetString(stmt, line.number);
	stmtSetString(stmt, line.size);
	stmtSetFixed(stmt, line.size_qty);
	stmtSetFixed(stmt, line.quantity);
	stmtSetFixed(stmt, line.ext_cost);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("slip_orders", "slip_id", "seq_num,order_number");
    stmt.setCommand(cmd);

    for (unsigned int i = 0; i < slip.orders().size(); ++i) {
	stmtSetId(stmt, slip.id());
	stmtSetInt(stmt, i);
	stmtSetString(stmt, slip.orders()[i]);
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const Slip& slip)
{
    if (!remove(slip, "slip_orders", "slip_id")) return false;
    return remove(slip, "slip_item", "slip_id");
}
