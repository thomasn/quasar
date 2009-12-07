// $Id: quasar_pat.cpp,v 1.8 2005/03/01 19:59:42 bpepers Exp $
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
#include "pat_worksheet.h"

void
QuasarDB::patronageSales(QDate from, QDate to, vector<Id>& customer_ids,
			 vector<Id>& dept_ids, vector<fixed>& sales_amts)
{
    customer_ids.clear();
    dept_ids.clear();
    sales_amts.clear();

    QString command = "select gltx.card_id,item.dept_id,"
	"sum(gltx_item.sale_price) from (gltx_item join gltx on "
	"gltx_item.gltx_id = gltx.gltx_id) join item on "
	"gltx_item.item_id = item.item_id where gltx.inactive_on is null "
	"and gltx.post_date >= ? and gltx.post_date <= ? and "
	"gltx_item.voided = 'N' group by gltx.card_id, item.dept_id";

    Stmt stmt(_connection, command);
    stmtSetDate(stmt, from);
    stmtSetDate(stmt, to);

    execute(stmt);
    while (stmt.next()) {
	customer_ids.push_back(stmtGetId(stmt, 1));
	dept_ids.push_back(stmtGetId(stmt, 2));
	sales_amts.push_back(stmtGetFixed(stmt, 3));
    }
}

void
QuasarDB::patronagePurchases(QDate from, QDate to, vector<Id>& vendor_ids,
			     vector<fixed>& purchase_amts)
{
    vendor_ids.clear();
    purchase_amts.clear();

    QString command = "select gltx.card_id,sum(gltx_item.inv_cost) from "
	"(receive_item join gltx_item on receive_item.receive_id = "
	"gltx_item.gltx_id) join gltx on receive_item.receive_id = "
	"gltx.gltx_id where gltx.inactive_on is null and gltx.post_date >= ? "
	"and gltx.post_date <= ? and gltx_item.voided = 'N' "
	"group by gltx.card_id";

    Stmt stmt(_connection, command);
    stmtSetDate(stmt, from);
    stmtSetDate(stmt, to);

    execute(stmt);
    while (stmt.next()) {
	vendor_ids.push_back(stmtGetId(stmt, 1));
	purchase_amts.push_back(stmtGetFixed(stmt, 2));
    }
}

void
QuasarDB::patronageBalances(QDate from, QDate to, vector<Id>& customer_ids,
			    vector<fixed>& change_amts)
{
    customer_ids.clear();
    change_amts.clear();

    QString command = "select card_customer.customer_id,sum(amount) from "
	"card_change join card_customer on card_change.card_id = "
	"card_customer.customer_id where change_date >= ? and "
	"change_date <= ? group by card_customer.customer_id";

    Stmt stmt(_connection, command);
    stmtSetDate(stmt, from);
    stmtSetDate(stmt, to);

    execute(stmt);
    while (stmt.next()) {
	customer_ids.push_back(stmtGetId(stmt, 1));
	change_amts.push_back(stmtGetFixed(stmt, 2));
    }
}
