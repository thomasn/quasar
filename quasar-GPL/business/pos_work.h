// $Id: pos_work.h,v 1.2 2004/08/03 23:03:33 bpepers Exp $
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

#ifndef POS_WORK_H
#define POS_WORK_H

#include "id.h"

class PosWork {
public:
    // Constructors and Destructor
    PosWork();
    ~PosWork();

    // Get methods
    QString type() const			{ return _type; }
    QString id() const				{ return _id; }
    QString status() const			{ return _status; }
    Id storeId() const				{ return _store_id; }
    Id stationId() const			{ return _station_id; }
    Id employeeId() const			{ return _employee_id; }

    // Set methods
    void setType(const QString& type)		{ _type = type; }
    void setId(const QString& id)		{ _id = id; }
    void setStatus(const QString& status)	{ _status = status; }
    void setStoreId(Id store_id)		{ _store_id = store_id; }
    void setStationId(Id station_id)		{ _station_id = station_id; }
    void setEmployeeId(Id employee_id)		{ _employee_id = employee_id; }

    // Operations
    bool operator==(const PosWork& rhs) const;
    bool operator!=(const PosWork& rhs) const;

    // Return name to display
    QString name() const;

protected:
    QString _type;
    QString _id;
    QString _status;
    Id _store_id;
    Id _station_id;
    Id _employee_id;
};

#endif // POS_WORK_H
