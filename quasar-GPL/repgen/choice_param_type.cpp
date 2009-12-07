// $Id: choice_param_type.cpp,v 1.2 2004/10/12 05:46:26 bpepers Exp $
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

#include "choice_param_type.h"
#include <qcombobox.h>

ChoiceParamType::ChoiceParamType()
{
}

ChoiceParamType::~ChoiceParamType()
{
}

ChoiceParamType*
ChoiceParamType::clone() const
{
    ChoiceParamType* type = new ChoiceParamType();
    *type = *this;
    return type;
}

QWidget*
ChoiceParamType::getWidget(QWidget* parent)
{
    QComboBox* widget = new QComboBox(false, parent);

    QString choices = _param.getAttribute("choices");
    widget->insertStringList(QStringList::split(",", choices));

    return widget;
}

QVariant
ChoiceParamType::getValue(QWidget* w)
{
    QComboBox* widget = (QComboBox*)w;
    return widget->currentText();
}

void
ChoiceParamType::setValue(QWidget* w, const QVariant& value)
{
    QComboBox* widget = (QComboBox*)w;
    widget->setCurrentText(value.toString());
}

bool
ChoiceParamType::validate(QWidget*)
{
    return true;
}
