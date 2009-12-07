// $Id: employee.cpp,v 1.20 2004/12/30 00:07:58 bpepers Exp $
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

#include "employee.h"

Employee::Employee()
    : _login_name(""), _password(""), _pos_level(0)
{
    _data_type = EMPLOYEE;
}

Employee::~Employee()
{
}

bool
Employee::operator==(const Employee& rhs) const
{
    if ((const Card&)rhs != *this) return false;
    if (rhs._login_name != _login_name) return false;
    if (rhs._password != _password) return false;
    if (rhs._store_id != _store_id) return false;
    if (rhs._pos_level != _pos_level) return false;
    return true;
}

bool
Employee::operator!=(const Employee& rhs) const
{
    return !(*this == rhs);
}
