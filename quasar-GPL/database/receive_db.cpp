// $Id: receive_db.cpp,v 1.56 2005/05/13 23:56:00 bpepers Exp $
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

#include "receive.h"
#include "receive_select.h"
#include "vendor.h"
#include "item.h"
#include "tax.h"
#include "account.h"
#include "order.h"
#include "slip.h"
#include "charge.h"
#include "company.h"
#include "object_cache.h"

// Create a Receive
bool
QuasarDB::create(Receive& receive)
{
    if (!validate(receive)) return false;
    if (!create((Gltx&)receive)) return false;

    QString cmd = insertText("receive", "receive_id", "vendor_addr,"
			     "ship_id,ship_addr,ship_via,term_id,"
			     "comments,invoice_date,line_type,closed");
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, receive.id());
    stmtSetString(stmt, receive.vendorAddress());
    stmtSetId(stmt, receive.shipId());
    stmtSetString(stmt, receive.shipAddress());
    stmtSetString(stmt, receive.shipVia());
    stmtSetId(stmt, receive.termsId());
    stmtSetString(stmt, receive.comment());
    stmtSetDate(stmt, receive.invoiceDate());
    stmtSetInt(stmt, receive.lineType());
    stmtSetBool(stmt, receive.isClosed());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(receive)) return false;

    commit();
    dataSignal(DataEvent::Insert, receive);
    return true;
}

// Delete a Receive
bool
QuasarDB::remove(const Receive& receive)
{
    if (receive.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(receive)) return false;
    if (!remove((Gltx&)receive)) return false;

    commit();
    dataSignal(DataEvent::Delete, receive);
    return true;
}

// Update a Receive
bool
QuasarDB::update(const Receive& orig, Receive& receive)
{
    if (orig.id() == INVALID_ID || receive.id() == INVALID_ID) return false;
    if (!validate(receive)) return false;
    if (!update(orig, (Gltx&)receive)) return false;

    // Update the receive tables
    QString cmd = updateText("receive", "receive_id", "vendor_addr,"
			     "ship_id,ship_addr,ship_via,term_id,"
			     "comments,invoice_date,line_type,closed");
    Stmt stmt(_connection, cmd);

    stmtSetString(stmt, receive.vendorAddress());
    stmtSetId(stmt, receive.shipId());
    stmtSetString(stmt, receive.shipAddress());
    stmtSetString(stmt, receive.shipVia());
    stmtSetId(stmt, receive.termsId());
    stmtSetString(stmt, receive.comment());
    stmtSetDate(stmt, receive.invoiceDate());
    stmtSetInt(stmt, receive.lineType());
    stmtSetBool(stmt, receive.isClosed());
    stmtSetId(stmt, orig.id());

    if (!execute(stmt)) return false;
    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(receive)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Receive.
bool
QuasarDB::lookup(Id receive_id, Receive& receive)
{
    if (receive_id == INVALID_ID) return false;
    ReceiveSelect conditions;
    vector<Receive> receives;

    conditions.id = receive_id;
    if (!select(receives, conditions)) return false;
    if (receives.size() != 1) return false;

    receive = receives[0];
    return true;
}

// Returns a vector of Receives.
bool
QuasarDB::select(vector<Receive>& receives, const ReceiveSelect& conditions)
{
    receives.clear();

    QString cmd = gltxCmd("receive", "receive_id", "vendor_addr,"
			  "ship_id,ship_addr,ship_via,term_id,comments,"
			  "invoice_date,line_type,closed,data_type",
			  conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Receive receive;
	int next = 1;
	selectData(receive, stmt, next);
	selectGltx(receive, stmt, next);
	receive.setVendorAddress(stmtGetString(stmt, next++));
	receive.setShipId(stmtGetId(stmt, next++));
	receive.setShipAddress(stmtGetString(stmt, next++));
	receive.setShipVia(stmtGetString(stmt, next++));
	receive.setTermsId(stmtGetId(stmt, next++));
	receive.setComment(stmtGetString(stmt, next++));
	receive.setInvoiceDate(stmtGetDate(stmt, next++));
	receive.setLineType(Receive::LineType(stmtGetInt(stmt, next++)));
	receive.setClosed(stmtGetBool(stmt, next++));
	receive.setDataType(DataObject::DataType(stmtGetInt(stmt,next++)));
	receives.push_back(receive);
    }

    QString cmd1 = "select item_id,account_id,number,description,"
	"size_name,size_qty,quantity,inv_cost,sale_price,"
	"ext_deposit,voided,ordered,received,cost_id,cost_disc,"
	"unit_price,for_qty,for_price,ext_cost,ext_base,ext_tax,"
	"int_charges,ext_charges,tax_id,item_tax_id,include_tax,"
	"include_deposit,open_dept from gltx_item join receive_item "
	"on (gltx_item.gltx_id = receive_item.receive_id and "
	"gltx_item.seq_num = receive_item.seq_num) where "
	"gltx_item.gltx_id = ? order by gltx_item.seq_num";
    Stmt stmt1(_connection, cmd1);

    QString cmd2 = "select order_id from receive_order where "
	"receive_id = ? order by seq_num";
    Stmt stmt2(_connection, cmd2);

    QString cmd3 = "select slip_id from receive_slip where "
	"receive_id = ? order by seq_num";
    Stmt stmt3(_connection, cmd3);

    QString cmd4 = "select charge_id,tax_id,amount,base,internal from "
	"receive_charge where receive_id = ? order by seq_num";
    Stmt stmt4(_connection, cmd4);

    for (unsigned int i = 0; i < receives.size(); ++i) {
	Id gltx_id = receives[i].id();
	GLTX_ACCOUNTS(receives);
	GLTX_TAXES(receives);
	GLTX_CARDS(receives);
	GLTX_PAYMENTS(receives);

	stmtSetId(stmt1, gltx_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    ReceiveItem line;
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
	    line.ordered = stmtGetFixed(stmt1, next++);
	    line.received = stmtGetFixed(stmt1, next++);
	    line.cost_id = stmtGetId(stmt1, next++);
	    line.cost_disc = stmtGetFixed(stmt1, next++);
	    line.cost.setUnitPrice(stmtGetFixed(stmt1, next++));
	    line.cost.setForQty(stmtGetFixed(stmt1, next++));
	    line.cost.setForPrice(stmtGetFixed(stmt1, next++));
	    line.ext_cost = stmtGetFixed(stmt1, next++);
	    line.ext_base = stmtGetFixed(stmt1, next++);
	    line.ext_tax = stmtGetFixed(stmt1, next++);
	    line.int_charges = stmtGetFixed(stmt1, next++);
	    line.ext_charges = stmtGetFixed(stmt1, next++);
	    line.tax_id = stmtGetId(stmt1, next++);
	    line.item_tax_id = stmtGetId(stmt1, next++);
	    line.include_tax = stmtGetBool(stmt1, next++);
	    line.include_deposit = stmtGetBool(stmt1, next++);
	    line.open_dept = stmtGetBool(stmt1, next++);
	    receives[i].items().push_back(line);
	}

	stmtSetId(stmt2, gltx_id);
	if (!execute(stmt2)) return false;
	while (stmt2.next()) {
	    receives[i].orders().push_back(stmtGetId(stmt2, 1));
	}

	stmtSetId(stmt3, gltx_id);
	if (!execute(stmt3)) return false;
	while (stmt3.next()) {
	    receives[i].slips().push_back(stmtGetId(stmt3, 1));
	}

	stmtSetId(stmt4, gltx_id);
	if (!execute(stmt4)) return false;
	while (stmt4.next()) {
	    ReceiveCharge line;
	    line.charge_id = stmtGetId(stmt4, 1);
	    line.tax_id = stmtGetId(stmt4, 2);
	    line.amount = stmtGetFixed(stmt4, 3);
	    line.base = stmtGetFixed(stmt4, 4);
	    line.internal = stmtGetBool(stmt4, 5);
	    receives[i].charges().push_back(line);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Receive& receive)
{
    Card card;
    if (!lookup(receive.vendorId(), card))
	return error("Vendor doesn't exist");
    if (card.dataType() != DataObject::VENDOR)
	return error("Card is not a vendor");

    if (receive.shipId() != INVALID_ID) {
	if (!lookup(receive.shipId(), card))
	    return error("Ship to card doesn't exist");
    }

    if (receive.invoiceDate().isNull())
	return error("Invalid invoice date");

    if (receive.lineType() < Receive::Item)
	return error("Invalid line type");
    if (receive.lineType() > Receive::Account)
	return error("Invalid line type");

    if (receive.items().size() < 1)
	return error("Receiving must have at least one line");

    unsigned int i;
    for (i = 0; i < receive.items().size(); ++i) {
	const ReceiveItem& line = receive.items()[i];

	if (line.item_id == INVALID_ID && line.account_id == INVALID_ID)
	    return error("Both item and account can't be blank");
	if (line.item_id != INVALID_ID && line.account_id != INVALID_ID)
	    return error("Both item and account can't be set");
	if (line.item_id != INVALID_ID && line.number.isEmpty())
	    return error("Blank item number");

#if CHECK_ITEM
	if (line.item_id != INVALID_ID) {
	    Item item;
	    if (!lookup(line.item_id, item))
		return error("Item doesn't exist");
	    if (!item.isPurchased())
		return error("Item isn't purchased");
	}

	if (line.account_id != INVALID_ID) {
	    Account account;
	    if (!lookup(line.account_id, account))
		return error("Account doesn't exist");
	    if (account.isHeader())
		return error("Account is a header");
	}
#endif

	// TODO: any checks on ordered, received, shipped, or ext_cost?
    }

    for (i = 0; i < receive.payments().size(); ++i) {
	const PaymentLine& line = receive.payments()[i];

	Gltx gltx;
	if (!lookup(line.gltx_id, gltx))
	    return error("Payment gltx doesn't exist");
//	if (gltx.dataType() != DataObject::CHEQUE)
//	    return error("Payment is not a proper transaction");
    }

    for (i = 0; i < receive.orders().size(); ++i) {
	Order order;
	if (!lookup(receive.orders()[i], order))
	    return error("Order doesn't exist");
    }

    for (i = 0; i < receive.slips().size(); ++i) {
	Slip slip;
	if (!lookup(receive.slips()[i], slip))
	    return error("Receiving doesn't exist");
    }

    return validate((Gltx&)receive);
}

bool
QuasarDB::sqlCreateLines(const Receive& receive)
{
    QString cmd1 = insertText("gltx_item", "gltx_id", "seq_num,item_id,"
			      "account_id,number,description,size_name,"
			      "size_qty,quantity,inv_cost,sale_price,"
			      "ext_deposit,voided");
    Stmt stmt1(_connection, cmd1);

    QString cmd2 = insertText("receive_item", "receive_id", "seq_num,"
			      "ordered,received,cost_id,cost_disc,unit_price,"
			      "for_qty,for_price,ext_cost,ext_base,ext_tax,"
			      "int_charges,ext_charges,tax_id,item_tax_id,"
			      "include_tax,include_deposit,open_dept");
    Stmt stmt2(_connection, cmd2);

    // Item change info
    vector<Id> item_ids;
    vector<QString> sizes;
    vector<fixed> qtys;
    vector<fixed> costs;

    const vector<ReceiveItem>& items = receive.items();
    unsigned int i;
    for (i = 0; i < items.size(); ++i) {
	const ReceiveItem& line = items[i];

	stmtSetId(stmt1, receive.id());
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

	stmtSetId(stmt2, receive.id());
	stmtSetInt(stmt2, i);
	stmtSetFixed(stmt2, line.ordered);
	stmtSetFixed(stmt2, line.received);
	stmtSetId(stmt2, line.cost_id);
	stmtSetFixed(stmt2, line.cost_disc);
	stmtSetFixed(stmt2, line.cost.unitPrice());
	stmtSetFixed(stmt2, line.cost.forQty());
	stmtSetFixed(stmt2, line.cost.forPrice());
	stmtSetFixed(stmt2, line.ext_cost);
	stmtSetFixed(stmt2, line.ext_base);
	stmtSetFixed(stmt2, line.ext_tax);
	stmtSetFixed(stmt2, line.int_charges);
	stmtSetFixed(stmt2, line.ext_charges);
	stmtSetId(stmt2, line.tax_id);
	stmtSetId(stmt2, line.item_tax_id);
	stmtSetBool(stmt2, line.include_tax);
	stmtSetBool(stmt2, line.include_deposit);
	stmtSetBool(stmt2, line.open_dept);
	if (!execute(stmt2)) return false;

	if (line.item_id == INVALID_ID) continue;
	if (!receive.isActive()) continue;
	if (line.voided) continue;

	item_ids.push_back(line.item_id);
	sizes.push_back(line.size);
	if (line.open_dept)
	    qtys.push_back(0);
	else
	    qtys.push_back(line.quantity * line.size_qty);
	costs.push_back(line.inv_cost);
    }

    itemReceive(item_ids, sizes, receive.storeId(), receive.postDate(),
		qtys, costs);

    QString cmd = insertText("receive_order", "receive_id", "seq_num,"
			     "order_id");
    Stmt stmt(_connection, cmd);

    for (i = 0; i < receive.orders().size(); ++i) {
	stmtSetId(stmt, receive.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, receive.orders()[i]);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("receive_slip", "receive_id", "seq_num,slip_id");
    stmt.setCommand(cmd);

    for (i = 0; i < receive.slips().size(); ++i) {
	stmtSetId(stmt, receive.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, receive.slips()[i]);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("receive_charge", "receive_id", "seq_num,charge_id,"
		     "tax_id,amount,base,internal");
    stmt.setCommand(cmd);

    const vector<ReceiveCharge>& charges = receive.charges();
    for (i = 0; i < charges.size(); ++i) {
	const ReceiveCharge& line = charges[i];

	stmtSetId(stmt, receive.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.charge_id);
	stmtSetId(stmt, line.tax_id);
	stmtSetFixed(stmt, line.amount);
	stmtSetFixed(stmt, line.base);
	stmtSetBool(stmt, line.internal);
	if (!execute(stmt)) return false;
    }

    // Load the orders
    vector<Order> o_orig;
    vector<Order> orders;
    for (i = 0; i < receive.orders().size(); ++i) {
	Id order_id = receive.orders()[i];
	Order order;
	lookup(order_id, order);
	o_orig.push_back(order);
	orders.push_back(order);
    }

    // Load the slips
    vector<Slip> s_orig;
    vector<Slip> slips;
    for (i = 0; i < receive.slips().size(); ++i) {
	Id slip_id = receive.slips()[i];
	Slip slip;
	lookup(slip_id, slip);
	s_orig.push_back(slip);
	slips.push_back(slip);
    }

    // Try to allocate the items
    for (unsigned int item = 0; item < items.size(); ++item) {
	Id item_id = items[item].item_id;
	QString size = items[item].size;
	fixed shipped = items[item].quantity;
	if (item_id == INVALID_ID) continue;

	int found_num = -1;
	int found_line = -1;
	fixed remain = shipped;
	unsigned int i;
	for (i = 0; i < orders.size() && remain > 0.0; ++i) {
	    Order& order = orders[i];

	    for (unsigned int j = 0; j < order.items().size(); ++j) {
		if (order.items()[j].item_id != item_id) continue;
		if (order.items()[j].size != size) continue;
		fixed ordered = order.items()[j].ordered;
		fixed billed = order.items()[j].billed;
		found_num = i;
		found_line = j;
		if (billed >= ordered) continue;

		fixed alloc = ordered - billed;
		if (alloc > remain) alloc = remain;

		order.items()[j].billed += alloc;
		remain -= alloc;
	    }
	}
	if (remain > 0.0 && found_num != -1)
	    orders[found_num].items()[found_line].billed += remain;
    }

    // Get the vendor
    Vendor vendor;
    lookup(receive.vendorId(), vendor);

    // Check for closed and save changes
    for (i = 0; i < orders.size(); ++i) {
	Order& order = orders[i];
	if (order.allReceived() || !vendor.backorders())
	    setActive(order, false);
	if (o_orig[i] != orders[i])
	    update(o_orig[i], orders[i]);
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const Receive& receive)
{
    // Undo the on hand/total cost
    QString cmd = "select item_id,size_name,size_qty,quantity,inv_cost,"
	"voided,open_dept from gltx_item join receive_item on "
	"gltx_item.gltx_id = receive_item.receive_id and "
	"gltx_item.seq_num = receive_item.seq_num where gltx_item.gltx_id=?";
    Stmt stmt(_connection, cmd);
    stmtSetId(stmt, receive.id());

    // Item change info
    vector<Id> item_ids;
    vector<QString> sizes;
    vector<fixed> qtys;
    vector<fixed> costs;

    execute(stmt);
    while (stmt.next()) {
	Id item_id = stmtGetId(stmt, 1);
	QString size = stmtGetString(stmt, 2);
	fixed size_qty = stmtGetFixed(stmt, 3);
	fixed quantity = stmtGetFixed(stmt, 4);
	fixed inv_cost = stmtGetFixed(stmt, 5);
	bool voided = stmtGetBool(stmt, 6);
	bool open_dept = stmtGetBool(stmt, 7);

	if (item_id == INVALID_ID) continue;
	if (!receive.isActive()) continue;
	if (voided) continue;

	item_ids.push_back(item_id);
	sizes.push_back(size);
	if (open_dept)
	    qtys.push_back(0);
	else
	    qtys.push_back(-quantity * size_qty);
	costs.push_back(-inv_cost);
    }

    itemReceive(item_ids, sizes, receive.storeId(), receive.postDate(),
		qtys, costs);

    if (!remove(receive, "receive_item", "receive_id")) return false;
    if (!remove(receive, "gltx_item", "gltx_id")) return false;
    if (!remove(receive, "receive_order", "receive_id")) return false;
    if (!remove(receive, "receive_slip", "receive_id")) return false;
    if (!remove(receive, "receive_charge", "receive_id")) return false;

    if (!receive.isActive())
	return true;

    // Load the orders
    vector<Order> o_orig;
    vector<Order> orders;
    unsigned int i;
    for (i = 0; i < receive.orders().size(); ++i) {
	Id order_id = receive.orders()[i];
	Order order;
	lookup(order_id, order);
	o_orig.push_back(order);
	orders.push_back(order);
    }

    // Load the slips
    vector<Slip> s_orig;
    vector<Slip> slips;
    for (i = 0; i < receive.slips().size(); ++i) {
	Id slip_id = receive.slips()[i];
	Slip slip;
	lookup(slip_id, slip);
	s_orig.push_back(slip);
	slips.push_back(slip);
    }

    // Try to un-allocate the items
    const vector<ReceiveItem>& items = receive.items();
    for (unsigned int item = 0; item < items.size(); ++item) {
	Id item_id = items[item].item_id;
	QString size = items[item].size;
	fixed shipped = items[item].quantity;
	if (item_id == INVALID_ID) continue;

	fixed remain = shipped;
	int i;
	for (i = orders.size() - 1; i >= 0 && remain > 0.0; --i) {
	    Order& order = orders[i];

	    for (int j = order.items().size() - 1; j >= 0; --j) {
		if (order.items()[j].item_id != item_id) continue;
		if (order.items()[j].size != size) continue;
		fixed billed = order.items()[j].billed;
		if (billed == 0.0) continue;

		fixed alloc = billed;
		if (alloc > remain) alloc = remain;

		order.items()[j].billed -= alloc;
		remain -= alloc;
	    }
	}
    }

    // Get the vendor
    Vendor vendor;
    lookup(receive.vendorId(), vendor);

    // Check for closed and save changes
    for (i = 0; i < orders.size(); ++i) {
	Order& order = orders[i];
	if (!order.allReceived() && vendor.backorders())
	    setActive(order, true);
	if (o_orig[i] != orders[i])
	    update(o_orig[i], orders[i]);
    }

    return true;
}

static void
addTax(Receive& receive, fixed taxable, vector<Id>& ids, vector<fixed>& amts)
{
    for (unsigned int i = 0; i < ids.size(); ++i) {
	Id tax_id = ids[i];
	fixed tax_amt = amts[i];

	int found = -1;
	for (unsigned int j = 0; j < receive.taxes().size(); ++j) {
	    TaxLine& line = receive.taxes()[j];
	    if (line.tax_id == tax_id) {
		found = j;
		break;
	    }
	}

	if (found >= 0) {
	    receive.taxes()[found].taxable += taxable;
	    receive.taxes()[found].amount += tax_amt;
	} else {
	    TaxLine line(tax_id, taxable, tax_amt);
	    receive.taxes().push_back(line);
	}
    }
}

static void
addTaxInc(Receive& receive,fixed taxable, vector<Id>& ids, vector<fixed>& amts)
{
    for (unsigned int i = 0; i < ids.size(); ++i) {
	Id tax_id = ids[i];
	fixed tax_amt = amts[i];

	int found = -1;
	for (unsigned int j = 0; j < receive.taxes().size(); ++j) {
	    TaxLine& line = receive.taxes()[j];
	    if (line.tax_id == tax_id) {
		found = j;
		break;
	    }
	}

	if (found >= 0) {
	    receive.taxes()[found].inc_taxable += taxable;
	    receive.taxes()[found].inc_amount += tax_amt;
	} else {
	    TaxLine line(tax_id, 0.0, 0.0, taxable, tax_amt);
	    receive.taxes().push_back(line);
	}
    }
}

typedef QPair<Id, fixed> InfoPair;

static void
addInfo(vector<InfoPair>& info, Id id, fixed amount)
{
    if (id == INVALID_ID) return;
    for (unsigned int i = 0; i < info.size(); ++i) {
	if (info[i].first == id) {
	    info[i].second += amount;
	    return;
	}
    }
    info.push_back(InfoPair(id, amount));
}

bool
QuasarDB::prepare(Receive& receive, ObjectCache& cache)
{
    // Clear things to be filled in
    receive.accounts().clear();
    receive.taxes().clear();
    receive.cards().clear();

    // Account postings
    vector<InfoPair> asset_info;
    vector<InfoPair> expense_info;
    vector<InfoPair> tax_info;

    // Need company for deposit account
    Company company;
    lookup(company);

    // Need vendor for posting account
    Vendor vendor;
    cache.findVendor(receive.vendorId(), vendor);

    // No vendor tax exempt yet but the code is here
    Tax exempt;

    // Get sign from claim flag
    fixed sign = 1;
    if (receive.isClaim()) sign = -1;

    // Process items
    fixed item_total = 0.0;
    fixed total_weight = 0.0;
    for (unsigned int i = 0; i < receive.items().size(); ++i) {
	ReceiveItem& line = receive.items()[i];
	if (line.item_id == INVALID_ID && line.account_id == INVALID_ID)
	    return error(QString("No item or account on line %1").arg(i + 1));

	Item item;
	cache.findItem(line.item_id, item);

	// Calculate base price
	line.ext_base = line.ext_cost;
	if (line.include_deposit)
	    line.ext_base -= line.ext_deposit;

	// Calculate taxes
	fixed ext_tax = 0.0;
	if (line.tax_id != INVALID_ID) {
	    fixed base = line.ext_base;
	    Tax tax;
	    cache.findTax(line.tax_id, tax);
	    vector<Id> tax_ids;
	    vector<fixed> tax_amts;

	    if (line.include_tax) {
		ext_tax = calculateTaxOff(cache, tax, base, tax_ids,tax_amts);
		addTaxInc(receive, base, tax_ids, tax_amts);
		line.ext_base -= ext_tax;
	    } else {
		ext_tax = calculateTaxOn(cache, tax, base, exempt, tax_ids,
					 tax_amts);
		addTax(receive, base, tax_ids, tax_amts);
	    }
	}
	line.ext_tax = ext_tax;

	// Clear out old charges
	line.int_charges = 0.0;
	line.ext_charges = 0.0;

	// Add up totals
	item_total += line.ext_base;
	total_weight += item.weight(line.size) * line.quantity;
    }

    // Process internal charges
    for (unsigned int i = 0; i < receive.charges().size(); ++i) {
	ReceiveCharge& line = receive.charges()[i];
	if (!line.internal) continue;
	if (line.charge_id == INVALID_ID) continue;

	Charge charge;
	cache.findCharge(line.charge_id, charge);

	// Check if should calculate
	if (charge.calculateMethod() != Charge::MANUAL) {
	    fixed amount = 0.0;
	    switch (charge.calculateMethod()) {
	    case Charge::COST:
		amount = item_total * charge.amount() / 100.0;
		break;
	    case Charge::WEIGHT:
		amount = total_weight * charge.amount();
		break;
	    default:
		break;
	    }

	    amount.moneyRound();
	    line.amount = amount;
	}

	// Calculate tax
	line.base = line.amount;
	if (line.tax_id != INVALID_ID) {
	    Tax tax;
	    cache.findTax(line.tax_id, tax);
	    vector<Id> tax_ids;
	    vector<fixed> tax_amts;

	    if (charge.includeTax()) {
		line.base -= calculateTaxOff(cache, tax, line.amount, tax_ids,
					     tax_amts);
		addTaxInc(receive, line.amount, tax_ids, tax_amts);
	    } else {
		calculateTaxOn(cache, tax, line.amount, exempt, tax_ids,
			       tax_amts);
		addTax(receive, line.amount, tax_ids, tax_amts);
	    }
	}

	// Check if should allocate
	if (charge.allocateMethod() != Charge::NONE) {
	    int last = -1;
	    fixed remain = line.base;
	    for (unsigned int j = 0; j < receive.items().size(); ++j) {
		ReceiveItem& iline = receive.items()[j];
		if (iline.item_id == INVALID_ID) continue;

		Item item;
		cache.findItem(iline.item_id, item);

		fixed weight = item.weight(iline.size) * iline.quantity;
		fixed ratio = 0.0;
		switch (charge.allocateMethod()) {
		case Charge::COST:
		    ratio = iline.ext_base / item_total;
		    break;
		case Charge::WEIGHT:
		    ratio = weight / total_weight;
		    break;
		default:
		    break;
		}

		fixed alloc = line.base * ratio;
		alloc.moneyRound();
		iline.int_charges += alloc;
		remain -= alloc;
		last = j;
	    }
	    receive.items()[last].int_charges += remain;
	}
    }

    // Process external charges
    for (unsigned int i = 0; i < receive.charges().size(); ++i) {
	ReceiveCharge& line = receive.charges()[i];
	if (line.internal) continue;
	if (line.charge_id == INVALID_ID) continue;

	Charge charge;
	cache.findCharge(line.charge_id, charge);

	// Check if should calculate
	if (charge.calculateMethod() != Charge::MANUAL) {
	    fixed amount = 0.0;
	    switch (charge.calculateMethod()) {
	    case Charge::COST:
		amount = item_total * charge.amount() / 100.0;
		break;
	    case Charge::WEIGHT:
		amount = total_weight * charge.amount();
		break;
	    default:
		break;
	    }

	    amount.moneyRound();
	    line.amount = amount;
	}

	// Calculate tax
	line.base = line.amount;
	if (line.tax_id != INVALID_ID) {
	    Tax tax;
	    cache.findTax(line.tax_id, tax);
	    vector<Id> tax_ids;
	    vector<fixed> tax_amts;

	    if (charge.includeTax()) {
		line.base -= calculateTaxOff(cache, tax, line.amount, tax_ids,
					     tax_amts);
	    }
	}

	// Check if should allocate
	if (charge.allocateMethod() != Charge::NONE) {
	    int last = -1;
	    fixed remain = line.base;
	    for (unsigned int j = 0; j < receive.items().size(); ++j) {
		ReceiveItem& iline = receive.items()[j];
		if (iline.item_id == INVALID_ID) continue;

		Item item;
		cache.findItem(iline.item_id, item);

		fixed weight = item.weight(iline.size) * iline.quantity;
		fixed ratio = 0.0;
		switch (charge.allocateMethod()) {
		case Charge::COST:
		    ratio = iline.ext_base / item_total;
		    break;
		case Charge::WEIGHT:
		    ratio = weight / total_weight;
		    break;
		default:
		    break;
		}

		fixed alloc = line.base * ratio;
		alloc.moneyRound();
		iline.ext_charges += alloc;
		remain -= alloc;
		last = j;
	    }
	    receive.items()[last].ext_charges += remain;
	}
    }

    // Round taxes to two decimal places
    for (unsigned int i = 0; i < receive.taxes().size(); ++i) {
	TaxLine& line = receive.taxes()[i];
	line.amount.moneyRound();
    }

    // Account postings for items
    for (unsigned int i = 0; i < receive.items().size(); ++i) {
	ReceiveItem& line = receive.items()[i];

	Item item;
	cache.findItem(line.item_id, item);

	fixed inv_cost = line.ext_base + line.int_charges + line.ext_charges;
	if (receive.lineType() == Receive::Account) {
	    addInfo(asset_info, line.account_id, inv_cost);
	} else {
	    addInfo(expense_info, company.depositAccount(), line.ext_deposit);
	    if (item.isInventoried())
		addInfo(asset_info, item.assetAccount(), inv_cost);
	    else
		addInfo(expense_info, item.expenseAccount(), inv_cost);
	}

	line.inv_cost = inv_cost * sign;
	line.quantity = line.quantity * sign;
    }

    // Account postings for taxes
    for (unsigned int i = 0; i < receive.taxes().size(); ++i) {
	const TaxLine& line = receive.taxes()[i];

	Tax tax;
	cache.findTax(line.tax_id, tax);

	addInfo(tax_info, tax.paidAccount(), line.amount + line.inc_amount);
    }

    // Account postings for charges
    for (unsigned int i = 0; i < receive.charges().size(); ++i) {
	const ReceiveCharge& line = receive.charges()[i];

	Charge charge;
	cache.findCharge(line.charge_id, charge);

	if (charge.allocateMethod() == Charge::NONE && line.internal)
	    addInfo(expense_info, charge.accountId(), line.base);
	if (charge.allocateMethod() != Charge::NONE && !line.internal)
	    addInfo(expense_info, charge.accountId(), -line.base);
    }

    // Post to AP
    if (receive.vendorId() != INVALID_ID) {
	AccountLine line(vendor.accountId(), -receive.total());
	receive.accounts().push_back(line);
    }

    // Post the taxes
    for (unsigned int i = 0; i < tax_info.size(); ++i) {
	Id account_id = tax_info[i].first;
	fixed amount = tax_info[i].second * sign;
	if (amount != 0.0)
	    receive.accounts().push_back(AccountLine(account_id, amount));
    }

    // Post the asset adjustments
    for (unsigned int i = 0; i < asset_info.size(); ++i) {
	Id account_id = asset_info[i].first;
	fixed amount = asset_info[i].second * sign;
	if (amount != 0.0)
	    receive.accounts().push_back(AccountLine(account_id, amount));
    }

    // Post the expenses
    for (unsigned int i = 0; i < expense_info.size(); ++i) {
	Id account_id = expense_info[i].first;
	fixed amount = expense_info[i].second * sign;
	if (amount != 0.0)
	    receive.accounts().push_back(AccountLine(account_id, amount));
    }

    // Add card line
    if (receive.vendorId() != INVALID_ID) {
	receive.cards().clear();
	CardLine line(receive.vendorId(), receive.total());
	receive.cards().push_back(line);
    }

    return true;
}
