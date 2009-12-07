// $Id: group.cpp,v 1.8 2004/12/30 00:07:58 bpepers Exp $
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

#include "group.h"
#include <assert.h>

Group::Group()
    : _name(""), _desc(""), _type(ACCOUNT)
{
    _data_type = GROUP;
}

Group::~Group()
{
}

bool
Group::operator==(const Group& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._name != _name) return false;
    if (rhs._desc != _desc) return false;
    if (rhs._type != _type) return false;
    return true;
}

bool
Group::operator!=(const Group& rhs) const
{
    return !(*this == rhs);
}

QString
Group::typeName() const
{
    return typeName(_type);
}

QString
Group::typeName(int type)
{
    switch (type) {
    case ACCOUNT:		return "Account";
    case CUSTOMER:		return "Customer";
    case EMPLOYEE:		return "Employee";
    case VENDOR:		return "Vendor";
    case PERSONAL:		return "Personal";
    case ITEM:			return "Item";
    }
    return "Invalid Group Type";
}

int
Group::type(const QString& name)
{
    if (name == "Account") return ACCOUNT;
    if (name == "Customer") return CUSTOMER;
    if (name == "Employee") return EMPLOYEE;
    if (name == "Vendor") return VENDOR;
    if (name == "Personal") return PERSONAL;
    if (name == "Item") return ITEM;

    assert(false);
    return GROUP;
}
