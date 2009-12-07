// $Id: account.cpp,v 1.34 2004/12/30 00:07:58 bpepers Exp $
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

#include "account.h"

#include <qapplication.h>
#define tr(text) qApp->translate("Account",text)

Account::Account()
    : _name(""), _number(""), _type(Bank), _header(false), _next_number(1)
{
    _data_type = ACCOUNT;
}

Account::~Account()
{
}

bool
Account::operator==(const Account& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._name != _name) return false;
    if (rhs._number != _number) return false;
    if (rhs._type != _type) return false;
    if (rhs._header != _header) return false;
    if (rhs._parent_id != _parent_id) return false;
    if (rhs._last_reconciled != _last_reconciled) return false;
    if (rhs._next_number != _next_number) return false;
    if (rhs._groups != _groups) return false;
    return true;
}

bool
Account::operator!=(const Account& rhs) const
{
    return !(*this == rhs);
}

QString
Account::typeName() const
{
    return typeName(_type);
}

QString
Account::typeName(Type type)
{
    switch (type) {
    case Bank:			return tr("Bank");
    case AR:			return tr("Accounts Receivable");
    case Inventory:		return tr("Inventory");
    case OtherCurAsset:		return tr("Other Current Asset");
    case FixedAsset:		return tr("Fixed Asset");
    case OtherAsset:		return tr("Other Asset");
    case AP:			return tr("Accounts Payable");
    case CreditCard:		return tr("Credit Card");
    case OtherCurLiability:	return tr("Other Current Liability");
    case LongTermLiability:	return tr("Long Term Liability");
    case Equity:		return tr("Equity");
    case Income:		return tr("Income");
    case COGS:			return tr("Cost of Goods Sold");
    case Expense:		return tr("Expense");
    case OtherIncome:		return tr("Other Income");
    case OtherExpense:		return tr("Other Expense");
    }
    return tr("Invalid Account Type");
}

Account::Type
Account::type(const QString& name)
{
    if (name == tr("Bank")) return Bank;
    if (name == tr("Accounts Receivable")) return AR;
    if (name == tr("Inventory")) return Inventory;
    if (name == tr("Other Current Asset")) return OtherCurAsset;
    if (name == tr("Fixed Asset")) return FixedAsset;
    if (name == tr("Other Asset")) return OtherAsset;
    if (name == tr("Accounts Payable")) return AP;
    if (name == tr("Credit Card")) return CreditCard;
    if (name == tr("Other Current Liability")) return OtherCurLiability;
    if (name == tr("Long Term Liability")) return LongTermLiability;
    if (name == tr("Equity")) return Equity;
    if (name == tr("Income")) return Income;
    if (name == tr("Cost of Goods Sold")) return COGS;
    if (name == tr("Expense")) return Expense;
    if (name == tr("Other Income")) return OtherIncome;
    if (name == tr("Other Expense")) return OtherExpense;

    qFatal(tr("Invalid account type: ") + name);
    return Bank;
}
