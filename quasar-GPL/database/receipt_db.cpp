// $Id: receipt_db.cpp,v 1.31 2005/03/01 19:59:42 bpepers Exp $
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

#include "receipt.h"
#include "receipt_select.h"
#include "card.h"

// Create a Receipt
bool
QuasarDB::create(Receipt& receipt)
{
    if (!validate(receipt)) return false;
    if (!create((Gltx&)receipt)) return false;

    QString cmd = insertText("receipt", "receipt_id", "disc_id");
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, receipt.id());
    stmtSetId(stmt, receipt.discountId());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, receipt);
    return true;
}

// Delete a Receipt
bool
QuasarDB::remove(const Receipt& receipt)
{
    if (receipt.id() == INVALID_ID) return false;
    if (!remove((Gltx&)receipt)) return false;

    commit();
    dataSignal(DataEvent::Delete, receipt);
    return true;
}

// Update a Receipt
bool
QuasarDB::update(const Receipt& orig, Receipt& receipt)
{
    if (orig.id() == INVALID_ID || receipt.id() == INVALID_ID) return false;
    if (!validate(receipt)) return false;
    if (!update(orig, (Gltx&)receipt)) return false;

    // Update the receipt tables
    QString cmd = updateText("receipt", "receipt_id", "disc_id");
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, receipt.discountId());
    stmtSetId(stmt, receipt.id());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Receipt.
bool
QuasarDB::lookup(Id receipt_id, Receipt& receipt)
{
    if (receipt_id == INVALID_ID) return false;
    ReceiptSelect conditions;
    vector<Receipt> receipts;

    conditions.id = receipt_id;
    if (!select(receipts, conditions)) return false;
    if (receipts.size() != 1) return false;

    receipt = receipts[0];
    return true;
}

// Returns a vector of Receipts.
bool
QuasarDB::select(vector<Receipt>& receipts, const ReceiptSelect& conditions)
{
    receipts.clear();

    QString cmd = gltxCmd("receipt", "receipt_id", "disc_id", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Receipt receipt;
	int next = 1;
	selectData(receipt, stmt, next);
	selectGltx(receipt, stmt, next);
	receipt.setDiscountId(stmtGetId(stmt, next++));
	receipts.push_back(receipt);
    }

    for (unsigned int i = 0; i < receipts.size(); ++i) {
	Id gltx_id = receipts[i].id();
	GLTX_ACCOUNTS(receipts);
	GLTX_CARDS(receipts);
	GLTX_PAYMENTS(receipts);
	GLTX_TENDERS(receipts);
	GLTX_REFERENCES(receipts);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Receipt& receipt)
{
    Card card;
    if (!lookup(receipt.cardId(), card))
	return error("Customer doesn't exist");
    if (card.dataType() != DataObject::CUSTOMER)
	return error("Card isn't a customer");

    fixed totalPaid = 0.0;
    for (unsigned int i = 0; i < receipt.payments().size(); ++i) {
	const PaymentLine& line = receipt.payments()[i];

	Gltx gltx;
	if (!lookup(line.gltx_id, gltx))
	    return error("Transaction doesn't exist");

	totalPaid += line.amount + line.discount;
    }

    if (-totalPaid > -receipt.cardTotal() && receipt.accounts().size() > 0) {
	return error("Payment allocations larger than amount");
    }

    if (receipt.accounts().size() > 0 && receipt.isActive())
	if (receipt.tenderTotal() != receipt.total())
	    return error("Incomplete tendering");

    return validate((Gltx&)receipt);
}
