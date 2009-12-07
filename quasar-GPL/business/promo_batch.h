// $Id: promo_batch.h,v 1.3 2005/01/30 00:51:13 bpepers Exp $
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

#ifndef PROMO_BATCH_H
#define PROMO_BATCH_H

#include "data_object.h"
#include "price.h"

struct PromoBatchItem {
    PromoBatchItem();

    Id item_id;			// Item id of item
    QString number;		// Number of item
    QString size;		// Size of item
    Price price;		// Promotion price
    fixed ordered;		// Quantity ordered for promo
    Id price_id;		// Item price id

    bool operator==(const PromoBatchItem& rhs) const;
    bool operator!=(const PromoBatchItem& rhs) const;
};

class PromoBatch: public DataObject {
public:
    // Constructors and Destructor
    PromoBatch();
    ~PromoBatch();

    // Get methods
    QString number()			const { return _number; }
    QString description()		const { return _description; }
    Id storeId()			const { return _store_id; }
    QDate fromDate()			const { return _from_date; }
    QDate toDate()			const { return _to_date; }
    QDate executedOn()			const { return _exec_date; }

    // Set methods
    void setNumber(const QString& number)	{ _number = number; }
    void setDescription(const QString& desc)	{ _description = desc; }
    void setStoreId(Id store_id)		{ _store_id = store_id; }
    void setFromDate(QDate date)		{ _from_date = date; }
    void setToDate(QDate date)			{ _to_date = date; }
    void setExecutedOn(QDate date)		{ _exec_date = date; }

    // Item lines
    vector<PromoBatchItem>& items()		{ return _items; }
    const vector<PromoBatchItem>& items() const	{ return _items; }

    // Operations
    bool operator==(const PromoBatch& rhs) const;
    bool operator!=(const PromoBatch& rhs) const;

protected:
    QString _number;			// Batch number
    QString _description;		// Description
    Id _store_id;			// Store for promo change
    QDate _from_date;			// Start date
    QDate _to_date;			// End date
    QDate _exec_date;			// Date executed on
    vector<PromoBatchItem> _items;	// Items in batch
};

#endif // PROMO_BATCH_H
