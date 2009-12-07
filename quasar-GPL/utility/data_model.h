// $Id: data_model.h,v 1.1 2004/11/27 10:07:43 bpepers Exp $
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

#ifndef DATA_MODEL_H
#define DATA_MODEL_H

// The data model class is a version of the database that some version
// or versions of Quasar can use.  More than one version of Quasar can
// use the same data model (for instance when there is a new version of
// Quasar released that fixes some programming bugs but doesn't change
// the database in any way) and every change to the database requires a
// new data model.  There is also an UpdateDefn class that has the info
// on how to convert one data model to another.  The versions used in
// Quasar for the database is simply a date in YYYY-MM-DD format.

#include "business_object.h"
#include "update_defn.h"

class DataModel {
public:
    DataModel();

    // Information
    QString version;			// Version of this model
    QString description;		// Description of changes
    QString fromVersion;		// Version before this one
    BusinessObjectList objects;		// Business objects
    UpdateDefnList updates;		// Update from old version to new

    // Return all table definitions
    TableDefnList tables() const;

    // Load and save in XML format
    bool load(const QString& filePath);
    bool save(const QString& filePath) const;

    // Clear all data
    void clear();

    // Comparisons
    bool operator==(const DataModel& rhs) const;
    bool operator!=(const DataModel& rhs) const;
};

#endif // DATA_MODEL_H
