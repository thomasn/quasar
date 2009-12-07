// $Id: plu.cpp,v 1.2 2004/02/03 00:13:26 arandell Exp $
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

#include "plu.h"

Plu::Plu()
{
}

Plu::Plu(Id item_id, const QString& number)
    : _item_id(item_id), _number(number)
{
}

Plu::~Plu()
{
}

bool
Plu::operator==(const Plu& rhs) const
{
    if (rhs._item_id != _item_id) return false;
    if (rhs._number != _number) return false;
    return true;
}

bool
Plu::operator!=(const Plu& rhs) const
{
    return !(*this == rhs);
}

QString
Plu::toString() const
{
    return _number;
}

bool
Plu::fromString(const QString& text)
{
    _number = text;
    return true;
}
