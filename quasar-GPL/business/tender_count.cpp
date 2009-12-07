// $Id: tender_count.cpp,v 1.8 2004/12/30 00:07:58 bpepers Exp $
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

#include "tender_count.h"

TenderCount::TenderCount()
    : _number("")
{
    _data_type = TEND_COUNT;
}

TenderCount::~TenderCount()
{
}

bool
TenderCount::operator==(const TenderCount& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._number != _number) return false;
    if (rhs._date != _date) return false;
    if (rhs._station_id != _station_id) return false;
    if (rhs._employee_id != _employee_id) return false;
    if (rhs._store_id != _store_id) return false;
    if (rhs._shift_id != _shift_id) return false;
    if (rhs._tenders != _tenders) return false;
    return true;
}

bool
TenderCount::operator!=(const TenderCount& rhs) const
{
    return !(*this == rhs);
}

TenderInfo::TenderInfo()
    : amount(0.0)
{
}

bool
TenderInfo::operator==(const TenderInfo& rhs) const
{
    if (tender_id != rhs.tender_id) return false;
    if (amount != rhs.amount) return false;
    return true;
}

bool
TenderInfo::operator!=(const TenderInfo& rhs) const
{
    return !(*this == rhs);
}
