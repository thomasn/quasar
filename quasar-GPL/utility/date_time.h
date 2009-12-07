// $Id: date_time.h,v 1.1 2005/01/31 18:17:14 bpepers Exp $
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

#ifndef DATE_TIME_H
#define DATE_TIME_H

#include <qdatetime.h>
#include <time.h>

// Add or subract a number of days from a date
QDate operator+(QDate date, int numDays);
QDate operator-(QDate date, int numDays);

// Get the number of days between two dates
int operator-(QDate lhs, QDate rhs);

// Get the month number from a full or partial month name
int monthNumber(const QString& name);

// Construct a date from struct tm* or time_t
QDate makeDate(struct tm* tm);
QDate makeDate(time_t time);

// Extract struct tm* or time_t from a date
struct tm* makeTM(QDate date);
time_t makeTimeT(QDate date);

// Construct a time from struct tm* or time_t
QTime makeTime(struct tm* tm);
QTime makeTime(time_t time);

// Extract struct tm* or time_t from a time
struct tm* makeTM(QTime time);
time_t makeTimeT(QTime time);

#endif // DATE_TIME_H
