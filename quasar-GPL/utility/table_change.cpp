// $Id: table_change.cpp,v 1.3 2004/02/03 00:13:26 arandell Exp $
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

#include "table_change.h"
#include <qstringlist.h>

TableChange::TableChange(const QString& _table, int _type)
    : table(_table), type(_type)
{
}

void
TableChange::addId(const QString& name, Variant value)
{
    ids.push_back(ColumnData(name, value));
}

void
TableChange::addData(const QString& name, Variant value)
{
    columns.push_back(ColumnData(name, value));
}

bool
TableChange::operator==(const TableChange& rhs) const
{
    if (rhs.table != table) return false;
    if (rhs.type != type) return false;
    if (rhs.ids != ids) return false;
    if (rhs.columns != columns) return false;
    return true;
}

bool
TableChange::operator!=(const TableChange& rhs) const
{
    return !(*this == rhs);
}
