// $Id: count.h,v 1.9 2005/01/30 00:51:13 bpepers Exp $
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

#ifndef COUNT_H
#define COUNT_H

#include "data_object.h"

struct CountItem {
    CountItem();

    Id item_id;			// Item id of item
    QString number;		// Number of item
    QString size;		// Size of item
    fixed size_qty;		// Quantity of size
    fixed quantity;		// Quantity counted
    fixed on_hand;		// On hand
    bool oh_set;		// On hand is set?

    bool operator==(const CountItem& rhs) const;
    bool operator!=(const CountItem& rhs) const;
};

class Count: public DataObject {
public:
    // Constructors and Destructor
    Count();
    virtual ~Count();

    // Get methods
    QString number()			const { return _number; }
    QString description()		const { return _description; }
    QDate date()			const { return _date; }
    Id storeId()			const { return _store_id; }
    Id locationId()			const { return _location_id; }
    Id employeeId()			const { return _employee_id; }

    // Set methods
    void setNumber(const QString& number)	{ _number = number; }
    void setDescription(const QString& desc)	{ _description = desc; }
    void setDate(QDate date)			{ _date = date; }
    void setStoreId(Id id)			{ _store_id = id; }
    void setLocationId(Id id)			{ _location_id = id; }
    void setEmployeeId(Id id)			{ _employee_id = id; }

    // Item lines
    vector<CountItem>& items()			{ return _items; }
    const vector<CountItem>& items() const	{ return _items; }

    // Operations
    bool operator==(const Count& rhs) const;
    bool operator!=(const Count& rhs) const;

protected:
    QString _number;			// Count number
    QString _description;		// User entered description
    QDate _date;			// Date of count
    Id _store_id;			// Store
    Id _location_id;			// Where counted
    Id _employee_id;			// Who counted
    vector<CountItem> _items;		// Items in count
};

#endif // COUNT_H
