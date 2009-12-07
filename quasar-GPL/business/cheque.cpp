// $Id: cheque.cpp,v 1.24 2004/12/30 00:07:58 bpepers Exp $
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

#include "cheque.h"
#include <assert.h>

Cheque::Cheque()
    : _type(ACCOUNT), _address("")
{
    _data_type = CHEQUE;
}

Cheque::~Cheque()
{
}

bool
Cheque::operator==(const Cheque& rhs) const
{
    if ((const Gltx&)rhs != *this) return false;
    if (rhs._type != _type) return false;
    if (rhs._account_id != _account_id) return false;
    if (rhs._disc_id != _disc_id) return false;
    if (rhs._address != _address) return false;
    return true;
}

bool
Cheque::operator!=(const Cheque& rhs) const
{
    return !(*this == rhs);
}

fixed
Cheque::total() const
{
    if (_accounts.size() == 0) return 0.0;
    return -_accounts[0].amount;
}

QString
Cheque::typeName() const
{
    return typeName(_type);
}

QString
Cheque::typeName(Type type)
{
    switch (type) {
    case ACCOUNT:	return "account";
    case CUSTOMER:	return "customer";
    case VENDOR:	return "vendor";
    }
    return "Invalid Cheque Type";
}
