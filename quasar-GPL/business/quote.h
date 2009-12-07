// $Id: quote.h,v 1.7 2005/01/30 00:51:13 bpepers Exp $
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

#ifndef QUOTE_H
#define QUOTE_H

#include "invoice.h"

class Quote: public DataObject {
public:
    // Constructors and Destructor
    Quote();
    virtual ~Quote();

    enum LineType { Item, Account };

    // Get methods
    QString number()			const { return _number; }
    const QString& reference()		const { return _reference; }
    QDate date()			const { return _date; }
    Id storeId()			const { return _store_id; }
    Id employeeId()			const { return _employee_id; }
    Id customerId()			const { return _customer_id; }
    QString customerAddress()		const { return _customer_addr; }
    Id shipId()				const { return _ship_id; }
    QString shipAddress()		const { return _ship_addr; }
    QString shipVia()			const { return _ship_via; }
    Id termsId()			const { return _term_id; }
    QString comment()			const { return _comment; }
    QDate expiryDate()			const { return _expiry; }
    LineType lineType()			const { return _line_type; }
    Id taxExemptId()			const { return _tax_exempt_id; }
    Id invoiceId()			const { return _invoice_id; }

    // Set methods
    void setNumber(const QString& number)	{ _number = number; }
    void setReference(const QString& reference)	{ _reference = reference; }
    void setDate(QDate date)			{ _date = date; }
    void setStoreId(Id id)			{ _store_id = id; }
    void setEmployeeId(Id id)			{ _employee_id = id; }
    void setCustomerId(Id id)			{ _customer_id = id; }
    void setCustomerAddress(const QString& txt)	{ _customer_addr = txt; }
    void setShipId(Id id)			{ _ship_id = id; }
    void setShipAddress(const QString& addr)	{ _ship_addr = addr; }
    void setShipVia(const QString& via)		{ _ship_via = via; }
    void setTermsId(Id term_id)			{ _term_id = term_id; }
    void setComment(const QString& comment)	{ _comment = comment; }
    void setExpiryDate(QDate date)		{ _expiry = date; }
    void setLineType(LineType type)		{ _line_type = type; }
    void setTaxExemptId(Id id)			{ _tax_exempt_id = id; }
    void setInvoiceId(Id id)			{ _invoice_id = id; }

    // Tax lines
    vector<TaxLine>& taxes()			{ return _taxes; }
    const vector<TaxLine>& taxes()	  const { return _taxes; }
    fixed taxTotal() const;

    // Item lines
    vector<InvoiceItem>& items()		{ return _items; }
    const vector<InvoiceItem>& items()	  const	{ return _items; }
    fixed priceTotal() const;
    fixed depositTotal() const;
    fixed lineDiscountTotal() const;

    // Transaction discounts
    vector<InvoiceDisc>& discounts()		{ return _discounts; }
    const vector<InvoiceDisc>& discounts() const{ return _discounts; }
    fixed transDiscountTotal() const;

    // Special methods
    QString lineTypeName() const;
    static QString lineTypeName(LineType type);
    fixed itemDiscountTotal() const;
    fixed total() const;

    // Operations
    bool operator==(const Quote& rhs) const;
    bool operator!=(const Quote& rhs) const;

protected:
    QString _number;			// Quote number
    QString _reference;			// Extra reference (vendor invoice #)
    QDate _date;			// Date created
    Id _store_id;			// Store of quote
    Id _employee_id;			// Employee creating quote
    Id _customer_id;			// Customer
    QString _customer_addr;		// Customer address
    Id _ship_id;			// Ship to customer
    QString _ship_addr;			// Shipping address
    QString _ship_via;			// Method of shipment
    Id _term_id;			// Terms (Net 30, ...)
    QString _comment;			// Comment to show on quote
    QDate _expiry;			// Expiry date
    LineType _line_type;		// Type of lines (Item or Account)
    Id _tax_exempt_id;			// Tax exemption
    Id _invoice_id;			// Invoice created from quote
    vector<TaxLine> _taxes;		// Tax ids and amounts
    vector<InvoiceItem> _items;		// Items in quote
    vector<InvoiceDisc> _discounts;	// Transaction discounts
};

#endif // QUOTE_H
