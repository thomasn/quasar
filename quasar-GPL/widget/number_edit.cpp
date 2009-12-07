// $Id: number_edit.cpp,v 1.20 2004/02/03 00:56:02 arandell Exp $
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

#include "number_edit.h"

NumberEdit::NumberEdit(NumberValcon& valcon, QWidget* parent, const char* name)
    : ValconEdit(valcon, parent, name)
{
    setAlignment(AlignRight);
}

NumberEdit::~NumberEdit()
{
}

NumberValcon&
NumberEdit::valcon()
{
    return (NumberValcon&)_valcon;
}

const NumberValcon&
NumberEdit::valcon() const
{
    return (const NumberValcon&)_valcon;
}

Variant
NumberEdit::value()
{
    Variant value;
    QString txt = text().simplifyWhiteSpace();
    if (txt == "")
	return value;

    if (!_valcon.parse(txt))
	return value;

    return valcon().getValue();
}

fixed
NumberEdit::getFixed() const
{
    _valcon.parse(text());
    return valcon().getValue();
}

void
NumberEdit::setValue(Variant value)
{
    switch (value.type()) {
    case Variant::T_NULL:
	QLineEdit::setText("");
	_changed = false;
	_valid = true;
	break;
    case Variant::STRING:
	setText(value.toString());
	break;
    case Variant::FIXED:
	setFixed(value.toFixed());
	break;
    default:
	qWarning("Invalid type for NumberEdit: %s", value.typeName());
    }
}

void
NumberEdit::setFixed(fixed value)
{
    QLineEdit::setText(_valcon.format(value));
    _changed = false;
    _valid = true;
}
