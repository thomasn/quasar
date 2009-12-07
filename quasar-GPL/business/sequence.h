// $Id: sequence.h,v 1.3 2004/01/30 23:06:57 arandell Exp $
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

#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "fixed.h"

#define DEFAULT_MIN 1.0
#define DEFAULT_MAX 10000000000.0

class Sequence {
public:
    // Constructors and Destructor
    Sequence();
    ~Sequence();

    // Get methods
    QString tableName() const			{ return _table_name; }
    QString attrName() const			{ return _attr_name; }
    int systemId() const			{ return _system_id; }
    fixed minNumber() const			{ return _min_num; }
    fixed maxNumber() const			{ return _max_num; }
    fixed nextNumber() const			{ return _next_num; }

    // Set methods
    void setTableName(const QString& name)	{ _table_name = name; }
    void setAttrName(const QString& name)	{ _attr_name = name; }
    void setSystemId(int system_id)		{ _system_id = system_id; }
    void setMinNumber(fixed num)		{ _min_num = num; }
    void setMaxNumber(fixed num)		{ _max_num = num; }
    void setNextNumber(fixed num)		{ _next_num = num; }

    // Operations
    bool operator==(const Sequence& rhs) const;
    bool operator!=(const Sequence& rhs) const;

protected:
    QString _table_name;
    QString _attr_name;
    int _system_id;
    fixed _min_num;
    fixed _max_num;
    fixed _next_num;
};

#endif // SEQUENCE_H
