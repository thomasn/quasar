// $Id: date_valcon.cpp,v 1.25 2005/04/12 08:54:59 bpepers Exp $
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

#include "date_valcon.h"
#include "icu_util.h"

#include <unicode/datefmt.h>
#include <qvaluevector.h>

DateValcon::DateValcon()
{
}

DateValcon::~DateValcon()
{
}

bool
DateValcon::parse(const QString& text)
{
    // Simple cases
    QString simple = text.simplifyWhiteSpace().lower();
    if (simple.isEmpty()) {
	_value = QDate();
	return true;
    } else if (simple == "d") {
	_value = QDate::currentDate();
	return true;
    } else if (simple == "y") {
	_value = QDate::currentDate() - 1;
	return true;
    } else if (simple == "t") {
	_value = QDate::currentDate() + 1;
	return true;
    }

    UnicodeString utext = convertToICU(text);

    // First try ICU ways of parsing the date
    QValueVector<DateFormat::EStyle> styles;
    styles.push_back(DateFormat::kMedium);
    styles.push_back(DateFormat::kShort);
    styles.push_back(DateFormat::kLong);
    styles.push_back(DateFormat::kFull);
    for (unsigned int i = 0; i < styles.size(); ++i) {
	DateFormat* fmt = DateFormat::createDateInstance(styles[i]);
	UErrorCode status = U_ZERO_ERROR;
	UDate date = fmt->parse(utext, status);
	if (U_SUCCESS(status)) {
	    // Convert the UDate into a QDate
	    status = U_ZERO_ERROR;
	    Calendar* calendar = Calendar::createInstance(status);
	    if (U_SUCCESS(status)) {
		calendar->setTime(date, status);
		if (U_SUCCESS(status)) {
		    int year = calendar->get(UCAL_YEAR, status);
		    int month = calendar->get(UCAL_MONTH, status);
		    int day = calendar->get(UCAL_DAY_OF_MONTH, status);
		    _value = QDate(year, month+1, day);
		    return true;
		}
	    }
	}
    }

    // Next try Qt methods
    QDate date = QDate::fromString(text, Qt::ISODate);
    if (date.isValid()) {
	_value = date;
	return true;
    }
    date = QDate::fromString(text, Qt::TextDate);
    if (date.isValid()) {
	_value = date;
	return true;
    }

    return false;
}

QString
DateValcon::format()
{
    if (_value.isNull())
	return "";

    // Convert the QDate into a UDate
    UErrorCode status = U_ZERO_ERROR;
    Calendar* calendar = Calendar::createInstance(status);
    if (U_FAILURE(status)) {
	qWarning("Calendar::createInstance failed");
	return "";
    }
    calendar->set(_value.year(), _value.month()-1, _value.day(), 0, 0, 0);
    UDate date = calendar->getTime(status);
    if (U_FAILURE(status)) {
	qWarning("Calendar::getTime failed");
	return "";
    }

    // Format date using ICU default date format
    DateFormat* fmt = DateFormat::createDateInstance();
    UnicodeString text;
    fmt->format(Formattable(date, Formattable::kIsDate), text, status);
    if (U_FAILURE(status)) {
	qWarning("date format failed: " + _value.toString(Qt::ISODate));
	return "";
    }

    return convertToQt(text);
}

QString
DateValcon::format(Variant value)
{
    switch (value.type()) {
    case Variant::T_NULL:
	break;
    case Variant::FIXED:
	return format(makeDate(value.toFixed().toInt()));
    case Variant::DATE:
	return format(value.toDate());
    default:
	qWarning("Invalid type for DateValcon: %s", value.typeName());
    }
    return "";
}

QString
DateValcon::format(const QDate& date)
{
    setDate(date);
    return format();
}
