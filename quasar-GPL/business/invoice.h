// $Id: invoice.h,v 1.34 2005/01/30 00:51:13 bpepers Exp $
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

#ifndef INVOICE_H
#define INVOICE_H

#include "gltx.h"
#include "price.h"

struct InvoiceDisc {
    InvoiceDisc();

    Id discount_id;		// Discount id
    int method;			// Method (PERCENT or DOLLAR)
    fixed amount;		// Percent or dollar amount
    Id account_id;		// Account to allocate to (can be null)
    fixed total_amt;		// Total discount amount
    bool voided;

    bool operator==(const InvoiceDisc& rhs) const;
    bool operator!=(const InvoiceDisc& rhs) const;
};

struct InvoiceItem: public ItemLine {
    InvoiceItem();

    Id price_id;		// ItemPrice id used (can be INVALID_ID)
    fixed price_disc;		// Price change from user
    Price price;		// Price item was sold at
    fixed ext_price;		// Calculated price from quantity
    fixed ext_base;		// Total (minus discounts and included tax/dep)
    fixed ext_tax;		// Total taxes
    Id tax_id;			// Tax to use for line
    Id item_tax_id;		// Normal tax for item
    bool include_tax;		// Price includes tax?
    bool include_deposit;	// Price includes deposit?
    bool discountable;		// Can discount line?
    Id subdept_id;		// For subdept only sales
    bool scale;			// Item was weighed?
    fixed you_save;		// Amount saved for promotions
    bool open_dept;		// Open department item?
    InvoiceDisc line_disc;	// Line discount
    vector<int> tdisc_nums;	// Link to transaction discount
    vector<fixed> tdisc_amts;	// Amount of trans disc allocated to item

    bool operator==(const InvoiceItem& rhs) const;
    bool operator!=(const InvoiceItem& rhs) const;
};

class Invoice: public Gltx {
public:
    // Constructors and Destructor
    Invoice();
    virtual ~Invoice();

    enum LineType { Item, Account };

    // Get methods
    Id customerId()			const { return _card_id; }
    QString customerAddress()		const { return _customer_addr; }
    Id shipId()				const { return _ship_id; }
    QString shipAddress()		const { return _ship_addr; }
    QString shipVia()			const { return _ship_via; }
    Id termsId()			const { return _term_id; }
    QString comment()			const { return _comment; }
    QDate promisedDate()		const { return _promised; }
    LineType lineType()			const { return _line_type; }
    Id taxExemptId()			const { return _tax_exempt_id; }
    bool isReturn() const;

    // Set methods
    void setCustomerId(Id id)			{ _card_id = id; }
    void setCustomerAddress(const QString& txt)	{ _customer_addr = txt; }
    void setShipId(Id id)			{ _ship_id = id; }
    void setShipAddress(const QString& addr)	{ _ship_addr = addr; }
    void setShipVia(const QString& via)		{ _ship_via = via; }
    void setTermsId(Id term_id)			{ _term_id = term_id; }
    void setComment(const QString& comment)	{ _comment = comment; }
    void setPromisedDate(QDate date)		{ _promised = date; }
    void setLineType(LineType type)		{ _line_type = type; }
    void setTaxExemptId(Id id)			{ _tax_exempt_id = id; }
    void setReturn(bool flag);

    // Item lines
    vector<InvoiceItem>& items()		{ return _items; }
    const vector<InvoiceItem>& items() const	{ return _items; }
    unsigned int itemCnt() const		{ return _items.size(); }
    const ItemLine& item(unsigned int i) const	{ return _items[i]; }
    fixed priceTotal() const;
    fixed lineDiscountTotal() const;

    // Transaction discounts
    vector<InvoiceDisc>& discounts()		{ return _discounts; }
    const vector<InvoiceDisc>& discounts() const{ return _discounts; }
    fixed transDiscountTotal() const;

    // Special methods
    QString lineTypeName() const;
    static QString lineTypeName(LineType type);
    fixed itemDiscountTotal() const;
    fixed tenderTotal() const;
    fixed chargeTotal() const;
    fixed total() const;
    fixed paidTotal() const;
    fixed dueTotal() const;

    // Operations
    bool operator==(const Invoice& rhs) const;
    bool operator!=(const Invoice& rhs) const;

protected:
    QString _customer_addr;		// Customer address
    Id _ship_id;			// Ship to customer
    QString _ship_addr;			// Shipping address
    QString _ship_via;			// Method of shipment
    Id _term_id;			// Terms (Net 30, ...)
    QString _comment;			// Comment to show on invoice
    QDate _promised;			// Promised date
    LineType _line_type;		// Type of lines (Item or Account)
    Id _tax_exempt_id;			// Tax exemption
    vector<InvoiceItem> _items;		// Items in invoice
    vector<InvoiceDisc> _discounts;	// Transaction discounts
};

#endif // INVOICE_H
