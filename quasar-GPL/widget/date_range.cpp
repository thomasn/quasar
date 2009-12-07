// $Id: date_range.cpp,v 1.9 2005/01/30 04:40:58 bpepers Exp $
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

#include "date_range.h"
#include "date_popup.h"

DateRange::DateRange(QWidget* parent, const char* name)
    : QComboBox(parent, name), _from(NULL), _to(NULL)
{
    insertItem(tr("All"));
    insertItem(tr("Today"));
    insertItem(tr("This Week"));
    insertItem(tr("This Week-to-date"));
    insertItem(tr("This Month"));
    insertItem(tr("This Month-to-date"));
    insertItem(tr("This Year"));
    insertItem(tr("This Year-to-date"));
    insertItem(tr("Yesterday"));
    insertItem(tr("Last Week"));
    insertItem(tr("Last Week-to-date"));
    insertItem(tr("Last Month"));
    insertItem(tr("Last Month-to-date"));
    insertItem(tr("Last Year"));
    insertItem(tr("Last Year-to-date"));
    insertItem(tr("Next Week"));
    insertItem(tr("Next Month"));
    insertItem(tr("Next Year"));
    insertItem(tr("Custom"));

    connect(this, SIGNAL(activated(int)), SLOT(rangeChanged()));
}

DateRange::~DateRange()
{
}

void
DateRange::setFromPopup(DatePopup* popup)
{
    _from = popup;
    connect(_from, SIGNAL(validData()), SLOT(fromChanged()));
    rangeChanged();
}

void
DateRange::setToPopup(DatePopup* popup)
{
    _to = popup;
    connect(_to, SIGNAL(validData()), SLOT(toChanged()));
    rangeChanged();
}

void
DateRange::setCurrentItem(int item)
{
    QComboBox::setCurrentItem(item);
    rangeChanged();
}

void
DateRange::rangeChanged()
{
    int range = currentItem();
    QDate today = QDate::currentDate();
    QDate sunday = today - today.dayOfWeek();

    switch (range) {
    case Custom:
	// Do nothing if type became custom
	break;
    case All:
	if (_from) _from->setDate(QDate());
	if (_to) _to->setDate(QDate());
	break;
    case Today:
	if (_from) _from->setDate(today);
	if (_to) _to->setDate(today);
	break;
    case ThisWeek:
	if (_from) _from->setDate(sunday);
	if (_to) _to->setDate(sunday + 6);
	break;
    case ThisWeekTD:
	if (_from) _from->setDate(sunday);
	if (_to) _to->setDate(today);
	break;
    case ThisMonth:
	if (_from) _from->setDate(QDate(today.year(), today.month(), 1));
	if (_to) _to->setDate(QDate(today.year(), today.month() + 1, 1) - 1);
	break;
    case ThisMonthTD:
	if (_from) _from->setDate(QDate(today.year(), today.month(), 1));
	if (_to) _to->setDate(today);
	break;
    case ThisYear:
	if (_from) _from->setDate(QDate(today.year(), 1, 1));
	if (_to) _to->setDate(QDate(today.year(), 12, 31));
	break;
    case ThisYearTD:
	if (_from) _from->setDate(QDate(today.year(), 1, 1));
	if (_to) _to->setDate(today);
	break;
    case Yesterday:
	if (_from) _from->setDate(today - 1);
	if (_to) _to->setDate(today - 1);
	break;
    case LastWeek:
	if (_from) _from->setDate(sunday - 7);
	if (_to) _to->setDate(sunday - 1);
	break;
    case LastWeekTD:
	if (_from) _from->setDate(sunday - 7);
	if (_to) _to->setDate(today - 7);
	break;
    case LastMonth:
	if (_from) _from->setDate(QDate(today.year(), today.month() - 1, 1));
	if (_to) _to->setDate(QDate(today.year(), today.month(), 1) - 1);
	break;
    case LastMonthTD:
	if (_from) _from->setDate(QDate(today.year(), today.month() - 1, 1));
	if (_to) _to->setDate(QDate(today.year(),today.month()-1,today.day()));
	break;
    case LastYear:
	if (_from) _from->setDate(QDate(today.year() - 1, 1, 1));
	if (_to) _to->setDate(QDate(today.year() - 1, 12, 31));
	break;
    case LastYearTD:
	if (_from) _from->setDate(QDate(today.year() - 1, 1, 1));
	if (_to) _to->setDate(QDate(today.year()-1,today.month(),today.day()));
	break;
    case NextWeek:
	if (_from) _from->setDate(sunday + 7);
	if (_to) _to->setDate(sunday + 13);
	break;
    case NextMonth:
	if (_from) _from->setDate(QDate(today.year(), today.month() + 1, 1));
	if (_to) _to->setDate(QDate(today.year(), today.month() + 2, 1) - 1);
	break;
    case NextYear:
	if (_from) _from->setDate(QDate(today.year() + 1, 1, 1));
	if (_to) _to->setDate(QDate(today.year() + 1, 12, 31));
	break;
    default:
	qWarning("Error: invalid date range: %d", range);
    }

    QComboBox::setCurrentItem(range);
}

void
DateRange::fromChanged()
{
    setCurrentItem(Custom);
}

void
DateRange::toChanged()
{
    setCurrentItem(Custom);
    if (_to && _from && _to->getDate() < _from->getDate())
	_from->setDate(_to->getDate());
}
