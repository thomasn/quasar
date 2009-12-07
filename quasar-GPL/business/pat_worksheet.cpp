// $Id: pat_worksheet.cpp,v 1.14 2004/12/30 00:07:58 bpepers Exp $
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

#include "pat_worksheet.h"

PatWorksheet::PatWorksheet()
    : _desc(""), _equity_memo(""), _credit_memo("")
{
    _data_type = PAT_WS;
}

PatWorksheet::~PatWorksheet()
{
}

typedef QPair<Id,Id> IdPair;
typedef QPair<IdPair,Id> SalesId;

vector<PatSales>
PatWorksheet::salesSummary(bool customerDetail, bool deptDetail) const
{
    if (customerDetail && deptDetail) return _sales;

    QMap<SalesId,PatSales> salesMap;
    for (unsigned int i = 0; i < _sales.size(); ++i) {
	const PatSales& sales = _sales[i];
	Id group_id = sales.patgroup_id;
	Id customer_id = sales.customer_id;
	Id dept_id = sales.dept_id;

	if (!customerDetail) customer_id = INVALID_ID;
	if (!deptDetail) dept_id = INVALID_ID;
	SalesId key(IdPair(group_id, customer_id), dept_id);

	if (salesMap.contains(key)) {
	    salesMap[key].sales_amt += sales.sales_amt;
	} else {
	    salesMap.insert(key, sales);
	}
    }

    vector<PatSales> sales;
    QMap<SalesId,PatSales>::Iterator it;
    for (it = salesMap.begin(); it != salesMap.end(); ++it)
	sales.push_back(it.data());

    return sales;
}

vector<PatPurch>
PatWorksheet::purchaseSummary(bool vendorDetail) const
{
    if (vendorDetail) return _purchases;

    QMap<Id,PatPurch> purchaseMap;
    for (unsigned int i = 0; i < _purchases.size(); ++i) {
	const PatPurch& purchase = _purchases[i];
	Id group_id = purchase.patgroup_id;

	if (purchaseMap.contains(group_id)) {
	    purchaseMap[group_id].purchase_amt += purchase.purchase_amt;
	} else {
	    purchaseMap.insert(group_id, purchase);
	}
    }

    vector<PatPurch> purchases;
    QMap<Id,PatPurch>::Iterator it;
    for (it = purchaseMap.begin(); it != purchaseMap.end(); ++it)
	purchases.push_back(it.data());

    return purchases;
}

vector<PatBalance>
PatWorksheet::balanceSummary(bool customerDetail) const
{
    if (customerDetail) return _balances;

    QMap<Id,PatBalance> balanceMap;
    for (unsigned int i = 0; i < _balances.size(); ++i) {
	const PatBalance& balance = _balances[i];
	Id group_id = balance.patgroup_id;

	if (balanceMap.contains(group_id)) {
	    balanceMap[group_id].open_amt += balance.open_amt;
	    balanceMap[group_id].close_amt += balance.close_amt;
	} else {
	    balanceMap.insert(group_id, balance);
	}
    }

    vector<PatBalance> balances;
    QMap<Id,PatBalance>::Iterator it;
    for (it = balanceMap.begin(); it != balanceMap.end(); ++it)
	balances.push_back(it.data());

    return balances;
}

bool
PatWorksheet::operator==(const PatWorksheet& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._desc != _desc) return false;
    if (rhs._from_date != _from_date) return false;
    if (rhs._to_date != _to_date) return false;
    if (rhs._equity_amt != _equity_amt) return false;
    if (rhs._credit_amt != _credit_amt) return false;
    if (rhs._equity_id != _equity_id) return false;
    if (rhs._credit_id != _credit_id) return false;
    if (rhs._equity_memo != _equity_memo) return false;
    if (rhs._credit_memo != _credit_memo) return false;
    if (rhs._purch_pnts != _purch_pnts) return false;
    if (rhs._bal_pnts != _bal_pnts) return false;
    if (rhs._sales != _sales) return false;
    if (rhs._purchases != _purchases) return false;
    if (rhs._balances != _balances) return false;
    if (rhs._adjusts != _adjusts) return false;
    return true;
}

bool
PatWorksheet::operator!=(const PatWorksheet& rhs) const
{
    return !(*this == rhs);
}

bool
PatSales::operator==(const PatSales& rhs) const
{
    if (customer_id != rhs.customer_id) return false;
    if (patgroup_id != rhs.patgroup_id) return false;
    if (dept_id != rhs.dept_id) return false;
    if (sales_amt != rhs.sales_amt) return false;
    return true;
}

bool
PatSales::operator!=(const PatSales& rhs) const
{
    return !(*this == rhs);
}

bool
PatPurch::operator==(const PatPurch& rhs) const
{
    if (vendor_id != rhs.vendor_id) return false;
    if (patgroup_id != rhs.patgroup_id) return false;
    if (purchase_amt != rhs.purchase_amt) return false;
    return true;
}

bool
PatPurch::operator!=(const PatPurch& rhs) const
{
    return !(*this == rhs);
}

bool
PatBalance::operator==(const PatBalance& rhs) const
{
    if (customer_id != rhs.customer_id) return false;
    if (patgroup_id != rhs.patgroup_id) return false;
    if (open_amt != rhs.open_amt) return false;
    if (close_amt != rhs.close_amt) return false;
    return true;
}

bool
PatBalance::operator!=(const PatBalance& rhs) const
{
    return !(*this == rhs);
}

bool
PatAdjust::operator==(const PatAdjust& rhs) const
{
    if (patgroup_id != rhs.patgroup_id) return false;
    if (points != rhs.points) return false;
    if (reason != rhs.reason) return false;
    return true;
}

bool
PatAdjust::operator!=(const PatAdjust& rhs) const
{
    return !(*this == rhs);
}
