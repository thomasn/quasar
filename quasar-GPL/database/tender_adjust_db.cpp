// $Id: tender_adjust_db.cpp,v 1.10 2005/03/01 19:59:42 bpepers Exp $
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

#include "tender_adjust.h"
#include "tender_adjust_select.h"
#include "tender.h"
#include "account.h"

// Create a TenderAdjust
bool
QuasarDB::create(TenderAdjust& adjustment)
{
    if (!validate(adjustment)) return false;
    if (!create((Gltx&)adjustment)) return false;

    QString cmd = insertText("tender_adjust", "tender_adjust_id",
			     "account_id");
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, adjustment.id());
    stmtSetId(stmt, adjustment.accountId());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, adjustment);
    return true;
}

// Delete a TenderAdjust
bool
QuasarDB::remove(const TenderAdjust& adjustment)
{
    if (adjustment.id() == INVALID_ID) return false;
    if (!remove((Gltx&)adjustment)) return false;

    commit();
    dataSignal(DataEvent::Delete, adjustment);
    return true;
}

// Update a TenderAdjust
bool
QuasarDB::update(const TenderAdjust& orig, TenderAdjust& adjustment)
{
    if (orig.id() == INVALID_ID || adjustment.id() == INVALID_ID) return false;
    if (!validate(adjustment)) return false;
    if (!update(orig, (Gltx&)adjustment)) return false;

    // Update the tender_adjust table
    QString cmd = updateText("tender_adjust", "tender_adjust_id",
			     "account_id");
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, adjustment.accountId());
    stmtSetId(stmt, adjustment.id());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a TenderAdjust.
bool
QuasarDB::lookup(Id adjustment_id, TenderAdjust& adjustment)
{
    if (adjustment_id == INVALID_ID) return false;
    TenderAdjustSelect conditions;
    vector<TenderAdjust> adjustments;

    conditions.id = adjustment_id;
    if (!select(adjustments, conditions)) return false;
    if (adjustments.size() != 1) return false;

    adjustment = adjustments[0];
    return true;
}

// Returns a vector of TenderAdjusts.
bool
QuasarDB::select(vector<TenderAdjust>& adjusts, const TenderAdjustSelect& sel)
{
    adjusts.clear();

    QString cmd = gltxCmd("tender_adjust", "tender_adjust_id",
			  "account_id", sel);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	TenderAdjust adjustment;
	int next = 1;
	selectData(adjustment, stmt, next);
	selectGltx(adjustment, stmt, next);
	adjustment.setAccountId(stmtGetId(stmt, next++));
	adjusts.push_back(adjustment);
    }

    for (unsigned int i = 0; i < adjusts.size(); ++i) {
	Id gltx_id = adjusts[i].id();
	GLTX_ACCOUNTS(adjusts);
	GLTX_TENDERS(adjusts);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const TenderAdjust& adjustment)
{
    if (adjustment.accountId() != INVALID_ID) {
	Account account;
	if (!lookup(adjustment.accountId(), account))
	    return error("Expense account doesn't exist");
	if (account.isHeader())
	    return error("Expense account is a header");
    } else if (adjustment.total() != 0.0 && adjustment.isActive()) {
	return error("Expense account is required");
    }

    for (unsigned int i = 0; i < adjustment.tenders().size(); ++i) {
	const TenderLine& line = adjustment.tenders()[i];
	if (line.voided) continue;

	Tender tender;
	if (!lookup(line.tender_id, tender))
	    return error("Unknown tender");
    }

    return validate((Gltx&)adjustment);
}
