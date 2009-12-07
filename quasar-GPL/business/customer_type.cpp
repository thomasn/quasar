// $Id: customer_type.cpp,v 1.9 2004/12/30 00:07:58 bpepers Exp $
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

#include "customer_type.h"

CustomerType::CustomerType()
    : _name(""), _credit_limit(0.0), _can_charge(true), _can_withdraw(false),
      _can_payment(true), _sc_rate(0.0), _check_wd_bal(true),
      _print_stmts(true), _second_rcpt(false)
{
    _data_type = CUST_TYPE;
}

CustomerType::~CustomerType()
{
}

bool
CustomerType::operator==(const CustomerType& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._name != _name) return false;
    if (rhs._account_id != _account_id) return false;
    if (rhs._term_id != _term_id) return false;
    if (rhs._tax_exempt_id != _tax_exempt_id) return false;
    if (rhs._credit_limit != _credit_limit) return false;
    if (rhs._can_charge != _can_charge) return false;
    if (rhs._can_withdraw != _can_withdraw) return false;
    if (rhs._can_payment != _can_payment) return false;
    if (rhs._sc_rate != _sc_rate) return false;
    if (rhs._check_wd_bal != _check_wd_bal) return false;
    if (rhs._print_stmts != _print_stmts) return false;
    if (rhs._second_rcpt != _second_rcpt) return false;
    return true;
}

bool
CustomerType::operator!=(const CustomerType& rhs) const
{
    return !(*this == rhs);
}
