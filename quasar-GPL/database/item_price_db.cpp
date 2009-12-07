// $Id: item_price_db.cpp,v 1.16 2005/03/01 19:59:42 bpepers Exp $
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

#include "item_price.h"
#include "item_price_select.h"

// Create an ItemPrice
bool
QuasarDB::create(ItemPrice& price)
{
    if (!validate(price)) return false;

    QString cmd = insertCmd("item_price", "price_id", "is_cost,item_id,"
			    "number,item_group,dept_id,subdept_id,"
			    "size_name,card_id,card_group,store_id,"
			    "promotion,discountable,start_date,stop_date,"
			    "qty_limit,min_qty,day_of_week,method,"
			    "unit_price,for_qty,for_price,percent,dollar");
    Stmt stmt(_connection, cmd);

    insertData(price, stmt);
    stmtSetBool(stmt, price.isCost());
    stmtSetId(stmt, price.itemId());
    stmtSetString(stmt, price.number());
    stmtSetId(stmt, price.itemGroup());
    stmtSetId(stmt, price.deptId());
    stmtSetId(stmt, price.subdeptId());
    stmtSetString(stmt, price.size());
    stmtSetId(stmt, price.cardId());
    stmtSetId(stmt, price.cardGroup());
    stmtSetId(stmt, price.storeId());
    stmtSetBool(stmt, price.isPromotion());
    stmtSetBool(stmt, price.isDiscountable());
    stmtSetDate(stmt, price.startDate());
    stmtSetDate(stmt, price.stopDate());
    stmtSetFixed(stmt, price.qtyLimit());
    stmtSetFixed(stmt, price.minimumQty());
    stmtSetString(stmt, price.dayOfWeekString());
    stmtSetInt(stmt, price.method());
    stmtSetFixed(stmt, price.price().unitPrice());
    stmtSetFixed(stmt, price.price().forQty());
    stmtSetFixed(stmt, price.price().forPrice());
    stmtSetFixed(stmt, price.percentDiscount());
    stmtSetFixed(stmt, price.dollarDiscount());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, price);
    return true;
}

// Delete an ItemPrice
bool
QuasarDB::remove(const ItemPrice& price)
{
    if (price.id() == INVALID_ID) return false;
    if (!removeData(price, "item_price", "price_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, price);
    return true;
}

// Update an ItemPrice
bool
QuasarDB::update(const ItemPrice& orig, ItemPrice& price)
{
    if (orig.id() == INVALID_ID || price.id() == INVALID_ID) return false;
    if (!validate(price)) return false;

    QString cmd = updateCmd("item_price", "price_id", "is_cost,item_id,"
			    "number,item_group,dept_id,subdept_id,"
			    "size_name,card_id,card_group,store_id,"
			    "promotion,discountable,start_date,stop_date,"
			    "qty_limit,min_qty,day_of_week,method,"
			    "unit_price,for_qty,for_price,percent,dollar");
    Stmt stmt(_connection, cmd);

    updateData(orig, price, stmt);
    stmtSetBool(stmt, price.isCost());
    stmtSetId(stmt, price.itemId());
    stmtSetString(stmt, price.number());
    stmtSetId(stmt, price.itemGroup());
    stmtSetId(stmt, price.deptId());
    stmtSetId(stmt, price.subdeptId());
    stmtSetString(stmt, price.size());
    stmtSetId(stmt, price.cardId());
    stmtSetId(stmt, price.cardGroup());
    stmtSetId(stmt, price.storeId());
    stmtSetBool(stmt, price.isPromotion());
    stmtSetBool(stmt, price.isDiscountable());
    stmtSetDate(stmt, price.startDate());
    stmtSetDate(stmt, price.stopDate());
    stmtSetFixed(stmt, price.qtyLimit());
    stmtSetFixed(stmt, price.minimumQty());
    stmtSetString(stmt, price.dayOfWeekString());
    stmtSetInt(stmt, price.method());
    stmtSetFixed(stmt, price.price().unitPrice());
    stmtSetFixed(stmt, price.price().forQty());
    stmtSetFixed(stmt, price.price().forPrice());
    stmtSetFixed(stmt, price.percentDiscount());
    stmtSetFixed(stmt, price.dollarDiscount());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup an ItemPrice.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id price_id, ItemPrice& price)
{
    if (price_id == INVALID_ID) return false;
    ItemPriceSelect conditions;
    vector<ItemPrice> prices;

    conditions.id = price_id;
    if (!select(prices, conditions)) return false;
    if (prices.size() != 1) return false;

    price = prices[0];
    return true;
}

// Returns a vector of ItemPrices.
bool
QuasarDB::select(vector<ItemPrice>& prices, const ItemPriceSelect& conditions)
{
    prices.clear();

    QString cmd = selectCmd("item_price", "price_id", "is_cost,item_id,"
			    "number,item_group,dept_id,subdept_id,"
			    "size_name,card_id,card_group,store_id,"
			    "promotion,discountable,start_date,stop_date,"
			    "qty_limit,min_qty,day_of_week,method,"
			    "unit_price,for_qty,for_price,percent,dollar",
			    conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	ItemPrice price;
	int next = 1;
	selectData(price, stmt, next);
	price.setIsCost(stmtGetBool(stmt, next++));
	price.setItemId(stmtGetId(stmt, next++));
	price.setNumber(stmtGetString(stmt, next++));
	price.setItemGroup(stmtGetId(stmt, next++));
	price.setDeptId(stmtGetId(stmt, next++));
	price.setSubdeptId(stmtGetId(stmt, next++));
	price.setSize(stmtGetString(stmt, next++));
	price.setCardId(stmtGetId(stmt, next++));
	price.setCardGroup(stmtGetId(stmt, next++));
	price.setStoreId(stmtGetId(stmt, next++));
	price.setPromotion(stmtGetBool(stmt, next++));
	price.setDiscountable(stmtGetBool(stmt, next++));
	price.setStartDate(stmtGetDate(stmt, next++));
	price.setStopDate(stmtGetDate(stmt, next++));
	price.setQtyLimit(stmtGetFixed(stmt, next++));
	price.setMinimumQty(stmtGetFixed(stmt, next++));
	price.setDayOfWeek(stmtGetString(stmt, next++));
	price.setMethod(stmtGetInt(stmt, next++));
	Price single;
	single.setUnitPrice(stmtGetFixed(stmt, next++));
	single.setForQty(stmtGetFixed(stmt, next++));
	single.setForPrice(stmtGetFixed(stmt, next++));
	price.setPrice(single);
	price.setPercentDiscount(stmtGetFixed(stmt, next++));
	price.setDollarDiscount(stmtGetFixed(stmt, next++));
	prices.push_back(price);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const ItemPrice& price)
{
    if (!validate((DataObject&)price)) return false;

    // Only one of card or group can be set
    int cardCnt = 0;
    if (price.cardId() != INVALID_ID) ++cardCnt;
    if (price.cardGroup() != INVALID_ID) ++cardCnt;
    if (cardCnt > 1)
	return error("Too many card selection criteria");

    // Only one of item, group, dept, or subdept can be set
    int itemCnt = 0;
    if (price.itemId() != INVALID_ID) ++itemCnt;
    if (price.itemGroup() != INVALID_ID) ++itemCnt;
    if (price.deptId() != INVALID_ID) ++itemCnt;
    if (price.subdeptId() != INVALID_ID) ++itemCnt;
    if (itemCnt > 1)
	return error("Too many item selection criteria");

    // Item and card selection criteria can't both be blank
    if (itemCnt == 0 && cardCnt == 0)
	return error("No selection criteria picked");

    if (price.size().stripWhiteSpace().isEmpty()) {
	switch (price.method()) {
	case ItemPrice::COST_PLUS:
	case ItemPrice::PERCENT_OFF:
	    break;
	default:
	    return error("Blank size");
	}
    }

    if (price.startDate() > price.stopDate())
	return error("Start date after stop date");

    if (price.qtyLimit() < 0.0)
	return error("Negative quantity limit");

    if (price.minimumQty() < 0.0)
	return error("Negative minimum quantity");

    if (price.price().price() < 0.0)
	return error("Negative price");

    if (price.percentDiscount() < 0.0)
	return error("Negative percent discount");

    if (price.dollarDiscount() < 0.0)
	return error("Negative dollar discount");

    if (price.isCost() && price.method() == ItemPrice::COST_PLUS)
	return error("Can't use cost+ with costs");

    return true;
}
