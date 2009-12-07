// $Id: payout_db.cpp,v 1.9 2005/03/01 19:59:42 bpepers Exp $
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

#include "payout.h"
#include "payout_select.h"
#include "expense.h"
#include "tender.h"
#include "tax.h"

// Create a Payout
bool
QuasarDB::create(Payout& tx)
{
    if (!validate(tx)) return false;
    if (!create((Gltx&)tx)) return false;
    if (!sqlCreateLines(tx)) return false;

    commit();
    dataSignal(DataEvent::Insert, tx);
    return true;
}

// Delete a Payout
bool
QuasarDB::remove(const Payout& tx)
{
    if (tx.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(tx)) return false;
    if (!remove((Gltx&)tx)) return false;

    commit();
    dataSignal(DataEvent::Delete, tx);
    return true;
}

// Update a Payout
bool
QuasarDB::update(const Payout& orig, Payout& tx)
{
    if (orig.id() == INVALID_ID || tx.id() == INVALID_ID) return false;
    if (!validate(tx)) return false;
    if (!update(orig, (Gltx&)tx)) return false;
    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(tx)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Payout.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id payout_id, Payout& payout)
{
    if (payout_id == INVALID_ID) return false;
    PayoutSelect conditions;
    vector<Payout> payouts;

    conditions.id = payout_id;
    if (!select(payouts, conditions)) return false;
    if (payouts.size() != 1) return false;

    payout = payouts[0];
    return true;
}

// Returns a vector of Payouts
bool
QuasarDB::select(vector<Payout>& payouts, const PayoutSelect& conditions)
{
    payouts.clear();

    QString cmd = gltxCmd("gltx", "gltx_id", "", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Payout payout;
	int next = 1;
	selectData(payout, stmt, next);
	selectGltx(payout, stmt, next);
	payouts.push_back(payout);
    }

    QString cmd1 = "select expense_id,amount,tax_id,voided from "
	"payout_expense where gltx_id = ? order by seq_num";
    Stmt stmt1(_connection, cmd1);

    for (unsigned int i = 0; i < payouts.size(); ++i) {
	Id gltx_id = payouts[i].id();
	GLTX_ACCOUNTS(payouts);
	GLTX_TAXES(payouts);
	GLTX_CARDS(payouts);
	GLTX_PAYMENTS(payouts);
	GLTX_TENDERS(payouts);

	stmtSetId(stmt1, gltx_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    ExpenseLine line;
	    line.expense_id = stmtGetId(stmt1, 1);
	    line.amount = stmtGetFixed(stmt1, 2);
	    line.tax_id = stmtGetId(stmt1, 3);
	    line.voided = stmtGetBool(stmt1, 4);
	    payouts[i].expenses().push_back(line);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Payout& payout)
{
    for (unsigned int i = 0; i < payout.expenses().size(); ++i) {
	Id expense_id = payout.expenses()[i].expense_id;
	fixed amount = payout.expenses()[i].amount;
	Id tax_id = payout.expenses()[i].tax_id;

	Expense expense;
	if (!lookup(expense_id, expense))
	    return "Expense doesn't exist";

	if (amount < 0.0)
	    return "Invalid amount";

	if (tax_id != INVALID_ID) {
	    Tax tax;
	    if (!lookup(tax_id, tax))
		return error("Tax doesn't exist");
	}
    }

    if (payout.accounts().size() > 0 && payout.isActive())
	if (payout.tenderTotal() != payout.total())
	    return error("Incomplete tendering");

    return validate((const Gltx&)payout);
}

bool
QuasarDB::sqlCreateLines(const Payout& tx)
{
    QString cmd = insertText("payout_expense", "gltx_id", "seq_num,"
			     "expense_id,amount,tax_id,voided");
    Stmt stmt(_connection, cmd);

    const vector<ExpenseLine>& expenses = tx.expenses();
    for (unsigned int i = 0; i < expenses.size(); ++i) {
	const ExpenseLine& line = expenses[i];

	stmtSetId(stmt, tx.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.expense_id);
	stmtSetFixed(stmt, line.amount);
	stmtSetId(stmt, line.tax_id);
	stmtSetBool(stmt, line.voided);
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const Payout& tx)
{
    return remove(tx, "payout_expense", "gltx_id");
}
