// $Id: price_batch.h,v 1.4 2005/01/30 00:51:13 bpepers Exp $
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

#ifndef PRICE_BATCH_H
#define PRICE_BATCH_H

#include "data_object.h"
#include "price.h"

struct PriceBatchItem {
    PriceBatchItem();

    Id item_id;			// Item id of item
    QString number;		// Number of item
    QString size;		// Size of item
    Price new_price;		// New price for item
    Price old_cost;		// Old cost (set when executed)
    Price old_price;		// Old price (set when executed)

    bool operator==(const PriceBatchItem& rhs) const;
    bool operator!=(const PriceBatchItem& rhs) const;
};

class PriceBatch: public DataObject {
public:
    // Constructors and Destructor
    PriceBatch();
    ~PriceBatch();

    // Get methods
    QString number()			const { return _number; }
    QString description()		const { return _description; }
    Id storeId()			const { return _store_id; }
    QDate executedOn()			const { return _exec_date; }

    // Set methods
    void setNumber(const QString& number)	{ _number = number; }
    void setDescription(const QString& desc)	{ _description = desc; }
    void setStoreId(Id store_id)		{ _store_id = store_id; }
    void setExecutedOn(QDate date)		{ _exec_date = date; }

    // Item lines
    vector<PriceBatchItem>& items()		{ return _items; }
    const vector<PriceBatchItem>& items() const	{ return _items; }

    // Operations
    bool operator==(const PriceBatch& rhs) const;
    bool operator!=(const PriceBatch& rhs) const;

protected:
    QString _number;			// Batch number
    QString _description;		// Description
    Id _store_id;			// Store for price change
    QDate _exec_date;			// Date executed on
    vector<PriceBatchItem> _items;	// Items in batch
};

#endif // PRICE_BATCH_H
