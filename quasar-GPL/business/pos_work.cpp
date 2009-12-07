// $Id: pos_work.cpp,v 1.2 2004/08/03 23:03:33 bpepers Exp $
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

#include "pos_work.h"

PosWork::PosWork()
{
}

PosWork::~PosWork()
{
}

bool
PosWork::operator==(const PosWork& rhs) const
{
    if (rhs._type != _type) return false;
    if (rhs._id != _id) return false;
    if (rhs._status != _status) return false;
    if (rhs._store_id != _store_id) return false;
    if (rhs._station_id != _station_id) return false;
    if (rhs._employee_id != _employee_id) return false;
    return true;
}

bool
PosWork::operator!=(const PosWork& rhs) const
{
    return !(*this == rhs);
}

QString
PosWork::name() const
{
    return _type + "," + _id;
}
