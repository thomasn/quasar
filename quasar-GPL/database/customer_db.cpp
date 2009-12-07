// $Id: customer_db.cpp,v 1.35 2005/03/01 19:59:42 bpepers Exp $
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

#include "customer.h"
#include "customer_select.h"
#include "account.h"

// Create a Customer
bool
QuasarDB::create(Customer& customer)
{
    if (!validate(customer)) return false;
    if (!create((Card&)customer)) return false;

    QString cmd = insertText("card_customer", "customer_id", "type_id,"
			     "account_id,term_id,tax_exempt_id,"
			     "credit_limit,can_charge,can_withdraw,"
			     "can_payment,sc_rate,check_wd_bal,"
			     "print_stmts,credit_hold,second_rcpt");
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, customer.id());
    stmtSetId(stmt, customer.typeId());
    stmtSetId(stmt, customer.accountId());
    stmtSetId(stmt, customer.termsId());
    stmtSetId(stmt, customer.taxExemptId());
    stmtSetFixed(stmt, customer.creditLimit());
    stmtSetBool(stmt, customer.canCharge());
    stmtSetBool(stmt, customer.canWithdraw());
    stmtSetBool(stmt, customer.canPayment());
    stmtSetFixed(stmt, customer.serviceCharge());
    stmtSetBool(stmt, customer.checkWithdrawBalance());
    stmtSetBool(stmt, customer.printStatements());
    stmtSetBool(stmt, customer.creditHold());
    stmtSetBool(stmt, customer.secondReceipt());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(customer)) return false;

    commit();
    dataSignal(DataEvent::Insert, customer);
    return true;
}

// Delete a Customer
bool
QuasarDB::remove(const Customer& customer)
{
    if (customer.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(customer)) return false;
    if (!remove((Card&)customer)) return false;

    commit();
    dataSignal(DataEvent::Delete, customer);
    return true;
}

// Update a Customer
bool
QuasarDB::update(const Customer& orig, Customer& customer)
{
    if (orig.id() == INVALID_ID || customer.id() == INVALID_ID) return false;
    if (!validate(customer)) return false;
    if (!update(orig, (Card&)customer)) return false;

    // Update the customer tables
    QString cmd = updateText("card_customer", "customer_id", "type_id,"
			     "account_id,term_id,tax_exempt_id,"
			     "credit_limit,can_charge,can_withdraw,"
			     "can_payment,sc_rate,check_wd_bal,"
			     "print_stmts,credit_hold,second_rcpt");
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, customer.typeId());
    stmtSetId(stmt, customer.accountId());
    stmtSetId(stmt, customer.termsId());
    stmtSetId(stmt, customer.taxExemptId());
    stmtSetFixed(stmt, customer.creditLimit());
    stmtSetBool(stmt, customer.canCharge());
    stmtSetBool(stmt, customer.canWithdraw());
    stmtSetBool(stmt, customer.canPayment());
    stmtSetFixed(stmt, customer.serviceCharge());
    stmtSetBool(stmt, customer.checkWithdrawBalance());
    stmtSetBool(stmt, customer.printStatements());
    stmtSetBool(stmt, customer.creditHold());
    stmtSetBool(stmt, customer.secondReceipt());
    stmtSetId(stmt, customer.id());

    if (!execute(stmt)) return false;
    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(customer)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Customer.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id customer_id, Customer& customer)
{
    if (customer_id == INVALID_ID) return false;
    CustomerSelect conditions;
    vector<Customer> customers;

    conditions.id = customer_id;
    if (!select(customers, conditions)) return false;
    if (customers.size() != 1) return false;

    customer = customers[0];
    return true;
}

// Lookup a Customer by its number
bool
QuasarDB::lookup(const QString& number, Customer& customer)
{
    if (number.isEmpty()) return false;
    CustomerSelect conditions;
    vector<Customer> customers;

    conditions.number = number;
    if (!select(customers, conditions)) return false;
    if (customers.size() != 1) return false;

    customer = customers[0];
    return true;
}

// Returns a vector of Customers.  Returns customers sorted by name.
bool
QuasarDB::select(vector<Customer>& customers, const CustomerSelect& conditions)
{
    customers.clear();

    QString table = "card_customer join card on card.card_id = "
	"card_customer.customer_id";

    QString cmd = selectCmd(table, "card_id", "company,first_name,"
			    "last_name,number,street,street2,city,"
			    "province,country,postal,phone_num,phone2_num,"
			    "fax_num,email,web_page,contact,comments,"
			    "patgroup_id,type_id,account_id,term_id,"
			    "tax_exempt_id,credit_limit,can_charge,"
			    "can_withdraw,can_payment,sc_rate,check_wd_bal,"
			    "print_stmts,credit_hold,second_rcpt",
			    conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Customer customer;
	int next = 1;
	selectData(customer, stmt, next);
	selectCard(customer, stmt, next);
	customer.setTypeId(stmtGetId(stmt, next++));
	customer.setAccountId(stmtGetId(stmt, next++));
	customer.setTermsId(stmtGetId(stmt, next++));
	customer.setTaxExemptId(stmtGetId(stmt, next++));
	customer.setCreditLimit(stmtGetFixed(stmt, next++));
	customer.setCanCharge(stmtGetBool(stmt, next++));
	customer.setCanWithdraw(stmtGetBool(stmt, next++));
	customer.setCanPayment(stmtGetBool(stmt, next++));
	customer.setServiceCharge(stmtGetFixed(stmt, next++));
	customer.setCheckWithdrawBalance(stmtGetBool(stmt, next++));
	customer.setPrintStatements(stmtGetBool(stmt, next++));
	customer.setCreditHold(stmtGetBool(stmt, next++));
	customer.setSecondReceipt(stmtGetBool(stmt, next++));
	customers.push_back(customer);
    }

    QString cmd1 = "select group_id from card_group where card_id = ? "
	"order by seq_num";
    Stmt stmt1(_connection, cmd1);

    QString cmd2 = "select discount_id from card_discount where "
	"card_id = ? order by seq_num";
    Stmt stmt2(_connection, cmd2);

    QString cmd3 = "select extra_id,data_value from card_extra "
	"where card_id = ? order by seq_num";
    Stmt stmt3(_connection, cmd3);

    QString cmd4 = "select ref_name from cust_refs where customer_id = ? "
	"order by seq_num";
    Stmt stmt4(_connection, cmd4);

    for (unsigned int i = 0; i < customers.size(); ++i) {
	Id card_id = customers[i].id();

	stmtSetId(stmt1, card_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    Id group_id = stmtGetId(stmt1, 1);
	    customers[i].groups().push_back(group_id);
	}

	stmtSetId(stmt2, card_id);
	if (!execute(stmt2)) return false;
	while (stmt2.next()) {
	    Id discount_id = stmtGetId(stmt2, 1);
	    customers[i].discounts().push_back(discount_id);
	}

	stmtSetId(stmt3, card_id);
	if (!execute(stmt3)) return false;
	while (stmt3.next()) {
	    Id extra_id = stmtGetId(stmt3, 1);
	    QString value = stmtGetString(stmt3, 2);
	    customers[i].setValue(extra_id, value);
	}

	stmtSetId(stmt4, card_id);
	if (!execute(stmt4)) return false;
	while (stmt4.next()) {
	    QString name = stmtGetString(stmt4, 1);
	    customers[i].references().push_back(name);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Customer& customer)
{
    if (!validate((Card&)customer)) return false;

    if (customer.name().stripWhiteSpace().isEmpty())
	return error("Blank customer name");

    Account account;
    if (!lookup(customer.accountId(), account))
	return error("GL account doesn't exist");
    if (account.isHeader())
	return error("GL account is a header");

    if (customer.creditLimit() < 0.0)
	return error("Negative credit limit");

    if (customer.serviceCharge() < 0.0)
	return error("Negative service charge");
    if (customer.serviceCharge() >= 100.0)
	return error("Invalid service charge");

    return true;
}

bool
QuasarDB::sqlCreateLines(const Customer& customer)
{
    QString cmd = insertText("cust_refs", "customer_id", "seq_num,ref_name");
    Stmt stmt(_connection, cmd);

    for (unsigned int i = 0; i < customer.references().size(); ++i) {
	stmtSetId(stmt, customer.id());
	stmtSetInt(stmt, i);
	stmtSetString(stmt, customer.references()[i]);
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const Customer& customer)
{
    return remove(customer, "cust_refs", "customer_id");
}
