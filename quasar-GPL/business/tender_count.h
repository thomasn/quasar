// $Id: tender_count.h,v 1.7 2005/01/30 00:51:13 bpepers Exp $
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

#ifndef TENDER_COUNT_H
#define TENDER_COUNT_H

#include "data_object.h"

struct TenderInfo {
    TenderInfo();

    Id tender_id;
    fixed amount;

    bool operator==(const TenderInfo& rhs) const;
    bool operator!=(const TenderInfo& rhs) const;
};

class TenderCount: public DataObject {
public:
    // Constructors and Destructor
    TenderCount();
    ~TenderCount();

    // Get methods
    QString number() const			{ return _number; }
    QDate date() const				{ return _date; }
    Id stationId() const			{ return _station_id; }
    Id employeeId() const			{ return _employee_id; }
    Id storeId() const				{ return _store_id; }
    Id shiftId() const				{ return _shift_id; }

    // Set methods
    void setNumber(const QString& number)	{ _number = number; }
    void setDate(QDate date)			{ _date = date; }
    void setStationId(Id station_id)		{ _station_id = station_id; }
    void setEmployeeId(Id employee_id)		{ _employee_id = employee_id; }
    void setStoreId(Id store_id)		{ _store_id = store_id; }
    void setShiftId(Id shift_id)		{ _shift_id = shift_id; }

    // Tenders
    vector<TenderInfo>& tenders()		{ return _tenders; }
    const vector<TenderInfo>& tenders() const	{ return _tenders; }

    // Operations
    bool operator==(const TenderCount& rhs) const;
    bool operator!=(const TenderCount& rhs) const;

private:
    QString _number;			// Count number
    QDate _date;			// Date of count
    Id _station_id;
    Id _employee_id;
    Id _store_id;
    Id _shift_id;			// Set when closed
    vector<TenderInfo> _tenders;
};

#endif // TENDER_COUNT_H
