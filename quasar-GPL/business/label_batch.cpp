// $Id: label_batch.cpp,v 1.7 2004/12/30 00:07:58 bpepers Exp $
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

#include "label_batch.h"

LabelBatch::LabelBatch()
    : _number(""), _description(""), _type("")
{
    _data_type = LABEL_BATCH;
}

LabelBatch::~LabelBatch()
{
}

bool
LabelBatch::operator==(const LabelBatch& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._number != _number) return false;
    if (rhs._description != _description) return false;
    if (rhs._type != _type) return false;
    if (rhs._store_id != _store_id) return false;
    if (rhs._print_date != _print_date) return false;
    if (rhs._items != _items) return false;
    return true;
}

bool
LabelBatch::operator!=(const LabelBatch& rhs) const
{
    return !(*this == rhs);
}

LabelBatchItem::LabelBatchItem()
    : item_id(INVALID_ID), number(""), size(""), count(1)
{
}

bool
LabelBatchItem::operator==(const LabelBatchItem& rhs) const
{
    if (item_id != rhs.item_id) return false;
    if (number != rhs.number) return false;
    if (size != rhs.size) return false;
    if (price != rhs.price) return false;
    if (count != rhs.count) return false;
    return true;
}

bool
LabelBatchItem::operator!=(const LabelBatchItem& rhs) const
{
    return !(*this == rhs);
}
