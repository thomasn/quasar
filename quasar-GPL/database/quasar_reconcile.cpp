// $Id: quasar_reconcile.cpp,v 1.17 2005/03/01 19:59:42 bpepers Exp $
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

void
QuasarDB::selectReconcile(Id account_id, QDate stmt_date, QDate end_date,
			  fixed& open_balance, vector<Id>& gltx_ids,
			  vector<int>& seq_nums)
{
    open_balance = 0.0;
    gltx_ids.clear();
    seq_nums.clear();

    QString cmd = "select sum(gltx_account.amount) from gltx_account "
	"join gltx on gltx_account.gltx_id = gltx.gltx_id where "
	"account_id = ? and cleared < ? and inactive_on is null";
    Stmt stmt(_connection, cmd);
    stmtSetId(stmt, account_id);
    stmtSetDate(stmt, stmt_date);
    if (!execute(stmt)) return;
    if (stmt.next())
	open_balance = stmtGetFixed(stmt, 1);

    cmd = "select gltx_account.gltx_id,seq_num from gltx join "
	"gltx_account on gltx.gltx_id = gltx_account.gltx_id "
	"where account_id = ? and (cleared is null or cleared = ?) "
	"and post_date <= ? and inactive_on is null order by "
	"post_date, number";

    stmt.setCommand(cmd);
    stmtSetId(stmt, account_id);
    stmtSetDate(stmt, stmt_date);
    stmtSetDate(stmt, end_date);
    if (!execute(stmt)) return;
    while (stmt.next()) {
	gltx_ids.push_back(stmtGetId(stmt, 1));
	seq_nums.push_back(stmtGetInt(stmt, 2));
    }
}
