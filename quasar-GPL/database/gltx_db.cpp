// $Id: gltx_db.cpp,v 1.72 2005/03/26 01:25:56 bpepers Exp $
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
#include "gltx.h"
#include "gltx_select.h"
#include "account.h"
#include "company.h"
#include "tax.h"
#include "store.h"
#include "station.h"
#include "card.h"
#include "shift.h"
#include "money_valcon.h"

// Create a Gltx
bool
QuasarDB::create(Gltx& gltx)
{
    // Can't create transaction before close date
    Company company;
    lookup(company);
    QDate startOfYear = company.startOfYear();

    if (!company.closeDate().isNull()) {
	if (gltx.postDate() <= company.closeDate())
	    return error("Can't create transaction before close date");
    }
    if (!startOfYear.isNull() && company.lastYearClosed()) {
	if (gltx.postDate() < startOfYear)
	    return error("Can't post to last year");
    }

    // Verify station/employee are specified
    if (company.shiftMethod() == Company::BY_STATION) {
	if (gltx.stationId() == INVALID_ID) {
	    // Error if its a transaction type involving cashrec
	    switch (gltx.dataType()) {
	    case DataObject::INVOICE:
	    case DataObject::RECEIPT:
	    case DataObject::NOSALE:
	    case DataObject::PAYOUT:
	    case DataObject::RETURN:
	    case DataObject::SHIFT:
	    case DataObject::TEND_ADJUST:
	    case DataObject::WITHDRAW:
		return error("A station is required");
	    default:
		break;
	    }
	}
    }
    if (company.shiftMethod() == Company::BY_EMPLOYEE) {
	if (gltx.employeeId() == INVALID_ID) {
	    // Error if its a transaction type involving cashrec
	    switch (gltx.dataType()) {
	    case DataObject::INVOICE:
	    case DataObject::RECEIPT:
	    case DataObject::NOSALE:
	    case DataObject::PAYOUT:
	    case DataObject::RETURN:
	    case DataObject::SHIFT:
	    case DataObject::TEND_ADJUST:
	    case DataObject::WITHDRAW:
		return error("An employee is required");
	    default:
		break;
	    }
	}
    }

    // Check shift is not posted if set
    if (gltx.shiftId() != INVALID_ID) {
	Shift shift;
	lookup(gltx.shiftId(), shift);
	if (shift.shiftId() != INVALID_ID)
	    return error("Can't post to a posted shift");
    }

    // Check that change data exists and create if needed
    checkAccount(gltx);
    checkCard(gltx);
    checkItem(gltx);

    // Auto allocate transaction number
    if (gltx.number().stripWhiteSpace() == "#") {
	fixed number = uniqueTxNumber(gltx.dataType());
	gltx.setNumber(number.toString());
    }

    QString cmd = insertCmd("gltx", "gltx_id", "number,reference_str,"
			    "post_date,post_time,memo,station_id,"
			    "employee_id,card_id,store_id,shift_id,"
			    "link_id,printed,paid,amount,data_type");
    Stmt stmt(_connection, cmd);

    insertData(gltx, stmt);
    stmtSetString(stmt, gltx.number());
    stmtSetString(stmt, gltx.reference());
    stmtSetDate(stmt, gltx.postDate());
    stmtSetTime(stmt, gltx.postTime());
    stmtSetString(stmt, gltx.memo());
    stmtSetId(stmt, gltx.stationId());
    stmtSetId(stmt, gltx.employeeId());
    stmtSetId(stmt, gltx.cardId());
    stmtSetId(stmt, gltx.storeId());
    stmtSetId(stmt, gltx.shiftId());
    stmtSetId(stmt, gltx.linkId());
    stmtSetBool(stmt, gltx.printed());
    stmtSetBool(stmt, false);
    stmtSetFixed(stmt, gltx.total());
    stmtSetInt(stmt, gltx.dataType());
    execute(stmt);

    sqlCreateLines(gltx);

    if (!startOfYear.isNull() && gltx.postDate() < startOfYear &&
	    gltx.isActive()) {
	Store store;
	if (!lookup(gltx.storeId(), store)) return false;

	Gltx orig, transfer;
	if (!lookup(store.yearEndTransferId(), transfer)) return false;
	orig = transfer;

	fixed total = 0.0;
	for (unsigned int i = 0; i < gltx.accounts().size(); ++i) {
	    Account account;
	    lookup(gltx.accounts()[i].account_id, account);
	    if (account.type() < Account::Income) continue;

	    AccountLine line;
	    line.account_id = account.id();
	    line.amount = -gltx.accounts()[i].amount;
	    transfer.accounts().push_back(line);

	    total += line.amount;
	}

	if (total != 0.0) {
	    AccountLine line;
	    line.account_id = company.retainedEarnings();
	    line.amount = -total;
	    transfer.accounts().push_back(line);
	}

	if (transfer != orig)
	    if (!update(orig, transfer))
		return false;
    }

    return true;
}

// Delete a Gltx
bool
QuasarDB::remove(const Gltx& gltx)
{
    // Can't delete reconciled transaction
    for (unsigned int i = 0; i < gltx.accounts().size(); ++i)
	if (!gltx.accounts()[i].cleared.isNull())
	    return error("Can't delete reconciled transaction");

    // Can't delete transaction before close date
    Company company;
    lookup(company);
    if (!company.closeDate().isNull()) {
	if (gltx.postDate() <= company.closeDate())
	    return error("Can't delete transaction before close date");
    }
    if (!company.startOfYear().isNull()) {
	if (gltx.postDate() < company.startOfYear())
	    return error("Can't delete transaction in last year");
    }

    // Can't delete if in a posted shift
    if (gltx.shiftId() != INVALID_ID) {
	Shift shift;
	lookup(gltx.shiftId(), shift);
	if (shift.shiftId() != INVALID_ID)
	    return error("Can't delete from a posted shift");
    }

    sqlDeleteLines(gltx);
    removeData(gltx, "gltx", "gltx_id");

    return true;
}

// Update a Gltx
bool
QuasarDB::update(const Gltx& orig, Gltx& gltx)
{
    if (gltx.number() == "#")
	return error("Can't change number to '#'");

    if (orig.dataType() != gltx.dataType()) {
	bool allowed = false;
	switch (orig.dataType()) {
	case DataObject::INVOICE:
	    if (gltx.dataType() == DataObject::RETURN)
		allowed = true;
	    break;
	case DataObject::RETURN:
	    if (gltx.dataType() == DataObject::INVOICE)
		allowed = true;
	    break;
	case DataObject::RECEIVE:
	    if (gltx.dataType() == DataObject::CLAIM)
		allowed = true;
	    break;
	case DataObject::CLAIM:
	    if (gltx.dataType() == DataObject::RECEIVE)
		allowed = true;
	    break;
	default:
	    break;
	}

	if (!allowed)
	    return error("Can't change data type");
    }

    // Can't void or unvoid reconciled or shift posted transaction
    if (orig.isActive() != gltx.isActive()) {
	for (unsigned int i = 0; i < gltx.accounts().size(); ++i)
	    if (!gltx.accounts()[i].cleared.isNull())
		return error("Can't change reconciled transaction");

	if (gltx.shiftId() != INVALID_ID) {
	    Shift shift;
	    lookup(gltx.shiftId(), shift);
	    if (shift.shiftId() != INVALID_ID)
		return error("Can't change transaction from a posted shift");
	}
    }

    // If shift changed, neither can be posted
    if (orig.shiftId() != gltx.shiftId()) {
	if (orig.shiftId() != INVALID_ID) {
	    Shift shift;
	    lookup(orig.shiftId(), shift);
	    if (shift.shiftId() != INVALID_ID)
		return error("Can't change from a posted shift");
	}

	if (gltx.shiftId() != INVALID_ID) {
	    Shift shift;
	    lookup(gltx.shiftId(), shift);
	    if (shift.shiftId() != INVALID_ID)
		return error("Can't insert into a posted shift");
	}
    }

    // Can't change tenders if in a posted shift
    if (gltx.shiftId() != INVALID_ID && orig.tenders() != gltx.tenders()) {
	Shift shift;
	lookup(gltx.shiftId(), shift);
	if (shift.shiftId() != INVALID_ID)
	    return error("Can't change tenders if posted");
    }

    // Reconcile information must match.  Also copy over cleared date
    // when needed since it can be lost on changed transactions.
    for (unsigned int i = 0; i < orig.accounts().size(); ++i) {
	const AccountLine& line = orig.accounts()[i];
	if (line.cleared.isNull()) continue;

	if (i >= gltx.accounts().size())
	    return error("Invalid change to reconciled transaction");
	if (gltx.accounts()[i].account_id != line.account_id)
	    return error("Invalid change to reconciled transaction");
	if (gltx.accounts()[i].amount != line.amount)
	    return error("Invalid change to reconciled transaction");
	gltx.accounts()[i].cleared = line.cleared;
    }

    // Can't change transaction before close date
    Company company;
    lookup(company);
    if (!company.closeDate().isNull()) {
	if (gltx.postDate() <= company.closeDate())
	    return error("Can't change date to before close date");
	if (orig.postDate() <= company.closeDate())
	    return error("Can't change transaction before close date");
    }
    if (!company.startOfYear().isNull()) {
	if (gltx.postDate() < company.startOfYear())
	    return error("Can't change date into last year");
	if (orig.postDate() < company.startOfYear())
	    return error("Can't change transaction in last year");
    }

    // Verify station/employee are specified
    if (company.shiftMethod() == Company::BY_STATION) {
	if (gltx.stationId() == INVALID_ID) {
	    // Error if its a transaction type involving cashrec
	    switch (gltx.dataType()) {
	    case DataObject::INVOICE:
	    case DataObject::RECEIPT:
	    case DataObject::NOSALE:
	    case DataObject::PAYOUT:
	    case DataObject::RETURN:
	    case DataObject::SHIFT:
	    case DataObject::TEND_ADJUST:
	    case DataObject::WITHDRAW:
		return error("A station is required");
	    default:
		break;
	    }
	}
    }
    if (company.shiftMethod() == Company::BY_EMPLOYEE) {
	if (gltx.employeeId() == INVALID_ID) {
	    // Error if its a transaction type involving cashrec
	    switch (gltx.dataType()) {
	    case DataObject::INVOICE:
	    case DataObject::RECEIPT:
	    case DataObject::NOSALE:
	    case DataObject::PAYOUT:
	    case DataObject::RETURN:
	    case DataObject::SHIFT:
	    case DataObject::TEND_ADJUST:
	    case DataObject::WITHDRAW:
		return error("An employee is required");
	    default:
		break;
	    }
	}
    }

    // Check that change data exists and create if needed
    checkAccount(gltx);
    checkCard(gltx);
    checkItem(gltx);

    // NOTE: this doesn't include paid and shouldn't
    QString cmd = updateCmd("gltx", "gltx_id", "number,reference_str,"
			    "post_date,post_time,memo,station_id,"
			    "employee_id,card_id,store_id,shift_id,"
			    "link_id,printed,amount,data_type");
    Stmt stmt(_connection, cmd);

    updateData(orig, gltx, stmt);
    stmtSetString(stmt, gltx.number());
    stmtSetString(stmt, gltx.reference());
    stmtSetDate(stmt, gltx.postDate());
    stmtSetTime(stmt, gltx.postTime());
    stmtSetString(stmt, gltx.memo());
    stmtSetId(stmt, gltx.stationId());
    stmtSetId(stmt, gltx.employeeId());
    stmtSetId(stmt, gltx.cardId());
    stmtSetId(stmt, gltx.storeId());
    stmtSetId(stmt, gltx.shiftId());
    stmtSetId(stmt, gltx.linkId());
    stmtSetBool(stmt, gltx.printed());
    stmtSetFixed(stmt, gltx.total());
    stmtSetInt(stmt, gltx.dataType());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());
    execute(stmt);
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    sqlDeleteLines(orig);
    sqlCreateLines(gltx);

    return true;
}

// Lookup a Gltx.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id gltx_id, Gltx& gltx)
{
    if (gltx_id == INVALID_ID) return false;
    GltxSelect conditions;
    vector<Gltx> gltxs;

    conditions.id = gltx_id;
    if (!select(gltxs, conditions)) return false;
    if (gltxs.size() != 1) return false;

    gltx = gltxs[0];
    return true;
}

// Returns a count of Gltxs based on the conditions.
bool
QuasarDB::count(int& count, const GltxSelect& conditions)
{
    count = 0;
    QString cmd = "select count(*) from gltx " + conditions.where();
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next())
	count = stmtGetInt(stmt, 1);

    commit();
    return true;
}

// Returns a vector of Gltxs.
bool
QuasarDB::select(vector<Gltx>& gltxs, const GltxSelect& conditions)
{
    gltxs.clear();

    QString cmd = gltxCmd("gltx", "gltx_id", "data_type", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Gltx gltx;
	int next = 1;
	selectData(gltx, stmt, next);
	selectGltx(gltx, stmt, next);
	gltx.setDataType(DataObject::DataType(stmtGetInt(stmt, next++)));
	gltxs.push_back(gltx);
    }

    for (unsigned int i = 0; i < gltxs.size(); ++i) {
	Id gltx_id = gltxs[i].id();
	GLTX_ACCOUNTS(gltxs);
	GLTX_CARDS(gltxs);
	GLTX_TAXES(gltxs);
	GLTX_PAYMENTS(gltxs);
	GLTX_TENDERS(gltxs);
	GLTX_REFERENCES(gltxs);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Gltx& gltx)
{
    if (!validate((DataObject&)gltx)) return false;

    if (gltx.postDate().isNull())
	return error("Invalid posting date");

    Store store;
    if (gltx.storeId() == INVALID_ID)
	return error("Store is required");
    if (!lookup(gltx.storeId(), store))
	return error("Invalid store");

    Station station;
    if (gltx.stationId() != INVALID_ID) {
	if (!lookup(gltx.stationId(), station))
	    return error("Station doesn't exist");
    }

    if (gltx.employeeId() != INVALID_ID) {
	Card card;
	if (!lookup(gltx.employeeId(), card))
	    return error("Employee card doesn't exist");
	if (card.dataType() != DataObject::EMPLOYEE)
	    return error("Invalid employee card");
    }
    if (gltx.cardId() != INVALID_ID) {
	Card card;
	if (!lookup(gltx.cardId(), card))
	    return error("Card doesn't exist");
	// TODO: verify right type of card by tx type
    }

    if (gltx.dataType() == DataObject::GENERAL) {
	if (gltx.memo().stripWhiteSpace().isEmpty())
	    return error("Blank memo");
	if (gltx.accounts().size() < 2 && gltx.isActive())
	    return error("GL transactions need 2 or more lines");
    }

    fixed total = 0.0;
    for (unsigned int i = 0; i < gltx.accounts().size(); ++i) {
	const AccountLine& line = gltx.accounts()[i];
	Account account;
	if (!lookup(line.account_id, account))
	    return error("Unknown account in lines");
	if (account.isHeader())
	    return error("Header account in lines");

	fixed rounded = line.amount;
	rounded.moneyRound();
	if (rounded != line.amount) {
	    return error("Extra decimal places: " + account.name() +
			 "," + line.amount.toString());
	}

	//qDebug("%s: %f", account.name().latin1(), line.amount.toDouble());
	total += line.amount;
    }
    if (total != 0.0)
	return error("Transaction doesn't balance to zero: " +
		     MoneyValcon().format(total));

    return true;
}

bool
QuasarDB::sqlCreateLines(const Gltx& gltx)
{
    QString cmd = insertText("gltx_account", "gltx_id", "seq_num,"
			     "account_id,amount,memo,cleared");
    Stmt stmt(_connection, cmd);
    const vector<AccountLine>& accounts = gltx.accounts();
    unsigned int i;
    for (i = 0; i < accounts.size(); ++i) {
	Id account_id = accounts[i].account_id;
	fixed amount = accounts[i].amount;
	QString memo = accounts[i].memo;
	QDate cleared = accounts[i].cleared;

	stmtSetId(stmt, gltx.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, account_id);
	stmtSetFixed(stmt, amount);
	stmtSetString(stmt, memo);
	stmtSetDate(stmt, cleared);
	if (!execute(stmt)) return false;

	if (gltx.isActive())
	    accountAdjust(account_id, gltx.storeId(), gltx.postDate(), amount);
    }

    cmd = insertText("gltx_card", "gltx_id", "seq_num,card_id,amount");
    stmt.setCommand(cmd);
    const vector<CardLine>& cards = gltx.cards();
    for (i = 0; i < cards.size(); ++i) {
	Id card_id = cards[i].card_id;
	fixed amount = cards[i].amount;

	stmtSetId(stmt, gltx.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, card_id);
	stmtSetFixed(stmt, amount);
	if (!execute(stmt)) return false;

	if (gltx.isActive())
	    cardAdjust(card_id, gltx.storeId(), gltx.postDate(), amount);
    }

    cmd = insertText("gltx_tax", "gltx_id", "seq_num,tax_id,taxable,amount,"
		     "inc_taxable,inc_amount");
    stmt.setCommand(cmd);
    const vector<TaxLine>& taxes = gltx.taxes();
    for (i = 0; i < taxes.size(); ++i) {
	Id tax_id = taxes[i].tax_id;
	fixed taxable = taxes[i].taxable;
	fixed amount = taxes[i].amount;
	fixed inc_taxable = taxes[i].inc_taxable;
	fixed inc_amount = taxes[i].inc_amount;

	stmtSetId(stmt, gltx.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, tax_id);
	stmtSetFixed(stmt, taxable);
	stmtSetFixed(stmt, amount);
	stmtSetFixed(stmt, inc_taxable);
	stmtSetFixed(stmt, inc_amount);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("gltx_tender", "gltx_id", "seq_num,tender_id,"
		     "amount,convert_rate,convert_amt,card_num,"
		     "expiry_date,auth_num,is_change,voided");
    stmt.setCommand(cmd);
    const vector<TenderLine>& tenders = gltx.tenders();
    for (i = 0; i < tenders.size(); ++i) {
	const TenderLine& line = tenders[i];

	stmtSetId(stmt, gltx.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.tender_id);
	stmtSetFixed(stmt, line.amount);
	stmtSetFixed(stmt, line.conv_rate);
	stmtSetFixed(stmt, line.conv_amt);
	stmtSetString(stmt, line.card_num);
	stmtSetDate(stmt, line.expiry_date);
	stmtSetString(stmt, line.auth_num);
	stmtSetBool(stmt, line.is_change);
	stmtSetBool(stmt, line.voided);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("gltx_paid", "gltx_id", "seq_num,pay_id,amount,discount");
    stmt.setCommand(cmd);
    const vector<PaymentLine>& payments = gltx.payments();
    for (i = 0; i < payments.size(); ++i) {
	Id pay_id = payments[i].gltx_id;
	fixed amount = payments[i].amount;
	fixed discount = payments[i].discount;

	if (gltx.isActive()) {
	    stmtSetId(stmt, gltx.id());
	    stmtSetInt(stmt, i);
	    stmtSetId(stmt, pay_id);
	    stmtSetFixed(stmt, amount);
	    stmtSetFixed(stmt, discount);
	    if (!execute(stmt)) return false;
	}

	checkPaid(pay_id);
    }

    cmd = insertText("gltx_refs", "gltx_id", "seq_num,ref_name,ref_data");
    stmt.setCommand(cmd);
    for (i = 0; i < gltx.referenceName().size(); ++i) {
	stmtSetId(stmt, gltx.id());
	stmtSetInt(stmt, i);
	stmtSetString(stmt, gltx.referenceName()[i]);
	stmtSetString(stmt, gltx.referenceData()[i]);
	if (!execute(stmt)) return false;
    }

    if (gltx.isActive()) {
	checkPaid(gltx.id());
    } else {
	cmd = "update gltx set paid = ? where gltx_id = ?";
	stmt.setCommand(cmd);
	stmtSetBool(stmt, false);
	stmtSetId(stmt, gltx.id());
	if (!execute(stmt)) return false;
    }

    return true;
}

void
QuasarDB::checkPaid(Id gltx_id)
{
    fixed cardTotal = 0.0;
    QString cmd = "select sum(amount) from gltx_card where gltx_id = ?";
    Stmt stmt(_connection, cmd);
    stmtSetId(stmt, gltx_id);
    execute(stmt);
    if (stmt.next())
	cardTotal = stmtGetFixed(stmt, 1);

    fixed paidTotal = 0.0;
    cmd = "select sum(amount),sum(discount) from gltx_paid where gltx_id = ?";
    stmt.setCommand(cmd);
    stmtSetId(stmt, gltx_id);
    execute(stmt);
    if (stmt.next())
	paidTotal = stmtGetFixed(stmt, 1) + stmtGetFixed(stmt, 2);

    cmd = "select sum(amount),sum(discount) from gltx_paid where pay_id = ?";
    stmt.setCommand(cmd);
    stmtSetId(stmt, gltx_id);
    execute(stmt);
    if (stmt.next())
	paidTotal -= stmtGetFixed(stmt, 1) + stmtGetFixed(stmt, 2);

    cmd = "update gltx set paid = ? where gltx_id = ?";
    stmt.setCommand(cmd);
    stmtSetBool(stmt, cardTotal == paidTotal);
    stmtSetId(stmt, gltx_id);
    execute(stmt);
}

bool
QuasarDB::sqlDeleteLines(const Gltx& gltx)
{
    if (gltx.isActive()) {
	const vector<AccountLine>& accounts = gltx.accounts();
	unsigned int i;
	for (i = 0; i < accounts.size(); ++i) {
	    Id account_id = accounts[i].account_id;
	    fixed amount = accounts[i].amount;

	    accountAdjust(account_id, gltx.storeId(), gltx.postDate(),-amount);
	}

	const vector<CardLine>& cards = gltx.cards();
	for (i = 0; i < cards.size(); ++i) {
	    Id card_id = cards[i].card_id;
	    fixed amount = cards[i].amount;
	    cardAdjust(card_id, gltx.storeId(), gltx.postDate(), -amount);
	}
    }

    if (!remove(gltx, "gltx_account", "gltx_id")) return false;
    if (!remove(gltx, "gltx_card", "gltx_id")) return false;
    if (!remove(gltx, "gltx_tax", "gltx_id")) return false;
    if (!remove(gltx, "gltx_tender", "gltx_id")) return false;
    if (!remove(gltx, "gltx_refs", "gltx_id")) return false;

    QString cmd = "delete from gltx_paid where gltx_id = ? or pay_id = ?";
    Stmt stmt(_connection, cmd);
    stmtSetId(stmt, gltx.id());
    stmtSetId(stmt, gltx.id());
    if (!execute(stmt)) return false;

    if (gltx.isActive()) {
	const vector<PaymentLine>& payments = gltx.payments();
	for (unsigned int i = 0; i < payments.size(); ++i) {
	    Id gltx_id = payments[i].gltx_id;
	    checkPaid(gltx_id);
	}
	checkPaid(gltx.id());
    }

    return true;
}

// Allocate a payment between two gltx rows
bool
QuasarDB::allocPayment(Id from_id, Id to_id, fixed amount)
{
    QString cmd = "select max(seq_num) from gltx_paid where gltx_id = ?";
    Stmt stmt(_connection, cmd);
    stmtSetId(stmt, from_id);
    if (!execute(stmt)) return false;
    if (!stmt.next()) return false;
    int seq_num = stmtGetInt(stmt, 1);

    cmd = "insert into gltx_paid (gltx_id,seq_num,pay_id,amount,"
	"discount) values (?,?,?,?,?)";
    stmt.setCommand(cmd);
    stmtSetId(stmt, from_id);
    stmtSetInt(stmt, seq_num + 1);
    stmtSetId(stmt, to_id);
    stmtSetFixed(stmt, amount);
    stmtSetFixed(stmt, 0.0);
    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1) return false;

    checkPaid(from_id);
    checkPaid(to_id);

    return true;
}

void
QuasarDB::checkAccount(const Gltx& gltx)
{
    for (unsigned int i = 0; i < gltx.accounts().size(); ++i) {
	const AccountLine& line = gltx.accounts()[i];

	QString cmd = "select count(*) from account_change where "
	    "account_id = ? and store_id = ? and change_date = ?";
	Stmt stmt(_connection, cmd);
	stmtSetId(stmt, line.account_id);
	stmtSetId(stmt, gltx.storeId());
	stmtSetDate(stmt, gltx.postDate());
	execute(stmt);
	stmt.next();
	if (stmtGetInt(stmt, 1) != 0) continue;

	cmd = "insert into account_change (account_id,store_id,"
	    "change_date,amount,credit_amt,debit_amt) values "
	    "(?,?,?,0,0,0)";
	stmt.setCommand(cmd);
	stmtSetId(stmt, line.account_id);
	stmtSetId(stmt, gltx.storeId());
	stmtSetDate(stmt, gltx.postDate());
	execute(stmt);
	commit();
    }
}

void
QuasarDB::checkCard(const Gltx& gltx)
{
    for (unsigned int i = 0; i < gltx.cards().size(); ++i) {
	const CardLine& line = gltx.cards()[i];

	QString cmd = "select count(*) from card_change where "
	    "card_id = ? and store_id = ? and change_date = ?";
	Stmt stmt(_connection, cmd);
	stmtSetId(stmt, line.card_id);
	stmtSetId(stmt, gltx.storeId());
	stmtSetDate(stmt, gltx.postDate());
	execute(stmt);
	stmt.next();
	if (stmtGetInt(stmt, 1) != 0) continue;

	cmd = "insert into card_change (card_id,store_id,"
	    "change_date,amount,debit_amt,credit_amt) values "
	    "(?,?,?,0,0,0)";
	stmt.setCommand(cmd);
	stmtSetId(stmt, line.card_id);
	stmtSetId(stmt, gltx.storeId());
	stmtSetDate(stmt, gltx.postDate());
	execute(stmt);
	commit();
    }
}

void
QuasarDB::checkItem(const Gltx& gltx)
{
    for (unsigned int i = 0; i < gltx.itemCnt(); ++i) {
	const ItemLine& line = gltx.item(i);
	if (line.item_id == INVALID_ID) continue;
	if (line.voided) continue;

	QString cmd = "select count(*) from item_change where "
	    "item_id = ? and size_name = ? and store_id = ? "
	    "and change_date = ?";
	Stmt stmt(_connection, cmd);
	stmtSetId(stmt, line.item_id);
	stmtSetString(stmt, line.size);
	stmtSetId(stmt, gltx.storeId());
	stmtSetDate(stmt, gltx.postDate());
	execute(stmt);
	stmt.next();
	if (stmtGetInt(stmt, 1) != 0) continue;

	cmd = "insert into item_change (item_id,size_name,store_id,"
	    "change_date,on_hand,total_cost,on_order,sold_qty,sold_cost,"
	    "sold_price,recv_qty,recv_cost,adj_qty,adj_cost) values "
	    "(?,?,?,?,0,0,0,0,0,0,0,0,0,0)";
	stmt.setCommand(cmd);
	stmtSetId(stmt, line.item_id);
	stmtSetString(stmt, line.size);
	stmtSetId(stmt, gltx.storeId());
	stmtSetDate(stmt, gltx.postDate());
	execute(stmt);
	commit();
    }
}
