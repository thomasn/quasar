// $Id: percent_text_format.cpp,v 1.2 2005/02/15 08:07:36 bpepers Exp $
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

#include "percent_text_format.h"
#include "percent_valcon.h"

PercentTextFormat::PercentTextFormat()
{
}

PercentTextFormat::~PercentTextFormat()
{
}

PercentTextFormat*
PercentTextFormat::clone() const
{
    PercentTextFormat* format = new PercentTextFormat();
    *format = *this;
    return format;
}

QString
PercentTextFormat::format(const QString& value)
{
    if (value.isEmpty()) return value;

    PercentValcon valcon;

    // First try converting string straight to a number
    bool ok = false;
    double val = value.toDouble(&ok);
    if (ok)
	return valcon.format(fixed(val));

    // If wasn't a straight number, try parsing it
    if (valcon.parse(value))
	return valcon.format();

    qWarning("Invalid percent: " + value);
    return "";
}
