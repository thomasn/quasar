// $Id: pat_worksheet_db.cpp,v 1.9 2005/03/01 19:59:42 bpepers Exp $
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
#include "pat_worksheet_select.h"

// Create a PatWorksheet
bool
QuasarDB::create(PatWorksheet& worksheet)
{
    if (!validate(worksheet)) return false;

    QString cmd = insertCmd("pat_ws", "worksheet_id", "description,"
			    "from_date,to_date,equity_amt,credit_amt,"
			    "equity_id,credit_id,equity_memo,credit_memo,"
			    "purch_pnts,bal_pnts");
    Stmt stmt(_connection, cmd);

    insertData(worksheet, stmt);
    stmtSetString(stmt, worksheet.description());
    stmtSetDate(stmt, worksheet.fromDate());
    stmtSetDate(stmt, worksheet.toDate());
    stmtSetFixed(stmt, worksheet.equityAmount());
    stmtSetFixed(stmt, worksheet.creditAmount());
    stmtSetId(stmt, worksheet.equityId());
    stmtSetId(stmt, worksheet.creditId());
    stmtSetString(stmt, worksheet.equityMemo());
    stmtSetString(stmt, worksheet.creditMemo());
    stmtSetFixed(stmt, worksheet.purchasePoints());
    stmtSetFixed(stmt, worksheet.balancePoints());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(worksheet)) return false;

    commit();
    dataSignal(DataEvent::Insert, worksheet);
    return true;
}

// Delete a PatWorksheet
bool
QuasarDB::remove(const PatWorksheet& worksheet)
{
    if (worksheet.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(worksheet)) return false;
    if (!removeData(worksheet, "pat_ws", "worksheet_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, worksheet);
    return true;
}

// Update a PatWorksheet
bool
QuasarDB::update(const PatWorksheet& orig, PatWorksheet& worksheet)
{
    if (orig.id() == INVALID_ID || worksheet.id() == INVALID_ID) return false;
    if (!validate(worksheet)) return false;

    // Update the pat_ws table
    QString cmd = updateCmd("pat_ws", "worksheet_id", "description,"
			    "from_date,to_date,equity_amt,credit_amt,"
			    "equity_id,credit_id,equity_memo,credit_memo,"
			    "purch_pnts,bal_pnts");
    Stmt stmt(_connection, cmd);

    updateData(orig, worksheet, stmt);
    stmtSetString(stmt, worksheet.description());
    stmtSetDate(stmt, worksheet.fromDate());
    stmtSetDate(stmt, worksheet.toDate());
    stmtSetFixed(stmt, worksheet.equityAmount());
    stmtSetFixed(stmt, worksheet.creditAmount());
    stmtSetId(stmt, worksheet.equityId());
    stmtSetId(stmt, worksheet.creditId());
    stmtSetString(stmt, worksheet.equityMemo());
    stmtSetString(stmt, worksheet.creditMemo());
    stmtSetFixed(stmt, worksheet.purchasePoints());
    stmtSetFixed(stmt, worksheet.balancePoints());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");
    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(worksheet)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a PatWorksheet.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id worksheet_id, PatWorksheet& worksheet)
{
    if (worksheet_id == INVALID_ID) return false;
    PatWorksheetSelect conditions;
    vector<PatWorksheet> worksheets;

    conditions.id = worksheet_id;
    if (!select(worksheets, conditions)) return false;
    if (worksheets.size() != 1) return false;

    worksheet = worksheets[0];
    return true;
}

// Returns a vector of PatWorksheets.  Returns PatWorksheets sorted by name.
bool
QuasarDB::select(vector<PatWorksheet>& worksheets, const PatWorksheetSelect& conditions)
{
    worksheets.clear();

    QString cmd = selectCmd("pat_ws", "worksheet_id", "description,"
			    "from_date,to_date,equity_amt,credit_amt,"
			    "equity_id,credit_id,equity_memo,credit_memo,"
			    "purch_pnts,bal_pnts", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	PatWorksheet worksheet;
	int next = 1;
	selectData(worksheet, stmt, next);
	worksheet.setDescription(stmtGetString(stmt, next++));
	worksheet.setFromDate(stmtGetDate(stmt, next++));
	worksheet.setToDate(stmtGetDate(stmt, next++));
	worksheet.setEquityAmount(stmtGetFixed(stmt, next++));
	worksheet.setCreditAmount(stmtGetFixed(stmt, next++));
	worksheet.setEquityId(stmtGetId(stmt, next++));
	worksheet.setCreditId(stmtGetId(stmt, next++));
	worksheet.setEquityMemo(stmtGetString(stmt, next++));
	worksheet.setCreditMemo(stmtGetString(stmt, next++));
	worksheet.setPurchasePoints(stmtGetFixed(stmt, next++));
	worksheet.setBalancePoints(stmtGetFixed(stmt, next++));
	worksheets.push_back(worksheet);
    }

    QString cmd1 = "select customer_id,patgroup_id,dept_id,sales_amt "
	"from pat_ws_sales where worksheet_id = ? order by seq_num";
    Stmt stmt1(_connection, cmd1);

    QString cmd2 = "select vendor_id,patgroup_id,purchase_amt from "
	"pat_ws_purch where worksheet_id = ? order by seq_num";
    Stmt stmt2(_connection, cmd2);

    QString cmd3 = "select customer_id,patgroup_id,open_amt,close_amt "
	"from pat_ws_bal where worksheet_id = ? order by seq_num";
    Stmt stmt3(_connection, cmd3);

    QString cmd4 = "select patgroup_id,points,reason from "
	"pat_ws_adjust where worksheet_id = ? order by seq_num";
    Stmt stmt4(_connection, cmd4);

    for (unsigned int i = 0; i < worksheets.size(); ++i) {
	Id worksheet_id = worksheets[i].id();

	stmtSetId(stmt1, worksheet_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    PatSales line;
	    int next = 1;
	    line.customer_id = stmtGetId(stmt1, next++);
	    line.patgroup_id = stmtGetId(stmt1, next++);
	    line.dept_id = stmtGetId(stmt1, next++);
	    line.sales_amt = stmtGetFixed(stmt1, next++);
	    worksheets[i].sales().push_back(line);
	}

	stmtSetId(stmt2, worksheet_id);
	if (!execute(stmt2)) return false;
	while (stmt2.next()) {
	    PatPurch line;
	    int next = 1;
	    line.vendor_id = stmtGetId(stmt2, next++);
	    line.patgroup_id = stmtGetId(stmt2, next++);
	    line.purchase_amt = stmtGetFixed(stmt2, next++);
	    worksheets[i].purchases().push_back(line);
	}

	stmtSetId(stmt3, worksheet_id);
	if (!execute(stmt3)) return false;
	while (stmt3.next()) {
	    PatBalance line;
	    int next = 1;
	    line.customer_id = stmtGetId(stmt3, next++);
	    line.patgroup_id = stmtGetId(stmt3, next++);
	    line.open_amt = stmtGetFixed(stmt3, next++);
	    line.close_amt = stmtGetFixed(stmt3, next++);
	    worksheets[i].balances().push_back(line);
	}

	stmtSetId(stmt4, worksheet_id);
	if (!execute(stmt4)) return false;
	while (stmt4.next()) {
	    PatAdjust line;
	    int next = 1;
	    line.patgroup_id = stmtGetId(stmt4, next++);
	    line.points = stmtGetFixed(stmt4, next++);
	    line.reason = stmtGetString(stmt4, next++);
	    worksheets[i].adjusts().push_back(line);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const PatWorksheet& worksheet)
{
    if (!validate((DataObject&)worksheet)) return false;

    if (worksheet.description().isEmpty())
	return error("Description is manditory");
    if (worksheet.fromDate().isNull())
	return error("From date is manditory");
    if (worksheet.toDate().isNull())
	return error("To date is manditory");

    if (worksheet.equityAmount() != 0.0) {
	if (worksheet.equityId() == INVALID_ID)
	    return error("Equity account is manditory");
	if (worksheet.equityMemo().isEmpty())
	    return error("Equity memo is manditory");
    }

    if (worksheet.creditAmount() != 0.0) {
	if (worksheet.creditId() == INVALID_ID)
	    return error("Credit account is manditory");
	if (worksheet.creditMemo().isEmpty())
	    return error("Credit memo is manditory");
    }

    return true;
}

bool
QuasarDB::sqlCreateLines(const PatWorksheet& worksheet)
{
    QString cmd = insertText("pat_ws_sales", "worksheet_id", "seq_num,"
			     "customer_id,patgroup_id,dept_id,sales_amt");
    Stmt stmt(_connection, cmd);

    unsigned int i;
    for (i = 0; i < worksheet.sales().size(); ++i) {
	const PatSales& line = worksheet.sales()[i];

	stmtSetId(stmt, worksheet.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.customer_id);
	stmtSetId(stmt, line.patgroup_id);
	stmtSetId(stmt, line.dept_id);
	stmtSetFixed(stmt, line.sales_amt);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("pat_ws_purch", "worksheet_id", "seq_num,vendor_id,"
		     "patgroup_id,purchase_amt");
    stmt.setCommand(cmd);

    for (i = 0; i < worksheet.purchases().size(); ++i) {
	const PatPurch& line = worksheet.purchases()[i];

	stmtSetId(stmt, worksheet.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.vendor_id);
	stmtSetId(stmt, line.patgroup_id);
	stmtSetFixed(stmt, line.purchase_amt);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("pat_ws_bal", "worksheet_id", "seq_num,customer_id,"
		     "patgroup_id,open_amt,close_amt");
    stmt.setCommand(cmd);

    for (i = 0; i < worksheet.balances().size(); ++i) {
	const PatBalance& line = worksheet.balances()[i];

	stmtSetId(stmt, worksheet.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.customer_id);
	stmtSetId(stmt, line.patgroup_id);
	stmtSetFixed(stmt, line.open_amt);
	stmtSetFixed(stmt, line.close_amt);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("pat_ws_adjust", "worksheet_id", "seq_num,patgroup_id,"
		     "points,reason");
    stmt.setCommand(cmd);

    for (i = 0; i < worksheet.adjusts().size(); ++i) {
	const PatAdjust& line = worksheet.adjusts()[i];

	stmtSetId(stmt, worksheet.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.patgroup_id);
	stmtSetFixed(stmt, line.points);
	stmtSetString(stmt, line.reason);
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const PatWorksheet& worksheet)
{
    if (!remove(worksheet, "pat_ws_sales", "worksheet_id")) return false;
    if (!remove(worksheet, "pat_ws_purch", "worksheet_id")) return false;
    if (!remove(worksheet, "pat_ws_bal", "worksheet_id")) return false;
    if (!remove(worksheet, "pat_ws_adjust", "worksheet_id")) return false;
    return true;
}
