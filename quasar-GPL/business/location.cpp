// $Id: location.cpp,v 1.11 2004/12/30 00:07:58 bpepers Exp $
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

#include "location.h"

Location::Location()
    : _name(""), _store_id(INVALID_ID), _section(""), _fixture(""), _bin("")
{
    _data_type = LOCATION;
}

Location::~Location()
{
}

bool
Location::operator==(const Location& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._name != _name) return false;
    if (rhs._store_id != _store_id) return false;
    if (rhs._section != _section) return false;
    if (rhs._fixture != _fixture) return false;
    if (rhs._bin != _bin) return false;
    return true;
}

bool
Location::operator!=(const Location& rhs) const
{
    return !(*this == rhs);
}

QString
Location::canonicalName() const
{
    QString name = _section;
    if (!_fixture.isEmpty()) name += "," + _fixture;
    if (!_bin.isEmpty()) name += "," + _bin;
    return name;
}
