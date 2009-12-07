// $Id: column_defn.h,v 1.7 2004/11/30 03:51:09 bpepers Exp $
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

#ifndef COLUMN_DEFN_H
#define COLUMN_DEFN_H

// The column definition class is for defining a SQL column at a higher
// level than straight SQL code.  This allows the individual drivers
// to handle the column creation in their own way mapping column types
// into what the database supports.

#include <qdom.h>
#include <qvaluelist.h>

class ColumnDefn {
public:
    ColumnDefn();
    ColumnDefn(QDomElement e);

    // Data types
    enum { TYPE_CHAR, TYPE_STRING, TYPE_DATE, TYPE_TIME, TYPE_BOOL,
	   TYPE_INT, TYPE_BIGINT, TYPE_NUMERIC, TYPE_ID, TYPE_MONEY,
	   TYPE_PERCENT, TYPE_QUANTITY };

    // Information
    QString name;			// Name of column
    QString description;		// Description of column
    int type;				// Data type
    int size;				// Size for certain data types
    int decimals;			// Decimals for numeric types
    bool manditory;			// Are nulls allowed?
    bool unique;			// Values must be unique?
    QString table;			// Linked table for id type

    // Convert from/to XML element
    void fromXML(QDomElement e);
    void toXML(QDomElement& e) const;

    // Clear all data
    void clear();

    // Type to/from string
    static QString typeName(int type);
    static int typeFromName(const QString& type);
    QString typeName() const;

    // Comparisons
    bool operator==(const ColumnDefn& rhs) const;
    bool operator!=(const ColumnDefn& rhs) const;
};

typedef QValueList<ColumnDefn> ColumnDefnList;

#endif // COLUMN_DEFN_H
