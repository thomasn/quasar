// $Id: price_valcon.cpp,v 1.3 2004/02/03 00:13:26 arandell Exp $
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

#include "price_valcon.h"

PriceValcon::PriceValcon()
{
}

PriceValcon::~PriceValcon()
{
}

bool
PriceValcon::parse(const QString& text)
{
    return _price.fromString(text);
}

QString
PriceValcon::format()
{
    return _price.toString();
}

QString
PriceValcon::format(Variant value)
{
    switch (value.type()) {
    case Variant::T_NULL:
	break;
    case Variant::FIXED:
	_price.setPrice(value.toFixed());
	return format();
    case Variant::PRICE:
	_price = value.toPrice();
	return format();
    default:
	qWarning("Invalid type for PriceValcon: %s", value.typeName());
    }
    return "";
}
