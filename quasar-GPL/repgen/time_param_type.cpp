// $Id: time_param_type.cpp,v 1.5 2005/01/31 23:22:34 bpepers Exp $
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

#include "time_param_type.h"
#include "time_edit.h"

TimeParamType::TimeParamType()
{
}

TimeParamType::~TimeParamType()
{
}

TimeParamType*
TimeParamType::clone() const
{
    TimeParamType* type = new TimeParamType();
    *type = *this;
    return type;
}

QWidget*
TimeParamType::getWidget(QWidget* parent)
{
    TimeEdit* widget = new TimeEdit(parent);
    return widget;
}

QVariant
TimeParamType::getValue(QWidget* w)
{
    TimeEdit* widget = (TimeEdit*)w;
    return widget->text();
}

void
TimeParamType::setValue(QWidget* w, const QVariant& value)
{
    TimeEdit* widget = (TimeEdit*)w;
    widget->setValue(value.toTime());
}

bool
TimeParamType::validate(QWidget* w)
{
    TimeEdit* widget = (TimeEdit*)w;
    return widget->valid();
}

bool
TimeParamType::convert(const QString& text, QVariant& value)
{
    if (ReportParamType::convert(text, value))
	return true;

    TimeValcon valcon;
    if (valcon.parse(text)) {
	value = valcon.getTime();
	return true;
    }

    qWarning("Invalid time param value: " + text);
    return false;
}
