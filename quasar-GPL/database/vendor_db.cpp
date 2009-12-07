// $Id: vendor_db.cpp,v 1.29 2005/03/01 19:59:42 bpepers Exp $
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

#include "vendor.h"
#include "vendor_select.h"
#include "account.h"

// Create a Vendor
bool
QuasarDB::create(Vendor& vendor)
{
    if (!validate(vendor)) return false;
    if (!create((Card&)vendor)) return false;

    QString cmd = insertText("card_vendor", "vendor_id", "account_id,"
			     "term_id,backorders");
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, vendor.id());
    stmtSetId(stmt, vendor.accountId());
    stmtSetId(stmt, vendor.termsId());
    stmtSetBool(stmt, vendor.backorders());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, vendor);
    return true;
}

// Delete a Vendor
bool
QuasarDB::remove(const Vendor& vendor)
{
    if (vendor.id() == INVALID_ID) return false;
    if (!remove((Card&)vendor)) return false;

    commit();
    dataSignal(DataEvent::Delete, vendor);
    return true;
}

// Update a Vendor
bool
QuasarDB::update(const Vendor& orig, Vendor& vendor)
{
    if (orig.id() == INVALID_ID || vendor.id() == INVALID_ID) return false;
    if (!validate(vendor)) return false;
    if (!update(orig, (Card&)vendor)) return false;

    // Update the vendor tables
    QString cmd = updateText("card_vendor", "vendor_id", "account_id,"
			     "term_id,backorders");
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, vendor.accountId());
    stmtSetId(stmt, vendor.termsId());
    stmtSetBool(stmt, vendor.backorders());
    stmtSetId(stmt, vendor.id());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Vendor.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id vendor_id, Vendor& vendor)
{
    if (vendor_id == INVALID_ID) return false;
    VendorSelect conditions;
    vector<Vendor> vendors;

    conditions.id = vendor_id;
    if (!select(vendors, conditions)) return false;
    if (vendors.size() != 1) return false;

    vendor = vendors[0];
    return true;
}

// Lookup a Vendor.  This is just a shortcut to using select.
bool
QuasarDB::lookup(const QString& number, Vendor& vendor)
{
    if (number.isEmpty()) return false;
    VendorSelect conditions;
    vector<Vendor> vendors;

    conditions.number = number;
    if (!select(vendors, conditions)) return false;
    if (vendors.size() != 1) return false;

    vendor = vendors[0];
    return true;
}

// Returns a vector of Vendors.  Returns vendors sorted by name.
bool
QuasarDB::select(vector<Vendor>& vendors, const VendorSelect& conditions)
{
    vendors.clear();

    QString table = "card_vendor join card on card.card_id = "
	"card_vendor.vendor_id";

    QString cmd = selectCmd(table, "card_id", "company,first_name,"
			    "last_name,number,street,street2,city,"
			    "province,country,postal,phone_num,phone2_num,"
			    "fax_num,email,web_page,contact,comments,"
			    "patgroup_id,account_id,term_id,backorders",
			    conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Vendor vendor;
	int next = 1;
	selectData(vendor, stmt, next);
	selectCard(vendor, stmt, next);
	vendor.setAccountId(stmtGetId(stmt, next++));
	vendor.setTermsId(stmtGetId(stmt, next++));
	vendor.setBackorders(stmtGetBool(stmt, next++));
	vendors.push_back(vendor);
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

    for (unsigned int i = 0; i < vendors.size(); ++i) {
	Id card_id = vendors[i].id();

	stmtSetId(stmt1, card_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    Id group_id = stmtGetId(stmt1, 1);
	    vendors[i].groups().push_back(group_id);
	}

	stmtSetId(stmt2, card_id);
	if (!execute(stmt2)) return false;
	while (stmt2.next()) {
	    Id discount_id = stmtGetId(stmt2, 1);
	    vendors[i].discounts().push_back(discount_id);
	}

	stmtSetId(stmt3, card_id);
	if (!execute(stmt3)) return false;
	while (stmt3.next()) {
	    Id extra_id = stmtGetId(stmt3, 1);
	    QString value = stmtGetString(stmt3, 2);
	    vendors[i].setValue(extra_id, value);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Vendor& vendor)
{
    if (!validate((Card&)vendor)) return false;

    Account account;
    if (!lookup(vendor.accountId(), account))
	return error("GL account doesn't exist");
    if (account.isHeader())
	return error("GL account is a header");

    return true;
}
