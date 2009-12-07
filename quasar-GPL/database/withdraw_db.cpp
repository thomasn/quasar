// $Id: withdraw_db.cpp,v 1.9 2005/03/01 19:59:43 bpepers Exp $
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

#include "withdraw.h"
#include "withdraw_select.h"
#include "card.h"

// Create a Withdraw
bool
QuasarDB::create(Withdraw& withdraw)
{
    if (!validate(withdraw)) return false;
    if (!create((Gltx&)withdraw)) return false;

    commit();
    dataSignal(DataEvent::Insert, withdraw);
    return true;
}

// Delete a Withdraw
bool
QuasarDB::remove(const Withdraw& withdraw)
{
    if (withdraw.id() == INVALID_ID) return false;
    if (!remove((Gltx&)withdraw)) return false;

    commit();
    dataSignal(DataEvent::Delete, withdraw);
    return true;
}

// Update a Withdraw
bool
QuasarDB::update(const Withdraw& orig, Withdraw& withdraw)
{
    if (orig.id() == INVALID_ID || withdraw.id() == INVALID_ID) return false;
    if (!validate(withdraw)) return false;
    if (!update(orig, (Gltx&)withdraw)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Withdraw.
bool
QuasarDB::lookup(Id withdraw_id, Withdraw& withdraw)
{
    if (withdraw_id == INVALID_ID) return false;
    WithdrawSelect conditions;
    vector<Withdraw> withdraws;

    conditions.id = withdraw_id;
    if (!select(withdraws, conditions)) return false;
    if (withdraws.size() != 1) return false;

    withdraw = withdraws[0];
    return true;
}

// Returns a vector of Withdraws.
bool
QuasarDB::select(vector<Withdraw>& withdraws, const WithdrawSelect& conditions)
{
    withdraws.clear();

    QString cmd = gltxCmd("gltx", "gltx_id", "", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Withdraw withdraw;
	int next = 1;
	selectData(withdraw, stmt, next);
	selectGltx(withdraw, stmt, next);
	withdraws.push_back(withdraw);
    }

    for (unsigned int i = 0; i < withdraws.size(); ++i) {
	Id gltx_id = withdraws[i].id();
	GLTX_ACCOUNTS(withdraws);
	GLTX_CARDS(withdraws);
	GLTX_PAYMENTS(withdraws);
	GLTX_TENDERS(withdraws);
	GLTX_REFERENCES(withdraws);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Withdraw& withdraw)
{
    Card card;
    if (!lookup(withdraw.cardId(), card))
	return error("Customer doesn't exist");
    if (card.dataType() != DataObject::CUSTOMER)
	return error("Card isn't a customer");

    if (withdraw.accounts().size() > 0 && withdraw.isActive())
	if (withdraw.tenderTotal() != withdraw.total())
	    return error("Incomplete tendering");

    return validate((Gltx&)withdraw);
}
