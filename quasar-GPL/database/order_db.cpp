// $Id: order_db.cpp,v 1.37 2005/03/01 19:59:42 bpepers Exp $
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

#include "order.h"
#include "order_select.h"
#include "card.h"
#include "item.h"
#include "tax.h"

// Create an Order
bool
QuasarDB::create(Order& order)
{
    if (!validate(order)) return false;

    // Auto allocate order number
    if (order.number().stripWhiteSpace() == "#") {
	fixed number = uniqueNumber("porder", "number");
	order.setNumber(number.toString());
    }

    QString cmd = insertCmd("porder", "order_id", "vendor_id,"
			    "vendor_addr,ship_id,ship_addr,ship_via,"
			    "term_id,store_id,comments,number,"
			    "reference_str,order_date");
    Stmt stmt(_connection, cmd);

    insertData(order, stmt);
    stmtSetId(stmt, order.vendorId());
    stmtSetString(stmt, order.vendorAddress());
    stmtSetId(stmt, order.shipId());
    stmtSetString(stmt, order.shipAddress());
    stmtSetString(stmt, order.shipVia());
    stmtSetId(stmt, order.termsId());
    stmtSetId(stmt, order.storeId());
    stmtSetString(stmt, order.comment());
    stmtSetString(stmt, order.number());
    stmtSetString(stmt, order.reference());
    stmtSetDate(stmt, order.date());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(order)) return false;

    commit();
    dataSignal(DataEvent::Insert, order);
    return true;
}

// Delete an Order
bool
QuasarDB::remove(const Order& order)
{
    if (order.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(order)) return false;
    if (!removeData(order, "porder", "order_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, order);
    return true;
}

// Update an Order
bool
QuasarDB::update(const Order& orig, Order& order)
{
    if (orig.id() == INVALID_ID || order.id() == INVALID_ID) return false;
    if (!validate(order)) return false;

    if (order.number().stripWhiteSpace() == "#")
	return error("Can't change number to '#'");

    // Update the porder table
    QString cmd = updateCmd("porder", "order_id", "vendor_id,"
			    "vendor_addr,ship_id,ship_addr,ship_via,"
			    "term_id,store_id,comments,number,"
			    "reference_str,order_date");
    Stmt stmt(_connection, cmd);

    updateData(orig, order, stmt);
    stmtSetId(stmt, order.vendorId());
    stmtSetString(stmt, order.vendorAddress());
    stmtSetId(stmt, order.shipId());
    stmtSetString(stmt, order.shipAddress());
    stmtSetString(stmt, order.shipVia());
    stmtSetId(stmt, order.termsId());
    stmtSetId(stmt, order.storeId());
    stmtSetString(stmt, order.comment());
    stmtSetString(stmt, order.number());
    stmtSetString(stmt, order.reference());
    stmtSetDate(stmt, order.date());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");
    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(order)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup an Order.
bool
QuasarDB::lookup(Id order_id, Order& order)
{
    if (order_id == INVALID_ID) return false;
    OrderSelect conditions;
    vector<Order> orders;

    conditions.id = order_id;
    if (!select(orders, conditions)) return false;
    if (orders.size() != 1) return false;

    order = orders[0];
    return true;
}

// Lookup an Order.  This is just a shortcut to using select.
bool
QuasarDB::lookup(const QString& number, Order& order)
{
    if (number.isEmpty()) return false;
    OrderSelect conditions;
    vector<Order> orders;

    conditions.number = number;
    if (!select(orders, conditions)) return false;
    if (orders.size() != 1) return false;

    order = orders[0];
    return true;
}

// Returns a vector of Orders.
bool
QuasarDB::select(vector<Order>& orders, const OrderSelect& conditions)
{
    orders.clear();

    QString cmd = selectCmd("porder", "order_id", "vendor_id,"
			    "vendor_addr,ship_id,ship_addr,ship_via,"
			    "term_id,store_id,comments,number,"
			    "reference_str,order_date", conditions,
			    "order_id");
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Order order;
	int next = 1;
	selectData(order, stmt, next);
	order.setVendorId(stmtGetId(stmt, next++));
	order.setVendorAddress(stmtGetString(stmt, next++));
	order.setShipId(stmtGetId(stmt, next++));
	order.setShipAddress(stmtGetString(stmt, next++));
	order.setShipVia(stmtGetString(stmt, next++));
	order.setTermsId(stmtGetId(stmt, next++));
	order.setStoreId(stmtGetId(stmt, next++));
	order.setComment(stmtGetString(stmt, next++));
	order.setNumber(stmtGetString(stmt, next++));
	order.setReference(stmtGetString(stmt, next++));
	order.setDate(stmtGetDate(stmt, next++));
	orders.push_back(order);
    }

    QString cmd1 = "select item_id,number,description,size_name,size_qty,"
	"ordered,billed,cost_id,cost_disc,unit_price,for_qty,"
	"for_price,ext_cost,ext_base,ext_deposit,ext_tax,"
	"int_charges,ext_charges,tax_id,item_tax_id,include_tax,"
	"include_deposit from porder_item where order_id = ? "
	"order by seq_num";
    Stmt stmt1(_connection, cmd1);

    QString cmd2 = "select tax_id,taxable,amount,inc_taxable,inc_amount "
	"from porder_tax where order_id = ? order by seq_num";
    Stmt stmt2(_connection, cmd2);

    QString cmd3 = "select charge_id,tax_id,amount,base,internal from "
	"porder_charge where order_id = ? order by seq_num";
    Stmt stmt3(_connection, cmd3);

    for (unsigned int i = 0; i < orders.size(); ++i) {
	Id order_id = orders[i].id();

	stmtSetId(stmt1, order_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    OrderItem line;
	    int next = 1;
	    line.item_id = stmtGetId(stmt1, next++);
	    line.number = stmtGetString(stmt1, next++);
	    line.description = stmtGetString(stmt1, next++);
	    line.size = stmtGetString(stmt1, next++);
	    line.size_qty = stmtGetFixed(stmt1, next++);
	    line.ordered = stmtGetFixed(stmt1, next++);
	    line.billed = stmtGetFixed(stmt1, next++);
	    line.cost_id = stmtGetId(stmt1, next++);
	    line.cost_disc = stmtGetFixed(stmt1, next++);
	    line.cost.setUnitPrice(stmtGetFixed(stmt1, next++));
	    line.cost.setForQty(stmtGetFixed(stmt1, next++));
	    line.cost.setForPrice(stmtGetFixed(stmt1, next++));
	    line.ext_cost = stmtGetFixed(stmt1, next++);
	    line.ext_base = stmtGetFixed(stmt1, next++);
	    line.ext_deposit = stmtGetFixed(stmt1, next++);
	    line.ext_tax = stmtGetFixed(stmt1, next++);
	    line.int_charges = stmtGetFixed(stmt1, next++);
	    line.ext_charges = stmtGetFixed(stmt1, next++);
	    line.tax_id = stmtGetId(stmt1, next++);
	    line.item_tax_id = stmtGetId(stmt1, next++);
	    line.include_tax = stmtGetBool(stmt1, next++);
	    line.include_deposit = stmtGetBool(stmt1, next++);
	    orders[i].items().push_back(line);
	}

	stmtSetId(stmt2, order_id);
	if (!execute(stmt2)) return false;
	while (stmt2.next()) {
	    OrderTax line;
	    line.tax_id = stmtGetId(stmt2, 1);
	    line.taxable = stmtGetFixed(stmt2, 2);
	    line.amount = stmtGetFixed(stmt2, 3);
	    line.inc_taxable = stmtGetFixed(stmt2, 4);
	    line.inc_amount = stmtGetFixed(stmt2, 5);
	    orders[i].taxes().push_back(line);
	}

	stmtSetId(stmt3, order_id);
	if (!execute(stmt3)) return false;
	while (stmt3.next()) {
	    OrderCharge line;
	    line.charge_id = stmtGetId(stmt3, 1);
	    line.tax_id = stmtGetId(stmt3, 2);
	    line.amount = stmtGetFixed(stmt3, 3);
	    line.base = stmtGetFixed(stmt3, 4);
	    line.internal = stmtGetBool(stmt3, 5);
	    orders[i].charges().push_back(line);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Order& order)
{
    if (!validate((DataObject&)order)) return false;

    if (order.number().stripWhiteSpace().isEmpty())
	return error("Blank order number");

    Card card;
    if (!lookup(order.vendorId(), card))
	return error("Vendor doesn't exist");
    if (card.dataType() != DataObject::VENDOR)
	return error("Card is not a vendor");

    if (order.shipId() != INVALID_ID) {
	if (!lookup(order.shipId(), card))
	    return error("Ship to card doesn't exist");
    }

    if (order.date().isNull())
	return error("Blank order date");

    if (order.items().size() < 1)
	return error("Order must have at least one line");

    unsigned int i;
    for (i = 0; i < order.items().size(); ++i) {
	const OrderItem& line = order.items()[i];

	Item item;
	if (!lookup(line.item_id, item))
	    return error("Item doesn't exist");
	if (!item.isPurchased())
	    return error("Item isn't purchased");

	if (line.number.isEmpty())
	    return error("Blank item number");

	if (line.ordered < 0.0)
	    return error("Negative ordered quantity");
	if (line.billed < 0.0)
	    return error("Negative received quantity");

	if (line.tax_id != INVALID_ID) {
	    Tax tax;
	    if (!lookup(line.tax_id, tax))
		return error("Tax doesn't exist");
	}
    }

    for (i = 0; i < order.taxes().size(); ++i) {
	const OrderTax& line = order.taxes()[i];

	Tax tax;
	if (!lookup(line.tax_id, tax))
	    return error("Tax doesn't exist");
    }

    return true;
}

bool
QuasarDB::sqlCreateLines(const Order& order)
{
    QString cmd = insertText("porder_item", "order_id", "seq_num,item_id,"
			     "number,description,size_name,size_qty,"
			     "ordered,billed,cost_id,cost_disc,unit_price,"
			     "for_qty,for_price,ext_cost,ext_base,"
			     "ext_deposit,ext_tax,int_charges,ext_charges,"
			     "tax_id,item_tax_id,include_tax,include_deposit");
    Stmt stmt(_connection, cmd);

    // Item change info
    vector<Id> item_ids;
    vector<QString> sizes;
    vector<fixed> qtys;

    const vector<OrderItem>& items = order.items();
    unsigned int i;
    for (i = 0; i < items.size(); ++i) {
	const OrderItem& line = items[i];

	stmtSetId(stmt, order.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.item_id);
	stmtSetString(stmt, line.number);
	stmtSetString(stmt, line.description);
	stmtSetString(stmt, line.size);
	stmtSetFixed(stmt, line.size_qty);
	stmtSetFixed(stmt, line.ordered);
	stmtSetFixed(stmt, line.billed);
	stmtSetId(stmt, line.cost_id);
	stmtSetFixed(stmt, line.cost_disc);
	stmtSetFixed(stmt, line.cost.unitPrice());
	stmtSetFixed(stmt, line.cost.forQty());
	stmtSetFixed(stmt, line.cost.forPrice());
	stmtSetFixed(stmt, line.ext_cost);
	stmtSetFixed(stmt, line.ext_base);
	stmtSetFixed(stmt, line.ext_deposit);
	stmtSetFixed(stmt, line.ext_tax);
	stmtSetFixed(stmt, line.int_charges);
	stmtSetFixed(stmt, line.ext_charges);
	stmtSetId(stmt, line.tax_id);
	stmtSetId(stmt, line.item_tax_id);
	stmtSetBool(stmt, line.include_tax);
	stmtSetBool(stmt, line.include_deposit);
	if (!execute(stmt)) return false;

	if (!order.isActive()) continue;

	fixed on_order = line.ordered - line.billed;
	if (on_order < 0.0) continue;

	item_ids.push_back(line.item_id);
	sizes.push_back(line.size);
	qtys.push_back(on_order * line.size_qty);
    }

    if (!itemOrder(item_ids, sizes, order.storeId(), order.date(), qtys))
	return false;

    cmd = insertText("porder_tax", "order_id", "seq_num,tax_id,taxable,"
		     "amount,inc_taxable,inc_amount");
    stmt.setCommand(cmd);

    const vector<OrderTax>& taxes = order.taxes();
    for (i = 0; i < taxes.size(); ++i) {
	const OrderTax& line = taxes[i];

	stmtSetId(stmt, order.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.tax_id);
	stmtSetFixed(stmt, line.taxable);
	stmtSetFixed(stmt, line.amount);
	stmtSetFixed(stmt, line.inc_taxable);
	stmtSetFixed(stmt, line.inc_amount);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("porder_charge", "order_id", "seq_num,charge_id,"
		     "tax_id,amount,base,internal");
    stmt.setCommand(cmd);

    const vector<OrderCharge>& charges = order.charges();
    for (i = 0; i < charges.size(); ++i) {
	const OrderCharge& line = charges[i];

	stmtSetId(stmt, order.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.charge_id);
	stmtSetId(stmt, line.tax_id);
	stmtSetFixed(stmt, line.amount);
	stmtSetFixed(stmt, line.base);
	stmtSetBool(stmt, line.internal);
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const Order& order)
{
    // Undo the on order
    QString cmd = "select item_id,size_name,size_qty,ordered,billed "
	"from porder_item where order_id = ?";
    Stmt stmt(_connection, cmd);
    stmtSetId(stmt, order.id());

    // Item change info
    vector<Id> item_ids;
    vector<QString> sizes;
    vector<fixed> qtys;

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Id item_id = stmtGetId(stmt, 1);
	QString size = stmtGetString(stmt, 2);
	fixed size_qty = stmtGetFixed(stmt, 3);
	fixed ordered = stmtGetFixed(stmt, 4);
	fixed billed = stmtGetFixed(stmt, 5);

	if (!order.isActive()) continue;

	fixed on_order = ordered - billed;
	if (on_order < 0.0) continue;

	item_ids.push_back(item_id);
	sizes.push_back(size);
	qtys.push_back(-on_order * size_qty);
    }

    if (!itemOrder(item_ids, sizes, order.storeId(), order.date(), qtys))
	return false;

    if (!remove(order, "porder_item", "order_id")) return false;
    if (!remove(order, "porder_tax", "order_id")) return false;
    if (!remove(order, "porder_charge", "order_id")) return false;

    return true;
}
