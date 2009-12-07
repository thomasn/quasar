// $Id: percent_valcon.cpp,v 1.13 2005/04/04 19:23:27 bpepers Exp $
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

#include "percent_valcon.h"
#include "icu_util.h"

#include <unicode/numfmt.h>

PercentValcon::PercentValcon()
{
}

PercentValcon::~PercentValcon()
{
}

bool
PercentValcon::parse(const QString& text)
{
    if (text.isEmpty()) {
	_value = 0;
	return true;
    }

    UnicodeString utext = convertToICU(text);

    // First try a percent parse
    UErrorCode status = U_ZERO_ERROR;
    NumberFormat* fmt = NumberFormat::createPercentInstance(status);
    if (U_SUCCESS(status)) {
	Formattable value;
	ParsePosition pos;
	fmt->parse(utext, value, pos);
	if (pos.getErrorIndex() == -1 && pos.getIndex() == utext.length()) {
#if U_ICU_VERSION_MAJOR_NUM < 3
	    _value = value.getDouble(&status) * 100.0;
#else
	    _value = value.getDouble(status) * 100.0;
#endif
	    return true;
	}
    }

    // Next try as just a number
    status = U_ZERO_ERROR;
    fmt = NumberFormat::createInstance(status);
    if (U_SUCCESS(status)) {
	Formattable value;
	ParsePosition pos;
	fmt->parse(utext, value, pos);
	if (pos.getErrorIndex() == -1 && pos.getIndex() == utext.length()) {
#if U_ICU_VERSION_MAJOR_NUM < 3
	    _value = value.getDouble(&status);
#else
	    _value = value.getDouble(status);
#endif
	    return true;
	}
    }

    return false;
}

QString
PercentValcon::format()
{
    // Format money using ICU default percent format
    UnicodeString text;
    UErrorCode status = U_ZERO_ERROR;
    NumberFormat* fmt = NumberFormat::createPercentInstance(status);
    fmt->setMaximumFractionDigits(4);
    fmt->format(_value.toDouble() / 100.0, text, status);
    if (U_FAILURE(status))
	return "";

    return convertToQt(text);
}

QString
PercentValcon::format(Variant value)
{
    if (value.type() == Variant::T_NULL) return "";
    return format(value.toFixed());
}

QString
PercentValcon::format(fixed value)
{
    setValue(value);
    return format();
}
