// $Id: term.cpp,v 1.8 2004/12/30 00:07:58 bpepers Exp $
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

#include "term.h"
#include "percent_valcon.h"

Term::Term()
    : _cod(false), _disc_days(0), _due_days(0), _discount(0.0)
{
    _data_type = TERM;
}

Term::~Term()
{
}

QString
Term::name() const
{
    if (_cod)
	return "COD";
    if (_disc_days == 0 && _due_days == 0)
	return "";
    if (_disc_days == 0)
	return "Net " + QString::number(_due_days);
    return PercentValcon().format(_discount) + " " +
	QString::number(_disc_days) + " Net " + QString::number(_due_days);
}

bool
Term::operator==(const Term& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._cod != _cod) return false;
    if (rhs._disc_days != _disc_days) return false;
    if (rhs._due_days != _due_days) return false;
    if (rhs._discount != _discount) return false;
    return true;
}

bool
Term::operator!=(const Term& rhs) const
{
    return !(*this == rhs);
}
