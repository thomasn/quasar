// $Id: vendor.h,v 1.9 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef VENDOR_H
#define VENDOR_H

#include "card.h"

class Vendor: public Card {
public:
    // Constructors and Destructor
    Vendor();
    ~Vendor();

    // Get methods
    Id accountId() const			{ return _account_id; }
    Id termsId() const				{ return _term_id; }
    bool backorders() const			{ return _backorders; }

    // Set methods
    void setAccountId(Id account_id)		{ _account_id = account_id; }
    void setTermsId(Id term_id)			{ _term_id = term_id; }
    void setBackorders(bool flag)		{ _backorders = flag; }

    // Operations
    bool operator==(const Vendor& rhs) const;
    bool operator!=(const Vendor& rhs) const;

protected:
    Id _account_id;		// AP account for purchases
    Id _term_id;		// Terms (Net 30, ...)
    bool _backorders;		// Accept backorders?
};

#endif // VENDOR_H
