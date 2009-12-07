// $Id: dept.h,v 1.10 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef DEPT_H
#define DEPT_H

#include "data_object.h"

class Dept: public DataObject {
public:
    // Constructors and Destructor
    Dept();
    ~Dept();

    // Get methods
    const QString& name()		const { return _name; }
    const QString& number()		const { return _number; }
    fixed patPoints()			const { return _pat_points; }

    // Set methods
    void setName(const QString& name)		{ _name = name; }
    void setNumber(const QString& number)	{ _number = number; }
    void setPatPoints(fixed points)		{ _pat_points = points; }

    // Operations
    bool operator==(const Dept& rhs) const;
    bool operator!=(const Dept& rhs) const;

private:
    QString _name;		// Name of dept
    QString _number;		// Unique dept number (optional)
    fixed _pat_points;		// Patronage points per dollar sold
};

#endif // DEPT_H
