// $Id: data_change.cpp,v 1.3 2004/02/03 00:13:26 arandell Exp $
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

#include "data_change.h"

DataChange::DataChange()
    : dataType(0), changeType(0), userName("")
{
}

DataChange::DataChange(int dt, Id id, int ct, const QString& user)
    : dataType(dt), objectId(id), changeType(ct), userName(user)
{
}

void
DataChange::add(const TableChange& change)
{
    changes.push_back(change);
}

bool
DataChange::operator==(const DataChange& rhs) const
{
    if (rhs.dataType != dataType) return false;
    if (rhs.objectId != objectId) return false;
    if (rhs.changeType != changeType) return false;
    if (rhs.date != date) return false;
    if (rhs.time != time) return false;
    if (rhs.userName != userName) return false;
    if (rhs.changes != changes) return false;
    return true;
}

bool
DataChange::operator!=(const DataChange& rhs) const
{
    return !(*this == rhs);
}
