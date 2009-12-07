// $Id: id_uuid.h,v 1.1 2004/05/06 04:21:44 bpepers Exp $
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

#ifndef ID_H
#define ID_H

#include <qstring.h>

class Id {
public:
    // Constructors and Destructor
    Id();
    ~Id();

    // Null check/set
    bool isNull() const;
    void setNull();

    // Set to a new unique value
    void newValue();

    // Convert to/from string
    QString toString() const;
    bool fromString(const QString& text);
    static Id fromStringStatic(const QString& text);

    // SQL data type to use
    static QString sqlDataType();

    // Operations
    bool operator==(const Id& rhs) const;
    bool operator!=(const Id& rhs) const;
    bool operator<(const Id& rhs) const;
    bool operator<=(const Id& rhs) const;
    bool operator>(const Id& rhs) const;
    bool operator>=(const Id& rhs) const;

protected:
    unsigned char _id[16];
};

// Invalid identifier
const Id INVALID_ID;

#endif // ID_H
