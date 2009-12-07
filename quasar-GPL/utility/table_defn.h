// $Id: table_defn.h,v 1.6 2004/11/27 10:07:43 bpepers Exp $
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

#ifndef TABLE_DEFN_H
#define TABLE_DEFN_H

// The table definition class is for defining a SQL table at a higher
// level than straight SQL code.  This allows the individual drivers
// to handle the table creation in their own way mapping column types
// into what the database supports.

#include "column_defn.h"
#include "constraint_defn.h"
#include "index_defn.h"

class TableDefn {
public:
    TableDefn();
    TableDefn(QDomElement e);

    // Information
    QString name;			// Name of table
    QString description;		// Description of table
    ColumnDefnList columns;		// Columns
    QStringList primaryKey;		// Primary key columns
    ConstraintDefnList constraints;	// Constraints
    IndexDefnList indexes;		// Indexes

    // Return list of tables this one is dependant on
    QStringList dependencies() const;

    // Add columns
    void addStringColumn(const QString& name, int length,
			 bool manditory=true, bool unique=false);
    void addDateColumn(const QString& name, bool manditory=true);
    void addTimeColumn(const QString& name, bool manditory=true);
    void addBooleanColumn(const QString& name);
    void addIntegerColumn(const QString& name);
    void addBigIntColumn(const QString& name, bool manditory=true);
    void addNumericColumn(const QString& name);
    void addMoneyColumn(const QString& name);
    void addPercentColumn(const QString& name);
    void addQuantityColumn(const QString& name);
    void addIdColumn(const QString& name, const QString& table,
		     bool manditory=true);

    // Add constraints and indexes
    void addPrimaryKey(const QString& columns);
    void addUnique(const QString& columns);
    void addCheck(const QString& check);
    void addReference(const QString& columns, const QString& referenceTable,
		      const QString& referenceColumns, int flags=0);
    void addReference(const QString& columns, const QString& referenceTable,
		      int flags=0);
    void addIndex(const QString& columns, bool unique=false);

    // Convert from/to XML element
    void fromXML(QDomElement e);
    void toXML(QDomElement& e) const;

    // Clear all data
    void clear();

    // Comparisons
    bool operator==(const TableDefn& rhs) const;
    bool operator!=(const TableDefn& rhs) const;
};

typedef QValueList<TableDefn> TableDefnList;

#endif // TABLE_DEFN_H
