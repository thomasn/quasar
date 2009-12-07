// $Id: item_price.cpp,v 1.17 2004/12/30 00:07:58 bpepers Exp $
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

#include "item_price.h"

ItemPrice::ItemPrice()
    : _is_cost(false), _item_id(INVALID_ID), _number(""),
      _item_group(INVALID_ID), _dept_id(INVALID_ID), _subdept_id(INVALID_ID),
      _card_id(INVALID_ID), _card_group(INVALID_ID), _store_id(INVALID_ID),
      _promotion(false), _discountable(true), _method(PRICE)
{
    _data_type = ITEM_PRICE;
    setNull();
    for (int day = 0; day < 7; ++day)
	_day_of_week[day] = true;
}

ItemPrice::~ItemPrice()
{
}

bool
ItemPrice::operator==(const ItemPrice& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._is_cost != _is_cost) return false;
    if (rhs._item_id != _item_id) return false;
    if (rhs._number != _number) return false;
    if (rhs._item_group != _item_group) return false;
    if (rhs._dept_id != _dept_id) return false;
    if (rhs._subdept_id != _subdept_id) return false;
    if (rhs._size != _size) return false;
    if (rhs._card_id != _card_id) return false;
    if (rhs._card_group != _card_group) return false;
    if (rhs._store_id != _store_id) return false;
    if (rhs._promotion != _promotion) return false;
    if (rhs._discountable != _discountable) return false;
    if (rhs._start_date != _start_date) return false;
    if (rhs._stop_date != _stop_date) return false;
    if (rhs._qty_limit != _qty_limit) return false;
    if (rhs._min_qty != _min_qty) return false;
    for (int d = 0; d < 7; ++d)
	if (rhs._day_of_week[d] != _day_of_week[d]) return false;
    if (rhs._method != _method) return false;
    if (rhs._price != _price) return false;
    if (rhs._percent != _percent) return false;
    if (rhs._dollar != _dollar) return false;
    return true;
}

bool
ItemPrice::operator!=(const ItemPrice& rhs) const
{
    return !(*this == rhs);
}

bool
ItemPrice::operator>(const ItemPrice& rhs) const
{
    return _price > rhs._price;
}

bool
ItemPrice::operator>=(const ItemPrice& rhs) const
{
    return _price >= rhs._price;
}

bool
ItemPrice::operator<(const ItemPrice& rhs) const
{
    return _price < rhs._price;
}

bool
ItemPrice::operator<=(const ItemPrice& rhs) const
{
    return _price <= rhs._price;
}

QString
ItemPrice::dayOfWeekString() const
{
    QString text = "";
    for (int day = 0; day < 7; ++day) {
	if (_day_of_week[day])
	    text += "Y";
	else
	    text += "N";
    }
    return text;
}

void
ItemPrice::setDayOfWeek(const QString& days)
{
    for (int i = 0; i < 7; ++i) {
	_day_of_week[i] = (days[i] == 'Y');
    }
}
