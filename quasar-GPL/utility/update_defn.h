// $Id: update_defn.h,v 1.1 2004/11/27 10:07:43 bpepers Exp $
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

#ifndef UPDATE_DEFN_H
#define UPDATE_DEFN_H

// The update definition class is for defining the commands needed to
// upgrade from one version to another.  They can be for a specific
// database or they can be for more than one or for all types.

#include <qdom.h>
#include <qstringlist.h>

class UpdateDefn {
public:
    UpdateDefn();
    UpdateDefn(QDomElement e);

    // Information
    int number;				// Used for ordering the updates
    QString description;		// What does this update do?
    QStringList databases;		// Databases this applies to
    QStringList updateCmds;		// Commands to perform update
    QStringList restoreCmds;		// Commands to restore if update fails
    QStringList cleanupCmds;		// Commands to cleanup when complete

    // Convert from/to XML element
    void fromXML(QDomElement e);
    void toXML(QDomElement& e) const;

    // Clear all data
    void clear();

    // Comparisons
    bool operator==(const UpdateDefn& rhs) const;
    bool operator!=(const UpdateDefn& rhs) const;
};

typedef QValueList<UpdateDefn> UpdateDefnList;

#endif // UPDATE_DEFN_H
