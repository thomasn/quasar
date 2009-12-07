// $Id: string_param_type.cpp,v 1.4 2004/10/12 05:46:26 bpepers Exp $
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

#include "string_param_type.h"
#include "line_edit.h"

StringParamType::StringParamType()
{
}

StringParamType::~StringParamType()
{
}

StringParamType*
StringParamType::clone() const
{
    StringParamType* type = new StringParamType();
    *type = *this;
    return type;
}

QWidget*
StringParamType::getWidget(QWidget* parent)
{
    LineEdit* widget = new LineEdit(parent);
    if (!_param.getAttribute("maxLength").isEmpty())
	widget->setMaxLength(_param.getAttribute("maxLength").toInt());
    return widget;
}

QVariant
StringParamType::getValue(QWidget* w)
{
    LineEdit* widget = (LineEdit*)w;
    return widget->text();
}

void
StringParamType::setValue(QWidget* w, const QVariant& value)
{
    LineEdit* widget = (LineEdit*)w;
    widget->setText(value.toString());
}

bool
StringParamType::validate(QWidget*)
{
    return true;
}
