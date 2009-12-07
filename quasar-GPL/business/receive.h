// $Id: receive.h,v 1.24 2005/01/30 00:51:13 bpepers Exp $
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

#ifndef RECEIVE_H
#define RECEIVE_H

#include "gltx.h"
#include "price.h"

struct ReceiveItem: public ItemLine {
    ReceiveItem();

    fixed ordered;		// Quantity ordered
    fixed received;		// Quantity received
    Id cost_id;			// ItemPrice id used (can be INVALID_ID)
    fixed cost_disc;		// Orderer cost change discount
    Price cost;			// Cost item has purchased at
    fixed ext_cost;		// Calculated cost from quantity
    fixed ext_base;		// Total (minus discounts and included tax/dep)
    fixed ext_tax;		// Total taxes
    fixed int_charges;		// Internal charges
    fixed ext_charges;		// External charges
    Id tax_id;			// Tax to use for line
    Id item_tax_id;		// Normal tax for item
    bool include_tax;		// Cost includes tax?
    bool include_deposit;	// Cost includes deposit?
    bool open_dept;		// Open dept item?

    bool operator==(const ReceiveItem& rhs) const;
    bool operator!=(const ReceiveItem& rhs) const;
};

struct ReceiveCharge {
    ReceiveCharge(Id charge_id=INVALID_ID, Id tax_id=INVALID_ID,
		  fixed amount=0.0, fixed base=0.0, bool internal=true);

    Id charge_id;
    Id tax_id;
    fixed amount;
    fixed base;
    bool internal;

    bool operator==(const ReceiveCharge& rhs) const;
    bool operator!=(const ReceiveCharge& rhs) const;
};

class Receive: public Gltx {
public:
    // Constructors and Destructor
    Receive();
    virtual ~Receive();

    enum LineType { Item, Account };

    // Get methods
    Id vendorId()			const { return _card_id; }
    QString vendorAddress()		const { return _vendor_addr; }
    Id shipId()				const { return _ship_id; }
    QString shipAddress()		const { return _ship_addr; }
    QString shipVia()			const { return _ship_via; }
    Id termsId()			const { return _term_id; }
    QString comment()			const { return _comment; }
    QDate invoiceDate()			const { return _invoice_date; }
    LineType lineType()			const { return _line_type; }
    bool isClosed()			const { return _closed; }
    bool isClaim() const;

    // Set methods
    void setVendorId(Id id)			{ _card_id = id; }
    void setVendorAddress(const QString& addr)	{ _vendor_addr = addr; }
    void setShipId(Id id)			{ _ship_id = id; }
    void setShipAddress(const QString& addr)	{ _ship_addr = addr; }
    void setShipVia(const QString& text)	{ _ship_via = text; }
    void setTermsId(Id term_id)			{ _term_id = term_id; }
    void setComment(const QString& text)	{ _comment = text; }
    void setInvoiceDate(QDate date)		{ _invoice_date = date; }
    void setLineType(LineType type)		{ _line_type = type; }
    void setClosed(bool flag)			{ _closed = flag; }
    void setClaim(bool flag);

    // Item lines
    vector<ReceiveItem>& items()		{ return _items; }
    const vector<ReceiveItem>& items() const	{ return _items; }
    unsigned int itemCnt() const		{ return _items.size(); }
    const ItemLine& item(unsigned int i) const	{ return _items[i]; }
    fixed costTotal() const;

    // Order lines
    vector<Id>& orders()			{ return _orders; }
    const vector<Id>& orders() const		{ return _orders; }

    // Slip lines
    vector<Id>& slips()				{ return _slips; }
    const vector<Id>& slips() const		{ return _slips; }

    // Charges
    vector<ReceiveCharge>& charges()		 { return _charges; }
    const vector<ReceiveCharge>& charges() const { return _charges; }
    fixed chargeTotal() const;

    // Special methods
    fixed total() const;
    QString lineTypeName() const;
    static QString lineTypeName(LineType type);

    // Operations
    bool operator==(const Receive& rhs) const;
    bool operator!=(const Receive& rhs) const;

protected:
    QString _vendor_addr;		// Vendor address
    Id _ship_id;			// Ship to vendor
    QString _ship_addr;			// Shipping address
    QString _ship_via;			// Method of shipment
    Id _term_id;			// Terms (Net 30, ...)
    QString _comment;			// Comments to show on invoice
    QDate _invoice_date;		// Date on invoice (for terms)
    LineType _line_type;		// Type of lines (Item or Account)
    bool _closed;			// Closed?
    vector<ReceiveItem> _items;		// Items received
    vector<Id> _orders;			// Orders used in receiving
    vector<Id> _slips;			// Slips used in receiving
    vector<ReceiveCharge> _charges;	// Charges in vendor invoice
};

#endif // RECEIVE_H
