// $Id: data_object.h,v 1.46 2005/01/30 00:51:13 bpepers Exp $
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

#ifndef DATA_OBJECT_H
#define DATA_OBJECT_H

#include "quasar_include.h"
#include "variant.h"

// Special types
typedef QPair<Id, QString> DataPair;

class DataObject {
public:
    // Constructors and Destructor
    DataObject();
    virtual ~DataObject();

    // Data types
    enum DataType { ACCOUNT, ITEM_ADJUST, CHEQUE, CUSTOMER, DEPT, EMPLOYEE,
		    GENERAL, INVOICE, ITEM, ORDER, PERSONAL, RECEIPT,
		    RECEIVE, REPORT, SUBDEPT, TAX, TENDER, TODO, VENDOR,
		    EXPENSE, NOSALE, PAYOUT, RETURN, SHIFT, TEND_ADJUST,
		    RECONCILE, ITEM_PRICE, TERM, PLU, STORE, DISCOUNT,
		    SLIP, CHARGE, COUNT, LOCATION, QUOTE, EXTRA, STATION,
		    CLAIM, RECURRING, GROUP, CARD_ADJUST, WITHDRAW,
		    CUST_TYPE, TEND_COUNT, SECURITY_TYPE, PO_TEMPLATE,
		    PAT_GROUP, PAT_WS, USER, COMPANY, LABEL_BATCH,
		    PRICE_BATCH, PROMO_BATCH, ADJUST_REASON };

    // Shared by all data objects
    Id id()				const { return _id; }
    int version()			const { return _version; }
    DataType dataType()			const { return _data_type; }
    QString createdBy()			const { return _created_by; }
    QString updatedBy()			const { return _updated_by; }
    QString inactiveBy()		const { return _inactive_by; }
    QDate createdOn()			const { return _created_on; }
    QDate updatedOn()			const { return _updated_on; }
    QDate inactiveOn()			const { return _inactive_on; }
    fixed externalId()			const { return _external_id; }

    // Set shared data
    void setId(Id id)				{ _id = id; }
    void setVersion(int v)			{ _version = v; }
    void setDataType(DataType type)		{ _data_type = type; }
    void setCreatedBy(const QString& name)	{ _created_by = name; }
    void setUpdatedBy(const QString& name)	{ _updated_by = name; }
    void setInactiveBy(const QString& name)	{ _inactive_by = name; }
    void setCreatedOn(QDate date)		{ _created_on = date; }
    void setUpdatedOn(QDate date)		{ _updated_on = date; }
    void setInactiveOn(QDate date)		{ _inactive_on = date; }
    void setExternalId(fixed id)		{ _external_id = id; }

    // Extra data that can be linked to a data object
    const vector<DataPair>& extra() const	{ return _extra; }
    vector<DataPair>& extra()			{ return _extra; }
    QString value(Id extra_id) const;
    void setValue(Id extra_id, const QString& value);

    // Special methods for active and type
    bool isActive() const;
    bool isInactive() const;
    QString dataTypeName() const;
    static QString dataTypeName(int type);
    static int dataType(const QString& name);

    // Get the last error when any method returns false because it failed
    QString lastError() const { return _last_error; }

    // Operations
    bool operator==(const DataObject& rhs) const;
    bool operator!=(const DataObject& rhs) const;

protected:
    // Error handling
    bool error(const QString& message);

    Id _id;			// Unique identifier (assigned by database)
    int _version;		// Number updated everytime record changed
    DataType _data_type;	// Type of data
    QString _created_by;	// User who created
    QString _updated_by;	// User who last updated
    QString _inactive_by;	// User who set inactive (voided/done)
    QDate _created_on;		// Date created
    QDate _updated_on;		// Date last updated
    QDate _inactive_on;		// Date set inactive
    fixed _external_id;		// Unique identifier (assigned by user)
    vector<DataPair> _extra;	// Extra data
    QString _last_error;	// Last error message
};

#endif // DATA_OBJECT_H
