// $Id: term.h,v 1.6 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef TERM_H
#define TERM_H

#include "data_object.h"

class Term: public DataObject {
public:
    // Constructors and Destructor
    Term();
    ~Term();

    // Get methods
    bool isCOD()			const { return _cod; }
    int discountDays()			const { return _disc_days; }
    int dueDays()			const { return _due_days; }
    fixed discount()			const { return _discount; }

    // Set methods
    void setCOD(bool flag)			{ _cod = flag; }
    void setDiscountDays(int days)		{ _disc_days = days; }
    void setDueDays(int days)			{ _due_days = days; }
    void setDiscount(fixed discount)		{ _discount = discount; }

    // Get name
    QString name() const;

    // Operations
    bool operator==(const Term& rhs) const;
    bool operator!=(const Term& rhs) const;

protected:
    bool _cod;			// Cash On Delivery? (implies no terms)
    int _disc_days;		// Discount days
    int _due_days;		// Due days
    fixed _discount;		// Percount percentage
};

#endif // TERM_H
