// $Id: pat_worksheet.h,v 1.10 2005/01/30 00:51:13 bpepers Exp $
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

#ifndef PAT_WORKSHEET_H
#define PAT_WORKSHEET_H

#include "data_object.h"

// Patronage sales is the sales by customer and department and
// it is multiplied by a points factor from the department to give a
// total points value.  The patronage group is stored for quicker
// operation of the worksheet screen.
struct PatSales {
    Id customer_id;
    Id patgroup_id;
    Id dept_id;
    fixed sales_amt;

    bool operator==(const PatSales& rhs) const;
    bool operator!=(const PatSales& rhs) const;
};

// Patronage purchased is the purchases by vendor.  It is multiplied by
// a points factor from the worksheet to give a total points value.  The
// patronage group is stored for quicker operation of the worksheet screen.
struct PatPurch {
    Id vendor_id;
    Id patgroup_id;
    fixed purchase_amt;

    bool operator==(const PatPurch& rhs) const;
    bool operator!=(const PatPurch& rhs) const;
};

// Patronage balance is the change in customer balance during the patronage
// period.  It is multiplied by a points factor from the worksheet to give
// a total points value.  The patronage group is stored for quicker
// operation of the worksheet screen.
struct PatBalance {
    Id customer_id;
    Id patgroup_id;
    fixed open_amt;
    fixed close_amt;

    bool operator==(const PatBalance& rhs) const;
    bool operator!=(const PatBalance& rhs) const;
};

// Patronage adjustments by patronage group.  A reason must be given for
// the adjustment.
struct PatAdjust {
    Id patgroup_id;
    fixed points;
    QString reason;

    bool operator==(const PatAdjust& rhs) const;
    bool operator!=(const PatAdjust& rhs) const;
};

class PatWorksheet: public DataObject {
public:
    // Constructors and Destructor
    PatWorksheet();
    ~PatWorksheet();

    // Get methods
    const QString& description()	const { return _desc; }
    QDate fromDate()			const { return _from_date; }
    QDate toDate()			const { return _to_date; }
    fixed equityAmount()		const { return _equity_amt; }
    fixed creditAmount()		const { return _credit_amt; }
    Id equityId()			const { return _equity_id; }
    Id creditId()			const { return _credit_id; }
    const QString& equityMemo()		const { return _equity_memo; }
    const QString& creditMemo()		const { return _credit_memo; }
    fixed purchasePoints()		const { return _purch_pnts; }
    fixed balancePoints()		const { return _bal_pnts; }

    // Set methods
    void setDescription(const QString& desc)	{ _desc = desc; }
    void setFromDate(QDate date)		{ _from_date = date; }
    void setToDate(QDate date)			{ _to_date = date; }
    void setEquityAmount(fixed amt)		{ _equity_amt = amt; }
    void setCreditAmount(fixed amt)		{ _credit_amt = amt; }
    void setEquityId(Id account_id)		{ _equity_id = account_id; }
    void setCreditId(Id account_id)		{ _credit_id = account_id; }
    void setEquityMemo(const QString& memo)	{ _equity_memo = memo; }
    void setCreditMemo(const QString& memo)	{ _credit_memo = memo; }
    void setPurchasePoints(fixed points)	{ _purch_pnts = points; }
    void setBalancePoints(fixed points)		{ _bal_pnts = points; }

    // Sales
    vector<PatSales>& sales()			{ return _sales; }
    const vector<PatSales>& sales() const	{ return _sales; }

    // Purchases
    vector<PatPurch>& purchases()		{ return _purchases; }
    const vector<PatPurch>& purchases() const	{ return _purchases; }

    // Balance changes
    vector<PatBalance>& balances()		{ return _balances; }
    const vector<PatBalance>& balances() const	{ return _balances; }

    // Adjustments
    vector<PatAdjust>& adjusts()		{ return _adjusts; }
    const vector<PatAdjust>& adjusts() const	{ return _adjusts; }

    // Summarized by patronage group
    vector<PatSales> salesSummary(bool custDetail, bool deptDetail) const;
    vector<PatPurch> purchaseSummary(bool vendorDetail) const;
    vector<PatBalance> balanceSummary(bool customerDetail) const;

    // Operations
    bool operator==(const PatWorksheet& rhs) const;
    bool operator!=(const PatWorksheet& rhs) const;

protected:
    QString _desc;			// Worksheet description
    QDate _from_date;			// Start of date range
    QDate _to_date;			// End of date range
    fixed _equity_amt;			// Total equity to allocate
    fixed _credit_amt;			// Total credit to allocate
    Id _equity_id;			// Account for equity postings
    Id _credit_id;			// Account for credit postings
    QString _equity_memo;		// Memo for equity postings
    QString _credit_memo;		// Memo for credit postings
    fixed _purch_pnts;			// Points per purchase $
    fixed _bal_pnts;			// Points per balance change $
    vector<PatSales> _sales;		// Sales by customer/dept
    vector<PatPurch> _purchases;	// Purchase by vendor
    vector<PatBalance> _balances;	// Balance changes by customer
    vector<PatAdjust> _adjusts;		// Adjustments by pat group
};

#endif // PAT_WORKSHEET_H
