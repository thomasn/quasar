// $Id: date_time.cpp,v 1.5 2005/02/28 21:10:19 bpepers Exp $
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

#include "date_time.h"
#include <stdlib.h>

QDate
operator+(QDate date, int numDays)
{
    return date.addDays(numDays);
}

QDate
operator-(QDate date, int numDays)
{
    return date.addDays(-numDays);
}

int
operator-(QDate lhs, QDate rhs)
{
    return abs(lhs.daysTo(rhs));
}

int
monthNumber(const QString& name)
{
    int month = -1;
    for (int i = 1; i <= 12; ++i) {
	QString nm = QDate::longMonthName(i);
	if (name.lower() == nm.lower().left(name.length())) {
	    if (month != -1) return -1;
	    month = i;
	}
    }
    return month;
}

QDate
makeDate(struct tm* tm)
{
    return QDate(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
}

QDate
makeDate(time_t time)
{
    return makeDate(localtime(&time));
}

struct tm*
makeTM(QDate date)
{
    static struct tm tm;
    tm.tm_year = date.year() - 1900;
    tm.tm_mon = date.month() - 1;
    tm.tm_mday = date.day();
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    tm.tm_wday = date.dayOfWeek() - 1;
    tm.tm_yday = date.dayOfYear() - 1;
    tm.tm_isdst = -1;
    return &tm;
}

time_t
makeTimeT(QDate date)
{
    return mktime(makeTM(date));
}

QTime
makeTime(struct tm* tm)
{
    return QTime(tm->tm_hour, tm->tm_min, tm->tm_sec);
}

QTime
makeTime(time_t time)
{
    return makeTime(localtime(&time));
}

struct tm*
makeTM(QTime time)
{
    static struct tm tm;
    tm.tm_year = 70;
    tm.tm_mon = 0;
    tm.tm_mday = 1;
    tm.tm_hour = time.hour();
    tm.tm_min = time.minute();
    tm.tm_sec = time.second();
    tm.tm_wday = 0;
    tm.tm_yday = 0;
    tm.tm_isdst = -1;
    return &tm;
}

time_t
makeTimeT(QTime time)
{
    return mktime(makeTM(time));
}
