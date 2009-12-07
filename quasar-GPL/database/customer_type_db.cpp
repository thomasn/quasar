// $Id: customer_type_db.cpp,v 1.8 2005/03/01 19:59:42 bpepers Exp $
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

#include "customer_type.h"
#include "customer_type_select.h"
#include "account.h"
#include "tax.h"

// Create a CustomerType
bool
QuasarDB::create(CustomerType& type)
{
    if (!validate(type)) return false;

    QString cmd = insertCmd("customer_type", "type_id", "name,"
			    "account_id,term_id,tax_exempt_id,"
			    "credit_limit,can_charge,can_withdraw,"
			    "can_payment,sc_rate,check_wd_bal,"
			    "print_stmts,second_rcpt");
    Stmt stmt(_connection, cmd);

    insertData(type, stmt);
    stmtSetString(stmt, type.name());
    stmtSetId(stmt, type.accountId());
    stmtSetId(stmt, type.termsId());
    stmtSetId(stmt, type.taxExemptId());
    stmtSetFixed(stmt, type.creditLimit());
    stmtSetBool(stmt, type.canCharge());
    stmtSetBool(stmt, type.canWithdraw());
    stmtSetBool(stmt, type.canPayment());
    stmtSetFixed(stmt, type.serviceCharge());
    stmtSetBool(stmt, type.checkWithdrawBalance());
    stmtSetBool(stmt, type.printStatements());
    stmtSetBool(stmt, type.secondReceipt());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, type);
    return true;
}

// Delete a CustomerType
bool
QuasarDB::remove(const CustomerType& type)
{
    if (type.id() == INVALID_ID) return false;
    if (!removeData(type, "customer_type", "type_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, type);
    return true;
}

// Update a CustomerType.
bool
QuasarDB::update(const CustomerType& orig, CustomerType& type)
{
    if (orig.id() == INVALID_ID || type.id() == INVALID_ID) return false;
    if (!validate(type)) return false;

    // Update the type table
    QString cmd = updateCmd("customer_type", "type_id", "name,"
			    "account_id,term_id,tax_exempt_id,"
			    "credit_limit,can_charge,can_withdraw,"
			    "can_payment,sc_rate,check_wd_bal,"
			    "print_stmts,second_rcpt");
    Stmt stmt(_connection, cmd);

    updateData(orig, type, stmt);
    stmtSetString(stmt, type.name());
    stmtSetId(stmt, type.accountId());
    stmtSetId(stmt, type.termsId());
    stmtSetId(stmt, type.taxExemptId());
    stmtSetFixed(stmt, type.creditLimit());
    stmtSetBool(stmt, type.canCharge());
    stmtSetBool(stmt, type.canWithdraw());
    stmtSetBool(stmt, type.canPayment());
    stmtSetFixed(stmt, type.serviceCharge());
    stmtSetBool(stmt, type.checkWithdrawBalance());
    stmtSetBool(stmt, type.printStatements());
    stmtSetBool(stmt, type.secondReceipt());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    if (orig.accountId() != type.accountId()) {
	cmd = "update card_customer set account_id = ? where "
	    "type_id = ? and account_id = ?";
	Stmt stmt(_connection, cmd);
	stmtSetId(stmt, type.accountId());
	stmtSetId(stmt, orig.id());
	stmtSetId(stmt, orig.accountId());
	if (!execute(stmt)) return false;
    }
    if (orig.termsId() != type.termsId()) {
	cmd = "update card_customer set term_id = ? where "
	    "type_id = ? and term_id = ?";
	Stmt stmt(_connection, cmd);
	stmtSetId(stmt, type.termsId());
	stmtSetId(stmt, orig.id());
	stmtSetId(stmt, orig.termsId());
	if (!execute(stmt)) return false;
    }
    if (orig.taxExemptId() != type.taxExemptId()) {
	cmd = "update card_customer set tax_exempt_id = ? where "
	    "type_id = ? and tax_exempt_id = ?";
	Stmt stmt(_connection, cmd);
	stmtSetId(stmt, type.taxExemptId());
	stmtSetId(stmt, orig.id());
	stmtSetId(stmt, orig.taxExemptId());
	if (!execute(stmt)) return false;
    }
    if (orig.creditLimit() != type.creditLimit()) {
	cmd = "update card_customer set credit_limit = ? where "
	    "type_id = ? and credit_limit = ?";
	Stmt stmt(_connection, cmd);
	stmtSetFixed(stmt, type.creditLimit());
	stmtSetId(stmt, orig.id());
	stmtSetFixed(stmt, orig.creditLimit());
	if (!execute(stmt)) return false;
    }
    if (orig.canCharge() != type.canCharge()) {
	cmd = "update card_customer set can_charge = ? where "
	    "type_id = ? and can_charge = ?";
	Stmt stmt(_connection, cmd);
	stmtSetBool(stmt, type.canCharge());
	stmtSetId(stmt, orig.id());
	stmtSetBool(stmt, orig.canCharge());
	if (!execute(stmt)) return false;
    }
    if (orig.canWithdraw() != type.canWithdraw()) {
	cmd = "update card_customer set can_withdraw = ? where "
	    "type_id = ? and can_withdraw = ?";
	Stmt stmt(_connection, cmd);
	stmtSetBool(stmt, type.canWithdraw());
	stmtSetId(stmt, orig.id());
	stmtSetBool(stmt, orig.canWithdraw());
	if (!execute(stmt)) return false;
    }
    if (orig.canPayment() != type.canPayment()) {
	cmd = "update card_customer set can_payment = ? where "
	    "type_id = ? and can_payment = ?";
	Stmt stmt(_connection, cmd);
	stmtSetBool(stmt, type.canPayment());
	stmtSetId(stmt, orig.id());
	stmtSetBool(stmt, orig.canPayment());
	if (!execute(stmt)) return false;
    }
    if (orig.serviceCharge() != type.serviceCharge()) {
	cmd = "update card_customer set sc_rate = ? where "
	    "type_id = ? and sc_rate = ?";
	Stmt stmt(_connection, cmd);
	stmtSetFixed(stmt, type.serviceCharge());
	stmtSetId(stmt, orig.id());
	stmtSetFixed(stmt, orig.serviceCharge());
	if (!execute(stmt)) return false;
    }
    if (orig.checkWithdrawBalance() != type.checkWithdrawBalance()) {
	cmd = "update card_customer set check_wd_bal = ? where "
	    "type_id = ? and check_wd_bal = ?";
	Stmt stmt(_connection, cmd);
	stmtSetBool(stmt, type.checkWithdrawBalance());
	stmtSetId(stmt, orig.id());
	stmtSetBool(stmt, orig.checkWithdrawBalance());
	if (!execute(stmt)) return false;
    }
    if (orig.printStatements() != type.printStatements()) {
	cmd = "update card_customer set print_stmts = ? where "
	    "type_id = ? and print_stmts = ?";
	Stmt stmt(_connection, cmd);
	stmtSetBool(stmt, type.printStatements());
	stmtSetId(stmt, orig.id());
	stmtSetBool(stmt, orig.printStatements());
	if (!execute(stmt)) return false;
    }
    if (orig.secondReceipt() != type.secondReceipt()) {
	cmd = "update card_customer set second_rcpt = ? where "
	    "type_id = ? and second_rcpt = ?";
	Stmt stmt(_connection, cmd);
	stmtSetBool(stmt, type.secondReceipt());
	stmtSetId(stmt, orig.id());
	stmtSetBool(stmt, orig.secondReceipt());
	if (!execute(stmt)) return false;
    }

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a CustomerType.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id type_id, CustomerType& type)
{
    if (type_id == INVALID_ID) return false;
    CustomerTypeSelect conditions;
    vector<CustomerType> types;

    conditions.id = type_id;
    if (!select(types, conditions)) return false;
    if (types.size() != 1) return false;

    type = types[0];
    return true;
}

// Returns a vector of CustomerTypes.  Returns CustomerTypes sorted by name.
bool
QuasarDB::select(vector<CustomerType>& types, const CustomerTypeSelect& conds)
{
    types.clear();

    QString cmd = selectCmd("customer_type", "type_id", "name,"
			    "account_id,term_id,tax_exempt_id,"
			    "credit_limit,can_charge,can_withdraw,"
			    "can_payment,sc_rate,check_wd_bal,"
			    "print_stmts,second_rcpt", conds);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	CustomerType type;
	int next = 1;
	selectData(type, stmt, next);
	type.setName(stmtGetString(stmt, next++));
	type.setAccountId(stmtGetId(stmt, next++));
	type.setTermsId(stmtGetId(stmt, next++));
	type.setTaxExemptId(stmtGetId(stmt, next++));
	type.setCreditLimit(stmtGetFixed(stmt, next++));
	type.setCanCharge(stmtGetBool(stmt, next++));
	type.setCanWithdraw(stmtGetBool(stmt, next++));
	type.setCanPayment(stmtGetBool(stmt, next++));
	type.setServiceCharge(stmtGetFixed(stmt, next++));
	type.setCheckWithdrawBalance(stmtGetBool(stmt, next++));
	type.setPrintStatements(stmtGetBool(stmt, next++));
	type.setSecondReceipt(stmtGetBool(stmt, next++));
	types.push_back(type);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const CustomerType& type)
{
    if (!validate((DataObject&)type)) return false;

    if (type.name().stripWhiteSpace().isEmpty())
	return error("Blank customer type name");

    Account account;
    if (!lookup(type.accountId(), account))
	return error("GL account doesn't exist");
    if (account.isHeader())
        return error("GL account is a header");

    if (type.creditLimit() < 0.0)
	return error("Negative credit limit");

    if (type.serviceCharge() < 0.0)
	return error("Negative service charge");
    if (type.serviceCharge() >= 100.0)
	return error("Invalid service charge");

    return true;
}
