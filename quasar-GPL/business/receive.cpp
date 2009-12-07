// $Id: receive.cpp,v 1.31 2005/01/11 09:05:44 bpepers Exp $
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

#include "receive.h"

Receive::Receive()
    : _vendor_addr(""), _ship_id(INVALID_ID), _ship_addr(""),
      _ship_via(""), _term_id(INVALID_ID), _comment(""),
      _line_type(Item), _closed(false)
{
    _data_type = RECEIVE;
}

Receive::~Receive()
{
}

bool
Receive::operator==(const Receive& rhs) const
{
    if ((const Gltx&)rhs != *this) return false;
    if (rhs._vendor_addr != _vendor_addr) return false;
    if (rhs._ship_id != _ship_id) return false;
    if (rhs._ship_addr != _ship_addr) return false;
    if (rhs._ship_via != _ship_via) return false;
    if (rhs._term_id != _term_id) return false;
    if (rhs._comment != _comment) return false;
    if (rhs._invoice_date != _invoice_date) return false;
    if (rhs._line_type != _line_type) return false;
    if (rhs._closed != _closed) return false;
    if (rhs._items != _items) return false;
    if (rhs._orders != _orders) return false;
    if (rhs._slips != _slips) return false;
    if (rhs._charges != _charges) return false;
    return true;
}

bool
Receive::operator!=(const Receive& rhs) const
{
    return !(*this == rhs);
}

bool
Receive::isClaim() const
{
    return _data_type == CLAIM;
}

void
Receive::setClaim(bool flag)
{
    if (flag)
	_data_type = CLAIM;
    else
	_data_type = RECEIVE;
}

ReceiveItem::ReceiveItem()
    : ordered(0.0), received(0.0), cost_id(INVALID_ID), cost_disc(0.0),
      ext_cost(0.0), ext_base(0.0), ext_tax(0.0), int_charges(0.0),
      ext_charges(0.0), tax_id(INVALID_ID), item_tax_id(INVALID_ID),
      include_tax(false), include_deposit(false), open_dept(false)
{
}

bool
ReceiveItem::operator==(const ReceiveItem& rhs) const
{
    if ((const ItemLine&)rhs != *this) return false;
    if (ordered != rhs.ordered) return false;
    if (received != rhs.received) return false;
    if (cost_id != rhs.cost_id) return false;
    if (cost_disc != rhs.cost_disc) return false;
    if (cost != rhs.cost) return false;
    if (ext_cost != rhs.ext_cost) return false;
    if (ext_base != rhs.ext_base) return false;
    if (ext_tax != rhs.ext_tax) return false;
    if (int_charges != rhs.int_charges) return false;
    if (ext_charges != rhs.ext_charges) return false;
    if (tax_id != rhs.tax_id) return false;
    if (item_tax_id != rhs.item_tax_id) return false;
    if (include_tax != rhs.include_tax) return false;
    if (include_deposit != rhs.include_deposit) return false;
    if (open_dept != rhs.open_dept) return false;
    return true;
}

bool
ReceiveItem::operator!=(const ReceiveItem& rhs) const
{
    return !(*this == rhs);
}

ReceiveCharge::ReceiveCharge(Id _charge_id, Id _tax_id, fixed _amt,
			     fixed _base, bool _int)
    : charge_id(_charge_id), tax_id(_tax_id), amount(_amt), base(_base),
      internal(_int)
{
}

bool
ReceiveCharge::operator==(const ReceiveCharge& rhs) const
{
    if (charge_id != rhs.charge_id) return false;
    if (tax_id != rhs.tax_id) return false;
    if (amount != rhs.amount) return false;
    if (base != rhs.base) return false;
    if (internal != rhs.internal) return false;
    return true;
}

bool
ReceiveCharge::operator!=(const ReceiveCharge& rhs) const
{
    return !(*this == rhs);
}

fixed
Receive::costTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _items.size(); ++i)
	total += _items[i].ext_base;
    return total;
}

fixed
Receive::chargeTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _charges.size(); ++i)
	if (_charges[i].internal)
	    total += _charges[i].base;
    return total;
}

fixed
Receive::total() const
{
    fixed total = costTotal() + depositTotal() + taxTotal() + chargeTotal();
    if (isClaim()) return -total;
    return total;
}

QString
Receive::lineTypeName() const
{
    return lineTypeName(_line_type);
}

QString
Receive::lineTypeName(LineType type)
{
    switch (type) {
    case Item:		return "item";
    case Account:	return "account";
    }
    return "Invalid Line Type";
}
