// $Id: tax.cpp,v 1.16 2004/12/30 00:07:58 bpepers Exp $
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

#include "tax.h"

Tax::Tax()
    : _name(""), _description(""), _group(false), _number(""), _rate(0.0),
      _collected_id(INVALID_ID), _paid_id(INVALID_ID)
{
    _data_type = TAX;
}

Tax::~Tax()
{
}

bool
Tax::operator==(const Tax& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._name != _name) return false;
    if (rhs._description != _description) return false;
    if (rhs._group != _group) return false;
    if (rhs._number != _number) return false;
    if (rhs._rate != _rate) return false;
    if (rhs._collected_id != _collected_id) return false;
    if (rhs._paid_id != _paid_id) return false;
    if (rhs._tot_ids != _tot_ids) return false;
    if (rhs._group_ids != _group_ids) return false;
    return true;
}

bool
Tax::operator!=(const Tax& rhs) const
{
    return !(*this == rhs);
}
