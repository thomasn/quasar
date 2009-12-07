// $Id: id_edit.cpp,v 1.9 2005/03/02 03:07:29 bpepers Exp $
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

#include "id_edit.h"

IdEdit::IdEdit(QWidget* parent, const char* name)
    : IntegerEdit(14, parent, name)
{
}

IdEdit::~IdEdit()
{
}

Variant
IdEdit::value()
{
    if (text() == "#") return Variant();
    return IntegerEdit::value();
}

fixed
IdEdit::getFixed() const
{
    if (text() == "#") return -1;
    return IntegerEdit::getFixed();
}

void
IdEdit::setValue(Variant value)
{
    if (value.isNull() || value == "#" || value == -1)
	QLineEdit::setText("#");
    else
	IntegerEdit::setValue(value);
}

void
IdEdit::setText(const QString& value)
{
    if (value == "#" || value.isEmpty())
	QLineEdit::setText("#");
    else
	IntegerEdit::setText(value);
}

void
IdEdit::setFixed(fixed value)
{
    if (value == -1)
	QLineEdit::setText("#");
    else
	IntegerEdit::setFixed(value);
}

bool
IdEdit::doValidation(Reason reason)
{
    if (text() == "#") return true;
    return IntegerEdit::doValidation(reason);
}
