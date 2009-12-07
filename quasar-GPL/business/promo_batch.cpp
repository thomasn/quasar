// $Id: promo_batch.cpp,v 1.5 2004/12/30 00:07:58 bpepers Exp $
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

#include "promo_batch.h"

PromoBatch::PromoBatch()
    : _number(""), _description("")
{
    _data_type = PROMO_BATCH;
}

PromoBatch::~PromoBatch()
{
}

bool
PromoBatch::operator==(const PromoBatch& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._number != _number) return false;
    if (rhs._description != _description) return false;
    if (rhs._store_id != _store_id) return false;
    if (rhs._from_date != _from_date) return false;
    if (rhs._to_date != _to_date) return false;
    if (rhs._exec_date != _exec_date) return false;
    if (rhs._items != _items) return false;
    return true;
}

bool
PromoBatch::operator!=(const PromoBatch& rhs) const
{
    return !(*this == rhs);
}

PromoBatchItem::PromoBatchItem()
    : item_id(INVALID_ID), number(""), size(""), price(0.0), ordered(0.0)
{
}

bool
PromoBatchItem::operator==(const PromoBatchItem& rhs) const
{
    if (item_id != rhs.item_id) return false;
    if (number != rhs.number) return false;
    if (size != rhs.size) return false;
    if (price != rhs.price) return false;
    if (ordered != rhs.ordered) return false;
    if (price_id != rhs.price_id) return false;
    return true;
}

bool
PromoBatchItem::operator!=(const PromoBatchItem& rhs) const
{
    return !(*this == rhs);
}
