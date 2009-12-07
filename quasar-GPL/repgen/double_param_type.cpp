// $Id: double_param_type.cpp,v 1.4 2004/10/12 05:46:26 bpepers Exp $
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

#include "double_param_type.h"
#include "double_edit.h"

DoubleParamType::DoubleParamType()
{
}

DoubleParamType::~DoubleParamType()
{
}

DoubleParamType*
DoubleParamType::clone() const
{
    DoubleParamType* type = new DoubleParamType();
    *type = *this;
    return type;
}

QWidget*
DoubleParamType::getWidget(QWidget* parent)
{
    DoubleEdit* widget = new DoubleEdit(parent);
    return widget;
}

QVariant
DoubleParamType::getValue(QWidget* w)
{
    DoubleEdit* widget = (DoubleEdit*)w;
    return widget->text();
}

void
DoubleParamType::setValue(QWidget* w, const QVariant& value)
{
    DoubleEdit* widget = (DoubleEdit*)w;
    widget->setValue(value.toDouble());
}

bool
DoubleParamType::validate(QWidget* w)
{
    DoubleEdit* widget = (DoubleEdit*)w;
    return widget->valid();
}

bool
DoubleParamType::convert(const QString& text, QVariant& value)
{
    if (ReportParamType::convert(text, value))
	return true;

    NumberValcon valcon;
    if (valcon.parse(text)) {
	value = valcon.getValue().toDouble();
	return true;
    }

    qWarning("Invalid double param value: " + text);
    return false;
}
