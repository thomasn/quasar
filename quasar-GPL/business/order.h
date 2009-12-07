// $Id: order.h,v 1.21 2005/01/30 00:51:13 bpepers Exp $
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

#ifndef ORDER_H
#define ORDER_H

#include "data_object.h"
#include "price.h"

struct OrderItem {
    OrderItem();

    Id item_id;			// Item id of item
    QString number;		// Number used for item
    QString description;	// Description over-ride
    QString size;		// Size of item
    fixed size_qty;		// Quantity of size
    fixed ordered;		// Quantity ordered
    fixed billed;		// Quantity billed on vendor invoices
    Id cost_id;			// ItemPrice id used (can be INVALID_ID)
    fixed cost_disc;		// Cost change from user
    Price cost;			// Cost item was purchased at
    fixed ext_cost;		// Calculated cost from quantity
    fixed ext_base;		// Total (minus discounts and included tax/dep)
    fixed ext_deposit;		// Total deposit
    fixed ext_tax;		// Total taxes
    fixed int_charges;		// Internal charges
    fixed ext_charges;		// External charges
    Id tax_id;			// Tax to use for line
    Id item_tax_id;		// Normal tax for item
    bool include_tax;		// Cost includes tax?
    bool include_deposit;	// Cost includes deposit?

    bool operator==(const OrderItem& rhs) const;
    bool operator!=(const OrderItem& rhs) const;
};

struct OrderTax {
    OrderTax(Id tax_id=INVALID_ID, fixed taxable=0.0, fixed amount=0.0,
	     fixed inc_taxable=0.0, fixed inc_amount=0.0);

    Id tax_id;
    fixed taxable;
    fixed amount;
    fixed inc_taxable;
    fixed inc_amount;

    bool operator==(const OrderTax& rhs) const;
    bool operator!=(const OrderTax& rhs) const;
};

struct OrderCharge {
    OrderCharge(Id charge_id=INVALID_ID, Id tax_id=INVALID_ID,
		fixed amount=0.0, fixed base=0.0, bool internal=true);

    Id charge_id;
    Id tax_id;
    fixed amount;
    fixed base;
    bool internal;

    bool operator==(const OrderCharge& rhs) const;
    bool operator!=(const OrderCharge& rhs) const;
};

class Order: public DataObject {
public:
    // Constructors and Destructor
    Order();
    virtual ~Order();

    // Get methods
    Id vendorId()			const { return _vendor_id; }
    QString vendorAddress()		const { return _vendor_addr; }
    Id shipId()				const { return _ship_id; }
    QString shipAddress()		const { return _ship_addr; }
    QString shipVia()			const { return _ship_via; }
    Id termsId()			const { return _term_id; }
    Id storeId()			const { return _store_id; }
    QString comment()			const { return _comment; }
    QString number()			const { return _number; }
    QString reference()			const { return _reference; }
    QDate date()			const { return _date; }

    // Set methods
    void setVendorId(Id id)			{ _vendor_id = id; }
    void setVendorAddress(const QString& addr)	{ _vendor_addr = addr; }
    void setShipId(Id id)			{ _ship_id = id; }
    void setShipAddress(const QString& addr)	{ _ship_addr = addr; }
    void setShipVia(const QString& text)	{ _ship_via = text; }
    void setTermsId(Id term_id)			{ _term_id = term_id; }
    void setStoreId(Id id)			{ _store_id = id; }
    void setComment(const QString& text)	{ _comment = text; }
    void setNumber(const QString& num)		{ _number = num; }
    void setReference(const QString& ref)	{ _reference = ref; }
    void setDate(QDate date)			{ _date = date; }

    // Item lines
    vector<OrderItem>& items()			{ return _items; }
    const vector<OrderItem>& items() const	{ return _items; }
    fixed costTotal() const;
    fixed depositTotal() const;

    // Taxes
    vector<OrderTax>& taxes()			{ return _taxes; }
    const vector<OrderTax>& taxes() const	{ return _taxes; }
    fixed taxTotal() const;

    // Charges
    vector<OrderCharge>& charges()		{ return _charges; }
    const vector<OrderCharge>& charges() const	{ return _charges; }
    fixed chargeTotal() const;

    // Special methods
    fixed total() const;
    bool allReceived() const;

    // Operations
    bool operator==(const Order& rhs) const;
    bool operator!=(const Order& rhs) const;

protected:
    Id _vendor_id;			// Vendor card id
    QString _vendor_addr;		// Vendor address
    Id _ship_id;			// Ship to vendor
    QString _ship_addr;			// Shipping address
    QString _ship_via;			// Method of shipment
    Id _term_id;			// Terms (Net 30, ...)
    Id _store_id;			// Store
    QString _comment;			// Comments to show on order
    QString _number;			// Order number
    QString _reference;			// Reference
    QDate _date;			// Date of order
    vector<OrderItem> _items;		// Items in order
    vector<OrderTax> _taxes;		// Tax totals in order
    vector<OrderCharge> _charges;	// Charges in order
};

#endif // ORDER_H
