// $Id: recurring.cpp,v 1.10 2005/01/30 00:51:13 bpepers Exp $
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

#include "recurring.h"

Recurring::Recurring()
    : _gltx_id(INVALID_ID), _desc(""), _frequency(Monthly), _day1(1),
      _day2(15), _max_post(0), _post_count(0), _card_group(INVALID_ID)
{
    _data_type = RECURRING;
}

Recurring::~Recurring()
{
}

bool
Recurring::operator==(const Recurring& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._gltx_id != _gltx_id) return false;
    if (rhs._desc != _desc) return false;
    if (rhs._frequency != _frequency) return false;
    if (rhs._day1 != _day1) return false;
    if (rhs._day2 != _day2) return false;
    if (rhs._max_post != _max_post) return false;
    if (rhs._last_posted != _last_posted) return false;
    if (rhs._post_count != _post_count) return false;
    if (rhs._card_group != _card_group) return false;
    return true;
}

bool
Recurring::operator!=(const Recurring& rhs) const
{
    return !(*this == rhs);
}

int
Recurring::overdueDays() const
{
    return QDate::currentDate().daysTo(nextDue());
}

QDate
Recurring::nextDue() const
{
    int dueDay = _last_posted.day();
    int dueMonth = _last_posted.month();
    int dueYear = _last_posted.year();

    switch (_frequency) {
    case Daily:
	dueDay += 1;
	break;
    case Weekly:
	dueDay += 7;
	break;
    case BiWeekly:
	dueDay += 14;
	break;
    case SemiMonthly:
	if (dueDay == _day1) {
	    dueDay = _day2;
	    if (dueDay > _last_posted.daysInMonth())
		dueDay = _last_posted.daysInMonth();
	} else {
	    dueDay = _day1;
	    dueMonth += 1;
	}
	break;
    case Monthly:
	dueMonth += 1;
	break;
    case Quarterly:
	dueMonth += 3;
	break;
    case SemiAnually:
	dueMonth += 6;
	break;
    case Anually:
	dueYear += 1;
	break;
    default:
	break;
    }

    if (dueMonth > 12) {
	dueYear += 1;
	dueMonth -= 12;
    }

    return QDate(dueYear, dueMonth, dueDay);
}
