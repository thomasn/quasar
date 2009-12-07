// $Id: label_batch.h,v 1.5 2005/01/30 00:51:13 bpepers Exp $
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

#ifndef LABEL_BATCH_H
#define LABEL_BATCH_H

#include "data_object.h"
#include "price.h"

struct LabelBatchItem {
    LabelBatchItem();

    Id item_id;			// Item id of item
    QString number;		// Number of item
    QString size;		// Size of item
    Price price;		// Price of item
    int count;			// How many labels to print

    bool operator==(const LabelBatchItem& rhs) const;
    bool operator!=(const LabelBatchItem& rhs) const;
};

class LabelBatch: public DataObject {
public:
    // Constructors and Destructor
    LabelBatch();
    ~LabelBatch();

    // Get methods
    QString number()			const { return _number; }
    QString description()		const { return _description; }
    QString type()			const { return _type; }
    Id storeId()			const { return _store_id; }
    QDate printedOn()			const { return _print_date; }

    // Set methods
    void setNumber(const QString& number)	{ _number = number; }
    void setDescription(const QString& desc)	{ _description = desc; }
    void setType(const QString& type)		{ _type = type; }
    void setStoreId(Id store_id)		{ _store_id = store_id; }
    void setPrintedOn(QDate date)		{ _print_date = date; }

    // Item lines
    vector<LabelBatchItem>& items()		{ return _items; }
    const vector<LabelBatchItem>& items() const	{ return _items; }

    // Operations
    bool operator==(const LabelBatch& rhs) const;
    bool operator!=(const LabelBatch& rhs) const;

protected:
    QString _number;			// Batch number
    QString _description;		// Description
    QString _type;			// Type of labels
    Id _store_id;			// Store for labels
    QDate _print_date;			// Date printed on
    vector<LabelBatchItem> _items;	// Items in batch
};

#endif // LABEL_BATCH_H
