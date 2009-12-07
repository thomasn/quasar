// $Id: account.h,v 1.27 2005/01/30 00:51:13 bpepers Exp $
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

#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "data_object.h"

class Account: public DataObject {
public:
    // Constructors and Destructor
    Account();
    ~Account();

    // Enums
    enum Type { Bank, AR, Inventory, OtherCurAsset, FixedAsset, OtherAsset,
		AP, CreditCard, OtherCurLiability, LongTermLiability,
		Equity, Income, COGS, Expense, OtherIncome, OtherExpense };

    // Get methods
    const QString& name()		const { return _name; }
    const QString& number()		const { return _number; }
    Type type()				const { return _type; }
    bool isHeader()			const { return _header; }
    Id parentId()			const { return _parent_id; }
    QDate lastReconciled()		const { return _last_reconciled; }
    int nextNumber()			const { return _next_number; }

    // Set methods
    void setName(const QString& name)		{ _name = name; }
    void setNumber(const QString& number)	{ _number = number; }
    void setType(Type type)			{ _type = type; }
    void setHeader(bool flag)			{ _header = flag; }
    void setParentId(Id parent_id)		{ _parent_id = parent_id; }
    void setLastReconciled(QDate date)		{ _last_reconciled = date; }
    void setNextNumber(int number)		{ _next_number = number; }

    // Account groups
    const vector<Id>& groups()		const { return _groups; }
    vector<Id>& groups()		      { return _groups; }

    // Special methods
    QString typeName() const;
    static QString typeName(Type type);
    static Type type(const QString& name);

    // Operations
    bool operator==(const Account& rhs) const;
    bool operator!=(const Account& rhs) const;

protected:
    QString _name;		// Name of account
    QString _number;		// Number for account (not used much)
    Type _type;			// Type of account
    bool _header;		// Is this a header account?
    Id _parent_id;		// Parent link for account/subaccount setup
    QDate _last_reconciled;	// Date last reconciled
    int _next_number;		// Next number (for bank accounts)
    vector<Id> _groups;		// Groups (for Profit & loss and reporting)
};

#endif // ACCOUNT_H
