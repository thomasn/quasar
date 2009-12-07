// $Id: count.cpp,v 1.12 2004/12/30 00:07:58 bpepers Exp $
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

#include "count.h"

Count::Count()
    : _number(""), _description("")
{
    _data_type = COUNT;
}

Count::~Count()
{
}

bool
Count::operator==(const Count& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._number != _number) return false;
    if (rhs._description != _description) return false;
    if (rhs._date != _date) return false;
    if (rhs._store_id != _store_id) return false;
    if (rhs._location_id != _location_id) return false;
    if (rhs._employee_id != _employee_id) return false;
    if (rhs._items != _items) return false;
    return true;
}

bool
Count::operator!=(const Count& rhs) const
{
    return !(*this == rhs);
}

CountItem::CountItem()
    : item_id(INVALID_ID), number(""), size(""), size_qty(0.0),
      quantity(0.0), on_hand(0.0), oh_set(false)
{
}

bool
CountItem::operator==(const CountItem& rhs) const
{
    if (item_id != rhs.item_id) return false;
    if (number != rhs.number) return false;
    if (size != rhs.size) return false;
    if (size_qty != rhs.size_qty) return false;
    if (quantity != rhs.quantity) return false;
    if (on_hand != rhs.on_hand) return false;
    if (oh_set != rhs.oh_set) return false;
    return true;
}

bool
CountItem::operator!=(const CountItem& rhs) const
{
    return !(*this == rhs);
}
