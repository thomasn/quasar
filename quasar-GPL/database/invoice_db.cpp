// $Id: invoice_db.cpp,v 1.55 2005/06/11 02:52:20 bpepers Exp $
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

#include "invoice.h"
#include "invoice_select.h"
#include "customer.h"
#include "item.h"
#include "tax.h"
#include "account.h"
#include "tender.h"
#include "discount.h"
#include "company.h"
#include "object_cache.h"

// Create an Invoice
bool
QuasarDB::create(Invoice& invoice)
{
    if (!validate(invoice)) return false;
    if (!create((Gltx&)invoice)) return false;

    QString cmd = insertText("invoice", "invoice_id", "ship_id,"
			     "ship_via,term_id,promised_date,"
			     "line_type,tax_exempt_id,customer_addr,"
			     "ship_addr,comments");
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, invoice.id());
    stmtSetId(stmt, invoice.shipId());
    stmtSetString(stmt, invoice.shipVia());
    stmtSetId(stmt, invoice.termsId());
    stmtSetDate(stmt, invoice.promisedDate());
    stmtSetInt(stmt, invoice.lineType());
    stmtSetId(stmt, invoice.taxExemptId());
    stmtSetString(stmt, invoice.customerAddress());
    stmtSetString(stmt, invoice.shipAddress());
    stmtSetString(stmt, invoice.comment());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(invoice)) return false;

    commit();
    dataSignal(DataEvent::Insert, invoice);
    return true;
}

// Delete an Invoice
bool
QuasarDB::remove(const Invoice& invoice)
{
    if (invoice.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(invoice)) return false;
    if (!remove((Gltx&)invoice)) return false;

    commit();
    dataSignal(DataEvent::Delete, invoice);
    return true;
}

// Update an Invoice
bool
QuasarDB::update(const Invoice& orig, Invoice& invoice)
{
    if (orig.id() == INVALID_ID || invoice.id() == INVALID_ID) return false;
    if (!validate(invoice)) return false;
    if (!update(orig, (Gltx&)invoice)) return false;

    // Update the invoice tables
    QString cmd = updateText("invoice", "invoice_id", "ship_id,"
			     "ship_via,term_id,promised_date,"
			     "line_type,tax_exempt_id,customer_addr,"
			     "ship_addr,comments");
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, invoice.shipId());
    stmtSetString(stmt, invoice.shipVia());
    stmtSetId(stmt, invoice.termsId());
    stmtSetDate(stmt, invoice.promisedDate());
    stmtSetInt(stmt, invoice.lineType());
    stmtSetId(stmt, invoice.taxExemptId());
    stmtSetString(stmt, invoice.customerAddress());
    stmtSetString(stmt, invoice.shipAddress());
    stmtSetString(stmt, invoice.comment());
    stmtSetId(stmt, orig.id());

    if (!execute(stmt)) return false;
    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(invoice)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup an Invoice.
bool
QuasarDB::lookup(Id invoice_id, Invoice& invoice)
{
    if (invoice_id == INVALID_ID) return false;
    InvoiceSelect conditions;
    vector<Invoice> invoices;

    conditions.id = invoice_id;
    if (!select(invoices, conditions)) return false;
    if (invoices.size() != 1) return false;

    invoice = invoices[0];
    return true;
}

// Returns a count of Invoices based on the conditions.
bool
QuasarDB::count(int& count, const InvoiceSelect& conditions)
{
    count = 0;
    QString cmd = "select count(*) from invoice join gltx on "
	"invoice.invoice_id = gltx.gltx_id " + conditions.where();
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	count = stmtGetInt(stmt, 1);
    }

    commit();
    return true;
}

// Returns a vector of Invoices.
bool
QuasarDB::select(vector<Invoice>& invoices, const InvoiceSelect& conditions)
{
    invoices.clear();

    QString cmd = gltxCmd("invoice", "invoice_id", "ship_id,"
			  "ship_via,term_id,promised_date,line_type,"
			  "tax_exempt_id,customer_addr,ship_addr,"
			  "comments,data_type", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Invoice invoice;
	int next = 1;
	selectData(invoice, stmt, next);
	selectGltx(invoice, stmt, next);
	invoice.setShipId(stmtGetId(stmt, next++));
	invoice.setShipVia(stmtGetString(stmt, next++));
	invoice.setTermsId(stmtGetId(stmt, next++));
	invoice.setPromisedDate(stmtGetDate(stmt, next++));
	invoice.setLineType(Invoice::LineType(stmtGetInt(stmt, next++)));
	invoice.setTaxExemptId(stmtGetId(stmt, next++));
	invoice.setCustomerAddress(stmtGetString(stmt, next++));
	invoice.setShipAddress(stmtGetString(stmt, next++));
	invoice.setComment(stmtGetString(stmt, next++));
	invoice.setDataType(DataObject::DataType(stmtGetInt(stmt,next++)));
	invoices.push_back(invoice);
    }

    QString cmd1 = "select item_id,account_id,number,description,"
	"size_name,size_qty,quantity,inv_cost,sale_price,"
	"ext_deposit,voided,price_id,price_disc,unit_price,for_qty,"
	"for_price,ext_price,ext_base,ext_tax,tax_id,item_tax_id,"
	"include_tax,include_deposit,discountable,subdept_id,scale,"
	"you_save,open_dept from gltx_item join invoice_item on "
	"(gltx_item.gltx_id = invoice_item.invoice_id "
	"and gltx_item.seq_num = invoice_item.seq_num) where "
	"gltx_item.gltx_id = ? order by gltx_item.seq_num";
    Stmt stmt1(_connection, cmd1);

    QString cmd2 = "select seq_num,discount_id,method,amount,account_id,"
	"total_amt from invoice_item_disc where invoice_id = ? "
	"order by seq_num";
    Stmt stmt2(_connection, cmd2);

    QString cmd3 = "select discount_id,method,amount,account_id,"
	"total_amt,voided from invoice_discount where invoice_id = ? "
	"order by seq_num";
    Stmt stmt3(_connection, cmd3);

    QString cmd4 = "select disc_num,total_amt from invoice_item_tdisc "
	"where invoice_id = ? and item_num = ? order by seq_num";
    Stmt stmt4(_connection, cmd4);

    for (unsigned int i = 0; i < invoices.size(); ++i) {
	Id gltx_id = invoices[i].id();
	GLTX_ACCOUNTS(invoices);
	GLTX_TAXES(invoices);
	GLTX_CARDS(invoices);
	GLTX_PAYMENTS(invoices);
	GLTX_TENDERS(invoices);
	GLTX_REFERENCES(invoices);

	stmtSetId(stmt1, gltx_id);
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
	    line.inv_cost = stmtGetFixed(stmt1, next++);
	    line.sale_price = stmtGetFixed(stmt1, next++);
	    line.ext_deposit = stmtGetFixed(stmt1, next++);
	    line.voided = stmtGetBool(stmt1, next++);
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
	    invoices[i].items().push_back(line);
	}

	stmtSetId(stmt2, gltx_id);
	if (!execute(stmt2)) return false;
	while (stmt2.next()) {
	    int seq_num = stmtGetInt(stmt2, 1);
	    InvoiceItem& line = invoices[i].items()[seq_num];
	    line.line_disc.discount_id = stmtGetId(stmt2, 2);
	    line.line_disc.method = stmtGetInt(stmt2, 3);
	    line.line_disc.amount = stmtGetFixed(stmt2, 4);
	    line.line_disc.account_id = stmtGetId(stmt2, 5);
	    line.line_disc.total_amt = stmtGetFixed(stmt2, 6);
	}

	stmtSetId(stmt3, gltx_id);
	if (!execute(stmt3)) return false;
	while (stmt3.next()) {
	    InvoiceDisc line;
	    line.discount_id = stmtGetId(stmt3, 1);
	    line.method = stmtGetInt(stmt3, 2);
	    line.amount = stmtGetFixed(stmt3, 3);
	    line.account_id = stmtGetId(stmt3, 4);
	    line.total_amt = stmtGetFixed(stmt3, 5);
	    line.voided = stmtGetBool(stmt3, 6);
	    invoices[i].discounts().push_back(line);
	}

	for (unsigned int j = 0; j < invoices[i].items().size(); ++j) {
	    stmtSetId(stmt4, gltx_id);
	    stmtSetInt(stmt4, j);
	    if (!execute(stmt4)) return false;
	    while (stmt4.next()) {
		int disc_num = stmtGetInt(stmt4, 1);
		fixed total_amt = stmtGetFixed(stmt4, 2);
		invoices[i].items()[j].tdisc_nums.push_back(disc_num);
		invoices[i].items()[j].tdisc_amts.push_back(total_amt);
	    }
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Invoice& invoice)
{
    if (invoice.customerId() != INVALID_ID) {
	Card card;
	if (!lookup(invoice.customerId(), card))
	    return error("Customer doesn't exist");
	if (card.dataType() != DataObject::CUSTOMER)
	    return error("Card isn't a customer");
    }

    if (invoice.lineType() < Invoice::Item)
	return error("Invalid line type");
    if (invoice.lineType() > Invoice::Account)
	return error("Invalid line type");

    for (unsigned int i = 0; i < invoice.items().size(); ++i) {
	const InvoiceItem& line = invoice.items()[i];

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

    // Only check these when finally saving for POS case
    if (invoice.isActive() && invoice.accounts().size() != 0) {
	if (invoice.tenderTotal() + invoice.chargeTotal() != invoice.total())
	    return error("Incomplete tendering");
	if (invoice.customerId() == INVALID_ID && invoice.chargeTotal() != 0.0)
	    return error("Customer is required");
    }

    return validate((Gltx&)invoice);
}

bool
QuasarDB::sqlCreateLines(const Invoice& invoice)
{
    QString cmd1 = insertText("gltx_item", "gltx_id", "seq_num,item_id,"
			      "account_id,number,description,size_name,"
			      "size_qty,quantity,inv_cost,sale_price,"
			      "ext_deposit,voided");
    Stmt stmt1(_connection, cmd1);

    QString cmd2 = insertText("invoice_item", "invoice_id", "seq_num,price_id,"
			      "price_disc,unit_price,for_qty,for_price,"
			      "ext_price,ext_base,ext_tax,tax_id,item_tax_id,"
			      "include_tax,include_deposit,discountable,"
			      "subdept_id,scale,you_save,open_dept");
    Stmt stmt2(_connection, cmd2);

    QString cmd3 = insertText("invoice_item_disc", "invoice_id", "seq_num,"
			      "discount_id,method,amount,account_id,"
			      "total_amt");
    Stmt stmt3(_connection, cmd3);

    QString cmd4 = insertText("invoice_item_tdisc", "invoice_id", "item_num,"
			      "seq_num,disc_num,total_amt");
    Stmt stmt4(_connection, cmd4);

    // Item change info
    vector<Id> item_ids;
    vector<QString> sizes;
    vector<fixed> qtys;
    vector<fixed> costs;
    vector<fixed> prices;

    const vector<InvoiceItem>& items = invoice.items();
    unsigned int i;
    for (i = 0; i < items.size(); ++i) {
	const InvoiceItem& line = items[i];

	stmtSetId(stmt1, invoice.id());
	stmtSetInt(stmt1, i);
	stmtSetId(stmt1, line.item_id);
	stmtSetId(stmt1, line.account_id);
	stmtSetString(stmt1, line.number);
	stmtSetString(stmt1, line.description);
	stmtSetString(stmt1, line.size);
	stmtSetFixed(stmt1, line.size_qty);
	stmtSetFixed(stmt1, line.quantity);
	stmtSetFixed(stmt1, line.inv_cost);
	stmtSetFixed(stmt1, line.sale_price);
	stmtSetFixed(stmt1, line.ext_deposit);
	stmtSetBool(stmt1, line.voided);
	if (!execute(stmt1)) return false;

	stmtSetId(stmt2, invoice.id());
	stmtSetInt(stmt2, i);
	stmtSetId(stmt2, line.price_id);
	stmtSetFixed(stmt2, line.price_disc);
	stmtSetFixed(stmt2, line.price.unitPrice());
	stmtSetFixed(stmt2, line.price.forQty());
	stmtSetFixed(stmt2, line.price.forPrice());
	stmtSetFixed(stmt2, line.ext_price);
	stmtSetFixed(stmt2, line.ext_base);
	stmtSetFixed(stmt2, line.ext_tax);
	stmtSetId(stmt2, line.tax_id);
	stmtSetId(stmt2, line.item_tax_id);
	stmtSetBool(stmt2, line.include_tax);
	stmtSetBool(stmt2, line.include_deposit);
	stmtSetBool(stmt2, line.discountable);
	stmtSetId(stmt2, line.subdept_id);
	stmtSetBool(stmt2, line.scale);
	stmtSetFixed(stmt2, line.you_save);
	stmtSetBool(stmt2, line.open_dept);
	if (!execute(stmt2)) return false;

	if (line.line_disc.discount_id != INVALID_ID) {
	    stmtSetId(stmt3, invoice.id());
	    stmtSetInt(stmt3, i);
	    stmtSetId(stmt3, line.line_disc.discount_id);
	    stmtSetInt(stmt3, line.line_disc.method);
	    stmtSetFixed(stmt3, line.line_disc.amount);
	    stmtSetId(stmt3, line.line_disc.account_id);
	    stmtSetFixed(stmt3, line.line_disc.total_amt);
	    if (!execute(stmt3)) return false;
	}

	for (unsigned int j = 0; j < line.tdisc_nums.size(); ++j) {
	    stmtSetId(stmt4, invoice.id());
	    stmtSetInt(stmt4, i);
	    stmtSetInt(stmt4, j);
	    stmtSetInt(stmt4, line.tdisc_nums[j]);
	    stmtSetFixed(stmt4, line.tdisc_amts[j]);
	    if (!execute(stmt4)) return false;
	}

	if (line.item_id == INVALID_ID) continue;
	if (!invoice.isActive()) continue;
	if (line.voided) continue;

	item_ids.push_back(line.item_id);
	sizes.push_back(line.size);
	if (line.open_dept)
	    qtys.push_back(0);
	else
	    qtys.push_back(line.quantity * line.size_qty);
	costs.push_back(line.inv_cost);
	prices.push_back(line.sale_price);
    }

    itemSale(item_ids, sizes, invoice.storeId(), invoice.postDate(),
	     qtys, costs, prices);

    QString cmd5 = insertText("invoice_discount", "invoice_id", "seq_num,"
			      "discount_id,method,amount,account_id,"
			      "total_amt,voided");
    Stmt stmt5(_connection, cmd5);

    const vector<InvoiceDisc>& discounts = invoice.discounts();
    for (i = 0; i < discounts.size(); ++i) {
	const InvoiceDisc& line = discounts[i];

	stmtSetId(stmt5, invoice.id());
	stmtSetInt(stmt5, i);
	stmtSetId(stmt5, line.discount_id);
	stmtSetInt(stmt5, line.method);
	stmtSetFixed(stmt5, line.amount);
	stmtSetId(stmt5, line.account_id);
	stmtSetFixed(stmt5, line.total_amt);
	stmtSetBool(stmt5, line.voided);
	if (!execute(stmt5)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const Invoice& invoice)
{
    QString cmd = "select item_id,size_name,size_qty,quantity,inv_cost,"
	"sale_price,voided,open_dept from gltx_item join invoice_item on "
	"gltx_item.gltx_id = invoice_item.invoice_id and "
	"gltx_item.seq_num = invoice_item.seq_num where gltx_item.gltx_id=?";
    Stmt stmt(_connection, cmd);
    stmtSetId(stmt, invoice.id());

    // Sales history info
    vector<Id> item_ids;
    vector<QString> sizes;
    vector<fixed> qtys;
    vector<fixed> costs;
    vector<fixed> prices;

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Id item_id = stmtGetId(stmt, 1);
	QString size = stmtGetString(stmt, 2);
	fixed size_qty = stmtGetFixed(stmt, 3);
	fixed quantity = stmtGetFixed(stmt, 4);
	fixed inv_cost = stmtGetFixed(stmt, 5);
	fixed sale_price = stmtGetFixed(stmt, 6);
	bool voided = stmtGetBool(stmt, 7);
	bool open_dept = stmtGetBool(stmt, 8);

	if (item_id == INVALID_ID) continue;
	if (!invoice.isActive()) continue;
	if (voided) continue;

	item_ids.push_back(item_id);
	sizes.push_back(size);
	if (open_dept)
	    qtys.push_back(0);
	else
	    qtys.push_back(-quantity * size_qty);
	costs.push_back(-inv_cost);
	prices.push_back(-sale_price);
    }

    itemSale(item_ids, sizes, invoice.storeId(), invoice.postDate(),
	     qtys, costs, prices);

    if (!remove(invoice, "invoice_item", "invoice_id")) return false;
    if (!remove(invoice, "invoice_item_disc", "invoice_id")) return false;
    if (!remove(invoice, "invoice_item_tdisc", "invoice_id")) return false;
    if (!remove(invoice, "invoice_discount", "invoice_id")) return false;
    if (!remove(invoice, "gltx_item", "gltx_id")) return false;

    return true;
}

static void
addTax(Invoice& invoice, fixed taxable, vector<Id>& ids, vector<fixed>& amts)
{
    for (unsigned int i = 0; i < ids.size(); ++i) {
	Id tax_id = ids[i];
	fixed tax_amt = amts[i];

	int found = -1;
	for (unsigned int j = 0; j < invoice.taxes().size(); ++j) {
	    TaxLine& line = invoice.taxes()[j];
	    if (line.tax_id == tax_id) {
		found = j;
		break;
	    }
	}

	if (found >= 0) {
	    invoice.taxes()[found].taxable += taxable;
	    invoice.taxes()[found].amount += tax_amt;
	} else {
	    TaxLine line(tax_id, taxable, tax_amt);
	    invoice.taxes().push_back(line);
	}
    }
}

static void
addTaxInc(Invoice& invoice,fixed taxable, vector<Id>& ids, vector<fixed>& amts)
{
    for (unsigned int i = 0; i < ids.size(); ++i) {
	Id tax_id = ids[i];
	fixed tax_amt = amts[i];

	int found = -1;
	for (unsigned int j = 0; j < invoice.taxes().size(); ++j) {
	    TaxLine& line = invoice.taxes()[j];
	    if (line.tax_id == tax_id) {
		found = j;
		break;
	    }
	}

	if (found >= 0) {
	    invoice.taxes()[found].inc_taxable += taxable;
	    invoice.taxes()[found].inc_amount += tax_amt;
	} else {
	    TaxLine line(tax_id, 0.0, 0.0, taxable, tax_amt);
	    invoice.taxes().push_back(line);
	}
    }
}

typedef QPair<Id, fixed> InfoPair;

static void
addInfo(vector<InfoPair>& info, Id id, fixed amount)
{
    if (id == INVALID_ID) return;
    for (unsigned int i = 0; i < info.size(); ++i) {
	if (info[i].first == id) {
	    info[i].second += amount;
	    return;
	}
    }
    info.push_back(InfoPair(id, amount));
}

bool
QuasarDB::prepare(Invoice& invoice, ObjectCache& cache)
{
    // Clear things to be filled in
    invoice.accounts().clear();
    invoice.taxes().clear();
    invoice.cards().clear();

    // Account postings
    vector<InfoPair> income_info;
    vector<InfoPair> expense_info;
    vector<InfoPair> asset_info;
    vector<InfoPair> tax_info;

    // Need company for deposit account
    Company company;
    lookup(company);

    // Need customer for posting account
    Customer customer;
    cache.findCustomer(invoice.customerId(), customer);

    // Get tax exempt
    Tax exempt;
    cache.findTax(invoice.taxExemptId(), exempt);

    // Get sign from return flag
    fixed sign = 1;
    if (invoice.isReturn()) sign = -1;

    // Total up discountable, calculate line discount, and clear tx discounts
    fixed discountable = 0.0;
    for (unsigned int i = 0; i < invoice.items().size(); ++i) {
	InvoiceItem& line = invoice.items()[i];
	if (line.item_id == INVALID_ID && line.account_id == INVALID_ID)
	    continue;

	// Calculate base price and tax
	line.ext_base = line.ext_price;
	if (line.include_deposit)
	    line.ext_base -= line.ext_deposit;
	if (line.include_tax && line.tax_id != INVALID_ID) {
	    Tax tax;
	    cache.findTax(line.tax_id, tax);
	    line.ext_tax = calculateTaxOff(cache, tax, line.ext_base);
	    line.ext_base -= line.ext_tax;
	}

	// Clear old discount information
	line.line_disc.total_amt = 0.0;
	line.tdisc_nums.clear();
	line.tdisc_amts.clear();
	if (!line.discountable) continue;

	// Calculate line discount
	InvoiceDisc& disc = line.line_disc;
	if (disc.discount_id != INVALID_ID) {
	    if (disc.method == Discount::PERCENT) {
		disc.total_amt = line.ext_base * disc.amount / 100.0;
	    } else {
		disc.total_amt = disc.amount * line.quantity;
	    }
	    disc.total_amt.moneyRound();
	}

	discountable += line.ext_base - disc.total_amt;
    }

    // Calculate transaction discounts and distribute over items
    fixed remain_disc = discountable;
    for (unsigned int i = 0; i < invoice.discounts().size(); ++i) {
	InvoiceDisc& discount = invoice.discounts()[i];
	if (discount.discount_id == INVALID_ID) continue;

	// Calculate discount amount
	if (discount.method == Discount::PERCENT) {
	    discount.total_amt = remain_disc * discount.amount / 100.0;
	    discount.total_amt.moneyRound();
	} else {
	    discount.total_amt = discount.amount;
	    if (discount.total_amt > remain_disc)
		discount.total_amt = remain_disc;
	}
	remain_disc -= discount.total_amt;

	// Distribute back over items
	fixed remain = discount.total_amt;
	int last = -1;
	for (unsigned int j = 0; j < invoice.items().size(); ++j) {
	    InvoiceItem& line = invoice.items()[j];
	    if (line.item_id == INVALID_ID && line.account_id == INVALID_ID)
		continue;
	    if (!line.discountable) continue;

	    fixed line_amt = line.ext_base - line.line_disc.total_amt;
	    fixed alloc = line_amt * discount.total_amt / discountable;
	    alloc.moneyRound();

	    line.tdisc_nums.push_back(i);
	    line.tdisc_amts.push_back(alloc);

	    remain -= alloc;
	    last = j;
	}

	// If didn't divide out perfectly, add remaining to last line
	if (remain != 0.0 && last != -1) {
	    int index = invoice.items()[last].tdisc_amts.size() - 1;
	    invoice.items()[last].tdisc_amts[index] += remain;
	}
    }

    // Total items/discount/deposits and taxable
    fixed item_total = 0.0;
    fixed deposit_total = 0.0;
    fixed ldisc_total = 0.0;
    fixed tdisc_total = 0.0;
    for (unsigned int i = 0; i < invoice.items().size(); ++i) {
	InvoiceItem& line = invoice.items()[i];
	if (line.item_id == INVALID_ID && line.account_id == INVALID_ID)
	    continue;

	Item item;
	cache.findItem(line.item_id, item);

	fixed ldisc_amt = line.line_disc.total_amt;
	fixed tdisc_amt = 0.0;
	for (unsigned int j = 0; j < line.tdisc_amts.size(); ++j)
	    tdisc_amt += line.tdisc_amts[j];

	// Calculate base price and prepare tax info
	fixed base = line.ext_base - ldisc_amt - tdisc_amt;
	Tax tax;
	cache.findTax(line.tax_id, tax);
	vector<Id> tax_ids;
	vector<fixed> tax_amts;

	// Calculate taxes
	if (line.include_tax && ldisc_amt == 0.0 && tdisc_amt == 0.0 &&
		exempt.id() == INVALID_ID && (line.item_id == INVALID_ID ||
		line.tax_id == line.item_tax_id)) {
	    base = line.ext_price;
	    if (line.include_deposit)
		base -= line.ext_deposit;
	    line.ext_tax = calculateTaxOff(cache, tax, base, tax_ids, tax_amts);
	    addTaxInc(invoice, base, tax_ids, tax_amts);
	    base -= line.ext_tax;
	} else {
	    line.ext_tax = calculateTaxOn(cache, tax, base, exempt, tax_ids,
					  tax_amts);
	    addTax(invoice, base, tax_ids, tax_amts);
	}

	item_total += line.ext_base;
	deposit_total += line.ext_deposit;
	ldisc_total += ldisc_amt;
	tdisc_total += tdisc_amt;
    }

    // Round taxes to two decimal places and total up
    fixed tax_total = 0.0;
    for (unsigned int i = 0; i < invoice.taxes().size(); ++i) {
	TaxLine& line = invoice.taxes()[i];
	Id tax_id = invoice.taxes()[i].tax_id;

	// Check for tax exemptions
	if (exempt.id() != INVALID_ID) {
	    bool found = false;
	    if (line.tax_id == exempt.id()) {
		found = true;
	    } else if (exempt.isGroup()) {
		for (unsigned int j = 0; j < exempt.group_ids().size(); ++j) {
		    if (line.tax_id == exempt.group_ids()[j]) {
			found = true;
			break;
		    }
		}
	    }
	    if (found) {
		line.amount = 0.0;
		line.inc_amount = 0.0;
	    }
	}

	// NOTE: don't need to round inc_amount since its always returned
	// rounded from the tax calculation.
	line.amount.moneyRound();

	tax_total += line.amount + line.inc_amount;
    }

    fixed total = item_total - ldisc_total - tdisc_total + tax_total +
	deposit_total;

    // Account postings for tenders
    for (unsigned int i = 0; i < invoice.tenders().size(); ++i) {
	TenderLine& line = invoice.tenders()[i];

	Tender tender;
	cache.findTender(line.tender_id, tender);

	AccountLine info;
	info.account_id = tender.accountId();
	info.amount = line.amount;
	invoice.accounts().push_back(info);
    }

    // Add charge if not fully tendered
    if (invoice.tenderTotal() != total) {
	fixed amount = total - invoice.tenderTotal();
	invoice.cards().push_back(CardLine(customer.id(), amount));
	invoice.accounts().push_back(AccountLine(customer.accountId(), amount));
    }

    // Account postings for items
    for (unsigned int i = 0; i < invoice.items().size(); ++i) {
	InvoiceItem& line = invoice.items()[i];

	Item item;
	cache.findItem(line.item_id, item);

	fixed sale_price = line.ext_base;
	if (line.line_disc.account_id == INVALID_ID) {
	    sale_price -= line.line_disc.total_amt;
	} else {
	    addInfo(expense_info, line.line_disc.account_id,
		    line.line_disc.total_amt);
	}
	for (unsigned int j = 0; j < line.tdisc_nums.size(); ++j) {
	    InvoiceDisc& disc = invoice.discounts()[line.tdisc_nums[j]];
	    if (disc.account_id != INVALID_ID) continue;
	    sale_price -= line.tdisc_amts[j];
	}

	if (invoice.lineType() == Invoice::Account) {
	    addInfo(income_info, line.account_id, -sale_price);
	} else {
	    addInfo(income_info, item.incomeAccount(), -sale_price);
	    addInfo(expense_info, company.depositAccount(), -line.ext_deposit);
	    if (item.isInventoried()) {
		addInfo(asset_info, item.assetAccount(), -line.inv_cost);
		addInfo(expense_info, item.expenseAccount(), line.inv_cost);
	    }
	}
    }

    // Account postings for discounts
    for (unsigned int i = 0; i < invoice.discounts().size(); ++i) {
	const InvoiceDisc& line = invoice.discounts()[i];
	if (line.discount_id == INVALID_ID) continue;

	if (line.account_id != INVALID_ID)
	    addInfo(expense_info, line.account_id, line.total_amt);
    }

    // Account postings for taxes
    for (unsigned int i = 0; i < invoice.taxes().size(); ++i) {
	const TaxLine& line = invoice.taxes()[i];

	Tax tax;
	cache.findTax(line.tax_id, tax);

	fixed total_amt = line.amount + line.inc_amount;
	addInfo(tax_info, tax.collectedAccount(), -total_amt);
    }

    // Post the taxes
    for (unsigned int i = 0; i < tax_info.size(); ++i) {
	Id account_id = tax_info[i].first;
	fixed amount = tax_info[i].second * sign;
	if (amount != 0.0)
	    invoice.accounts().push_back(AccountLine(account_id, amount));
    }

    // Post the sales
    for (unsigned int i = 0; i < income_info.size(); ++i) {
	Id account_id = income_info[i].first;
	fixed amount = income_info[i].second * sign;
	if (amount != 0.0)
	    invoice.accounts().push_back(AccountLine(account_id, amount));
    }

    // Post the expenses
    for (unsigned int i = 0; i < expense_info.size(); ++i) {
	Id account_id = expense_info[i].first;
	fixed amount = expense_info[i].second * sign;
	if (amount != 0.0)
	    invoice.accounts().push_back(AccountLine(account_id, amount));
    }

    // Post the asset adjustments
    for (unsigned int i = 0; i < asset_info.size(); ++i) {
	Id account_id = asset_info[i].first;
	fixed amount = asset_info[i].second * sign;
	if (amount != 0.0)
	    invoice.accounts().push_back(AccountLine(account_id, amount));
    }

    // TODO: signs for returns?

    return true;
}
