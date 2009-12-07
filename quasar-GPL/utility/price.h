// $Id: price.h,v 1.3 2004/02/03 00:13:26 arandell Exp $
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

#ifndef PRICE_H
#define PRICE_H

#include "fixed.h"

class Price {
public:
    // Constructors and Destructor
    Price();
    Price(fixed price, fixed for_qty=0.0, fixed for_price=0.0);
    ~Price();

    // Get methods
    fixed unitPrice()			const { return _unit_price; }
    fixed forQty()			const { return _for_qty; }
    fixed forPrice()			const { return _for_price; }

    // Set methods
    void setUnitPrice(fixed price)		{ _unit_price = price; }
    void setForQty(fixed qty)			{ _for_qty = qty; }
    void setForPrice(fixed price)		{ _for_price = price; }

    // Simple methods
    fixed price() const;
    void setPrice(fixed price);

    // A null price is one with _for_qty == 0.0 and _for_price != 0.0
    bool isNull() const;
    void setNull();

    // Operations
    bool operator==(const Price& rhs) const;
    bool operator!=(const Price& rhs) const;
    bool operator< (const Price& rhs) const;
    bool operator<=(const Price& rhs) const;
    bool operator> (const Price& rhs) const;
    bool operator>=(const Price& rhs) const;

    // Calculate price for a quantity of items
    fixed calculate(fixed qty) const;

    // Calculate new price using markup or margin
    Price markup(fixed percent) const;
    Price margin(fixed percent) const;

    // Convert to/from string
    QString toString() const;
    bool fromString(const QString& text);

protected:
    fixed _unit_price;		// Single unit price
    fixed _for_qty;		// At/For price quantity
    fixed _for_price;		// At/For price value
};

#endif // PRICE_H
