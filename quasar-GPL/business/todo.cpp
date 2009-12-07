// $Id: todo.cpp,v 1.10 2004/12/30 00:07:58 bpepers Exp $
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

#include "todo.h"

Todo::Todo()
    : _note("")
{
    _data_type = TODO;
}

Todo::~Todo()
{
}

QString
Todo::description() const
{
    int pos = _note.find('\n');
    if (pos > 60) pos = 60;
    return _note.left(pos);
}

bool
Todo::operator==(const Todo& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._note != _note) return false;
    if (rhs._remind_on != _remind_on) return false;
    return true;
}

bool
Todo::operator!=(const Todo& rhs) const
{
    return !(*this == rhs);
}
