// $Id: quasar_item.cpp,v 1.45 2005/05/13 23:54:31 bpepers Exp $
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
#include "object_cache.h"

#include "item.h"
#include "item_select.h"
#include "item_price.h"
#include "item_price_select.h"
#include "card.h"
#include "tax.h"

#define add_where(cmd,where) { if (cmd == "") cmd += " where "; else cmd += " and "; cmd += where; }

// Return a whole group of totals from the item_change table.
void
QuasarDB::itemTotals(Id item_id, const QString& size, Id store_id, QDate date,
		     vector<Id>& item_ids, vector<QString>& sizes,
		     vector<Id>& stores, vector<fixed>& on_hands,
		     vector<fixed>& total_costs, vector<fixed>& on_orders,
		     vector<fixed>& sold_qtys, vector<fixed>& sold_costs,
		     vector<fixed>& sold_prices, vector<fixed>& recv_qtys,
		     vector<fixed>& recv_costs, vector<fixed>& adj_qtys,
		     vector<fixed>& adj_costs)
{
    item_ids.clear();
    sizes.clear();
    stores.clear();
    on_hands.clear();
    total_costs.clear();
    on_orders.clear();
    sold_qtys.clear();
    sold_costs.clear();
    sold_prices.clear();
    recv_qtys.clear();
    recv_costs.clear();
    adj_qtys.clear();
    adj_costs.clear();

    QString cmd = "select item_id,size_name,store_id,sum(on_hand),"
	"sum(total_cost),sum(on_order),sum(sold_qty),sum(sold_cost),"
	"sum(sold_price),sum(recv_qty),sum(recv_cost),sum(adj_qty),"
	"sum(adj_cost) from item_change";
    QString sel = "";
    if (item_id != INVALID_ID) add_where(sel, "item_id = ?");
    if (!size.isEmpty()) add_where(sel, "size_name = ?");
    if (store_id != INVALID_ID) add_where(sel, "store_id = ?");
    if (!date.isNull()) add_where(sel, "change_date <= ?");
    cmd += sel + " group by item_id,size_name,store_id";
    Stmt stmt(_connection, cmd);

    if (item_id != INVALID_ID) stmtSetId(stmt, item_id);
    if (!size.isEmpty()) stmtSetString(stmt, size);
    if (store_id != INVALID_ID) stmtSetId(stmt, store_id);
    if (!date.isNull()) stmtSetDate(stmt, date);

    if (!execute(stmt)) return;
    while (stmt.next()) {
	item_ids.push_back(stmtGetId(stmt, 1));
	sizes.push_back(stmtGetString(stmt, 2));
	stores.push_back(stmtGetId(stmt, 3));
	on_hands.push_back(stmtGetFixed(stmt, 4));
	total_costs.push_back(stmtGetFixed(stmt, 5));
	on_orders.push_back(stmtGetFixed(stmt, 6));
	sold_qtys.push_back(stmtGetFixed(stmt, 7));
	sold_costs.push_back(stmtGetFixed(stmt, 8));
	sold_prices.push_back(stmtGetFixed(stmt, 9));
	recv_qtys.push_back(stmtGetFixed(stmt, 10));
	recv_costs.push_back(stmtGetFixed(stmt, 11));
	adj_qtys.push_back(stmtGetFixed(stmt, 12));
	adj_costs.push_back(stmtGetFixed(stmt, 13));
    }

    commit();
}

// Return a whole group of totals from the item_change table.
void
QuasarDB::itemTotals(Id item_id, const QString& size, Id store_id, QDate from,
		     QDate to, vector<QDate>& dates, vector<fixed>& qtys,
		     vector<fixed>& costs, vector<fixed>& sold_qtys,
		     vector<fixed>& sold_costs, vector<fixed>& sold_prices,
		     vector<fixed>& recv_qtys, vector<fixed>& recv_costs,
		     vector<fixed>& adj_qtys, vector<fixed>& adj_costs)
{
    dates.clear();
    qtys.clear();
    costs.clear();
    sold_qtys.clear();
    sold_costs.clear();
    sold_prices.clear();
    recv_qtys.clear();
    recv_costs.clear();
    adj_qtys.clear();
    adj_costs.clear();

    QString cmd = "select change_date,sum(on_hand),sum(total_cost),"
	"sum(sold_qty),sum(sold_cost),sum(sold_price),sum(recv_qty),"
	"sum(recv_cost),sum(adj_qty),sum(adj_cost) from item_change "
	"where item_id=?";
    if (!from.isNull() && !to.isNull())
	cmd += " and change_date between ? and ?";
    else if (!from.isNull())
	cmd += " and change_date >= ?";
    else if (!to.isNull())
	cmd += " and change_date <= ?";
    if (!size.isEmpty()) cmd += " and size_name = ?";
    if (store_id != INVALID_ID) cmd += " and store_id = ?";
    cmd += " group by change_date order by change_date";
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, item_id);
    if (!from.isNull() && !to.isNull()) {
	stmtSetDate(stmt, from);
	stmtSetDate(stmt, to);
    } else if (!from.isNull()) {
	stmtSetDate(stmt, from);
    } else if (!to.isNull()) {
	stmtSetDate(stmt, to);
    }
    if (!size.isEmpty()) stmtSetString(stmt, size);
    if (store_id != INVALID_ID) stmtSetId(stmt, store_id);

    if (!execute(stmt)) return;
    while (stmt.next()) {
	dates.push_back(stmtGetDate(stmt, 1));
	qtys.push_back(stmtGetFixed(stmt, 2));
	costs.push_back(stmtGetFixed(stmt, 3));
	sold_qtys.push_back(stmtGetFixed(stmt, 4));
	sold_costs.push_back(stmtGetFixed(stmt, 5));
	sold_prices.push_back(stmtGetFixed(stmt, 6));
	recv_qtys.push_back(stmtGetFixed(stmt, 7));
	recv_costs.push_back(stmtGetFixed(stmt, 8));
	adj_qtys.push_back(stmtGetFixed(stmt, 9));
	adj_costs.push_back(stmtGetFixed(stmt, 10));
    }

    commit();
}

void
QuasarDB::itemGeneral(Id item_id, const QString& size, Id store_id, QDate date,
		      vector<Id>& item_ids, vector<QString>& sizes,
		      vector<Id>& stores, vector<fixed>& on_hands,
		      vector<fixed>& total_costs, vector<fixed>& on_orders)
{
    vector<fixed> sold_qtys;
    vector<fixed> sold_costs;
    vector<fixed> sold_prices;
    vector<fixed> recv_qtys;
    vector<fixed> recv_costs;
    vector<fixed> adj_qtys;
    vector<fixed> adj_costs;
    itemTotals(item_id, size, store_id, date, item_ids, sizes, stores,
	       on_hands, total_costs, on_orders, sold_qtys, sold_costs,
	       sold_prices, recv_qtys, recv_costs, adj_qtys, adj_costs);
}

void
QuasarDB::itemSold(Id item_id, const QString& size, Id store_id, QDate date,
		   vector<Id>& item_ids, vector<QString>& sizes,
		   vector<Id>& stores, vector<fixed>& sold_qtys,
		   vector<fixed>& sold_costs, vector<fixed>& sold_prices)
{
    vector<fixed> on_hands;
    vector<fixed> total_costs;
    vector<fixed> on_orders;
    vector<fixed> recv_qtys;
    vector<fixed> recv_costs;
    vector<fixed> adj_qtys;
    vector<fixed> adj_costs;
    itemTotals(item_id, size, store_id, date, item_ids, sizes, stores,
	       on_hands, total_costs, on_orders, sold_qtys, sold_costs,
	       sold_prices, recv_qtys, recv_costs, adj_qtys, adj_costs);
}

void
QuasarDB::itemReceived(Id item_id, const QString& size, Id store_id,QDate date,
		       vector<Id>& item_ids, vector<QString>& sizes,
		       vector<Id>& stores, vector<fixed>& recv_qtys,
		       vector<fixed>& recv_costs)
{
    vector<fixed> on_hands;
    vector<fixed> total_costs;
    vector<fixed> on_orders;
    vector<fixed> sold_qtys;
    vector<fixed> sold_costs;
    vector<fixed> sold_prices;
    vector<fixed> adj_qtys;
    vector<fixed> adj_costs;
    itemTotals(item_id, size, store_id, date, item_ids, sizes, stores,
	       on_hands, total_costs, on_orders, sold_qtys, sold_costs,
	       sold_prices, recv_qtys, recv_costs, adj_qtys, adj_costs);
}

void
QuasarDB::itemAdjusted(Id item_id, const QString& size, Id store_id,QDate date,
		       vector<Id>& item_ids, vector<QString>& sizes,
		       vector<Id>& stores, vector<fixed>& adj_qtys,
		       vector<fixed>& adj_costs)
{
    vector<fixed> on_hands;
    vector<fixed> total_costs;
    vector<fixed> on_orders;
    vector<fixed> sold_qtys;
    vector<fixed> sold_costs;
    vector<fixed> sold_prices;
    vector<fixed> recv_qtys;
    vector<fixed> recv_costs;
    itemTotals(item_id, size, store_id, date, item_ids, sizes, stores,
	       on_hands, total_costs, on_orders, sold_qtys, sold_costs,
	       sold_prices, recv_qtys, recv_costs, adj_qtys, adj_costs);
}

// This routine is specially coded to make the select as fast as possible
// since this is likely the most commonly called routine (used for getting
// the item selling cost and such).
void
QuasarDB::itemGeneral(Id item_id, const QString& size, Id store_id, QDate date,
		      fixed& on_hand, fixed& total_cost, fixed& on_order)
{
    on_hand = 0.0;
    total_cost = 0.0;
    on_order = 0.0;

    QString cmd = "select sum(on_hand),sum(total_cost),sum(on_order) "
	"from item_change";
    QString sel = "";
    if (item_id != INVALID_ID) add_where(sel, "item_id = ?");
    if (!size.isEmpty()) add_where(sel, "size_name = ?");
    if (store_id != INVALID_ID) add_where(sel, "store_id = ?");
    if (!date.isNull()) add_where(sel, "change_date <= ?");
    cmd += sel;
    Stmt stmt(_connection, cmd);

    if (item_id != INVALID_ID) stmtSetId(stmt, item_id);
    if (!size.isEmpty()) stmtSetString(stmt, size);
    if (store_id != INVALID_ID) stmtSetId(stmt, store_id);
    if (!date.isNull()) stmtSetDate(stmt, date);

    if (!execute(stmt)) return;
    if (stmt.next()) {
	on_hand = stmtGetFixed(stmt, 1);
	total_cost = stmtGetFixed(stmt, 2);
	on_order = stmtGetFixed(stmt, 3);
    }

    commit();
}

void
QuasarDB::itemSold(Id item_id, const QString& size, Id store_id, QDate date,
		   fixed& sold_qty, fixed& sold_cost, fixed& sold_price)
{
    vector<Id> item_ids;
    vector<QString> sizes;
    vector<Id> stores;
    vector<fixed> on_hands;
    vector<fixed> total_costs;
    vector<fixed> on_orders;
    vector<fixed> sold_qtys;
    vector<fixed> sold_costs;
    vector<fixed> sold_prices;
    vector<fixed> recv_qtys;
    vector<fixed> recv_costs;
    vector<fixed> adj_qtys;
    vector<fixed> adj_costs;

    itemTotals(item_id, size, store_id, date, item_ids, sizes, stores,
	       on_hands, total_costs, on_orders, sold_qtys, sold_costs,
	       sold_prices, recv_qtys, recv_costs, adj_qtys, adj_costs);

    sold_qty = 0.0;
    sold_cost = 0.0;
    sold_price = 0.0;
    for (unsigned int i = 0; i < item_ids.size(); ++i) {
	sold_qty += sold_qtys[i];
	sold_cost += sold_costs[i];
	sold_price += sold_prices[i];
    }
}

void
QuasarDB::itemReceived(Id item_id, const QString& size, Id store_id,QDate date,
		       fixed& recv_qty, fixed& recv_cost)
{
    vector<Id> item_ids;
    vector<QString> sizes;
    vector<Id> stores;
    vector<fixed> on_hands;
    vector<fixed> total_costs;
    vector<fixed> on_orders;
    vector<fixed> sold_qtys;
    vector<fixed> sold_costs;
    vector<fixed> sold_prices;
    vector<fixed> recv_qtys;
    vector<fixed> recv_costs;
    vector<fixed> adj_qtys;
    vector<fixed> adj_costs;

    itemTotals(item_id, size, store_id, date, item_ids, sizes, stores,
	       on_hands, total_costs, on_orders, sold_qtys, sold_costs,
	       sold_prices, recv_qtys, recv_costs, adj_qtys, adj_costs);

    recv_qty = 0.0;
    recv_cost = 0.0;
    for (unsigned int i = 0; i < item_ids.size(); ++i) {
	recv_qty += recv_qtys[i];
	recv_cost += recv_costs[i];
    }
}

void
QuasarDB::itemAdjusted(Id item_id, const QString& size, Id store_id,QDate date,
		       fixed& adj_qty, fixed& adj_cost)
{
    vector<Id> item_ids;
    vector<QString> sizes;
    vector<Id> stores;
    vector<fixed> on_hands;
    vector<fixed> total_costs;
    vector<fixed> on_orders;
    vector<fixed> sold_qtys;
    vector<fixed> sold_costs;
    vector<fixed> sold_prices;
    vector<fixed> recv_qtys;
    vector<fixed> recv_costs;
    vector<fixed> adj_qtys;
    vector<fixed> adj_costs;

    itemTotals(item_id, size, store_id, date, item_ids, sizes, stores,
	       on_hands, total_costs, on_orders, sold_qtys, sold_costs,
	       sold_prices, recv_qtys, recv_costs, adj_qtys, adj_costs);

    adj_qty = 0.0;
    adj_cost = 0.0;
    for (unsigned int i = 0; i < item_ids.size(); ++i) {
	adj_qty += adj_qtys[i];
	adj_cost += adj_costs[i];
    }
}

// Return balances of items on a given date
void
QuasarDB::itemBalances(QDate date, ItemSelect& conditions,vector<Id>& item_ids,
		       vector<fixed>& on_hands, vector<fixed>& total_costs)
{
    if (date.isNull()) date = QDate::currentDate();
    item_ids.clear();
    on_hands.clear();
    total_costs.clear();

    QString table;
    QString where = conditions.where();
    if (where.isEmpty()) {
	table = "item_change";
	where = "where change_date <= ?";
    } else {
	table = "item_change join item on item_change.item_id = item.item_id";
	where += " and change_date <= ?";
    }

    QString cmd = "select item_change.item_id,sum(on_hand),sum(total_cost) "
	"from " + table + " " + where + " group by item_change.item_id "
	"order by item_change.item_id";

    Stmt stmt(_connection, cmd);
    stmtSetDate(stmt, date);
    if (!execute(stmt)) return;
    while (stmt.next()) {
	item_ids.push_back(stmtGetId(stmt, 1));
	on_hands.push_back(stmtGetFixed(stmt, 2));
	total_costs.push_back(stmtGetFixed(stmt, 3));
    }
}

// Return the quantity, cost, and price of items sold
void
QuasarDB::itemSold(Id item_id, const QString& size, Id store_id, QDate start,
		   QDate end, fixed& qty, fixed& cost, fixed& price)
{
    qty = 0.0;
    cost = 0.0;
    price = 0.0;

    QString cmd = "select sum(sold_qty),sum(sold_cost),sum(sold_price) "
	"from item_change where item_id = ? and change_date >= ? and "
	"change_date <= ?";
    if (!size.isEmpty()) cmd += " and size_name = ?";
    if (store_id != INVALID_ID) cmd += " and store_id = ?";
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, item_id);
    stmtSetDate(stmt, start);
    stmtSetDate(stmt, end);
    if (!size.isEmpty()) stmtSetString(stmt, size);
    if (store_id != INVALID_ID) stmtSetId(stmt, store_id);

    if (!execute(stmt)) return;
    if (stmt.next()) {
	qty = stmtGetFixed(stmt, 1);
	cost = stmtGetFixed(stmt, 2);
	price = stmtGetFixed(stmt, 3);
    }
}

// Return the quantity and cost of the last time received
void
QuasarDB::itemLastRecv(Id item_id, const QString& size, Id store_id,QDate date,
		       fixed& qty, fixed& cost)
{
    qty = 0.0;
    cost = 0.0;

    QString cmd = "select recv_qty,recv_cost from item_change "
	"where item_id = ? and recv_qty > 0.0";
    if (!date.isNull()) cmd += " and change_date <= ?";
    if (!size.isEmpty()) cmd += " and size_name = ?";
    if (store_id != INVALID_ID) cmd += " and store_id = ?";
    cmd += " order by change_date desc";
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, item_id);
    if (!date.isNull()) stmtSetDate(stmt, date);
    if (!size.isEmpty()) stmtSetString(stmt, size);
    if (store_id != INVALID_ID) stmtSetId(stmt, store_id);

    if (!execute(stmt)) return;
    if (stmt.next()) {
	qty = stmtGetFixed(stmt, 1);
	cost = stmtGetFixed(stmt, 2);
    }
}

// Adjust the on hand and total cost levels of an item
bool
QuasarDB::itemAdjust(Id item_id, const QString& size, Id store_id, QDate date,
		     fixed qty, fixed cost)
{
    if (date.isNull()) date = QDate::currentDate();

    QString cmd = "update item_change set on_hand = on_hand+?, total_cost = "
	"total_cost+? where item_id=? and size_name=? and store_id=? "
	"and change_date=?";
    Stmt stmt(_connection, cmd);
    stmtSetFixed(stmt, qty);
    stmtSetFixed(stmt, cost);
    stmtSetId(stmt, item_id);
    stmtSetString(stmt, size);
    stmtSetId(stmt, store_id);
    stmtSetDate(stmt, date);

    // If update works, thats it!
    execute(stmt);
    fixed cnt = stmt.getUpdateCount();
    if (cnt == 1) return true;

    // Have to insert a new blank record
    QString cmd2 = "insert into item_change (item_id,size_name,"
	"store_id,change_date,on_hand,total_cost,on_order,sold_qty,"
	"sold_cost,sold_price,recv_qty,recv_cost,adj_qty,adj_cost) "
	"values (?,?,?,?,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0)";
    Stmt stmt2(_connection, cmd2);
    stmtSetId(stmt2, item_id);
    stmtSetString(stmt2, size);
    stmtSetId(stmt2, store_id);
    stmtSetDate(stmt2, date);
    execute(stmt2);
    if (stmt2.getUpdateCount() != 1)
	return error("invalid insert in itemAdjust");

    // Try again and it better work!
    execute(stmt);
    cnt = stmt.getUpdateCount();
    if (cnt == 1) return true;

    return error("invalid update count in itemAdjust");
}

// Record a sale
bool
QuasarDB::itemSale(Id item_id, const QString& size, Id store_id, QDate date,
		   fixed qty, fixed cost, fixed price)
{
    if (date.isNull()) date = QDate::currentDate();

    QString cmd = "update item_change set on_hand = on_hand - ?,"
	"total_cost = total_cost - ?, sold_qty = sold_qty + ?,"
	"sold_cost = sold_cost + ?, sold_price = sold_price + ? "
	"where item_id=? and size_name=? and store_id=? and "
	"change_date=?";
    Stmt stmt(_connection, cmd);
    stmtSetFixed(stmt, qty);
    stmtSetFixed(stmt, cost);
    stmtSetFixed(stmt, qty);
    stmtSetFixed(stmt, cost);
    stmtSetFixed(stmt, price);
    stmtSetId(stmt, item_id);
    stmtSetString(stmt, size);
    stmtSetId(stmt, store_id);
    stmtSetDate(stmt, date);

    // If update works, thats it!
    execute(stmt);
    fixed cnt = stmt.getUpdateCount();
    if (cnt == 1) return true;

    // Have to insert a new blank record
    QString cmd2 = "insert into item_change (item_id,size_name,"
	"store_id,change_date,on_hand,total_cost,on_order,sold_qty,"
	"sold_cost,sold_price,recv_qty,recv_cost,adj_qty,adj_cost) "
	"values (?,?,?,?,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0)";
    Stmt stmt2(_connection, cmd2);
    stmtSetId(stmt2, item_id);
    stmtSetString(stmt2, size);
    stmtSetId(stmt2, store_id);
    stmtSetDate(stmt2, date);
    execute(stmt2);
    if (stmt2.getUpdateCount() != 1)
	return error("invalid insert in itemAdjust");

    // Try again and it better work!
    execute(stmt);
    cnt = stmt.getUpdateCount();
    if (cnt == 1) return true;

    return error("invalid update count in itemAdjust");
}

// Record sales
bool
QuasarDB::itemSale(vector<Id>& item_ids, vector<QString>& sizes, Id store_id,
		   QDate date, vector<fixed>& qtys, vector<fixed>& costs,
		   vector<fixed>& prices)
{
    if (date.isNull()) date = QDate::currentDate();

    QString cmd1 = "update item_change set on_hand = on_hand - ?,"
	"total_cost = total_cost - ?, sold_qty = sold_qty + ?,"
	"sold_cost = sold_cost + ?, sold_price = sold_price + ? "
	"where item_id=? and size_name=? and store_id=? and "
	"change_date=?";
    Stmt stmt1(_connection, cmd1);

    QString cmd2 = "insert into item_change (item_id,size_name,"
	"store_id,change_date,on_hand,total_cost,on_order,sold_qty,"
	"sold_cost,sold_price,recv_qty,recv_cost,adj_qty,adj_cost) "
	"values (?,?,?,?,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0)";
    Stmt stmt2(_connection, cmd2);

    for (unsigned int i = 0; i < item_ids.size(); ++i) {
	stmtSetFixed(stmt1, qtys[i]);
	stmtSetFixed(stmt1, costs[i]);
	stmtSetFixed(stmt1, qtys[i]);
	stmtSetFixed(stmt1, costs[i]);
	stmtSetFixed(stmt1, prices[i]);
	stmtSetId(stmt1, item_ids[i]);
	stmtSetString(stmt1, sizes[i]);
	stmtSetId(stmt1, store_id);
	stmtSetDate(stmt1, date);

	// If update works, thats it!
	execute(stmt1);
	fixed cnt = stmt1.getUpdateCount();
	if (cnt == 1) continue;

	// Have to insert a new blank record
	stmtSetId(stmt2, item_ids[i]);
	stmtSetString(stmt2, sizes[i]);
	stmtSetId(stmt2, store_id);
	stmtSetDate(stmt2, date);
	execute(stmt2);
	if (stmt2.getUpdateCount() != 1)
	    return error("invalid insert in itemSale");

	// Try again and it better work!
	execute(stmt1);
	cnt = stmt1.getUpdateCount();
	if (cnt != 1)
	    return error("invalid update in itemSale");
    }
    return true;
}

// Record a receiving
bool
QuasarDB::itemReceive(Id item_id, const QString& size, Id store_id, QDate date,
		      fixed qty, fixed cost)
{
    if (date.isNull()) date = QDate::currentDate();

    itemAdjust(item_id, size, store_id, date, qty, cost);

    QString cmd = "update item_change set recv_qty = recv_qty + ?, "
	"recv_cost = recv_cost + ? where item_id=? and size_name=? "
	"and store_id=? and change_date=?";

    Stmt stmt(_connection, cmd);
    stmtSetFixed(stmt, qty);
    stmtSetFixed(stmt, cost);
    stmtSetId(stmt, item_id);
    stmtSetString(stmt, size);
    stmtSetId(stmt, store_id);
    stmtSetDate(stmt, date);

    execute(stmt);
    if (stmt.getUpdateCount() != 1)
	return error("invalid update in itemReceive");

    return true;
}

// Record receive
bool
QuasarDB::itemReceive(vector<Id>& item_ids,vector<QString>& sizes, Id store_id,
		      QDate date, vector<fixed>& qtys, vector<fixed>& costs)
{
    if (date.isNull()) date = QDate::currentDate();

    QString cmd1 = "update item_change set on_hand = on_hand + ?,"
	"total_cost = total_cost + ?, recv_qty = recv_qty + ?,"
	"recv_cost = recv_cost + ? where item_id=? and size_name=? "
	"and store_id=? and change_date=?";
    Stmt stmt1(_connection, cmd1);

    QString cmd2 = "insert into item_change (item_id,size_name,"
	"store_id,change_date,on_hand,total_cost,on_order,sold_qty,"
	"sold_cost,sold_price,recv_qty,recv_cost,adj_qty,adj_cost) "
	"values (?,?,?,?,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0)";
    Stmt stmt2(_connection, cmd2);

    for (unsigned int i = 0; i < item_ids.size(); ++i) {
	stmtSetFixed(stmt1, qtys[i]);
	stmtSetFixed(stmt1, costs[i]);
	stmtSetFixed(stmt1, qtys[i]);
	stmtSetFixed(stmt1, costs[i]);
	stmtSetId(stmt1, item_ids[i]);
	stmtSetString(stmt1, sizes[i]);
	stmtSetId(stmt1, store_id);
	stmtSetDate(stmt1, date);

	// If update works, thats it!
	execute(stmt1);
	fixed cnt = stmt1.getUpdateCount();
	if (cnt == 1) continue;

	// Have to insert a new blank record
	stmtSetId(stmt2, item_ids[i]);
	stmtSetString(stmt2, sizes[i]);
	stmtSetId(stmt2, store_id);
	stmtSetDate(stmt2, date);
	execute(stmt2);
	if (stmt2.getUpdateCount() != 1)
	    return error("invalid insert in itemRecv");

	// Try again and it better work!
	execute(stmt1);
	cnt = stmt1.getUpdateCount();
	if (cnt != 1)
	    return error("invalid update in itemRecv");
    }
    return true;
}

// Record an adjustment
bool
QuasarDB::itemAdjustment(Id item_id, const QString& size, Id store_id,
			 QDate date, fixed qty, fixed cost)
{
    if (date.isNull()) date = QDate::currentDate();

    itemAdjust(item_id, size, store_id, date, qty, cost);

    QString cmd = "update item_change set adj_qty = adj_qty + ?, "
	"adj_cost = adj_cost + ? where item_id=? and size_name=? "
	"and store_id=? and change_date=?";

    Stmt stmt(_connection, cmd);
    stmtSetFixed(stmt, qty);
    stmtSetFixed(stmt, cost);
    stmtSetId(stmt, item_id);
    stmtSetString(stmt, size);
    stmtSetId(stmt, store_id);
    stmtSetDate(stmt, date);

    execute(stmt);
    if (stmt.getUpdateCount() != 1)
	return error("invalid update in itemAdjustment");

    return true;
}

// Record an order
bool
QuasarDB::itemOrder(Id item_id, const QString& size, Id store_id, QDate date,
		    fixed qty)
{
    if (date.isNull()) date = QDate::currentDate();

    itemAdjust(item_id, size, store_id, date, 0.0, 0.0);

    QString cmd = "update item_change set on_order = on_order + ? "
	"where item_id=? and size_name=? and store_id=? and "
	"change_date=?";

    Stmt stmt(_connection, cmd);
    stmtSetFixed(stmt, qty);
    stmtSetId(stmt, item_id);
    stmtSetString(stmt, size);
    stmtSetId(stmt, store_id);
    stmtSetDate(stmt, date);

    execute(stmt);
    if (stmt.getUpdateCount() != 1)
	return error("invalid update in itemOrder");

    return true;
}

// Record an order
bool
QuasarDB::itemOrder(vector<Id>& item_ids, vector<QString>& sizes, Id store_id,
		    QDate date, vector<fixed>& qtys)
{
    if (date.isNull()) date = QDate::currentDate();

    QString cmd1 = "update item_change set on_order = on_order + ? "
	"where item_id=? and size_name=? and store_id=? and "
	"change_date=?";
    Stmt stmt1(_connection, cmd1);

    QString cmd2 = "insert into item_change (item_id,size_name,"
	"store_id,change_date,on_hand,total_cost,on_order,sold_qty,"
	"sold_cost,sold_price,recv_qty,recv_cost,adj_qty,adj_cost) "
	"values (?,?,?,?,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0)";
    Stmt stmt2(_connection, cmd2);

    for (unsigned int i = 0; i < item_ids.size(); ++i) {
	stmtSetFixed(stmt1, qtys[i]);
	stmtSetId(stmt1, item_ids[i]);
	stmtSetString(stmt1, sizes[i]);
	stmtSetId(stmt1, store_id);
	stmtSetDate(stmt1, date);

	// If update works, thats it!
	execute(stmt1);
	fixed cnt = stmt1.getUpdateCount();
	if (cnt == 1) continue;

	// Have to insert a new blank record
	stmtSetId(stmt2, item_ids[i]);
	stmtSetString(stmt2, sizes[i]);
	stmtSetId(stmt2, store_id);
	stmtSetDate(stmt2, date);
	execute(stmt2);
	if (stmt2.getUpdateCount() != 1)
	    return error("invalid insert in itemOrder");

	// Try again and it better work!
	execute(stmt1);
	cnt = stmt1.getUpdateCount();
	if (cnt != 1)
	    return error("invalid update in itemOrder");
    }
    return true;
}

void
QuasarDB::itemRegularPrice(Id item_id, const QString& size, ItemPrice& price)
{
    price.setNull();

    Item item;
    if (!lookup(item_id, item)) return;

    ItemPriceSelect conditions;
    conditions.activeOnly = true;
    conditions.priceOnly = true;
    conditions.item = item;
    conditions.card.setId(INVALID_ID);
    conditions.size = size;
    vector<ItemPrice> prices;
    select(prices, conditions);

    if (prices.size() == 0) {
	conditions.size = "";
	select(prices, conditions);
	if (prices.size() != 0) {
	    price = prices[0];
	    fixed price_qty = item.sizeQty(price.size());
	    fixed size_qty = item.sizeQty(size);
	    price.setPrice(price.calculate(1.0) * size_qty / price_qty);
	}
    } else {
	price = prices[0];
    }
}

void
QuasarDB::itemRegularCost(Id item_id, const QString& size, ItemPrice& cost)
{
    cost.setNull();

    Item item;
    if (!lookup(item_id, item)) return;

    ItemPriceSelect conditions;
    conditions.activeOnly = true;
    conditions.costOnly = true;
    conditions.item = item;
    conditions.card.setId(INVALID_ID);
    conditions.size = size;
    vector<ItemPrice> costs;
    select(costs, conditions);

    if (costs.size() == 0) {
	conditions.size = "";
	select(costs, conditions);
	if (costs.size() != 0) {
	    cost = costs[0];
	    fixed cost_qty = item.sizeQty(cost.size());
	    fixed size_qty = item.sizeQty(size);
	    cost.setPrice(cost.calculate(1.0) * size_qty / cost_qty);
	}
    } else {
	cost = costs[0];
    }
}

void
QuasarDB::itemBestPrice(const Item& item, vector<ItemPrice>& prices,
			const QString& size, const Card& card, Id store_id,
			QDate date, fixed qty, bool includePromo,
			bool getCost, ItemPrice& best, fixed& ext_price,
			fixed& sell_qty, vector<Id>& price_ids,
			vector<fixed>& sold_qtys)
{
    int dow = date.dayOfWeek();
    fixed size_qty = item.sizeQty(size);

    vector<fixed> sell_qtys(prices.size(), qty);

    for (int i = prices.size() - 1; i >= 0; --i) {
	const ItemPrice& price = prices[i];
	bool remove = false;

	if (price.size() != size && !price.size().isEmpty())
	    remove = true;
	if (price.cardId() != INVALID_ID && price.cardId() != card.id())
	    remove = true;
	if (price.cardGroup() != INVALID_ID) {
	    bool found = false;
	    for (unsigned int i = 0; i < card.groups().size(); ++i)
		if (card.groups()[i] == price.cardGroup())
		    found = true;
	    if (!found) remove = true;
	}
	if (price.storeId() != INVALID_ID && price.storeId() != store_id)
	    remove = true;
	if (price.isPromotion() && !includePromo)
	    remove = true;
	if (!price.startDate().isNull() && price.startDate() > date)
	    remove = true;
	if (!price.stopDate().isNull() && price.stopDate() < date)
	    remove = true;
	if (price.qtyLimit() != 0.0) {
	    fixed used = 0.0;
	    for (unsigned int j = 0; j < price_ids.size(); ++j) {
		if (price_ids[j] == price.id()) {
		    used = sold_qtys[j];
		    break;
		}
	    }
	    if (used >= price.qtyLimit())
		remove = true;
	    else if (used + qty > price.qtyLimit())
		sell_qtys[i] = price.qtyLimit() - used;
	}
	if (price.minimumQty() > qty)
	    remove = true;
	if (!price.dayOfWeek(dow - 1))
	    remove = true;

	if (remove) {
	    prices.erase(prices.begin() + i);
	    sell_qtys.erase(sell_qtys.begin() + i);
	}
    }

    // If no price, return regular
    if (prices.size() == 0) {
	ItemPrice regular;
	if (getCost)
	    regular.setPrice(item.cost(store_id, size));
	else
	    regular.setPrice(item.price(store_id, size));
	best = regular;
	ext_price = regular.calculate(qty);
	sell_qty = qty;
	return;
    }

    ObjectCache cache(this);

    // Pick lowest price
    int lowest = -1;
    fixed lowPrice = 99999999.0;
    fixed lowExtPrice = 99999999.0;
    for (int j = prices.size() - 1; j >= 0; --j) {
	const ItemPrice& price = prices[j];
	fixed qty = sell_qtys[j];
	fixed regCost = item.cost(store_id, size, qty);
	fixed regPrice = item.price(store_id, size).calculate(qty);
	fixed percent = price.percentDiscount();
	fixed dollar = price.dollarDiscount();
	fixed extPrice = 0.0;
	fixed baseCost;

	if (regCost == 0.0 && price.method() == ItemPrice::COST_PLUS)
	    continue;

	switch (price.method()) {
	case ItemPrice::PRICE:
	    extPrice = price.calculate(qty);
	    break;
	case ItemPrice::COST_PLUS:
	    baseCost = regCost;
	    if (item.costIncludesDeposit())
		baseCost -= item.deposit() * size_qty * qty;
	    if (item.costIncludesTax()) {
		Tax tax;
		lookup(item.purchaseTax(), tax);
		baseCost -= calculateTaxOff(cache, tax, baseCost);
	    }
	    extPrice = baseCost + percent * baseCost / 100.0;
	    if (item.priceIncludesTax()) {
		Tax tax;
		lookup(item.sellTax(), tax);
		fixed tax_amt = calculateTaxOn(cache, tax, extPrice);
		tax_amt.moneyRound();
		extPrice += tax_amt;
	    }
	    if (item.priceIncludesDeposit())
		extPrice += item.deposit() * size_qty * qty;
	    break;
	case ItemPrice::PERCENT_OFF:
	    if (getCost)
		extPrice = regCost - percent * regCost / 100.0;
	    else
		extPrice = regPrice - percent * regPrice / 100.0;
	    break;
	case ItemPrice::DOLLAR_OFF:
	    if (getCost)
		extPrice = regCost - dollar * qty;
	    else
		extPrice = regPrice - dollar * qty;
	    if (extPrice < 0.0) extPrice = 0.0;
	    break;
	}
	extPrice.moneyRound();

	fixed singlePrice = extPrice / qty;
	if (singlePrice < lowPrice) {
	    lowest = j;
	    lowPrice = singlePrice;
	    lowExtPrice = extPrice;
	}
    }

    // If not getting a cost, pick the regular price if lower
    if (!getCost) {
	fixed single = item.price(store_id, size).calculate(qty) / qty;
	if (single < lowPrice && single != 0.0) {
	    ItemPrice regular;
	    regular.setPrice(item.price(store_id, size));
	    best = regular;
	    ext_price = regular.calculate(qty);
	    sell_qty = qty;
	    return;
	}
    }

    if (lowest == -1) {
	ItemPrice regular;
	regular.setPrice(item.price(store_id, size));
	best = regular;
	ext_price = regular.calculate(qty);
	sell_qty = qty;
	return;
    }

    best = prices[lowest];
    ext_price = lowExtPrice;
    sell_qty = sell_qtys[lowest];
}

// This routine tries to get the best cost to sell a quantity of an item
// of a certain size in a certain store.  Normally it will use the
// average cost but it has to handle cases where there is not enough on
// hand or indeed nothing on hand.  It works by trying the most likely
// methods and eventually falling back on the replacement cost.
void
QuasarDB::itemSellingCost(const Item& item, const QString& size, Id store_id,
			  fixed qty, fixed sell_price, fixed& ext_cost)
{
    ext_cost = 0.0;
    fixed size_qty = item.sizeQty(size);
    QDate date = QDate::currentDate();

    // Use positive quantity but retain sign
    fixed sign = 1.0;
    if (qty < 0.0) {
	sign = -1.0;
	qty = -qty;
    }

    if (item.isOpenDept()) {
	fixed margin = item.targetGM(store_id, size) / 100.0;
	ext_cost = sell_price - (margin * sell_price);
	ext_cost.moneyRound();
	return;
    }

    // First try with the current on hand and cost
    fixed on_hand, total_cost, on_order;
    itemGeneral(item.id(), "", store_id, date, on_hand, total_cost, on_order);
    on_hand = on_hand / size_qty;

    // Best case is when on hand matches quantity
    if (on_hand == qty) {
	ext_cost = total_cost * sign;
	ext_cost.moneyRound();
	return;
    }

    // Check for zero here to let the above line catch residual values
    if (qty == 0.0) {
	return;
    }

    // Next best is when we have enough on hand for the selling qty
    if (on_hand > qty) {
	ext_cost = total_cost * qty / on_hand * sign;
	ext_cost.moneyRound();
	return;
    }

    // Use up the existing on hand
    if (on_hand > 0.0 || total_cost > 0.0) {
	qty -= on_hand;
	ext_cost += total_cost;
    }

    // TODO: could try in here to get the last selling cost and use that

    // Try to find a replacement cost
    QString cost_size = size;
    Price cost = item.cost(store_id, cost_size);
    if (cost.isNull()) {
	cost_size = item.purchaseSize();
	cost = item.cost(store_id, cost_size);
    }
    if (cost.isNull()) {
	cost_size = item.sellSize();
	cost = item.cost(store_id, cost_size);
    }
    if (cost.isNull()) {
	for (unsigned int i = 0; i < item.costs().size(); ++i) {
	    if (item.costs()[i].cost.isNull()) continue;
	    cost_size = item.costs()[i].size;
	    cost = item.costs()[i].cost;
	    break;
	}
    }

    ObjectCache cache(this);

    // Use replacement cost if found
    if (!cost.isNull()) {
	fixed cost_size_qty = item.sizeQty(cost_size);
	fixed item_cost = cost.calculate(qty);
	if (item.costIncludesDeposit())
	    item_cost -= item.deposit() * cost_size_qty * qty;
	if (item.costIncludesTax()) {
	    Tax tax;
	    lookup(item.purchaseTax(), tax);
	    item_cost -= calculateTaxOff(cache, tax, item_cost);
	}
	item_cost = item_cost * size_qty / cost_size_qty;

	ext_cost += item_cost;
	ext_cost = ext_cost * sign;
	ext_cost.moneyRound();
	return;
    }

    // Thats it!  No more ways to guess so the extended cost just ends
    // up being zero (or perhaps a partial cost from on hand amounts
    // used up).  Shouldn't get here though since normally you will have
    // regular costs or if you've ever received the item it will have a
    // last cost.
    ext_cost = ext_cost * sign;
    ext_cost.moneyRound();
}

static QString
upcCheckDigit(const QString& number)
{
    int even = 0;
    int odd = 0;

    if (number.length() != 11)
	return "0";

    for (unsigned int i = 0; i < 11; i += 2) {
	odd += number.mid(i, 1).toInt();
	if (i < 10)
	    even += number.mid(i + 1, 1).toInt();
    }

    int check = (10 - ((odd * 3 + even) % 10)) % 10;

    return QString::number(check);
}

static QString
upcEtoA(const QString& number)
{
    if (number.length() != 6)
	return "0000000000";

    QString x1 = number.mid(0, 1);
    QString x2 = number.mid(1, 1);
    QString x3 = number.mid(2, 1);
    QString x4 = number.mid(3, 1);
    QString x5 = number.mid(4, 1);
    QString n = number.mid(5, 1);

    QString upc;
    if (n == "0")
	upc = x1 + x2 + "00000" + x3 + x4 + x5;
    else if (n == "1")
	upc = x1 + x2 + "10000" + x3 + x4 + x5;
    else if (n == "2")
	upc = x1 + x2 + "20000" + x3 + x4 + x5;
    else if (n == "3")
	upc = x1 + x2 + x3 + "00000" + x4 + x5;
    else if (n == "4")
	upc = x1 + x2 + x3 + x4 + "00000" + x5;
    else
	upc = x1 + x2 + x3 + x4 + x5 + "0000" + n;

    return upc;
}

bool
QuasarDB::lookup(ItemSelect& conditions, const QString& number,
		 vector<Item>& items)
{
    // First try straight lookup
    conditions.number = number;
    select(items, conditions);
    if (items.size() != 0) return true;

    // Try removing leading zeros
    for (unsigned int i = 0; i < number.length(); ++i) {
        if (number.at(i).latin1() != '0') break;
	conditions.number = number.mid(i + 1);
	select(items, conditions);
	if (items.size() != 0) return true;
    }

    QString type, upc, checkDigit;

    switch (number.length()) {
    case 12:
	// If valid 12 digit UPC then try various parts of it
	checkDigit = upcCheckDigit(number.left(11));
	if (checkDigit == number.right(1)) {
	    conditions.number = number.left(11);
	    select(items, conditions);
	    if (items.size() != 0) return true;

	    conditions.number = number.mid(1, 10);
	    select(items, conditions);
	    if (items.size() != 0) return true;

	    conditions.number = number.right(11);
	    select(items, conditions);
	    if (items.size() != 0) return true;
	}
	break;

    case 11:
	// First try assuming its the type and 10 digit number
	checkDigit = upcCheckDigit(number);
	lookup(conditions, number + checkDigit, items);
	if (items.size() != 0) return true;

	// Next try assuming its the 10 digit number and check digit
	type = "0";
	upc = number.left(10);
	checkDigit = upcCheckDigit(type + upc);
	if (checkDigit == number.right(1)) {
	    lookup(conditions, type + upc + checkDigit, items);
	    if (items.size() != 0) return true;
	}
	break;

    case 10:
	// If 10 digit UPC then convert to 12 and retry
	type = "0";
	upc = number;
	checkDigit = upcCheckDigit(type + upc);
	lookup(conditions, type + upc + checkDigit, items);
	if (items.size() != 0) return true;
	break;

    case 6:
	// If 6 digit UPC then try expanding with/without checkdigit
	type = "0";
	upc = upcEtoA(number);
	checkDigit = upcCheckDigit(type + upc);

	lookup(conditions, type + upc + checkDigit, items);
	if (items.size() != 0) return true;
	break;

    case 7:
	// If 7 digit UPC then first try as type and number
	type = number.left(1);
	upc = upcEtoA(number.right(6));
	checkDigit = upcCheckDigit(type + upc);
	if (type == "0" || type == "6" || type == "7") {
	    lookup(conditions, type + upc + checkDigit, items);
	    if (items.size() != 0) return true;
	}

	// Next try as number and check digit
	type = "0";
	upc = upcEtoA(number.left(6));
	checkDigit = number.right(1);
	if (upcCheckDigit(type + upc) == checkDigit) {
	    lookup(conditions, type + upc + checkDigit, items);
	    if (items.size() != 0) return true;
	}
	break;

    case 8:
	// If 8 digit UPC then try expanding
	type = number.left(1);
	upc = upcEtoA(number.mid(1, 6));
	checkDigit = number.right(1);

	if (checkDigit == upcCheckDigit(type + upc)) {
	    lookup(conditions, type + upc + checkDigit, items);
	    if (items.size() != 0) return true;
	}
	break;
    }

    return false;
}

bool
QuasarDB::lookup(ItemSelect& conditions, const QString& number,
		 vector<Id>& item_ids)
{
    // First try straight lookup
    conditions.number = number;
    select(item_ids, conditions);
    if (item_ids.size() != 0) return true;

    // Try removing leading zeros
    for (unsigned int i = 0; i < number.length(); ++i) {
        if (number.at(i).latin1() != '0') break;
	conditions.number = number.mid(i + 1);
	select(item_ids, conditions);
	if (item_ids.size() != 0) return true;
    }

    QString type, upc, checkDigit;

    switch (number.length()) {
    case 12:
	// If valid 12 digit UPC then try various parts of it
	checkDigit = upcCheckDigit(number.left(11));
	if (checkDigit == number.right(1)) {
	    conditions.number = number.left(11);
	    select(item_ids, conditions);
	    if (item_ids.size() != 0) return true;

	    conditions.number = number.mid(1, 10);
	    select(item_ids, conditions);
	    if (item_ids.size() != 0) return true;

	    conditions.number = number.right(11);
	    select(item_ids, conditions);
	    if (item_ids.size() != 0) return true;
	}
	break;

    case 11:
	// First try assuming its the type and 10 digit number
	checkDigit = upcCheckDigit(number);
	lookup(conditions, number + checkDigit, item_ids);
	if (item_ids.size() != 0) return true;

	// Next try assuming its the 10 digit number and check digit
	type = "0";
	upc = number.left(10);
	checkDigit = upcCheckDigit(type + upc);
	if (checkDigit == number.right(1)) {
	    lookup(conditions, type + upc + checkDigit, item_ids);
	    if (item_ids.size() != 0) return true;
	}
	break;

    case 10:
	// If 10 digit UPC then convert to 12 and retry
	type = "0";
	upc = number;
	checkDigit = upcCheckDigit(type + upc);
	lookup(conditions, type + upc + checkDigit, item_ids);
	if (item_ids.size() != 0) return true;
	break;

    case 6:
	// If 6 digit UPC then try expanding with/without checkdigit
	type = "0";
	upc = upcEtoA(number);
	checkDigit = upcCheckDigit(type + upc);

	lookup(conditions, type + upc + checkDigit, item_ids);
	if (item_ids.size() != 0) return true;
	break;

    case 7:
	// If 7 digit UPC then first try as type and number
	type = number.left(1);
	upc = upcEtoA(number.right(6));
	checkDigit = upcCheckDigit(type + upc);
	if (type == "0" || type == "6" || type == "7") {
	    lookup(conditions, type + upc + checkDigit, item_ids);
	    if (item_ids.size() != 0) return true;
	}

	// Next try as number and check digit
	type = "0";
	upc = upcEtoA(number.left(6));
	checkDigit = number.right(1);
	if (upcCheckDigit(type + upc) == checkDigit) {
	    lookup(conditions, type + upc + checkDigit, item_ids);
	    if (item_ids.size() != 0) return true;
	}
	break;

    case 8:
	// If 8 digit UPC then try expanding
	type = number.left(1);
	upc = upcEtoA(number.mid(1, 6));
	checkDigit = number.right(1);

	if (checkDigit == upcCheckDigit(type + upc)) {
	    lookup(conditions, type + upc + checkDigit, item_ids);
	    if (item_ids.size() != 0) return true;
	}
	break;
    }

    return false;
}
