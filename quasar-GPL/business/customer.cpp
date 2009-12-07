// $Id: customer.cpp,v 1.25 2004/12/30 00:07:58 bpepers Exp $
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

#include "customer.h"

Customer::Customer()
    : _credit_limit(0.0), _can_charge(true), _can_withdraw(false),
      _can_payment(true), _sc_rate(0.0), _check_wd_bal(true),
      _print_stmts(true), _credit_hold(false), _second_rcpt(false)
{
    _data_type = CUSTOMER;
}

Customer::~Customer()
{
}

bool
Customer::operator==(const Customer& rhs) const
{
    if ((const Card&)rhs != *this) return false;
    if (rhs._type_id != _type_id) return false;
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
    if (rhs._credit_hold != _credit_hold) return false;
    if (rhs._second_rcpt != _second_rcpt) return false;
    if (rhs._refs != _refs) return false;
    return true;
}

bool
Customer::operator!=(const Customer& rhs) const
{
    return !(*this == rhs);
}

void
Customer::setFromType(const CustomerType& type)
{
    setTypeId(type.id());
    setAccountId(type.accountId());
    setTermsId(type.termsId());
    setTaxExemptId(type.taxExemptId());
    setCreditLimit(type.creditLimit());
    setCanCharge(type.canCharge());
    setCanWithdraw(type.canWithdraw());
    setCanPayment(type.canPayment());
    setServiceCharge(type.serviceCharge());
    setCheckWithdrawBalance(type.checkWithdrawBalance());
    setPrintStatements(type.printStatements());
    setSecondReceipt(type.secondReceipt());
}
