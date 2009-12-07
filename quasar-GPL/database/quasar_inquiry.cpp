// $Id: quasar_inquiry.cpp,v 1.26 2005/03/01 19:59:42 bpepers Exp $
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

#include "gltx.h"
#include "select.h"

// Select based on a given account id.  Returns Gltx sorted by date and
// number.  Used for inquiry window.
void
QuasarDB::selectAccount(Id account_id, Id store_id, QDate start, QDate end,
			vector<Gltx>& gltxs, vector<fixed>& amounts)
{
    gltxs.clear();
    amounts.clear();

    QString table = "gltx join gltx_account on gltx.gltx_id = "
	"gltx_account.gltx_id";
    QString cmd = selectCmd(table, "gltx.gltx_id", "gltx.number,"
			    "reference_str,post_date,post_time,"
			    "gltx.memo,station_id,employee_id,"
			    "gltx.card_id,store_id,shift_id,link_id,"
			    "printed,paid,gltx.amount,data_type,"
			    "gltx_account.amount", Select());
    cmd += " where account_id = ?";
    if (store_id != INVALID_ID)
	cmd += " and store_id = ?";
    if (!start.isNull() && !end.isNull())
	cmd += " and post_date between ? and ?";
    else if (!start.isNull())
	cmd += " and post_date >= ?";
    else if (!end.isNull())
	cmd += " and post_date <= ?";
    cmd += " order by post_date, gltx.number";

    Stmt stmt(_connection, cmd);
    stmtSetId(stmt, account_id);
    if (store_id != INVALID_ID)
	stmtSetId(stmt, store_id);
    if (!start.isNull() && !end.isNull()) {
	stmtSetDate(stmt, start);
	stmtSetDate(stmt, end);
    } else if (!start.isNull()) {
	stmtSetDate(stmt, start);
    } else if (!end.isNull()) {
	stmtSetDate(stmt, end);
    }

    if (!execute(stmt)) return;
    while (stmt.next()) {
	Gltx gltx;
	int next = 1;
	selectData(gltx, stmt, next);
	selectGltx(gltx, stmt, next);
	gltx.setDataType(DataObject::DataType(stmtGetInt(stmt, next++)));
	gltxs.push_back(gltx);
	amounts.push_back(stmtGetFixed(stmt, next++));
    }

    commit();
}

// Select based on a given card id.  Returns just the base Gltx data
//sorted by date and number.  Used for inquiry window.
void
QuasarDB::selectCard(Id card_id, Id store_id, QDate start, QDate end,
		     vector<Gltx>& gltxs, vector<fixed>& amounts)
{
    gltxs.clear();
    amounts.clear();

    QString table = "gltx join gltx_card on gltx.gltx_id = "
	"gltx_card.gltx_id";
    QString cmd = selectCmd(table, "gltx.gltx_id", "gltx.number,"
			    "reference_str,post_date,post_time,"
			    "gltx.memo,station_id,employee_id,"
			    "gltx.card_id,store_id,shift_id,link_id,"
			    "printed,paid,gltx.amount,data_type,"
			    "gltx_card.amount", Select());
    cmd += " where gltx_card.card_id = ?";
    if (store_id != INVALID_ID)
	cmd += " and store_id = ?";
    if (!start.isNull() && !end.isNull())
	cmd += " and post_date between ? and ?";
    else if (!start.isNull())
	cmd += " and post_date >= ?";
    else if (!end.isNull())
	cmd += " and post_date <= ?";
    cmd += " order by post_date, gltx.number";

    Stmt stmt(_connection, cmd);
    stmtSetId(stmt, card_id);
    if (store_id != INVALID_ID)
	stmtSetId(stmt, store_id);
    if (!start.isNull() && !end.isNull()) {
	stmtSetDate(stmt, start);
	stmtSetDate(stmt, end);
    } else if (!start.isNull()) {
	stmtSetDate(stmt, start);
    } else if (!end.isNull()) {
	stmtSetDate(stmt, end);
    }

    if (!execute(stmt)) return;
    while (stmt.next()) {
	Gltx gltx;
	int next = 1;
	selectData(gltx, stmt, next);
	selectGltx(gltx, stmt, next);
	gltx.setDataType(DataObject::DataType(stmtGetInt(stmt, next++)));
	gltxs.push_back(gltx);
	amounts.push_back(stmtGetFixed(stmt, next++));
    }

    commit();
}

// Select based on a given item id.  Returns Gltx sorted by date and
// number.  Used for inquiry window.
void
QuasarDB::selectItem(Id item_id, Id store_id, QDate start, QDate end,
		     vector<Gltx>& gltxs, vector<fixed>& quantities,
		     vector<fixed>& ext_costs, vector<fixed>& ext_prices,
		     vector<bool>& void_flags)
{
    gltxs.clear();
    quantities.clear();
    ext_costs.clear();
    ext_prices.clear();
    void_flags.clear();

    QString table = "gltx join gltx_item on gltx.gltx_id = "
	"gltx_item.gltx_id";
    QString cmd = selectCmd(table, "gltx.gltx_id", "gltx.number,"
			    "reference_str,post_date,post_time,"
			    "gltx.memo,station_id,employee_id,"
			    "gltx.card_id,store_id,shift_id,link_id,"
			    "printed,paid,gltx.amount,data_type,"
			    "quantity,inv_cost,sale_price,voided",
			    Select());
    cmd += " where item_id = ?";
    if (store_id != INVALID_ID)
	cmd += " and store_id = ?";
    if (!start.isNull() && !end.isNull())
	cmd += " and post_date between ? and ?";
    else if (!start.isNull())
	cmd += " and post_date >= ?";
    else if (!end.isNull())
	cmd += " and post_date <= ?";
    cmd += " order by post_date, gltx.number";

    Stmt stmt(_connection, cmd);
    stmtSetId(stmt, item_id);
    if (store_id != INVALID_ID)
	stmtSetId(stmt, store_id);
    if (!start.isNull() && !end.isNull()) {
	stmtSetDate(stmt, start);
	stmtSetDate(stmt, end);
    } else if (!start.isNull()) {
	stmtSetDate(stmt, start);
    } else if (!end.isNull()) {
	stmtSetDate(stmt, end);
    }

    if (!execute(stmt)) return;
    while (stmt.next()) {
	Gltx gltx;
	int next = 1;
	selectData(gltx, stmt, next);
	selectGltx(gltx, stmt, next);
	gltx.setDataType(DataObject::DataType(stmtGetInt(stmt, next++)));
	gltxs.push_back(gltx);
	quantities.push_back(stmtGetFixed(stmt, next++));
	ext_costs.push_back(stmtGetFixed(stmt, next++));
	ext_prices.push_back(stmtGetFixed(stmt, next++));
	void_flags.push_back(stmtGetBool(stmt, next++));
    }

    commit();
}
