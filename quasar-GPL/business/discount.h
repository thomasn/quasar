// $Id: discount.h,v 1.6 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef DISCOUNT_H
#define DISCOUNT_H

#include "data_object.h"

class Discount: public DataObject {
public:
    // Constructors and Destructor
    Discount();
    ~Discount();

    enum Method { PERCENT, DOLLAR };

    // Get methods
    const QString& name()		const { return _name; }
    bool lineDiscount()			const { return _line_disc; }
    bool txDiscount()			const { return _tx_disc; }
    Id accountId()			const { return _account_id; }
    Method method()			const { return _method; }
    fixed amount()			const { return _amount; }

    // Set methods
    void setName(const QString& name)		{ _name = name; }
    void setLineDiscount(bool flag)		{ _line_disc = flag; }
    void setTxDiscount(bool flag)		{ _tx_disc = flag; }
    void setAccountId(Id account_id)		{ _account_id = account_id; }
    void setMethod(Method method)		{ _method = method; }
    void setAmount(fixed amount)		{ _amount = amount; }

    // Special methods
    QString methodName() const;
    QString typeName() const;
    static QString methodName(Method method);

    // Operations
    bool operator==(const Discount& rhs) const;
    bool operator!=(const Discount& rhs) const;

private:
    QString _name;		// Name of discount
    bool _line_disc;		// Can be used for line discounts?
    bool _tx_disc;		// Can be used for transaction discounts?
    Id _account_id;		// Account to post discount to (can be blank)
    Method _method;		// Method of calculating discount
    fixed _amount;		// Percent of dollar amount depending on method
};

#endif // DISCOUNT_H
