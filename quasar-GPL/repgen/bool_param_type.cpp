// $Id: bool_param_type.cpp,v 1.5 2004/10/12 05:46:26 bpepers Exp $
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

#include "bool_param_type.h"
#include <qcheckbox.h>

BoolParamType::BoolParamType()
{
}

BoolParamType::~BoolParamType()
{
}

BoolParamType*
BoolParamType::clone() const
{
    BoolParamType* type = new BoolParamType();
    *type = *this;
    return type;
}

QWidget*
BoolParamType::getWidget(QWidget* parent)
{
    QCheckBox* widget = new QCheckBox(parent);
    return widget;
}

QVariant
BoolParamType::getValue(QWidget* w)
{
    QCheckBox* widget = (QCheckBox*)w;
    return widget->isChecked() ? "yes" : "no";
}

void
BoolParamType::setValue(QWidget* w, const QVariant& value)
{
    QCheckBox* widget = (QCheckBox*)w;
    widget->setChecked(value.toBool());
}

bool
BoolParamType::validate(QWidget*)
{
    return true;
}

bool
BoolParamType::convert(const QString& text, QVariant& value)
{
    if (ReportParamType::convert(text, value))
	return true;

    QString txt = text.lower();
    if (txt == "yes" || txt == "true" || txt == "on") {
	value = true;
	return true;
    }
    if (txt == "no" || txt == "false" || txt == "off") {
	value = false;
	return true;
    }

    qWarning("Invalid bool param value: " + text);
    return false;
}
