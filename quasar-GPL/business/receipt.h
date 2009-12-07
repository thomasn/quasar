// $Id: receipt.h,v 1.12 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef RECEIPT_H
#define RECEIPT_H

#include "gltx.h"

class Receipt: public Gltx {
public:
    // Constructors and Destructor
    Receipt();
    virtual ~Receipt();

    // Get methods
    Id discountId()			const { return _disc_id; }

    // Set methods
    void setDiscountId(Id id)			{ _disc_id = id; }

    // Special methods
    fixed total() const;

    // Operations
    bool operator==(const Receipt& rhs) const;
    bool operator!=(const Receipt& rhs) const;

protected:
    Id _disc_id;	// GL account for discount amounts
};

#endif // RECEIPT_H
