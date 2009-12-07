// $Id: subdept.h,v 1.15 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef SUBDEPT_H
#define SUBDEPT_H

#include "data_object.h"

class Subdept: public DataObject {
public:
    // Constructors and Destructor
    Subdept();
    ~Subdept();

    // Get methods
    const QString& name()		const { return _name; }
    const QString& number()		const { return _number; }
    Id deptId()				const { return _dept_id; }
    bool isPurchased()			const { return _purchased; }
    bool isSold()			const { return _sold; }
    bool isInventoried()		const { return _inventoried; }
    Id expenseAccount()			const { return _expense_acct; }
    Id incomeAccount()			const { return _income_acct; }
    Id assetAccount()			const { return _asset_acct; }
    Id sellTax()			const { return _sell_tax; }
    Id purchaseTax()			const { return _purchase_tax; }
    fixed targetGM()			const { return _target_gm; }
    fixed allowedVariance()		const { return _allowed_var; }
    bool isDiscountable()		const { return _discountable; }

    // Set methods
    void setName(const QString& name)		{ _name = name; }
    void setNumber(const QString& number)	{ _number = number; }
    void setDeptId(Id dept_id)			{ _dept_id = dept_id; }
    void setPurchased(bool flag)		{ _purchased = flag; }
    void setSold(bool flag)			{ _sold = flag; }
    void setInventoried(bool flag)		{ _inventoried = flag; }
    void setExpenseAccount(Id acct_id)		{ _expense_acct = acct_id; }
    void setIncomeAccount(Id acct_id)		{ _income_acct = acct_id; }
    void setAssetAccount(Id acct_id)		{ _asset_acct = acct_id; }
    void setSellTax(Id tax_id)			{ _sell_tax = tax_id; }
    void setPurchaseTax(Id tax_id)		{ _purchase_tax = tax_id; }
    void setTargetGM(fixed gm)			{ _target_gm = gm; }
    void setAllowedVariance(fixed var)		{ _allowed_var = var; }
    void setDiscountable(bool flag)		{ _discountable = flag; }

    // Operations
    bool operator==(const Subdept& rhs) const;
    bool operator!=(const Subdept& rhs) const;

protected:
    QString _name;		// Name of subdept
    QString _number;		// Unique subdept number (optional)
    Id _dept_id;		// Link to department
    bool _purchased;		// Can be used in order/receive
    bool _sold;			// Can be used in invoice
    bool _inventoried;		// Can be used in adjustment & has on hands
    Id _expense_acct;		// Expense (COGS) account
    Id _income_acct;		// Income (Sales) account
    Id _asset_acct;		// Asset (Inventory) account
    Id _sell_tax;		// Tax when selling
    Id _purchase_tax;		// Tax when purchasing
    fixed _target_gm;		// Target Gross Margin
    fixed _allowed_var;		// Allowed variance on target GM
    bool _discountable;		// Default discountable for items
};

#endif // SUBDEPT_H
