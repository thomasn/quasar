// $Id: price_valcon.h,v 1.2 2004/02/03 00:13:26 arandell Exp $
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

#ifndef PRICE_VALCON_H
#define PRICE_VALCON_H

#include "valcon.h"
#include "price.h"

class PriceValcon: public Valcon {
public:
    PriceValcon();
    ~PriceValcon();

    bool parse(const QString& text);
    QString format();
    QString format(Variant value);

    Price getPrice() const		{ return _price; }
    void setPrice(const Price& price)	{ _price = price; }
    QString format(const Price& price)	{ setPrice(price); return format(); }

protected:
    Price _price;
};

#endif // PRICE_VALCON_H
