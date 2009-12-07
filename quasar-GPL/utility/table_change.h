// $Id: table_change.h,v 1.3 2004/02/03 00:13:26 arandell Exp $
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

#ifndef TABLE_CHANGE_H
#define TABLE_CHANGE_H

// The table change class is to represent changes to a SQL table at
// a higher level than straight SQL commands.  It can be used as a
// point for tracking changes for replication or auditing.  It has
// a list of column data for the ids involved in updates and deletes
// and a list of column data for the actual changes for inserts and
// updates.

#include "variant.h"
#include <qvaluelist.h>
#include <qpair.h>

// These are the types of changes that can happen to a table
enum { CHANGE_INSERT, CHANGE_UPDATE, CHANGE_DELETE };

// This just holds a pair matching a column name and some data
typedef QPair<QString, Variant> ColumnData;

class TableChange {
public:
    TableChange(const QString& table="", int type=0);

    // Change definition
    QString table;				// Table to change
    int type;					// Type of change
    QValueList<ColumnData> ids;			// Selection criteria
    QValueList<ColumnData> columns;		// Column changes

    // Add columns, constraints, and indexes to the table
    void addId(const QString& name, Variant value);
    void addData(const QString& name, Variant value);

    // Comparisons
    bool operator==(const TableChange& rhs) const;
    bool operator!=(const TableChange& rhs) const;
};

#endif // TABLE_CHANGE_H
