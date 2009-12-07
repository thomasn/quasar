// $Id: group.h,v 1.7 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef GROUP_H
#define GROUP_H

#include "data_object.h"

class Group: public DataObject {
public:
    // Constructors and Destructor
    Group();
    ~Group();

    // Group types
    enum { ACCOUNT, CUSTOMER, VENDOR, ITEM, EMPLOYEE, PERSONAL };

    // Get methods
    const QString& name()		const { return _name; }
    const QString& description()	const { return _desc; }
    int type()				const { return _type; }

    // Set methods
    void setName(const QString& name)		{ _name = name; }
    void setDescription(const QString& desc)	{ _desc = desc; }
    void setType(int type)			{ _type = type; }

    // Special methods
    QString typeName() const;
    static QString typeName(int type);
    static int type(const QString& name);

    // Operations
    bool operator==(const Group& rhs) const;
    bool operator!=(const Group& rhs) const;

private:
    QString _name;		// Name of group (for lookups)
    QString _desc;		// Description of group (longer than name)
    int _type;			// Type of group
};

#endif // GROUP_H
