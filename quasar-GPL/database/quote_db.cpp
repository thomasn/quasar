// $Id: quote_db.cpp,v 1.11 2005/03/29 20:57:33 bpepers Exp $
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

#include "quote.h"
#include "quote_select.h"
#include "card.h"
#include "item.h"
#include "account.h"
#include "tax.h"

// Create a Quote
bool
QuasarDB::create(Quote& quote)
{
    if (!validate(quote)) return false;

    // Auto allocate quote number
    if (quote.number().stripWhiteSpace() == "#") {
	fixed number = uniqueNumber("quote", "number");
	quote.setNumber(number.toString());
    }

    QString cmd = insertCmd("quote", "quote_id", "number,reference_str,"
			    "quote_date,store_id,employee_id,customer_id,"
			    "customer_addr,ship_id,ship_addr,ship_via,"
			    "term_id,comments,expiry_date,line_type,"
			    "tax_exempt_id,invoice_id");
    Stmt stmt(_connection, cmd);

    insertData(quote, stmt);
    stmtSetString(stmt, quote.number());
    stmtSetString(stmt, quote.reference());
    stmtSetDate(stmt, quote.date());
    stmtSetId(stmt, quote.storeId());
    stmtSetId(stmt, quote.employeeId());
    stmtSetId(stmt, quote.customerId());
    stmtSetString(stmt, quote.customerAddress());
    stmtSetId(stmt, quote.shipId());
    stmtSetString(stmt, quote.shipAddress());
    stmtSetString(stmt, quote.shipVia());
    stmtSetId(stmt, quote.termsId());
    stmtSetString(stmt, quote.comment());
    stmtSetDate(stmt, quote.expiryDate());
    stmtSetInt(stmt, quote.lineType());
    stmtSetId(stmt, quote.taxExemptId());
    stmtSetId(stmt, quote.invoiceId());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(quote)) return false;

    commit();
    dataSignal(DataEvent::Insert, quote);
    return true;
}

// Delete a Quote
bool
QuasarDB::remove(const Quote& quote)
{
    if (quote.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(quote)) return false;
    if (!removeData(quote, "quote", "quote_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, quote);
    return true;
}

// Update a Quote
bool
QuasarDB::update(const Quote& orig, Quote& quote)
{
    if (orig.id() == INVALID_ID || quote.id() == INVALID_ID) return false;
    if (!validate(quote)) return false;

    // Update the quote table
    QString cmd = updateCmd("quote", "quote_id", "number,reference_str,"
			    "quote_date,store_id,employee_id,customer_id,"
			    "customer_addr,ship_id,ship_addr,ship_via,"
			    "term_id,comments,expiry_date,line_type,"
			    "tax_exempt_id,invoice_id");
    Stmt stmt(_connection, cmd);

    updateData(orig, quote, stmt);
    stmtSetString(stmt, quote.number());
    stmtSetString(stmt, quote.reference());
    stmtSetDate(stmt, quote.date());
    stmtSetId(stmt, quote.storeId());
    stmtSetId(stmt, quote.employeeId());
    stmtSetId(stmt, quote.customerId());
    stmtSetString(stmt, quote.customerAddress());
    stmtSetId(stmt, quote.shipId());
    stmtSetString(stmt, quote.shipAddress());
    stmtSetString(stmt, quote.shipVia());
    stmtSetId(stmt, quote.termsId());
    stmtSetString(stmt, quote.comment());
    stmtSetDate(stmt, quote.expiryDate());
    stmtSetInt(stmt, quote.lineType());
    stmtSetId(stmt, quote.taxExemptId());
    stmtSetId(stmt, quote.invoiceId());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");
    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(quote)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Quote.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id quote_id, Quote& quote)
{
    if (quote_id == INVALID_ID) return false;
    QuoteSelect conditions;
    vector<Quote> quotes;

    conditions.id = quote_id;
    if (!select(quotes, conditions)) return false;
    if (quotes.size() != 1) return false;

    quote = quotes[0];
    return true;
}

// Returns a vector of Quotes.  Returns Quotes sorted by number.
bool
QuasarDB::select(vector<Quote>& quotes, const QuoteSelect& conditions)
{
    quotes.clear();

    QString cmd = selectCmd("quote", "quote_id", "number,reference_str,"
			    "quote_date,store_id,employee_id,customer_id,"
			    "customer_addr,ship_id,ship_addr,ship_via,"
			    "term_id,comments,expiry_date,line_type,"
			    "tax_exempt_id,invoice_id",
			    conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Quote quote;
	int next = 1;
	selectData(quote, stmt, next);
	quote.setNumber(stmtGetString(stmt, next++));
	quote.setReference(stmtGetString(stmt, next++));
	quote.setDate(stmtGetDate(stmt, next++));
	quote.setStoreId(stmtGetId(stmt, next++));
	quote.setEmployeeId(stmtGetId(stmt, next++));
	quote.setCustomerId(stmtGetId(stmt, next++));
	quote.setCustomerAddress(stmtGetString(stmt, next++));
	quote.setShipId(stmtGetId(stmt, next++));
	quote.setShipAddress(stmtGetString(stmt, next++));
	quote.setShipVia(stmtGetString(stmt, next++));
	quote.setTermsId(stmtGetId(stmt, next++));
	quote.setComment(stmtGetString(stmt, next++));
	quote.setExpiryDate(stmtGetDate(stmt, next++));
	quote.setLineType(Quote::LineType(stmtGetInt(stmt, next++)));
	quote.setTaxExemptId(stmtGetId(stmt, next++));
	quote.setInvoiceId(stmtGetId(stmt, next++));
	quotes.push_back(quote);
    }

    QString cmd1 = "select item_id,account_id,number,description,"
	"size_name,size_qty,quantity,sale_price,ext_deposit,"
	"price_id,price_disc,unit_price,for_qty,for_price,"
	"ext_price,ext_base,ext_tax,tax_id,item_tax_id,"
	"include_tax,include_deposit,discountable,subdept_id,scale,"
	"you_save,open_dept from quote_item where quote_id = ? order by "
	"seq_num";
    Stmt stmt1(_connection, cmd1);

    QString cmd2 = "select seq_num,discount_id,method,amount,account_id,"
	"total_amt from quote_item_disc where quote_id = ? "
	"order by seq_num";
    Stmt stmt2(_connection, cmd2);

    QString cmd3 = "select discount_id,method,amount,account_id,total_amt "
	"from quote_discount where quote_id = ? order by seq_num";
    Stmt stmt3(_connection, cmd3);

    QString cmd4 = "select disc_num,total_amt from quote_item_tdisc "
	"where quote_id = ? and item_num = ? order by seq_num";
    Stmt stmt4(_connection, cmd4);

    QString cmd5 = "select tax_id,taxable,amount,inc_taxable,inc_amount "
	"from quote_tax where quote_id = ? order by seq_num";
    Stmt stmt5(_connection, cmd5);

    for (unsigned int i = 0; i < quotes.size(); ++i) {
	Id quote_id = quotes[i].id();

	stmtSetId(stmt1, quote_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    InvoiceItem line;
	    int next = 1;
	    line.item_id = stmtGetId(stmt1, next++);
	    line.account_id = stmtGetId(stmt1, next++);
	    line.number = stmtGetString(stmt1, next++);
	    line.description = stmtGetString(stmt1, next++);
	    line.size = stmtGetString(stmt1, next++);
	    line.size_qty = stmtGetFixed(stmt1, next++);
	    line.quantity = stmtGetFixed(stmt1, next++);
	    line.sale_price = stmtGetFixed(stmt1, next++);
	    line.ext_deposit = stmtGetFixed(stmt1, next++);
	    line.price_id = stmtGetId(stmt1, next++);
	    line.price_disc = stmtGetFixed(stmt1, next++);
	    line.price.setUnitPrice(stmtGetFixed(stmt1, next++));
	    line.price.setForQty(stmtGetFixed(stmt1, next++));
	    line.price.setForPrice(stmtGetFixed(stmt1, next++));
	    line.ext_price = stmtGetFixed(stmt1, next++);
	    line.ext_base = stmtGetFixed(stmt1, next++);
	    line.ext_tax = stmtGetFixed(stmt1, next++);
	    line.tax_id = stmtGetId(stmt1, next++);
	    line.item_tax_id = stmtGetId(stmt1, next++);
	    line.include_tax = stmtGetBool(stmt1, next++);
	    line.include_deposit = stmtGetBool(stmt1, next++);
	    line.discountable = stmtGetBool(stmt1, next++);
	    line.subdept_id = stmtGetId(stmt1, next++);
	    line.scale = stmtGetBool(stmt1, next++);
	    line.you_save = stmtGetFixed(stmt1, next++);
	    line.open_dept = stmtGetBool(stmt1, next++);
	    quotes[i].items().push_back(line);
	}

	stmtSetId(stmt2, quote_id);
	if (!execute(stmt2)) return false;
	while (stmt2.next()) {
	    int seq_num = stmtGetInt(stmt2, 1);
	    InvoiceItem& line = quotes[i].items()[seq_num];
	    line.line_disc.discount_id = stmtGetId(stmt2, 2);
	    line.line_disc.method = stmtGetInt(stmt2, 3);
	    line.line_disc.amount = stmtGetFixed(stmt2, 4);
	    line.line_disc.account_id = stmtGetId(stmt2, 5);
	    line.line_disc.total_amt = stmtGetFixed(stmt2, 6);
	}

	stmtSetId(stmt3, quote_id);
	if (!execute(stmt3)) return false;
	while (stmt3.next()) {
	    InvoiceDisc line;
	    line.discount_id = stmtGetId(stmt3, 1);
	    line.method = stmtGetInt(stmt3, 2);
	    line.amount = stmtGetFixed(stmt3, 3);
	    line.account_id = stmtGetId(stmt3, 4);
	    line.total_amt = stmtGetFixed(stmt3, 5);
	    quotes[i].discounts().push_back(line);
	}

	for (unsigned int j = 0; j < quotes[i].items().size(); ++j) {
	    stmtSetId(stmt4, quote_id);
	    stmtSetInt(stmt4, j);
	    if (!execute(stmt4)) return false;
	    while (stmt4.next()) {
		int disc_num = stmtGetInt(stmt4, 1);
		fixed total_amt = stmtGetFixed(stmt4, 2);
		quotes[i].items()[j].tdisc_nums.push_back(disc_num);
		quotes[i].items()[j].tdisc_amts.push_back(total_amt);
	    }
	}

	stmtSetId(stmt5, quote_id);
	if (!execute(stmt5)) return false;
	while (stmt5.next()) {
	    TaxLine line;
	    line.tax_id = stmtGetId(stmt5, 1);
	    line.taxable = stmtGetFixed(stmt5, 2);
	    line.amount = stmtGetFixed(stmt5, 3);
	    line.inc_taxable = stmtGetFixed(stmt5, 4);
	    line.inc_amount = stmtGetFixed(stmt5, 5);
	    quotes[i].taxes().push_back(line);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Quote& quote)
{
    if (!validate((DataObject&)quote)) return false;

    if (quote.customerId() != INVALID_ID) {
	Card card;
	if (!lookup(quote.customerId(), card))
	    return error("Customer doesn't exist");
	if (card.dataType() != DataObject::CUSTOMER)
	    return error("Card isn't a customer");
    }

    if (quote.employeeId() != INVALID_ID) {
	Card card;
	if (!lookup(quote.employeeId(), card))
	    return error("Salesperson doesn't exist");
	if (card.dataType() != DataObject::EMPLOYEE)
	    return error("Salesperson isn't an employee");
    }

    if (quote.lineType() < Quote::Item)
	return error("Invalid line type");
    if (quote.lineType() > Quote::Account)
	return error("Invalid line type");

    if (quote.items().size() < 1)
	return error("Quote must have at least one line");

    unsigned int i;
    for (i = 0; i < quote.items().size(); ++i) {
	const InvoiceItem& line = quote.items()[i];

	if (line.item_id == INVALID_ID && line.account_id == INVALID_ID)
	    return error("Both item and account can't be blank");
	if (line.item_id != INVALID_ID && line.account_id != INVALID_ID)
	    return error("Both item and account can't be set");
	if (line.item_id != INVALID_ID && line.number.isEmpty())
	    return error("Blank item number");

#if CHECK_ITEM
	if (line.item_id != INVALID_ID) {
	    Item item;
	    if (!lookup(line.item_id, item))
		return error("Item doesn't exist");
	    if (!item.isSold())
		return error("Item isn't sold");
	}

	if (line.account_id != INVALID_ID) {
	    Account account;
	    if (!lookup(line.account_id, account))
		return error("Account doesn't exist");
	    if (account.isHeader())
		return error("Account is a header");
	}
#endif

	// TODO: any checks on quantity, ext_price, or inv_cost?
    }

    return true;
}

bool
QuasarDB::sqlCreateLines(const Quote& quote)
{
    QString cmd1 = insertText("quote_item", "quote_id", "seq_num,item_id,"
			      "account_id,number,description,size_name,"
			      "size_qty,quantity,sale_price,ext_deposit,"
			      "price_id,price_disc,unit_price,for_qty,"
			      "for_price,ext_price,ext_base,ext_tax,"
			      "tax_id,item_tax_id,include_tax,include_deposit,"
			      "discountable,subdept_id,scale,you_save,"
			      "open_dept");
    Stmt stmt1(_connection, cmd1);

    QString cmd2 = insertText("quote_item_disc", "quote_id", "seq_num,"
			      "discount_id,method,amount,account_id,"
			      "total_amt");
    Stmt stmt2(_connection, cmd2);

    QString cmd3 = insertText("quote_item_tdisc", "quote_id", "item_num,"
			      "seq_num,disc_num,total_amt");
    Stmt stmt3(_connection, cmd3);

    const vector<InvoiceItem>& items = quote.items();
    unsigned int i;
    for (i = 0; i < items.size(); ++i) {
	const InvoiceItem& line = items[i];

	stmtSetId(stmt1, quote.id());
	stmtSetInt(stmt1, i);
	stmtSetId(stmt1, line.item_id);
	stmtSetId(stmt1, line.account_id);
	stmtSetString(stmt1, line.number);
	stmtSetString(stmt1, line.description);
	stmtSetString(stmt1, line.size);
	stmtSetFixed(stmt1, line.size_qty);
	stmtSetFixed(stmt1, line.quantity);
	stmtSetFixed(stmt1, line.sale_price);
	stmtSetFixed(stmt1, line.ext_deposit);
	stmtSetId(stmt1, line.price_id);
	stmtSetFixed(stmt1, line.price_disc);
	stmtSetFixed(stmt1, line.price.unitPrice());
	stmtSetFixed(stmt1, line.price.forQty());
	stmtSetFixed(stmt1, line.price.forPrice());
	stmtSetFixed(stmt1, line.ext_price);
	stmtSetFixed(stmt1, line.ext_base);
	stmtSetFixed(stmt1, line.ext_tax);
	stmtSetId(stmt1, line.tax_id);
	stmtSetId(stmt1, line.item_tax_id);
	stmtSetBool(stmt1, line.include_tax);
	stmtSetBool(stmt1, line.include_deposit);
	stmtSetBool(stmt1, line.discountable);
	stmtSetId(stmt1, line.subdept_id);
	stmtSetBool(stmt1, line.scale);
	stmtSetFixed(stmt1, line.you_save);
	stmtSetBool(stmt1, line.open_dept);
	if (!execute(stmt1)) return false;

	if (line.line_disc.discount_id != INVALID_ID) {
	    stmtSetId(stmt2, quote.id());
	    stmtSetInt(stmt2, i);
	    stmtSetId(stmt2, line.line_disc.discount_id);
	    stmtSetInt(stmt2, line.line_disc.method);
	    stmtSetFixed(stmt2, line.line_disc.amount);
	    stmtSetId(stmt2, line.line_disc.account_id);
	    stmtSetFixed(stmt2, line.line_disc.total_amt);
	    if (!execute(stmt2)) return false;
	}

	for (unsigned int j = 0; j < line.tdisc_nums.size(); ++j) {
	    stmtSetId(stmt3, quote.id());
	    stmtSetInt(stmt3, i);
	    stmtSetInt(stmt3, j);
	    stmtSetInt(stmt3, line.tdisc_nums[j]);
	    stmtSetFixed(stmt3, line.tdisc_amts[j]);
	    if (!execute(stmt3)) return false;
	}
    }

    QString cmd = insertText("quote_discount", "quote_id", "seq_num,"
			     "discount_id,method,amount,account_id,"
			     "total_amt");
    Stmt stmt(_connection, cmd);

    const vector<InvoiceDisc>& discounts = quote.discounts();
    for (i = 0; i < discounts.size(); ++i) {
	const InvoiceDisc& line = discounts[i];

	stmtSetId(stmt, quote.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.discount_id);
	stmtSetInt(stmt, line.method);
	stmtSetFixed(stmt, line.amount);
	stmtSetId(stmt, line.account_id);
	stmtSetFixed(stmt, line.total_amt);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("quote_tax", "quote_id", "seq_num,tax_id,taxable,amount,"
		     "inc_taxable,inc_amount");
    stmt.setCommand(cmd);

    const vector<TaxLine>& taxes = quote.taxes();
    for (i = 0; i < taxes.size(); ++i) {
	Id tax_id = taxes[i].tax_id;
	fixed taxable = taxes[i].taxable;
	fixed amount = taxes[i].amount;
	fixed inc_taxable = taxes[i].inc_taxable;
	fixed inc_amount = taxes[i].inc_amount;

	stmtSetId(stmt, quote.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, tax_id);
	stmtSetFixed(stmt, taxable);
	stmtSetFixed(stmt, amount);
	stmtSetFixed(stmt, inc_taxable);
	stmtSetFixed(stmt, inc_amount);
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const Quote& quote)
{
    if (!remove(quote, "quote_item", "quote_id")) return false;
    if (!remove(quote, "quote_item_disc", "quote_id")) return false;
    if (!remove(quote, "quote_item_tdisc", "quote_id")) return false;
    if (!remove(quote, "quote_discount", "quote_id")) return false;
    if (!remove(quote, "quote_tax", "quote_id")) return false;
    return true;
}
