// $Id: order.cpp,v 1.26 2004/12/30 00:07:58 bpepers Exp $
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

#include "order.h"

Order::Order()
    : _vendor_id(INVALID_ID), _vendor_addr(""), _ship_id(INVALID_ID),
      _ship_addr(""), _ship_via(""), _term_id(INVALID_ID),
      _store_id(INVALID_ID), _comment(""), _number("#"), _reference("")
{
    _data_type = ORDER;
}

Order::~Order()
{
}

bool
Order::operator==(const Order& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._vendor_id != _vendor_id) return false;
    if (rhs._vendor_addr != _vendor_addr) return false;
    if (rhs._ship_id != _ship_id) return false;
    if (rhs._ship_addr != _ship_addr) return false;
    if (rhs._ship_via != _ship_via) return false;
    if (rhs._term_id != _term_id) return false;
    if (rhs._store_id != _store_id) return false;
    if (rhs._comment != _comment) return false;
    if (rhs._number != _number) return false;
    if (rhs._reference != _reference) return false;
    if (rhs._date != _date) return false;
    if (rhs._items != _items) return false;
    if (rhs._taxes != _taxes) return false;
    if (rhs._charges != _charges) return false;
    return true;
}

bool
Order::operator!=(const Order& rhs) const
{
    return !(*this == rhs);
}

OrderItem::OrderItem()
    : item_id(INVALID_ID), number(""), description(""), size(""),
      size_qty(0.0), ordered(0.0), billed(0.0), cost_id(INVALID_ID),
      cost_disc(0.0), ext_cost(0.0), ext_base(0.0), ext_deposit(0.0),
      ext_tax(0.0), int_charges(0.0), ext_charges(0.0), tax_id(INVALID_ID),
      item_tax_id(INVALID_ID), include_tax(false), include_deposit(false)
{
}

bool
OrderItem::operator==(const OrderItem& rhs) const
{
    if (item_id != rhs.item_id) return false;
    if (number != rhs.number) return false;
    if (description != rhs.description) return false;
    if (size != rhs.size) return false;
    if (size_qty != rhs.size_qty) return false;
    if (ordered != rhs.ordered) return false;
    if (billed != rhs.billed) return false;
    if (cost_id != rhs.cost_id) return false;
    if (cost_disc != rhs.cost_disc) return false;
    if (cost != rhs.cost) return false;
    if (ext_cost != rhs.ext_cost) return false;
    if (ext_base != rhs.ext_base) return false;
    if (ext_deposit != rhs.ext_deposit) return false;
    if (ext_tax != rhs.ext_tax) return false;
    if (int_charges != rhs.int_charges) return false;
    if (ext_charges != rhs.ext_charges) return false;
    if (tax_id != rhs.tax_id) return false;
    if (item_tax_id != rhs.item_tax_id) return false;
    if (include_tax != rhs.include_tax) return false;
    if (include_deposit != rhs.include_deposit) return false;
    return true;
}

bool
OrderItem::operator!=(const OrderItem& rhs) const
{
    return !(*this == rhs);
}

OrderTax::OrderTax(Id _tax_id, fixed _taxable, fixed _amount,
		   fixed _inc_taxable, fixed _inc_amount)
    : tax_id(_tax_id), taxable(_taxable), amount(_amount),
      inc_taxable(_inc_taxable), inc_amount(_inc_amount)
{
}

bool
OrderTax::operator==(const OrderTax& rhs) const
{
    if (tax_id != rhs.tax_id) return false;
    if (taxable != rhs.taxable) return false;
    if (amount != rhs.amount) return false;
    return true;
}

bool
OrderTax::operator!=(const OrderTax& rhs) const
{
    return !(*this == rhs);
}

OrderCharge::OrderCharge(Id _charge_id, Id _tax_id, fixed _amt, fixed _base,
			 bool _int)
    : charge_id(_charge_id), tax_id(_tax_id), amount(_amt), base(_base),
      internal(_int)
{
}

bool
OrderCharge::operator==(const OrderCharge& rhs) const
{
    if (charge_id != rhs.charge_id) return false;
    if (tax_id != rhs.tax_id) return false;
    if (amount != rhs.amount) return false;
    if (base != rhs.base) return false;
    if (internal != rhs.internal) return false;
    return true;
}

bool
OrderCharge::operator!=(const OrderCharge& rhs) const
{
    return !(*this == rhs);
}

fixed
Order::costTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _items.size(); ++i)
	total += _items[i].ext_base;
    return total;
}

fixed
Order::depositTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _items.size(); ++i)
	total += _items[i].ext_deposit;
    return total;
}

fixed
Order::taxTotal() const
{
    fixed total = 0;
    for (unsigned int i = 0; i < _taxes.size(); ++i)
	total += _taxes[i].amount + _taxes[i].inc_amount;
    return total;
}

fixed
Order::chargeTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _charges.size(); ++i)
	if (_charges[i].internal)
	    total += _charges[i].base;
    return total;
}

fixed
Order::total() const
{
    return costTotal() + depositTotal() + taxTotal() + chargeTotal();
}

bool
Order::allReceived() const
{
    for (unsigned int i = 0; i < _items.size(); ++i) {
	fixed ordered = _items[i].ordered;
	fixed billed = _items[i].billed;
	if (billed < ordered)
	    return false;
    }
    return true;
}
