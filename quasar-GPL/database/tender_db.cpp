// $Id: tender_db.cpp,v 1.32 2005/03/01 19:59:42 bpepers Exp $
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

#include "tender.h"
#include "tender_select.h"
#include "account.h"

// Create a Tender
bool
QuasarDB::create(Tender& tender)
{
    if (!validate(tender)) return false;

    // Auto allocate menu number
    if (tender.menuNumber() == -1) {
	fixed number = uniqueNumber("tender", "menu_num");
	tender.setMenuNumber(number.toInt());
    }

    QString cmd = insertCmd("tender", "tender_id", "name,tender_type,"
			    "limit_amt,convert_rate,over_tender,open_drawer,"
			    "force_amt,second_rcpt,account_id,menu_num,"
			    "safe_id,bank_id");
    Stmt stmt(_connection, cmd);

    insertData(tender, stmt);
    stmtSetString(stmt, tender.name());
    stmtSetInt(stmt, tender.type());
    stmtSetFixed(stmt, tender.limit());
    stmtSetFixed(stmt, tender.convertRate());
    stmtSetBool(stmt, tender.overTender());
    stmtSetBool(stmt, tender.openDrawer());
    stmtSetBool(stmt, tender.forceAmount());
    stmtSetBool(stmt, tender.secondReceipt());
    stmtSetId(stmt, tender.accountId());
    stmtSetInt(stmt, tender.menuNumber());
    stmtSetId(stmt, tender.safeId());
    stmtSetId(stmt, tender.bankId());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(tender)) return false;

    commit();
    dataSignal(DataEvent::Insert, tender);
    return true;
}

// Delete a Tender
bool
QuasarDB::remove(const Tender& tender)
{
    if (tender.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(tender)) return false;
    if (!removeData(tender, "tender", "tender_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, tender);
    return true;
}

// Update a Tender
bool
QuasarDB::update(const Tender& orig, Tender& tender)
{
    if (orig.id() == INVALID_ID || tender.id() == INVALID_ID) return false;
    if (!validate(tender)) return false;

    // Update the tender table
    QString cmd = updateCmd("tender", "tender_id", "name,tender_type,"
			    "limit_amt,convert_rate,over_tender,open_drawer,"
			    "force_amt,second_rcpt,account_id,menu_num,"
			    "safe_id,bank_id");
    Stmt stmt(_connection, cmd);

    updateData(orig, tender, stmt);
    stmtSetString(stmt, tender.name());
    stmtSetInt(stmt, tender.type());
    stmtSetFixed(stmt, tender.limit());
    stmtSetFixed(stmt, tender.convertRate());
    stmtSetBool(stmt, tender.overTender());
    stmtSetBool(stmt, tender.openDrawer());
    stmtSetBool(stmt, tender.forceAmount());
    stmtSetBool(stmt, tender.secondReceipt());
    stmtSetId(stmt, tender.accountId());
    stmtSetInt(stmt, tender.menuNumber());
    stmtSetId(stmt, tender.safeId());
    stmtSetId(stmt, tender.bankId());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");
    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(tender)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Tender.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id tender_id, Tender& tender)
{
    if (tender_id == INVALID_ID) return false;
    TenderSelect conditions;
    vector<Tender> tenders;

    conditions.id = tender_id;
    if (!select(tenders, conditions)) return false;
    if (tenders.size() != 1) return false;

    tender = tenders[0];
    return true;
}

// Lookup a Tender.  This is just a shortcut to using select.
bool
QuasarDB::lookup(const QString& name, Tender& tender)
{
    if (name.isEmpty()) return false;
    TenderSelect conditions;
    vector<Tender> tenders;

    conditions.name = name;
    if (!select(tenders, conditions)) return false;
    if (tenders.size() != 1) return false;

    tender = tenders[0];
    return true;
}

// Returns a vector of Tenders.  Returns tenders sorted by name.
bool
QuasarDB::select(vector<Tender>& tenders, const TenderSelect& conditions)
{
    tenders.clear();

    QString cmd = selectCmd("tender", "tender_id", "name,tender_type,"
			    "limit_amt,convert_rate,over_tender,open_drawer,"
			    "force_amt,second_rcpt,account_id,menu_num,"
			    "safe_id,bank_id", conditions, "menu_num");
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Tender tender;
	int next = 1;
	selectData(tender, stmt, next);
	tender.setName(stmtGetString(stmt, next++));
	tender.setType(Tender::Type(stmtGetInt(stmt, next++)));
	tender.setLimit(stmtGetFixed(stmt, next++));
	tender.setConvertRate(stmtGetFixed(stmt, next++));
	tender.setOverTender(stmtGetBool(stmt, next++));
	tender.setOpenDrawer(stmtGetBool(stmt, next++));
	tender.setForceAmount(stmtGetBool(stmt, next++));
	tender.setSecondReceipt(stmtGetBool(stmt, next++));
	tender.setAccountId(stmtGetId(stmt, next++));
	tender.setMenuNumber(stmtGetInt(stmt, next++));
	tender.setSafeId(stmtGetId(stmt, next++));
	tender.setBankId(stmtGetId(stmt, next++));
	tenders.push_back(tender);
    }

    QString cmd1 = "select range_start,range_end from tender_range "
	"where tender_id = ? order by seq_num";
    Stmt stmt1(_connection, cmd1);

    QString cmd2 = "select name,multiplier from tender_denom "
	"where tender_id = ? order by seq_num";
    Stmt stmt2(_connection, cmd2);

    QString cmd3 = "select ends_in,change from tender_round "
	"where tender_id = ? order by seq_num";
    Stmt stmt3(_connection, cmd3);

    for (unsigned int i = 0; i < tenders.size(); ++i) {
	Id tender_id = tenders[i].id();

	stmtSetId(stmt1, tender_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    TenderRange line;
	    int next = 1;
	    line.range_start = stmtGetString(stmt1, next++);
	    line.range_end = stmtGetString(stmt1, next++);
	    tenders[i].ranges().push_back(line);
	}

	stmtSetId(stmt2, tender_id);
	if (!execute(stmt2)) return false;
	while (stmt2.next()) {
	    TenderDenom line;
	    int next = 1;
	    line.name = stmtGetString(stmt2, next++);
	    line.multiplier = stmtGetFixed(stmt2, next++);
	    tenders[i].denominations().push_back(line);
	}

	stmtSetId(stmt3, tender_id);
	if (!execute(stmt3)) return false;
	while (stmt3.next()) {
	    TenderRound line;
	    int next = 1;
	    line.ends_in = stmtGetString(stmt3, next++);
	    line.change = stmtGetFixed(stmt3, next++);
	    tenders[i].rounding().push_back(line);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Tender& tender)
{
    if (!validate((DataObject&)tender)) return false;

    if (tender.name().stripWhiteSpace().isEmpty())
	return error("Blank tender name");

    Account account;
    if (!lookup(tender.accountId(), account))
	return error("Transaction account doesn't exist");
    if (account.isHeader())
	return error("Transaction account is a header");

    if (tender.safeId() != INVALID_ID) {
	Account account;
	if (!lookup(tender.safeId(), account))
	    return error("Safe account doesn't exist");
	if (account.isHeader())
	    return error("Safe account is a header");
    }

    if (tender.bankId() != INVALID_ID) {
	Account account;
	if (!lookup(tender.bankId(), account))
	    return error("Bank account doesn't exist");
	if (account.isHeader())
	    return error("Bank account is a header");
    }

    if (tender.limit() < 0.0)
	return error("Negative limit");

    if (tender.convertRate() <= 0.0)
	return error("Invalid conversion rate");

    return true;
}

bool
QuasarDB::sqlCreateLines(const Tender& tender)
{
    QString cmd = insertText("tender_range", "tender_id", "seq_num,"
			     "range_start,range_end");
    Stmt stmt(_connection, cmd);

    for (unsigned int i = 0; i < tender.ranges().size(); ++i) {
	const TenderRange& line = tender.ranges()[i];

	stmtSetId(stmt, tender.id());
	stmtSetInt(stmt, i);
	stmtSetString(stmt, line.range_start);
	stmtSetString(stmt, line.range_end);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("tender_denom", "tender_id", "seq_num,"
		     "name,multiplier");
    stmt.setCommand(cmd);

    for (unsigned int i = 0; i < tender.denominations().size(); ++i) {
	const TenderDenom& line = tender.denominations()[i];

	stmtSetId(stmt, tender.id());
	stmtSetInt(stmt, i);
	stmtSetString(stmt, line.name);
	stmtSetFixed(stmt, line.multiplier);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("tender_round", "tender_id", "seq_num,"
		     "ends_in,change");
    stmt.setCommand(cmd);

    for (unsigned int i = 0; i < tender.rounding().size(); ++i) {
	const TenderRound& line = tender.rounding()[i];

	stmtSetId(stmt, tender.id());
	stmtSetInt(stmt, i);
	stmtSetString(stmt, line.ends_in);
	stmtSetFixed(stmt, line.change);
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const Tender& tender)
{
    if (!remove(tender, "tender_range", "tender_id")) return false;
    if (!remove(tender, "tender_denom", "tender_id")) return false;
    if (!remove(tender, "tender_round", "tender_id")) return false;
    return true;
}
