// $Id: quasar_gl.cpp,v 1.35 2005/03/13 22:22:02 bpepers Exp $
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
#include "quasar_client.h"
#include "company.h"
#include "account.h"
#include "account_select.h"
#include "gltx_select.h"
#include "shift.h"
#include "tender_adjust.h"

typedef QValueList<Account> AccountList;

bool
QuasarDB::selectChart(vector<Account>& accounts, const AccountSelect& cond)
{
    QValueList<Account> account_list;

    QString cmd = selectCmd("account", "account_id", "name,number,"
			    "account_type,header,parent_id,"
			    "last_reconciled,next_number", cond,
			    "account_type,parent_id,number,name");
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Account account;
	int next = 1;
	selectData(account, stmt, next);
	account.setName(stmtGetString(stmt, next++));
	account.setNumber(stmtGetString(stmt, next++));
	account.setType(Account::Type(stmtGetInt(stmt, next++)));
	account.setHeader(stmtGetBool(stmt, next++));
	account.setParentId(stmtGetId(stmt, next++));
	account.setLastReconciled(stmtGetDate(stmt, next++));
	account.setNextNumber(stmtGetInt(stmt, next++));
	account_list.push_back(account);
    }

    cmd = "select group_id from account_group where account_id = ? "
	"order by seq_num";
    stmt.setCommand(cmd);

    AccountList::iterator curr = account_list.begin();
    for (; curr != account_list.end(); ++curr) {
	Account& account = *curr;

	stmtSetId(stmt, account.id());
	if (!execute(stmt)) return false;
	while (stmt.next()) {
	    Id group_id = stmtGetId(stmt, 1);
	    account.groups().push_back(group_id);
	}
    }

    commit();

    // Shuffle to move children below parents
    accounts.clear();
    do {
	AccountList::iterator curr = account_list.begin();
	AccountList::iterator next = account_list.begin();
	unsigned int startSize = account_list.size();
	for (; curr != account_list.end(); curr = next) {
	    ++next;
	    const Account& account = *curr;
	    Id parent = account.parentId();

	    // Top levels always at end
	    if (parent == INVALID_ID) {
		accounts.push_back(account);
		account_list.erase(curr);
		continue;
	    }

	    // Search back for place to insert
	    int after = int(accounts.size()) - 1;
	    while (after >= 0) {
		if (accounts[after].id() == parent) break;
		if (accounts[after].parentId() == parent) break;
		--after;
	    }

	    // If position not found, continue
	    if (after < 0) continue;

	    // If at end, just push_back
	    if (after == int(accounts.size()) - 1) {
		accounts.push_back(account);
		account_list.erase(curr);
		continue;
	    }

	    // Insert in place
	    accounts.insert(accounts.begin() + after + 1, account);
	    account_list.erase(curr);
	}

	if (startSize == account_list.size() && startSize != 0) {
	    qWarning("Error: invalid account heirarchy");
	    break;
	}
    } while (account_list.size() != 0);

    return true;
}

// Calculate net profit/loss as of a given date
fixed
QuasarDB::netProfitLoss(QDate date, Id store_id)
{
    fixed balance = 0.0;
    if (date.isNull()) return balance;

    QString cmd = "select sum(amount) from account_change join account "
	"on account_change.account_id = account.account_id where "
	"change_date <= ? and account_type >= ?";
    if (store_id != INVALID_ID)
	cmd += " and store_id = ?";
    Stmt stmt(_connection, cmd);

    stmtSetDate(stmt, date);
    stmtSetInt(stmt, Account::Income);
    if (store_id != INVALID_ID)
	stmtSetId(stmt, store_id);

    if (!execute(stmt)) return balance;
    if (stmt.next())
	balance = stmtGetFixed(stmt, 1);

    return balance;
}

// Return balances of accounts on a given date
void
QuasarDB::accountBalances(QDate date, vector<Id>& account_ids,
			  vector<fixed>& balances)
{
    if (date.isNull()) date = QDate::currentDate();
    account_ids.clear();
    balances.clear();

    QString cmd = "select account_id,sum(amount) from account_change "
	"where change_date <= ? group by account_id";
    Stmt stmt(_connection, cmd);
    stmtSetDate(stmt, date);
    if (!execute(stmt)) return;
    while (stmt.next()) {
	account_ids.push_back(stmtGetId(stmt, 1));
	balances.push_back(stmtGetFixed(stmt, 2));
    }
}

// Close a shift by marking all the transactions
bool
QuasarDB::shiftClose(const Shift& shift)
{
    QString cmd = "update gltx set shift_id = ? where data_type != ? "
	"and store_id = ? and shift_id is null and post_date <= ? ";
    if (shift.employeeId() != INVALID_ID)
	cmd += " and employee_id = ?";
    else if (shift.stationId() != INVALID_ID)
	cmd += " and station_id = ?";
    else {
	Company company;
	lookup(company);
	if (company.shiftMethod() == Company::BY_STATION) {
	    cmd += " and station_id is null";
	} else {
	    cmd += " and employee_id is null";
	}
    }
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, shift.id());
    stmtSetInt(stmt, DataObject::SHIFT);
    stmtSetId(stmt, shift.storeId());
    stmtSetDate(stmt, shift.postDate());
    if (shift.employeeId() != INVALID_ID)
	stmtSetId(stmt, shift.employeeId());
    if (shift.stationId() != INVALID_ID)
	stmtSetId(stmt, shift.stationId());

    if (!execute(stmt)) return false;

    commit();
    return true;
}

// Re-open a shift by removing the adjustments and clearing the shift ids
bool
QuasarDB::shiftOpen(const Shift& shift)
{
    Id shift_id = shift.id();
    if (shift.shiftId() != shift_id)
	shift_id = shift.shiftId();

    QString cmd = "update shift set adjust_id = null, transfer_id = null "
	"where exists (select gltx.gltx_id from gltx join shift "
	"on gltx.gltx_id = shift.shift_id where gltx.shift_id = ?)";
    Stmt stmt1(_connection, cmd);
    stmtSetId(stmt1, shift_id);
    if (!execute(stmt1)) return false;

    cmd = "update gltx set shift_id = null where shift_id = ? and "
	"data_type = 23";
    Stmt stmt2(_connection, cmd);
    stmtSetId(stmt2, shift_id);
    if (!execute(stmt2)) return false;

    cmd = "update tender_count set shift_id = null where shift_id = ?";
    Stmt stmt3(_connection, cmd);
    stmtSetId(stmt3, shift_id);
    if (!execute(stmt3)) return false;

    if (shift.adjustmentId() != INVALID_ID) {
	Gltx adjustment;
	if (!lookup(shift.adjustmentId(), adjustment)) return false;
	if (!remove(adjustment)) return false;
    }
    if (shift.transferId() != INVALID_ID) {
	Gltx transfer;
	if (!lookup(shift.transferId(), transfer)) return false;
	if (!remove(transfer)) return false;
	if (!lookup(transfer.linkId(), transfer)) return false;
	if (!remove(transfer)) return false;
    }

    commit();
    return true;
}

bool
QuasarDB::safeOpenBalance(QDate date, vector<Id>& tenders, vector<fixed>& amts)
{
    tenders.clear();
    amts.clear();

    Company company;
    lookup(company);

    QString cmd = "select tender_id,sum(gltx_tender.amount) from "
	"gltx_tender join gltx on gltx_tender.gltx_id = gltx.gltx_id "
	"where inactive_on is null and post_date < ? and ";
    if (company.shiftMethod() == Company::BY_STATION)
	cmd += " station_id = ?";
    else
	cmd += " employee_id = ?";
    cmd += " group by tender_id";

    Stmt stmt(_connection, cmd);
    stmtSetDate(stmt, date);
    if (company.shiftMethod() == Company::BY_STATION)
	stmtSetId(stmt, company.safeStation());
    else
	stmtSetId(stmt, company.safeEmployee());
    if (!execute(stmt)) return false;

    while (stmt.next()) {
	Id tender_id = stmtGetId(stmt, 1);
	fixed amount = stmtGetFixed(stmt, 2);

	tenders.push_back(tender_id);
	amts.push_back(amount);
    }

    commit();
    return true;
}
