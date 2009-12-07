// $Id: order_template.cpp,v 1.9 2004/12/30 00:07:58 bpepers Exp $
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

#include "order_template.h"

OrderTemplate::OrderTemplate()
    : _name("")
{
    _data_type = PO_TEMPLATE;
}

OrderTemplate::~OrderTemplate()
{
}

bool
OrderTemplate::operator==(const OrderTemplate& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._name != _name) return false;
    if (rhs._vendor_id != _vendor_id) return false;
    if (rhs._items != _items) return false;
    if (rhs._charges != _charges) return false;
    return true;
}

bool
OrderTemplate::operator!=(const OrderTemplate& rhs) const
{
    return !(*this == rhs);
}

TemplateItem::TemplateItem()
    : item_id(INVALID_ID), number(""), size("")
{
}

bool
TemplateItem::operator==(const TemplateItem& rhs) const
{
    if (item_id != rhs.item_id) return false;
    if (number != rhs.number) return false;
    if (size != rhs.size) return false;
    return true;
}

bool
TemplateItem::operator!=(const TemplateItem& rhs) const
{
    return !(*this == rhs);
}

TemplateCharge::TemplateCharge(Id _ch_id, Id _tax_id, fixed _amt, bool _int)
    : charge_id(_ch_id), tax_id(_tax_id), amount(_amt), internal(_int)
{
}

bool
TemplateCharge::operator==(const TemplateCharge& rhs) const
{
    if (charge_id != rhs.charge_id) return false;
    if (tax_id != rhs.tax_id) return false;
    if (amount != rhs.amount) return false;
    if (internal != rhs.internal) return false;
    return true;
}

bool
TemplateCharge::operator!=(const TemplateCharge& rhs) const
{
    return !(*this == rhs);
}
