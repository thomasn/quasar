// $Id: integer_param_type.cpp,v 1.4 2004/10/12 05:46:26 bpepers Exp $
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

#include "integer_param_type.h"
#include "integer_edit.h"

IntegerParamType::IntegerParamType()
{
}

IntegerParamType::~IntegerParamType()
{
}

IntegerParamType*
IntegerParamType::clone() const
{
    IntegerParamType* type = new IntegerParamType();
    *type = *this;
    return type;
}

QWidget*
IntegerParamType::getWidget(QWidget* parent)
{
    IntegerEdit* widget = new IntegerEdit(parent);
    return widget;
}

QVariant
IntegerParamType::getValue(QWidget* w)
{
    IntegerEdit* widget = (IntegerEdit*)w;
    return widget->text();
}

void
IntegerParamType::setValue(QWidget* w, const QVariant& value)
{
    IntegerEdit* widget = (IntegerEdit*)w;
    widget->setValue(value.toInt());
}

bool
IntegerParamType::validate(QWidget* w)
{
    IntegerEdit* widget = (IntegerEdit*)w;
    return widget->valid();
}

bool
IntegerParamType::convert(const QString& text, QVariant& value)
{
    if (ReportParamType::convert(text, value))
	return true;

    IntegerValcon valcon;
    if (valcon.parse(text)) {
	value = valcon.getValue().toInt();
	return true;
    }

    qWarning("Invalid integer param value: " + text);
    return false;
}

