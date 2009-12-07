// $Id: date_param_type.cpp,v 1.7 2005/01/31 09:12:50 bpepers Exp $
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

#include "date_param_type.h"
#include "date_popup.h"

DateParamType::DateParamType()
{
}

DateParamType::~DateParamType()
{
}

DateParamType*
DateParamType::clone() const
{
    DateParamType* type = new DateParamType();
    *type = *this;
    return type;
}

QWidget*
DateParamType::getWidget(QWidget* parent)
{
    DatePopup* widget = new DatePopup(parent);
    return widget;
}

QVariant
DateParamType::getValue(QWidget* w)
{
    DatePopup* widget = (DatePopup*)w;
    return widget->getDate();
}

void
DateParamType::setValue(QWidget* w, const QVariant& value)
{
    DatePopup* widget = (DatePopup*)w;
    widget->setValue(QDate(value.toDate()));
}

bool
DateParamType::validate(QWidget* w)
{
    DatePopup* widget = (DatePopup*)w;
    return widget->valid();
}

bool
DateParamType::convert(const QString& text, QVariant& value)
{
    if (ReportParamType::convert(text, value))
	return true;

    DateValcon valcon;
    if (valcon.parse(text)) {
	value = QDate(valcon.getDate());
	return true;
    }

    QDate date = QDate::fromString(text, Qt::ISODate);
    if (!date.isNull()) {
	value = QDate(date);
	return true;
    }

    qWarning("Invalid date param value: " + text);
    return false;
}
