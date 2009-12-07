// $Id: sequence.cpp,v 1.3 2004/01/30 23:06:57 arandell Exp $
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

#include "sequence.h"

Sequence::Sequence()
    : _system_id(0), _min_num(DEFAULT_MIN), _max_num(DEFAULT_MAX),
      _next_num(DEFAULT_MIN)
{
}

Sequence::~Sequence()
{
}

bool
Sequence::operator==(const Sequence& rhs) const
{
    if (rhs._table_name != _table_name) return false;
    if (rhs._attr_name != _attr_name) return false;
    if (rhs._system_id != _system_id) return false;
    if (rhs._min_num != _min_num) return false;
    if (rhs._max_num != _max_num) return false;
    if (rhs._next_num != _next_num) return false;
    return true;
}

bool
Sequence::operator!=(const Sequence& rhs) const
{
    return !(*this == rhs);
}
