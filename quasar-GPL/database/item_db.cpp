// $Id: item_db.cpp,v 1.58 2005/03/13 22:21:11 bpepers Exp $
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

#include "item.h"
#include "item_select.h"
#include "subdept.h"
#include "account.h"
#include "tax.h"
#include "vendor.h"

// Create an Item
bool
QuasarDB::create(Item& item)
{
    if (!validate(item)) return false;

    QString cmd = insertCmd("item", "item_id", "description,dept_id,"
			    "subdept_id,purchased,sold,inventoried,"
			    "sell_size,purchase_size,expense_acct,"
			    "income_acct,asset_acct,sell_tax,purchase_tax,"
			    "qty_enforced,weighed,deposit,cost_inc_tax,"
			    "cost_inc_dep,price_inc_tax,price_inc_dep,"
			    "build_qty,discountable,qty_decimals,"
			    "qty_price,discontinued,open_dept,cost_disc");
    Stmt stmt(_connection, cmd);

    insertData(item, stmt);
    stmtSetString(stmt, item.description());
    stmtSetId(stmt, item.deptId());
    stmtSetId(stmt, item.subdeptId());
    stmtSetBool(stmt, item.isPurchased());
    stmtSetBool(stmt, item.isSold());
    stmtSetBool(stmt, item.isInventoried());
    stmtSetString(stmt, item.sellSize());
    stmtSetString(stmt, item.purchaseSize());
    stmtSetId(stmt, item.expenseAccount());
    stmtSetId(stmt, item.incomeAccount());
    stmtSetId(stmt, item.assetAccount());
    stmtSetId(stmt, item.sellTax());
    stmtSetId(stmt, item.purchaseTax());
    stmtSetBool(stmt, item.isQuantityEnforced());
    stmtSetBool(stmt, item.isWeighed());
    stmtSetFixed(stmt, item.deposit());
    stmtSetBool(stmt, item.costIncludesTax());
    stmtSetBool(stmt, item.costIncludesDeposit());
    stmtSetBool(stmt, item.priceIncludesTax());
    stmtSetBool(stmt, item.priceIncludesDeposit());
    stmtSetFixed(stmt, item.buildQty());
    stmtSetBool(stmt, item.isDiscountable());
    stmtSetBool(stmt, item.isQuantityDecimals());
    stmtSetBool(stmt, item.isQtyFromPrice());
    stmtSetBool(stmt, item.isDiscontinued());
    stmtSetBool(stmt, item.isOpenDept());
    stmtSetFixed(stmt, item.costDiscount());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(item)) return false;

    commit();
    dataSignal(DataEvent::Insert, item);
    return true;
}

// Delete an Item
bool
QuasarDB::remove(const Item& item)
{
    if (item.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(item)) return false;
    if (!removeData(item, "item", "item_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, item);
    return true;
}

// Update an Item.  Changes to the sold/inventoried/purchased flags are
// not allowed when the item has already been used.
bool
QuasarDB::update(const Item& orig, Item& item)
{
    if (orig.id() == INVALID_ID || item.id() == INVALID_ID) return false;
    if (!validate(item)) return false;

    // If flags change, check if used
    if (orig.isSold() != item.isSold() || orig.isPurchased() !=
	    item.isPurchased() || orig.isInventoried() !=
	    item.isInventoried()) {
	int useCount = 0;

	QString cmd = "select count(*) from gltx_item where item_id=?";
	Stmt stmt(_connection, cmd);
	stmtSetId(stmt, orig.id());
	if (!execute(stmt)) return false;
	if (stmt.next())
	    useCount = stmtGetInt(stmt, 1);

	// If used, can only turn new flags on
	if (useCount != 0) {
	    if (orig.isSold() && !item.isSold())
		return error("Can't remove sold flag");
	    if (orig.isPurchased() && !item.isPurchased())
		return error("Can't remove purchased flag");
	    if (orig.isInventoried() && !item.isInventoried())
		return error("Can't remove inventoried flag");
	}
    }

    // Validate the changing of the active flag
    if (orig.isActive() && !item.isActive() && orig.isInventoried()) {
	fixed on_hand, total_cost, on_order;
	itemGeneral(orig.id(), "", INVALID_ID, QDate(), on_hand,
		    total_cost, on_order);
	if (on_hand != 0.0)
	    return error("Can't set inactive when item has on hand");
	if (total_cost != 0.0)
	    return error("Can't set inactive when item has a cost");
	if (on_order != 0.0)
	    return error("Can't set inactive when item is on order");
    }

    // Validate the changing of the stocked flag
    if (item.stores() != orig.stores()) {
	for (unsigned int i = 0; i < item.stores().size(); ++i) {
	    Id store_id = item.stores()[i].store_id;
	    bool stocked = item.stores()[i].stocked;
	    if (stocked == orig.stocked(store_id)) continue;

	    // If changing *to* stocked, don't check quantities
	    if (stocked) continue;

	    fixed on_hand, total_cost, on_order;
	    itemGeneral(orig.id(), "", store_id, QDate(), on_hand,
			total_cost, on_order);
	    if (on_hand != 0.0)
		return error("Can't change stocked when item has on hand");
	    if (on_hand != 0.0 || total_cost != 0.0)
		return error("Can't change stocked when item has a cost");
	    if (on_order != 0.0)
		return error("Can't change stocked when item is on order");
	}
    }

    // Update the item table
    QString cmd = updateCmd("item", "item_id", "description,dept_id,"
			    "subdept_id,purchased,sold,inventoried,"
			    "sell_size,purchase_size,expense_acct,"
			    "income_acct,asset_acct,sell_tax,purchase_tax,"
			    "qty_enforced,weighed,deposit,cost_inc_tax,"
			    "cost_inc_dep,price_inc_tax,price_inc_dep,"
			    "build_qty,discountable,qty_decimals,"
			    "qty_price,discontinued,open_dept,cost_disc");
    Stmt stmt(_connection, cmd);

    updateData(orig, item, stmt);
    stmtSetString(stmt, item.description());
    stmtSetId(stmt, item.deptId());
    stmtSetId(stmt, item.subdeptId());
    stmtSetBool(stmt, item.isPurchased());
    stmtSetBool(stmt, item.isSold());
    stmtSetBool(stmt, item.isInventoried());
    stmtSetString(stmt, item.sellSize());
    stmtSetString(stmt, item.purchaseSize());
    stmtSetId(stmt, item.expenseAccount());
    stmtSetId(stmt, item.incomeAccount());
    stmtSetId(stmt, item.assetAccount());
    stmtSetId(stmt, item.sellTax());
    stmtSetId(stmt, item.purchaseTax());
    stmtSetBool(stmt, item.isQuantityEnforced());
    stmtSetBool(stmt, item.isWeighed());
    stmtSetFixed(stmt, item.deposit());
    stmtSetBool(stmt, item.costIncludesTax());
    stmtSetBool(stmt, item.costIncludesDeposit());
    stmtSetBool(stmt, item.priceIncludesTax());
    stmtSetBool(stmt, item.priceIncludesDeposit());
    stmtSetFixed(stmt, item.buildQty());
    stmtSetBool(stmt, item.isDiscountable());
    stmtSetBool(stmt, item.isQuantityDecimals());
    stmtSetBool(stmt, item.isQtyFromPrice());
    stmtSetBool(stmt, item.isDiscontinued());
    stmtSetBool(stmt, item.isOpenDept());
    stmtSetFixed(stmt, item.costDiscount());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");
    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(item)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup an Item.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id item_id, Item& item)
{
    if (item_id == INVALID_ID) return false;
    ItemSelect conditions;
    vector<Item> items;

    conditions.id = item_id;
    if (!select(items, conditions)) return false;
    if (items.size() != 1) return false;

    item = items[0];
    return true;
}

// Returns a count of Items based on the conditions.
bool
QuasarDB::count(int& count, const ItemSelect& conditions)
{
    count = 0;
    QString cmd = "select count(*) from item " + conditions.where();
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	count = stmtGetInt(stmt, 1);
    }

    commit();
    return true;
}

// Returns a vector of Items.  Returns Items sorted by description.
bool
QuasarDB::select(vector<Item>& items, const ItemSelect& conditions)
{
    items.clear();

    QString cmd = selectCmd("item", "item_id", "description,dept_id,"
			    "subdept_id,purchased,sold,inventoried,"
			    "sell_size,purchase_size,expense_acct,"
			    "income_acct,asset_acct,sell_tax,purchase_tax,"
			    "qty_enforced,weighed,deposit,cost_inc_tax,"
			    "cost_inc_dep,price_inc_tax,price_inc_dep,"
			    "build_qty,discountable,qty_decimals,"
			    "qty_price,discontinued,open_dept,cost_disc",
			    conditions);
    Stmt stmt(_connection, cmd);

    QMap<Id,int> itemMap;

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Item item;
	int next = 1;
	selectData(item, stmt, next);
	item.setDescription(stmtGetString(stmt, next++));
	item.setDeptId(stmtGetId(stmt, next++));
	item.setSubdeptId(stmtGetId(stmt, next++));
	item.setPurchased(stmtGetBool(stmt, next++));
	item.setSold(stmtGetBool(stmt, next++));
	item.setInventoried(stmtGetBool(stmt, next++));
	item.setSellSize(stmtGetString(stmt, next++));
	item.setPurchaseSize(stmtGetString(stmt, next++));
	item.setExpenseAccount(stmtGetId(stmt, next++));
	item.setIncomeAccount(stmtGetId(stmt, next++));
	item.setAssetAccount(stmtGetId(stmt, next++));
	item.setSellTax(stmtGetId(stmt, next++));
	item.setPurchaseTax(stmtGetId(stmt, next++));
	item.setQuantityEnforced(stmtGetBool(stmt, next++));
	item.setWeighed(stmtGetBool(stmt, next++));
	item.setDeposit(stmtGetFixed(stmt, next++));
	item.setCostIncludesTax(stmtGetBool(stmt, next++));
	item.setCostIncludesDeposit(stmtGetBool(stmt, next++));
	item.setPriceIncludesTax(stmtGetBool(stmt, next++));
	item.setPriceIncludesDeposit(stmtGetBool(stmt, next++));
	item.setBuildQty(stmtGetFixed(stmt, next++));
	item.setDiscountable(stmtGetBool(stmt, next++));
	item.setQuantityDecimals(stmtGetBool(stmt, next++));
	item.setQtyFromPrice(stmtGetBool(stmt, next++));
	item.setDiscontinued(stmtGetBool(stmt, next++));
	item.setOpenDept(stmtGetBool(stmt, next++));
	item.setCostDiscount(stmtGetFixed(stmt, next++));
	items.push_back(item);
	itemMap[item.id()] = items.size() - 1;
    }

    if (items.size() > 0) {
	QString subSelect;
	if (items.size() > 50) {
	    subSelect = "select item_id from item " + conditions.where();
	} else {
	    for (unsigned int i = 0; i < items.size(); ++i) {
		if (i > 0) subSelect += ",";
		subSelect += "'" + items[i].id().toString() + "'";
	    }
	}

	cmd = "select item_id,description,card_id,card_group from "
	    "item_desc where item_id in (" + subSelect + ") order by "
	    "item_id,seq_num";
	stmt.setCommand(cmd);
	if (!execute(stmt)) return false;
	while (stmt.next()) {
	    Id item_id = stmtGetId(stmt, 1);
	    int pos = itemMap[item_id];
	    if (pos == -1) continue;

	    ItemDesc line;
	    line.description = stmtGetString(stmt, 2);
	    line.card_id = stmtGetId(stmt, 3);
	    line.card_group = stmtGetId(stmt, 4);
	    items[pos].descriptions().push_back(line);
	}

	cmd = "select item_id,name,qty,weight,multiple from "
	    "item_size where item_id in (" + subSelect + ") order by "
	    "item_id,seq_num";
	stmt.setCommand(cmd);
	if (!execute(stmt)) return false;
	while (stmt.next()) {
	    Id item_id = stmtGetId(stmt, 1);
	    int pos = itemMap[item_id];
	    if (pos == -1) continue;

	    ItemSize line;
	    line.name = stmtGetString(stmt, 2);
	    line.qty = stmtGetFixed(stmt, 3);
	    line.weight = stmtGetFixed(stmt, 4);
	    line.multiple = stmtGetFixed(stmt, 5);
	    items[pos].sizes().push_back(line);
	}

	cmd = "select item_id,number,size_name from item_plu where "
	    "item_id in (" + subSelect + ") order by item_id,seq_num";
	stmt.setCommand(cmd);
	if (!execute(stmt)) return false;
	while (stmt.next()) {
	    Id item_id = stmtGetId(stmt, 1);
	    int pos = itemMap[item_id];
	    if (pos == -1) continue;

	    ItemPlu line;
	    line.number = stmtGetString(stmt, 2);
	    line.size = stmtGetString(stmt, 3);
	    items[pos].numbers().push_back(line);
	}

	cmd = "select item_id,group_id from item_group where "
	    "item_id in (" + subSelect + ") order by item_id,seq_num";
	stmt.setCommand(cmd);
	if (!execute(stmt)) return false;
	while (stmt.next()) {
	    Id item_id = stmtGetId(stmt, 1);
	    int pos = itemMap[item_id];
	    if (pos == -1) continue;

	    Id group_id = stmtGetId(stmt, 2);
	    items[pos].groups().push_back(group_id);
	}

	cmd = "select item_id,vendor_id,order_number,size_name from "
	    "item_vendor where item_id in (" + subSelect + ") order by "
	    "item_id,seq_num";
	stmt.setCommand(cmd);
	if (!execute(stmt)) return false;
	while (stmt.next()) {
	    Id item_id = stmtGetId(stmt, 1);
	    int pos = itemMap[item_id];
	    if (pos == -1) continue;

	    ItemVendor line;
	    line.vendor_id = stmtGetId(stmt, 2);
	    line.number = stmtGetString(stmt, 3);
	    line.size = stmtGetString(stmt, 4);
	    items[pos].vendors().push_back(line);
	}

	cmd = "select item_id,component_id,number,size_name,quantity from "
	    "item_build where item_id in (" + subSelect + ") order by "
	    "item_id,seq_num";
	stmt.setCommand(cmd);
	if (!execute(stmt)) return false;
	while (stmt.next()) {
	    Id item_id = stmtGetId(stmt, 1);
	    int pos = itemMap[item_id];
	    if (pos == -1) continue;

	    Component line;
	    line.item_id = stmtGetId(stmt, 2);
	    line.number = stmtGetString(stmt, 3);
	    line.size = stmtGetString(stmt, 4);
	    line.qty = stmtGetFixed(stmt, 5);
	    items[pos].items().push_back(line);
	}

	cmd = "select item_id,store_id,stocked,location_id,min_qty,"
	    "max_qty from item_store where item_id in (" + subSelect +
	    ") order by item_id,seq_num";
	stmt.setCommand(cmd);
	if (!execute(stmt)) return false;
	while (stmt.next()) {
	    Id item_id = stmtGetId(stmt, 1);
	    int pos = itemMap[item_id];
	    if (pos == -1) continue;

	    ItemStore line;
	    line.store_id = stmtGetId(stmt, 2);
	    line.stocked = stmtGetBool(stmt, 3);
	    line.location_id = stmtGetId(stmt, 4);
	    line.min_qty = stmtGetFixed(stmt, 5);
	    line.max_qty = stmtGetFixed(stmt, 6);
	    items[pos].stores().push_back(line);
	}

	cmd = "select item_id,store_id,size_name,unit_cost,for_cost_qty,"
	    "for_cost,unit_price,for_price_qty,for_price,target_gm,"
	    "allowed_var from item_cost where item_id in (" + subSelect +
	    ") order by item_id,seq_num";
	stmt.setCommand(cmd);
	if (!execute(stmt)) return false;
	while (stmt.next()) {
	    Id item_id = stmtGetId(stmt, 1);
	    int pos = itemMap[item_id];
	    if (pos == -1) continue;

	    ItemCost line;
	    line.store_id = stmtGetId(stmt, 2);
	    line.size = stmtGetString(stmt, 3);
	    line.cost.setUnitPrice(stmtGetFixed(stmt, 4));
	    line.cost.setForQty(stmtGetFixed(stmt, 5));
	    line.cost.setForPrice(stmtGetFixed(stmt, 6));
	    line.price.setUnitPrice(stmtGetFixed(stmt, 7));
	    line.price.setForQty(stmtGetFixed(stmt, 8));
	    line.price.setForPrice(stmtGetFixed(stmt, 9));
	    line.target_gm = stmtGetFixed(stmt, 10);
	    line.allowed_var = stmtGetFixed(stmt, 11);
	    items[pos].costs().push_back(line);
	}

	cmd = "select item_id,extra_id,data_value from item_extra "
	    "where item_id in (" + subSelect + ") order by "
	    "item_id,seq_num";
	stmt.setCommand(cmd);
	if (!execute(stmt)) return false;
	while (stmt.next()) {
	    Id item_id = stmtGetId(stmt, 1);
	    int pos = itemMap[item_id];
	    if (pos == -1) continue;

	    Id extra_id = stmtGetId(stmt, 2);
	    QString value = stmtGetString(stmt, 3);
	    items[pos].setValue(extra_id, value);
	}
    }

    commit();
    return true;
}

// Returns a vector of Items.  Returns Items sorted by description.
bool
QuasarDB::select(vector<Id>& item_ids, const ItemSelect& conditions)
{
    item_ids.clear();

    QString cmd = "select item_id from item " + conditions.where();
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Id item_id = stmtGetId(stmt, 1);
	item_ids.push_back(item_id);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Item& item)
{
    if (!validate((DataObject&)item)) return false;

    if (item.numbers().size() == 0)
	return error("No item numbers");

    // Must have at least one size
    if (item.sizes().size() == 0)
	return error("Missing size information");

    // No blank or duplicate sizes
    unsigned int i;
    for (i = 0; i < item.sizes().size(); ++i) {
	QString size = item.sizes()[i].name;
	if (size.isEmpty())
	    return error("Blank size");
	if (item.sizes()[i].qty <= 0)
	    return error("Invalid size qty");

	for (unsigned int j = 0; j < item.sizes().size(); ++j) {
	    if (j == i) continue;
	    if (item.sizes()[j].name.upper() == size.upper())
		return error("Duplicate size");
	}
    }

    // Check item numbers and number sizes
    for (i = 0; i < item.numbers().size(); ++i) {
	QString number = item.numbers()[i].number;
	QString size = item.numbers()[i].size;

	if (number.isEmpty())
	    return error("Blank item number");

	if (!size.isEmpty()) {
	    bool found = false;
	    for (unsigned int j = 0; j < item.sizes().size(); ++j) {
		if (item.sizes()[j].name == size) {
		    found = true;
		    break;
		}
	    }
	    if (!found)
		return error("Invalid size in item " + number);
	}
    }

    // Check vendors and order numbers
    for (i = 0; i < item.vendors().size(); ++i) {
	Id vendor_id = item.vendors()[i].vendor_id;
	QString number = item.vendors()[i].number;
	QString size = item.vendors()[i].size;

	if (!size.isEmpty()) {
	    bool found = false;
	    for (unsigned int j = 0; j < item.sizes().size(); ++j) {
		if (item.sizes()[j].name == size) {
		    found = true;
		    break;
		}
	    }
	    if (!found)
		return error("Invalid size on order number " + number);
	}

	Vendor vendor;
	if (!lookup(vendor_id, vendor))
	    return error("Vendor doesn't exist");
    }

    // Check store info
    for (i = 0; i < item.stores().size(); ++i) {
	fixed min_qty = item.stores()[i].min_qty;
	fixed max_qty = item.stores()[i].max_qty;

	// Min/max can't be negative or out of wack
	if (min_qty < 0.0)
	    return error("Negative minimum qty");
	if (max_qty < 0.0)
	    return error("Negative maximum qty");
	if (min_qty > max_qty)
	    return error("Minimum qty is larger than maximum");
    }

    if (item.description().stripWhiteSpace().isEmpty())
	return error("Blank item description");

    if (item.subdeptId() != INVALID_ID) {
	Subdept subdept;
	if (!lookup(item.subdeptId(), subdept))
	    return error("Subdepartment doesn't exist");
	if (item.deptId() != subdept.deptId())
	    return error("Department doesn't match subdepartment");
    }

    bool purchased = item.isPurchased();
    bool sold = item.isSold();
    bool inventoried = item.isInventoried();

    QString expenseName = "Expense";
    if (inventoried && sold) expenseName = "COGS";

    if ((inventoried && sold) || (purchased && !inventoried)) {
	Account account;
	if (!lookup(item.expenseAccount(), account))
	    return error(expenseName + " account doesn't exist");
	if (account.isHeader())
	    return error(expenseName + " account is a header");
    } else {
	if (item.expenseAccount() != INVALID_ID)
	    return error("Expense account can't be set");
    }

    if (sold) {
	Account account;
	if (!lookup(item.incomeAccount(), account))
	    return error("Income account doesn't exist");
	if (account.isHeader())
	    return error("Income account is a header");
    } else {
	if (item.incomeAccount() != INVALID_ID)
	    return error("Income account can't be set");
    }

    if (inventoried) {
	Account account;
	if (!lookup(item.assetAccount(), account))
	    return error("Asset account doesn't exist");
	if (account.isHeader())
	    return error("Asset account is a header");
    } else {
	if (item.assetAccount() != INVALID_ID)
	    return error("Asset account can't be set");
    }

    if (sold && item.sellTax() != INVALID_ID) {
	Tax tax;
	if (!lookup(item.sellTax(), tax))
	    return error("Selling tax doesn't exist");
    } else if (item.sellTax() != INVALID_ID) {
	return error("Selling tax can't be set");
    }

    if (purchased && item.purchaseTax() != INVALID_ID) {
	Tax tax;
	if (!lookup(item.purchaseTax(), tax))
	    return error("Purchase tax doesn't exist");
    } else if (item.purchaseTax() != INVALID_ID) {
	return error("Purchase tax can't be set");
    }

    if (sold && item.sellSize().isEmpty())
	return error("Sell size is required");
    if (purchased && item.purchaseSize().isEmpty())
	return error("Purchase size is required");

    bool foundSell = !sold;
    bool foundPurchase = !purchased;
    for (i = 0; i < item.sizes().size(); ++i) {
	if (item.sizes()[i].name == item.sellSize())
	    foundSell = true;
	if (item.sizes()[i].name == item.purchaseSize())
	    foundPurchase = true;
    }
    if (!foundSell)
	return error("Unknown sell size");
    if (!foundPurchase)
	return error("Unknown purchase size");

    // Deposit must be positive
    if (item.deposit() < 0.0)
	return error("Negative deposit");

    return true;
}

bool
QuasarDB::sqlCreateLines(const Item& item)
{
    // List of numbers for this item
    vector<QString> numbers;

    QString cmd = insertText("item_size", "item_id", "seq_num,name,qty,"
			     "weight,multiple");
    Stmt stmt(_connection, cmd);
    unsigned int i;
    for (i = 0; i < item.sizes().size(); ++i) {
	const ItemSize& line = item.sizes()[i];

	stmtSetId(stmt, item.id());
	stmtSetInt(stmt, i);
	stmtSetString(stmt, line.name);
	stmtSetFixed(stmt, line.qty);
	stmtSetFixed(stmt, line.weight);
	stmtSetFixed(stmt, line.multiple);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("item_plu", "item_id", "seq_num,number,size_name");
    stmt.setCommand(cmd);
    for (i = 0; i < item.numbers().size(); ++i) {
	const ItemPlu& line = item.numbers()[i];

	stmtSetId(stmt, item.id());
	stmtSetInt(stmt, i);
	stmtSetString(stmt, line.number);
	stmtSetString(stmt, line.size);
	if (!execute(stmt)) return false;

	numbers.push_back(line.number);
    }

    cmd = insertText("item_group", "item_id", "seq_num,group_id");
    stmt.setCommand(cmd);
    for (i = 0; i < item.groups().size(); ++i) {
	stmtSetId(stmt, item.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, item.groups()[i]);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("item_vendor", "item_id", "seq_num,vendor_id,"
		     "order_number,size_name");
    stmt.setCommand(cmd);
    for (i = 0; i < item.vendors().size(); ++i) {
	const ItemVendor& line = item.vendors()[i];

	stmtSetId(stmt, item.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.vendor_id);
	stmtSetString(stmt, line.number);
	stmtSetString(stmt, line.size);
	if (!execute(stmt)) return false;

	if (!line.number.isEmpty()) {
	    bool found = false;
	    for (unsigned int i = 0; i < numbers.size(); ++i) {
		if (numbers[i] == line.number) {
		    found = true;
		    break;
		}
	    }
	    if (!found)
		numbers.push_back(line.number);
	}
    }

    cmd = insertText("item_store", "item_id", "seq_num,store_id,stocked,"
		     "location_id,min_qty,max_qty");
    stmt.setCommand(cmd);
    for (i = 0; i < item.stores().size(); ++i) {
	const ItemStore& line = item.stores()[i];

	stmtSetId(stmt, item.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.store_id);
	stmtSetBool(stmt, line.stocked);
	stmtSetId(stmt, line.location_id);
	stmtSetFixed(stmt, line.min_qty);
	stmtSetFixed(stmt, line.max_qty);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("item_cost", "item_id", "seq_num,store_id,size_name,"
		     "unit_cost,for_cost_qty,for_cost,unit_price,"
		     "for_price_qty,for_price,target_gm,allowed_var");
    stmt.setCommand(cmd);
    for (i = 0; i < item.costs().size(); ++i) {
	const ItemCost& line = item.costs()[i];

	stmtSetId(stmt, item.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.store_id);
	stmtSetString(stmt, line.size);
	stmtSetFixed(stmt, line.cost.unitPrice());
	stmtSetFixed(stmt, line.cost.forQty());
	stmtSetFixed(stmt, line.cost.forPrice());
	stmtSetFixed(stmt, line.price.unitPrice());
	stmtSetFixed(stmt, line.price.forQty());
	stmtSetFixed(stmt, line.price.forPrice());
	stmtSetFixed(stmt, line.target_gm);
	stmtSetFixed(stmt, line.allowed_var);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("item_build", "item_id", "seq_num,component_id,number,"
		     "size_name,quantity");
    stmt.setCommand(cmd);
    for (i = 0; i < item.items().size(); ++i) {
	const Component& line = item.items()[i];

	stmtSetId(stmt, item.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.item_id);
	stmtSetString(stmt, line.number);
	stmtSetString(stmt, line.size);
	stmtSetFixed(stmt, line.qty);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("item_extra", "item_id", "seq_num,extra_id,data_value");
    stmt.setCommand(cmd);
    for (i = 0; i < item.extra().size(); ++i) {
	const DataPair& line = item.extra()[i];

	stmtSetId(stmt, item.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.first);
	stmtSetString(stmt, line.second);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("item_num", "number", "item_id");
    stmt.setCommand(cmd);
    for (i = 0; i < numbers.size(); ++i) {
	stmtSetString(stmt, numbers[i]);
	stmtSetId(stmt, item.id());
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const Item& item)
{
    if (!remove(item, "item_size", "item_id")) return false;
    if (!remove(item, "item_plu", "item_id")) return false;
    if (!remove(item, "item_group", "item_id")) return false;
    if (!remove(item, "item_vendor", "item_id")) return false;
    if (!remove(item, "item_store", "item_id")) return false;
    if (!remove(item, "item_cost", "item_id")) return false;
    if (!remove(item, "item_build", "item_id")) return false;
    if (!remove(item, "item_extra", "item_id")) return false;
    if (!remove(item, "item_num", "item_id")) return false;
    return true;
}
