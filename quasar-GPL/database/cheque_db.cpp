// $Id: cheque_db.cpp,v 1.29 2005/03/01 19:59:42 bpepers Exp $
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

#include "cheque.h"
#include "cheque_select.h"
#include "card.h"
#include "account.h"

// Create a Cheque
bool
QuasarDB::create(Cheque& cheque)
{
    if (!validate(cheque)) return false;
    if (!create((Gltx&)cheque)) return false;

    QString cmd = insertText("cheque", "cheque_id", "cheque_type,"
			     "account_id,disc_id,address");
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, cheque.id());
    stmtSetInt(stmt, cheque.type());
    stmtSetId(stmt, cheque.accountId());
    stmtSetId(stmt, cheque.discountId());
    stmtSetString(stmt, cheque.address());
    if (!execute(stmt)) return false;

    // Set next number in account
    cmd = "update account set next_number = ? where account_id = ?";
    stmt.setCommand(cmd);
    stmtSetInt(stmt, cheque.number().toInt() + 1);
    stmtSetId(stmt, cheque.accountId());
    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, cheque);
    return true;
}

// Delete a Cheque
bool
QuasarDB::remove(const Cheque& cheque)
{
    if (cheque.id() == INVALID_ID) return false;
    if (!remove((Gltx&)cheque)) return false;

    commit();
    dataSignal(DataEvent::Delete, cheque);
    return true;
}

// Update a Cheque
bool
QuasarDB::update(const Cheque& orig, Cheque& cheque)
{
    if (orig.id() == INVALID_ID || cheque.id() == INVALID_ID) return false;
    if (!validate(cheque)) return false;
    if (!update(orig, (Gltx&)cheque)) return false;

    // Update the cheque tables
    QString cmd = updateText("cheque", "cheque_id", "cheque_type,"
			     "account_id,disc_id,address");
    Stmt stmt(_connection, cmd);

    stmtSetInt(stmt, cheque.type());
    stmtSetId(stmt, cheque.accountId());
    stmtSetId(stmt, cheque.discountId());
    stmtSetString(stmt, cheque.address());
    stmtSetId(stmt, cheque.id());
    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Cheque.
bool
QuasarDB::lookup(Id cheque_id, Cheque& cheque)
{
    if (cheque_id == INVALID_ID) return false;
    ChequeSelect conditions;
    vector<Cheque> cheques;

    conditions.id = cheque_id;
    if (!select(cheques, conditions)) return false;
    if (cheques.size() != 1) return false;

    cheque = cheques[0];
    return true;
}

// Returns a vector of Cheques.
bool
QuasarDB::select(vector<Cheque>& cheques, const ChequeSelect& conditions)
{
    cheques.clear();

    QString cmd = gltxCmd("cheque", "cheque_id", "cheque_type,"
			  "account_id,disc_id,address", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Cheque cheque;
	int next = 1;
	selectData(cheque, stmt, next);
	selectGltx(cheque, stmt, next);
	cheque.setType(Cheque::Type(stmtGetInt(stmt, next++)));
	cheque.setAccountId(stmtGetId(stmt, next++));
	cheque.setDiscountId(stmtGetId(stmt, next++));
	cheque.setAddress(stmtGetString(stmt, next++));
	cheques.push_back(cheque);
    }

    for (unsigned int i = 0; i < cheques.size(); ++i) {
	Id gltx_id = cheques[i].id();
	GLTX_ACCOUNTS(cheques);
	GLTX_CARDS(cheques);
	GLTX_PAYMENTS(cheques);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Cheque& cheque)
{
    Card card;
    lookup(cheque.cardId(), card);

    switch (cheque.type()) {
    case Cheque::ACCOUNT:
	// Note: any card type is allowed
	break;
    case Cheque::VENDOR:
	if (card.id() == INVALID_ID)
	    return error("Vendor is required");
	if (card.dataType() != DataObject::VENDOR)
	    return error("Card isn't a vendor");
	break;
    case Cheque::CUSTOMER:
	if (card.id() == INVALID_ID)
	    return error("Customer is reuqired");
	if (card.dataType() != DataObject::CUSTOMER)
	    return error("Card isn't a customer");
	break;
    }

    if (cheque.total() == 0.0)
	return error("Cannot create a zero value cheque");

    fixed totalPaid = 0.0;
    for (unsigned int i = 0; i < cheque.payments().size(); ++i) {
	const PaymentLine& line = cheque.payments()[i];

	Gltx gltx;
	if (!lookup(line.gltx_id, gltx))
	    return error("Transaction doesn't exist");

	totalPaid += line.amount;
    }

    if (totalPaid.abs() > cheque.cardTotal().abs()) {
	return error("Payment allocations larger than amount");
    }

    return validate((Gltx&)cheque);
}
