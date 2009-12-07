// $Id: payout.cpp,v 1.9 2004/12/30 00:07:58 bpepers Exp $
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

#include "payout.h"

Payout::Payout()
{
    _data_type = PAYOUT;
}

Payout::~Payout()
{
}

bool
Payout::operator==(const Payout& rhs) const
{
    if ((const Gltx&)rhs != *this) return false;
    if (rhs._expenses != _expenses) return false;
    return true;
}

bool
Payout::operator!=(const Payout& rhs) const
{
    return !(*this == rhs);
}

fixed
Payout::expenseTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _expenses.size(); ++i) {
	if (_expenses[i].voided) continue;
	total += _expenses[i].amount;
    }
    return total;
}

fixed
Payout::total() const
{
    return (expenseTotal() + taxTotal()) * -1.0;
}

ExpenseLine::ExpenseLine()
    : expense_id(INVALID_ID), amount(0.0), itax_id(INVALID_ID),
      tax_id(INVALID_ID), voided(false)
{
}

bool
ExpenseLine::operator==(const ExpenseLine& rhs) const
{
    if (expense_id != rhs.expense_id) return false;
    if (amount != rhs.amount) return false;
    if (itax_id != rhs.itax_id) return false;
    if (tax_id != rhs.tax_id) return false;
    if (voided != rhs.voided) return false;
    return true;
}

bool
ExpenseLine::operator!=(const ExpenseLine& rhs) const
{
    return !(*this == rhs);
}
