// $Id: extra.h,v 1.5 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef EXTRA_H
#define EXTRA_H

#include "data_object.h"

class Extra: public DataObject {
public:
    // Constructors and Destructor
    Extra();
    ~Extra();

    // Get methods
    const QString& table()		const { return _table; }
    const QString& name()		const { return _name; }

    // Set methods
    void setTable(const QString& table)		{ _table = table; }
    void setName(const QString& name)		{ _name = name; }

    // Operations
    bool operator==(const Extra& rhs) const;
    bool operator!=(const Extra& rhs) const;

private:
    QString _table;		// Table data is for
    QString _name;		// Name of extra data
};

#endif // EXTRA_H
