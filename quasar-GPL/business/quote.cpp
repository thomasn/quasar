// $Id: quote.cpp,v 1.11 2004/12/30 00:07:58 bpepers Exp $
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

#include "quote.h"

Quote::Quote()
    : _number("#"), _reference(""), _customer_addr(""),
      _ship_addr(""), _ship_via(""), _comment(""), _line_type(Item)
{
    _data_type = QUOTE;
}

Quote::~Quote()
{
}

bool
Quote::operator==(const Quote& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._number != _number) return false;
    if (rhs._reference != _reference) return false;
    if (rhs._date != _date) return false;
    if (rhs._store_id != _store_id) return false;
    if (rhs._employee_id != _employee_id) return false;
    if (rhs._customer_id != _customer_id) return false;
    if (rhs._customer_addr != _customer_addr) return false;
    if (rhs._ship_id != _ship_id) return false;
    if (rhs._ship_addr != _ship_addr) return false;
    if (rhs._ship_via != _ship_via) return false;
    if (rhs._term_id != _term_id) return false;
    if (rhs._comment != _comment) return false;
    if (rhs._expiry != _expiry) return false;
    if (rhs._line_type != _line_type) return false;
    if (rhs._tax_exempt_id != _tax_exempt_id) return false;
    if (rhs._invoice_id != _invoice_id) return false;
    if (rhs._taxes != _taxes) return false;
    if (rhs._items != _items) return false;
    if (rhs._discounts != _discounts) return false;
    return true;
}

bool
Quote::operator!=(const Quote& rhs) const
{
    return !(*this == rhs);
}

QString
Quote::lineTypeName() const
{
    return lineTypeName(_line_type);
}

QString
Quote::lineTypeName(LineType type)
{
    switch (type) {
    case Item:		return "item";
    case Account:	return "account";
    }
    return "Invalid Line Type";
}

fixed
Quote::priceTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _items.size(); ++i)
	total += _items[i].ext_base;
    return total;
}

fixed
Quote::depositTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _items.size(); ++i)
	total += _items[i].ext_deposit;
    return total;
}

fixed
Quote::lineDiscountTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _items.size(); ++i)
	total += _items[i].line_disc.total_amt;
    return total;
}

fixed
Quote::taxTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _taxes.size(); ++i)
	total += _taxes[i].amount + _taxes[i].inc_amount;
    return total;
}

fixed
Quote::transDiscountTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _discounts.size(); ++i)
	total += _discounts[i].total_amt;
    return total;
}

fixed
Quote::itemDiscountTotal() const
{
    return lineDiscountTotal() + transDiscountTotal();
}

fixed
Quote::total() const
{
    fixed total = priceTotal() - itemDiscountTotal() + depositTotal() +
	taxTotal();
    return total;
}
