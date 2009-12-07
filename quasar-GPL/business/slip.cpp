// $Id: slip.cpp,v 1.16 2005/03/13 22:16:34 bpepers Exp $
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

#include "slip.h"

Slip::Slip()
    : _vendor_id(INVALID_ID), _number(""), _waybill(""), _carrier(""),
      _store_id(INVALID_ID), _inv_num(""), _num_pieces(0), _status("Pending")
{
    _data_type = SLIP;
}

Slip::~Slip()
{
}

bool
Slip::operator==(const Slip& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._vendor_id != _vendor_id) return false;
    if (rhs._number != _number) return false;
    if (rhs._waybill != _waybill) return false;
    if (rhs._carrier != _carrier) return false;
    if (rhs._ship_date != _ship_date) return false;
    if (rhs._store_id != _store_id) return false;
    if (rhs._order_nums != _order_nums) return false;
    if (rhs._inv_num != _inv_num) return false;
    if (rhs._num_pieces != _num_pieces) return false;
    if (rhs._status != _status) return false;
    if (rhs._post_date != _post_date) return false;
    if (rhs._items != _items) return false;
    return true;
}

bool
Slip::operator!=(const Slip& rhs) const
{
    return !(*this == rhs);
}

SlipItem::SlipItem()
    : item_id(INVALID_ID), number(""), size(""), size_qty(0.0),
      quantity(0.0), ext_cost(0.0)
{
}

bool
SlipItem::operator==(const SlipItem& rhs) const
{
    if (item_id != rhs.item_id) return false;
    if (number != rhs.number) return false;
    if (size != rhs.size) return false;
    if (size_qty != rhs.size_qty) return false;
    if (quantity != rhs.quantity) return false;
    if (ext_cost != rhs.ext_cost) return false;
    return true;
}

bool
SlipItem::operator!=(const SlipItem& rhs) const
{
    return !(*this == rhs);
}
