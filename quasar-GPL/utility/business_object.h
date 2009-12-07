// $Id: business_object.h,v 1.2 2004/11/29 07:09:38 bpepers Exp $
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

#ifndef BUSINESS_OBJECT_H
#define BUSINESS_OBJECT_H

#include "table_defn.h"

class BusinessObject {
public:
    BusinessObject();
    BusinessObject(QDomElement e);

    // Information
    QString name;			// Name of object
    QString description;		// Description of business object
    int number;				// Unique business object number
    TableDefnList tables;		// Tables

    // Convert from/to XML element
    void fromXML(QDomElement e);
    void toXML(QDomElement& e) const;

    // Clear all data
    void clear();

    // Comparisons
    bool operator==(const BusinessObject& rhs) const;
    bool operator!=(const BusinessObject& rhs) const;
};

typedef QValueList<BusinessObject> BusinessObjectList;

#endif // BUSINESS_OBJECT_H
