// $Id: time_text_format.cpp,v 1.1 2004/08/09 22:20:48 bpepers Exp $
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

#include "time_text_format.h"
#include "time_valcon.h"

TimeTextFormat::TimeTextFormat()
{
}

TimeTextFormat::~TimeTextFormat()
{
}

TimeTextFormat*
TimeTextFormat::clone() const
{
    TimeTextFormat* format = new TimeTextFormat();
    *format = *this;
    return format;
}

QString
TimeTextFormat::format(const QString& value)
{
    if (value.isEmpty()) return value;

    TimeValcon valcon;
    if (valcon.parse(value))
	return valcon.format();

    qWarning("Invalid time: " + value);
    return "";
}
