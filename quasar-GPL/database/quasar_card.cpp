// $Id: quasar_card.cpp,v 1.16 2005/03/01 19:59:42 bpepers Exp $
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

#include "card.h"

// Calculate balance for a card
fixed
QuasarDB::cardBalance(Id card_id, QDate date, Id store_id)
{
    fixed balance = 0.0;
    if (date.isNull()) date = QDate::currentDate();

    QString cmd = "select sum(amount) from card_change where "
	"card_id = ? and change_date <= ?";
    if (store_id != INVALID_ID)
	cmd += " and store_id=?";
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, card_id);
    stmtSetDate(stmt, date);
    if (store_id != INVALID_ID)
	stmtSetId(stmt, store_id);

    if (!execute(stmt)) return balance;
    if (stmt.next())
	balance = stmtGetFixed(stmt, 1);

    return balance;
}

// Return balances of cards on a given date
void
QuasarDB::cardBalances(QDate date, Id store_id, vector<Id>& card_ids,
		       vector<fixed>& balances)
{
    if (date.isNull()) date = QDate::currentDate();
    card_ids.clear();
    balances.clear();

    QString cmd = "select card_id,sum(amount) from card_change "
	"where change_date <= ?";
    if (store_id != INVALID_ID)
	cmd += " and store_id = ?";
    cmd += " group by card_id order by card_id";
    Stmt stmt(_connection, cmd);

    stmtSetDate(stmt, date);
    if (store_id != INVALID_ID)
	stmtSetId(stmt, store_id);
    if (!execute(stmt)) return;
    while (stmt.next()) {
	card_ids.push_back(stmtGetId(stmt, 1));
	balances.push_back(stmtGetFixed(stmt, 2));
    }
}

// Adjust the balance of a card
bool
QuasarDB::cardAdjust(Id card_id, Id store_id, QDate date, fixed amount)
{
    if (amount == 0.0) return true;
    if (date.isNull()) date = QDate::currentDate();

    fixed debit_amt = (amount > 0.0) ? amount : 0.0;
    fixed credit_amt = (amount < 0.0) ? amount : 0.0;

    QString cmd = "select card_id from card_change where card_id = ? "
	"and store_id = ? and change_date = ?";
    Stmt stmt(_connection, cmd);
    stmtSetId(stmt, card_id);
    stmtSetId(stmt, store_id);
    stmtSetDate(stmt, date);

    execute(stmt);
    bool found = stmt.next();

    if (found)
	cmd = "update card_change set amount = amount + ?,"
	    "debit_amt = debit_amt + ?, credit_amt = credit_amt + ? "
	    "where card_id = ? and store_id = ? and change_date = ?";
    else
	cmd = "insert into card_change (amount,debit_amt,credit_amt,"
	    "card_id,store_id,change_date) values (?,?,?,?,?,?)";

    stmt.setCommand(cmd);
    stmtSetFixed(stmt, amount);
    stmtSetFixed(stmt, debit_amt);
    stmtSetFixed(stmt, credit_amt);
    stmtSetId(stmt, card_id);
    stmtSetId(stmt, store_id);
    stmtSetDate(stmt, date);

    execute(stmt);
    if (stmt.getUpdateCount() != 1)
	return error("invalid update count");

    return true;
}
