// $Id: charge.cpp,v 1.8 2004/12/30 00:07:58 bpepers Exp $
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

#include "charge.h"

Charge::Charge()
    : _name(""), _account_id(INVALID_ID), _tax_id(INVALID_ID),
      _calc_method(MANUAL), _amount(0.0), _alloc_method(NONE),
      _inc_tax(false)
{
    _data_type = CHARGE;
}

Charge::~Charge()
{
}

bool
Charge::operator==(const Charge& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._name != _name) return false;
    if (rhs._account_id != _account_id) return false;
    if (rhs._tax_id != _tax_id) return false;
    if (rhs._calc_method != _calc_method) return false;
    if (rhs._amount != _amount) return false;
    if (rhs._alloc_method != _alloc_method) return false;
    if (rhs._inc_tax != _inc_tax) return false;
    return true;
}

bool
Charge::operator!=(const Charge& rhs) const
{
    return !(*this == rhs);
}
