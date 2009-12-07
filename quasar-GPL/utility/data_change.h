// $Id: data_change.h,v 1.5 2005/01/31 23:22:34 bpepers Exp $
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
//

#ifndef DATA_CHANGE_H
#define DATA_CHANGE_H

// The data change class is to represent changes to a data object at
// a higher level than straight SQL commands.  It can be used as a
// point for tracking changes for replication or auditing.  It has
// information on who, what, and when the change involves and a list
// of TableChange's to track the actual SQL level changes.

#include "table_change.h"
#include <qstringlist.h>

class DataChange {
public:
    DataChange();
    DataChange(int dataType, Id id, int changeType, const QString& username);

    // Change definition
    int dataType;				// From DataObject
    Id objectId;				// Object being changed
    int changeType;				// Type of change over-all
    QDate date;					// Date change is done
    QTime time;					// Time change is done
    QString userName;				// User making the change
    QValueList<TableChange> changes;		// Table changes for data

    // Add a table change
    void add(const TableChange& change);

    // Comparisons
    bool operator==(const DataChange& rhs) const;
    bool operator!=(const DataChange& rhs) const;
};

#endif // DATA_CHANGE_H
