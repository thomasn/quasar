// $Id: discount.cpp,v 1.8 2004/12/30 00:07:58 bpepers Exp $
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

#include "discount.h"

Discount::Discount()
    : _name(""), _line_disc(true), _tx_disc(true), _account_id(INVALID_ID),
      _method(PERCENT), _amount(0.0)
{
    _data_type = DISCOUNT;
}

Discount::~Discount()
{
}

bool
Discount::operator==(const Discount& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._name != _name) return false;
    if (rhs._line_disc != _line_disc) return false;
    if (rhs._tx_disc != _tx_disc) return false;
    if (rhs._account_id != _account_id) return false;
    if (rhs._method != _method) return false;
    if (rhs._amount != _amount) return false;
    return true;
}

bool
Discount::operator!=(const Discount& rhs) const
{
    return !(*this == rhs);
}

QString
Discount::methodName() const
{
    return methodName(_method);
}

QString
Discount::typeName() const
{
    if (_line_disc && _tx_disc) return "Both";
    if (_line_disc) return "Line";
    if (_tx_disc) return "Transaction";
    return "None";
}

QString
Discount::methodName(Method method)
{
    switch (method) {
    case PERCENT:		return "Percent";
    case DOLLAR:		return "Dollar";
    }
    return "Invalid Discount Method";
}
