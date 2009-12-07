// $Id: constraint_defn.h,v 1.5 2004/12/11 01:30:32 bpepers Exp $
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

#ifndef CONSTRAINT_DEFN_H
#define CONSTRAINT_DEFN_H

// The constraint definition class is for defining a SQL constraint at
// a higher level than straight SQL code.  This allows the individual
// drivers to handle the constraint creation in their own way.

#include <qdom.h>
#include <qstringlist.h>

class ConstraintDefn {
public:
    ConstraintDefn();
    ConstraintDefn(QDomElement e);

    // Constraint types
    enum { TYPE_UNIQUE, TYPE_CHECK, TYPE_FOREIGN_KEY };

    // Foreign key reference flags
    enum { DELETE_CASCADE=0x001, DELETE_SET_NULL=0x002 };

    // Information
    QString name;			// Name of constraint
    QString description;		// Description of constraint
    QString table;			// Table name
    int type;				// Type of constraint
    QStringList columns;		// First set of columns
    QString check;			// Text of check constraint
    QString referenceTable;		// Foreign key table name
    QStringList referenceColumns;	// Foreign key columns
    int referenceFlags;			// Foreign key flags

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
    bool operator==(const ConstraintDefn& rhs) const;
    bool operator!=(const ConstraintDefn& rhs) const;
};

typedef QValueList<ConstraintDefn> ConstraintDefnList;

#endif // CONSTRAINT_DEFN_H
