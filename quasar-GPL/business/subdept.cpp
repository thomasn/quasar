// $Id: subdept.cpp,v 1.21 2004/12/30 00:07:58 bpepers Exp $
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

#include "subdept.h"

Subdept::Subdept()
    : _name(""), _number(""), _dept_id(INVALID_ID), _purchased(true),
      _sold(true), _inventoried(true), _target_gm(0.0), _allowed_var(0.0),
      _discountable(true)
{
    _data_type = SUBDEPT;
}

Subdept::~Subdept()
{
}

bool
Subdept::operator==(const Subdept& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._name != _name) return false;
    if (rhs._number != _number) return false;
    if (rhs._dept_id != _dept_id) return false;
    if (rhs._purchased != _purchased) return false;
    if (rhs._sold != _sold) return false;
    if (rhs._inventoried != _inventoried) return false;
    if (rhs._expense_acct != _expense_acct) return false;
    if (rhs._income_acct != _income_acct) return false;
    if (rhs._asset_acct != _asset_acct) return false;
    if (rhs._sell_tax != _sell_tax) return false;
    if (rhs._purchase_tax != _purchase_tax) return false;
    if (rhs._target_gm != _target_gm) return false;
    if (rhs._allowed_var != _allowed_var) return false;
    if (rhs._discountable != _discountable) return false;
    return true;
}

bool
Subdept::operator!=(const Subdept& rhs) const
{
    return !(*this == rhs);
}
