// $Id: store_db.cpp,v 1.15 2005/03/15 15:36:01 bpepers Exp $
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

#include "store.h"
#include "store_select.h"

// Create a Store
bool
QuasarDB::create(Store& store)
{
    if (!validate(store)) return false;

    // Auto allocate store number
    if (store.number().stripWhiteSpace() == "#") {
	fixed number = uniqueNumber("store", "number");
	store.setNumber(number.toString());
    }

    QString cmd = insertCmd("store", "store_id", "name,number,has_addr,"
			    "street,street2,city,province,country,"
			    "postal,phone_num,phone2_num,fax_num,email,"
			    "web_page,contact,can_sell,year_end_id,"
			    "company_id");
    Stmt stmt(_connection, cmd);

    insertData(store, stmt);
    stmtSetString(stmt, store.name());
    stmtSetString(stmt, store.number());
    stmtSetBool(stmt, store.hasAddress());
    stmtSetString(stmt, store.street());
    stmtSetString(stmt, store.street2());
    stmtSetString(stmt, store.city());
    stmtSetString(stmt, store.province());
    stmtSetString(stmt, store.country());
    stmtSetString(stmt, store.postal());
    stmtSetString(stmt, store.phoneNumber());
    stmtSetString(stmt, store.phone2Number());
    stmtSetString(stmt, store.faxNumber());
    stmtSetString(stmt, store.email());
    stmtSetString(stmt, store.webPage());
    stmtSetString(stmt, store.contact());
    stmtSetBool(stmt, store.canSell());
    stmtSetId(stmt, store.yearEndTransferId());
    stmtSetId(stmt, store.companyId());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, store);
    return true;
}

// Delete a Store
bool
QuasarDB::remove(const Store& store)
{
    if (store.id() == INVALID_ID) return false;
    if (!removeData(store, "store", "store_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, store);
    return true;
}

// Update a Store
bool
QuasarDB::update(const Store& orig, Store& store)
{
    if (orig.id() == INVALID_ID || store.id() == INVALID_ID) return false;
    if (!validate(store)) return false;

    if (store.number().stripWhiteSpace() == "#")
	return error("Can't change number to '#'");

    // Update the store table
    QString cmd = updateCmd("store", "store_id", "name,number,has_addr,"
			    "street,street2,city,province,country,"
			    "postal,phone_num,phone2_num,fax_num,email,"
			    "web_page,contact,can_sell,year_end_id,"
			    "company_id");
    Stmt stmt(_connection, cmd);

    updateData(orig, store, stmt);
    stmtSetString(stmt, store.name());
    stmtSetString(stmt, store.number());
    stmtSetBool(stmt, store.hasAddress());
    stmtSetString(stmt, store.street());
    stmtSetString(stmt, store.street2());
    stmtSetString(stmt, store.city());
    stmtSetString(stmt, store.province());
    stmtSetString(stmt, store.country());
    stmtSetString(stmt, store.postal());
    stmtSetString(stmt, store.phoneNumber());
    stmtSetString(stmt, store.phone2Number());
    stmtSetString(stmt, store.faxNumber());
    stmtSetString(stmt, store.email());
    stmtSetString(stmt, store.webPage());
    stmtSetString(stmt, store.contact());
    stmtSetBool(stmt, store.canSell());
    stmtSetId(stmt, store.yearEndTransferId());
    stmtSetId(stmt, store.companyId());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Store.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id store_id, Store& store)
{
    if (store_id == INVALID_ID) return false;
    StoreSelect conditions;
    vector<Store> stores;

    conditions.id = store_id;
    if (!select(stores, conditions)) return false;
    if (stores.size() != 1) return false;

    store = stores[0];
    return true;
}

// Lookup a Store by its number
bool
QuasarDB::lookup(const QString& number, Store& store)
{
    if (number.isEmpty()) return false;
    StoreSelect conditions;
    vector<Store> stores;

    conditions.number = number;
    if (!select(stores, conditions)) return false;
    if (stores.size() != 1) return false;

    store = stores[0];
    return true;
}

// Returns a vector of Store's.
bool
QuasarDB::select(vector<Store>& stores, const StoreSelect& conditions)
{
    stores.clear();

    QString cmd = selectCmd("store", "store_id", "name,number,has_addr,"
			    "street,street2,city,province,country,"
			    "postal,phone_num,phone2_num,fax_num,email,"
			    "web_page,contact,can_sell,year_end_id,"
			    "company_id", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Store store;
	int next = 1;
	selectData(store, stmt, next);
	store.setName(stmtGetString(stmt, next++));
	store.setNumber(stmtGetString(stmt, next++));
	store.setHasAddress(stmtGetBool(stmt, next++));
	store.setStreet(stmtGetString(stmt, next++));
	store.setStreet2(stmtGetString(stmt, next++));
	store.setCity(stmtGetString(stmt, next++));
	store.setProvince(stmtGetString(stmt, next++));
	store.setCountry(stmtGetString(stmt, next++));
	store.setPostal(stmtGetString(stmt, next++));
	store.setPhoneNumber(stmtGetString(stmt, next++));
	store.setPhone2Number(stmtGetString(stmt, next++));
	store.setFaxNumber(stmtGetString(stmt, next++));
	store.setEmail(stmtGetString(stmt, next++));
	store.setWebPage(stmtGetString(stmt, next++));
	store.setContact(stmtGetString(stmt, next++));
	store.setCanSell(stmtGetBool(stmt, next++));
	store.setYearEndTransferId(stmtGetId(stmt, next++));
	store.setCompanyId(stmtGetId(stmt, next++));
	stores.push_back(store);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Store& store)
{
    if (!validate((DataObject&)store)) return false;

    if (store.name().stripWhiteSpace().isEmpty())
	return error("Blank store name");

    if (store.number().stripWhiteSpace().isEmpty())
	return error("Blank store number");

    return true;
}
