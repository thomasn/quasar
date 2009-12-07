// $Id: time_valcon.cpp,v 1.13 2005/06/08 05:12:46 bpepers Exp $
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

#include "time_valcon.h"
#include "icu_util.h"

#include <unicode/datefmt.h>
#include <qvaluevector.h>

TimeValcon::TimeValcon()
{
}

TimeValcon::~TimeValcon()
{
}

bool
TimeValcon::parse(const QString& text)
{
    // Simple cases
    QString simple = text.simplifyWhiteSpace().lower();
    if (simple.isEmpty()) {
	_value = QTime();
	return true;
    } else if (simple == "t") {
	_value = QTime::currentTime();
	return true;
    }

    UnicodeString utext = convertToICU(text);

    // First try ICU ways of parsing the time
    QValueVector<DateFormat::EStyle> styles;
    styles.push_back(DateFormat::kMedium);
    styles.push_back(DateFormat::kShort);
    styles.push_back(DateFormat::kLong);
    styles.push_back(DateFormat::kFull);
    for (unsigned int i = 0; i < styles.size(); ++i) {
	DateFormat* fmt = DateFormat::createTimeInstance(styles[i]);
	UErrorCode status = U_ZERO_ERROR;
	UDate time = fmt->parse(utext, status);
	if (U_SUCCESS(status)) {
	    // Convert the UDate into a QTime
	    status = U_ZERO_ERROR;
	    Calendar* calendar = Calendar::createInstance(status);
	    if (U_SUCCESS(status)) {
		calendar->setTime(time, status);
		if (U_SUCCESS(status)) {
		    int hour = calendar->get(UCAL_HOUR_OF_DAY, status);
		    int minute = calendar->get(UCAL_MINUTE, status);
		    int second = calendar->get(UCAL_SECOND, status);
		    _value = QTime(hour, minute, second);
		    return true;
		}
	    }
	}
    }

    // Next try Qt methods
    QTime time = QTime::fromString(text, Qt::ISODate);
    if (time.isValid()) {
	_value = time;
	return true;
    }
    time = QTime::fromString(text, Qt::TextDate);
    if (time.isValid()) {
	_value = time;
	return true;
    }

    return false;
}

QString
TimeValcon::format()
{
    if (_value.isNull())
	return "";

    // Convert the QTime into a UDate
    UErrorCode status = U_ZERO_ERROR;
    Calendar* calendar = Calendar::createInstance(status);
    if (U_FAILURE(status)) {
	qWarning("Calendar::createInstance failed");
	return "";
    }
    calendar->set(2000, 0, 1, _value.hour(), _value.minute(), _value.second());
    UDate date = calendar->getTime(status);
    if (U_FAILURE(status)) {
	qWarning("Calendar::getTime failed");
	return "";
    }

    // Format time using ICU default time format
    DateFormat* fmt = DateFormat::createTimeInstance();
    UnicodeString text;
    fmt->format(Formattable(date, Formattable::kIsDate), text, status);
    if (U_FAILURE(status)) {
	qWarning("time format failed: " + _value.toString(Qt::ISODate));
	return "";
    }

    return convertToQt(text);
}

QString
TimeValcon::format(Variant value)
{
    switch (value.type()) {
    case Variant::T_NULL:
	break;
    case Variant::FIXED:
	return format(makeTime(value.toFixed().toInt()));
    case Variant::TIME:
	return format(value.toTime());
    default:
	qWarning("Invalid type for TimeValcon: %s", value.typeName());
    }
    return "";
}

QString
TimeValcon::format(const QTime& time)
{
    setTime(time);
    return format();
}
