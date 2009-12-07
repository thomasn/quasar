// $Id: cheque.h,v 1.19 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef CHEQUE_H
#define CHEQUE_H

#include "gltx.h"

class Cheque: public Gltx {
public:
    // Constructors and Destructor
    Cheque();
    virtual ~Cheque();

    // Enums
    enum Type { ACCOUNT, CUSTOMER, VENDOR };

    // Get methods
    Type type()				const { return _type; }
    Id accountId()			const { return _account_id; }
    Id discountId()			const { return _disc_id; }
    QString address()			const { return _address; }

    // Set methods
    void setType(Type type)			{ _type = type; }
    void setAccountId(Id id)			{ _account_id = id; }
    void setDiscountId(Id id)			{ _disc_id = id; }
    void setAddress(const QString& address)	{ _address = address; }

    // Special methods
    QString typeName() const;
    static QString typeName(Type type);
    fixed total() const;

    // Operations
    bool operator==(const Cheque& rhs) const;
    bool operator!=(const Cheque& rhs) const;

protected:
    Type _type;		// Type of cheque (for allocations)
    Id _account_id;	// GL account for bank
    Id _disc_id;	// GL account for discount amounts
    QString _address;	// Stored address
};

#endif // CHEQUE_H
