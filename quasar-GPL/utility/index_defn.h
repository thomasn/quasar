// $Id: index_defn.h,v 1.3 2004/11/27 10:07:43 bpepers Exp $
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

#ifndef INDEX_DEFN_H
#define INDEX_DEFN_H

// The index definition class is for defining a SQL index at a higher
// level than straight SQL code.  This allows the individual drivers
// to handle the index creation in their own way.

#include <qdom.h>
#include <qvaluelist.h>

struct IndexColumn {
    IndexColumn(const QString& name="", bool ascending=true);

    QString name;
    bool ascending;

    // Comparisons
    bool operator==(const IndexColumn& rhs) const;
    bool operator!=(const IndexColumn& rhs) const;
};

class IndexDefn {
public:
    IndexDefn();
    IndexDefn(QDomElement e);

    // Information
    QString name;			// Name of index
    QString description;		// Description of index
    QString table;			// Table name
    QValueList<IndexColumn> columns;	// Columns
    bool unique;			// Unique index?

    // Add a column to the index
    void addColumn(const QString& name, bool ascending=true);

    // Convert from/to XML element
    void fromXML(QDomElement e);
    void toXML(QDomElement& e) const;

    // Clear all data
    void clear();

    // Comparisons
    bool operator==(const IndexDefn& rhs) const;
    bool operator!=(const IndexDefn& rhs) const;
};

typedef QValueList<IndexDefn> IndexDefnList;

#endif // INDEX_DEFN_H
