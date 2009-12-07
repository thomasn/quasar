// $Id: gltx.h,v 1.58 2005/01/31 23:22:35 bpepers Exp $
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

#ifndef GLTX_H
#define GLTX_H

#include "data_object.h"

// Account lines record allocation to accounts and must total to zero.  The
// cleared value is for reconciliation purposes.
struct AccountLine {
    AccountLine();
    AccountLine(Id account_id, fixed amount);

    Id account_id;
    fixed amount;
    QString memo;
    QDate cleared;

    bool operator==(const AccountLine& rhs) const;
    bool operator!=(const AccountLine& rhs) const;
};

// Card lines record allocations to cards which effect the card balance.
// The card must be a customer of vendor card.  The paid is the total that
// has been applied to this card line via payment lines and when fully
// paid should equal the amount.
struct CardLine {
    CardLine();
    CardLine(Id card_id, fixed amount=0.0);

    Id card_id;
    fixed amount;

    bool operator==(const CardLine& rhs) const;
    bool operator!=(const CardLine& rhs) const;
};

// Item lines record allocations to items that effect on hand and total
// cost values.  There is no inter-item allocations done like for cards
// though in future there could be to support FIFO and LIFO inventory
// costing methods.  This class is used as the base class for all the
// transactions that involve inventory (adjustment, invoice, pos_tx,
// and receive).
struct ItemLine {
    ItemLine();

    Id item_id;			// Item id of item
    Id account_id;		// Account id if sold by account
    QString number;		// Number of item
    QString description;	// Description over-ride
    QString size;		// Size of item
    fixed size_qty;		// Quantity of size
    fixed quantity;		// Quantity of item at size
    fixed inv_cost;		// Total cost change
    fixed sale_price;		// Sales history price (invoice and sale)
    fixed ext_deposit;		// Total deposit
    bool voided;		// Line voided?

    bool operator==(const ItemLine& rhs) const;
    bool operator!=(const ItemLine& rhs) const;
};

// Tax lines records amounts posted to taxes.  This information is used
// for tax reporting.  The taxable field is the total amount that the tax
// was applied on.  Taxes are split into whether the tax was included in
// the price or not which is useful to show on receipts and such.
struct TaxLine {
    TaxLine(Id tax_id=INVALID_ID, fixed taxable=0.0, fixed amount=0.0,
	    fixed inc_taxable=0.0, fixed inc_amount=0.0);

    Id tax_id;
    fixed taxable;
    fixed amount;
    fixed inc_taxable;
    fixed inc_amount;

    bool operator==(const TaxLine& rhs) const;
    bool operator!=(const TaxLine& rhs) const;
};

// Payment lines record allocations of card lines between transactions.  It
// is used for the transactions that effect card balances (some cheque types,
// invoice, receiving, some pos_tx types, and some receipt types)
struct PaymentLine {
    PaymentLine();
    PaymentLine(Id gltx_id, fixed amount, fixed discount);

    Id gltx_id;
    fixed amount;
    fixed discount;

    bool operator==(const PaymentLine& rhs) const;
    bool operator!=(const PaymentLine& rhs) const;
};

// Tender lines record tenders received and dispursed.  It is used for
// cash reconciliation like shifts and closing.  It also gets involved
// for undeposited funds.
struct TenderLine {
    TenderLine();

    Id tender_id;
    fixed amount;
    fixed conv_rate;
    fixed conv_amt;
    QString card_num;
    QDate expiry_date;
    QString auth_num;
    bool is_change;
    bool voided;

    bool operator==(const TenderLine& rhs) const;
    bool operator!=(const TenderLine& rhs) const;
};

class Gltx: public DataObject {
public:
    // Constructors and Destructor
    Gltx();
    virtual ~Gltx();

    // Get methods
    const QString& number()		const { return _number; }
    const QString& reference()		const { return _reference; }
    QDate postDate()			const { return _post_date; }
    QTime postTime()			const { return _post_time; }
    const QString& memo()		const { return _memo; }
    Id stationId()			const { return _station_id; }
    Id employeeId()			const { return _employee_id; }
    Id cardId()				const { return _card_id; }
    Id storeId()			const { return _store_id; }
    Id shiftId()			const { return _shift_id; }
    Id linkId()				const { return _link_id; }
    bool printed()			const { return _printed; }
    bool paid()				const { return _paid; }
    fixed amount()			const { return _amount; }

    // Set methods
    void setNumber(const QString& number)	{ _number = number; }
    void setReference(const QString& reference)	{ _reference = reference; }
    void setPostDate(QDate date)		{ _post_date = date; }
    void setPostTime(QTime time)		{ _post_time = time; }
    void setMemo(const QString& memo)		{ _memo = memo; }
    void setStationId(Id id)			{ _station_id = id; }
    void setEmployeeId(Id id)			{ _employee_id = id; }
    void setCardId(Id id)			{ _card_id = id; }
    void setStoreId(Id id)			{ _store_id = id; }
    void setShiftId(Id id)			{ _shift_id = id; }
    void setLinkId(Id id)			{ _link_id = id; }
    void setPrinted(bool flag)			{ _printed = flag; }
    void setPaid(bool flag)			{ _paid = flag; }
    void setAmount(fixed amount)		{ _amount = amount; }

    // Account lines
    vector<AccountLine>& accounts()		{ return _accounts; }
    const vector<AccountLine>& accounts() const { return _accounts; }

    // Card lines
    vector<CardLine>& cards()			{ return _cards; }
    const vector<CardLine>& cards()	  const { return _cards; }
    fixed cardTotal() const;

    // Tax lines
    vector<TaxLine>& taxes()			{ return _taxes; }
    const vector<TaxLine>& taxes()	  const { return _taxes; }
    fixed taxTotal() const;

    // Payment lines
    vector<PaymentLine>& payments()		{ return _payments; }
    const vector<PaymentLine>& payments() const { return _payments; }
    fixed paymentTotal() const;
    fixed termsDiscountTotal() const;

    // Tender lines
    vector<TenderLine>& tenders()		{ return _tenders; }
    const vector<TenderLine>& tenders() const	{ return _tenders; }
    virtual fixed tenderTotal() const;
    virtual fixed chargeTotal() const;

    // Item lines (use virtual access methods instead of direct vector
    // access since ItemLine is a base class)
    virtual unsigned int itemCnt() const;
    virtual const ItemLine& item(unsigned int i) const;
    virtual fixed costTotal() const;
    virtual fixed depositTotal() const;

    // References
    const vector<QString>& referenceName() const{ return _refName; }
    vector<QString>& referenceName()		{ return _refName; }
    const vector<QString>& referenceData() const{ return _refData; }
    vector<QString>& referenceData()		{ return _refData; }

    // Special methods
    virtual fixed total() const;

    // Operations
    bool operator==(const Gltx& rhs) const;
    bool operator!=(const Gltx& rhs) const;

protected:
    QString _number;			// Transaction number
    QString _reference;			// Extra reference (vendor invoice #)
    QDate _post_date;			// Posting date for transaction
    QTime _post_time;			// Posting time for transaction
    QString _memo;			// Memo for transaction
    Id _station_id;			// Station transaction created at
    Id _employee_id;			// Person who created this
    Id _card_id;			// Customer, Vendor, or other card
    Id _store_id;			// Store of transaction
    Id _shift_id;			// Link to shift close transaction
    Id _link_id;			// Linked transaction (transfers)
    bool _printed;			// Transaction has been printed?
    bool _paid;				// Fully paid?
    fixed _amount;			// Amount (stored from total() method)
    vector<AccountLine> _accounts;	// Accounts and amounts
    vector<CardLine> _cards;		// Cards and amounts
    vector<TaxLine> _taxes;		// Tax ids and amounts
    vector<PaymentLine> _payments;	// List of payments
    vector<TenderLine> _tenders;	// Tenders and amounts
    vector<QString> _refName;		// References (names)
    vector<QString> _refData;		// References (customer data)
};

bool operator<(const Gltx& lhs, const Gltx& rhs);

#endif // GLTX_H
