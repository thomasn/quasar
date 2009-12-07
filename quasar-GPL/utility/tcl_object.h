// $Id: tcl_object.h,v 1.1 2005/02/28 10:31:49 bpepers Exp $
//
// Copyright (C) 1998-2004 Linux Canada Inc.  All rights reserved.
//
// This file is part of Quasar Accounting
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

#ifndef TCL_OBJ_H
#define TCL_OBJ_H

#include <tcl.h>
#include <qstringlist.h>

class TclObject {
public:
    TclObject();
    TclObject(const QString& value);
    TclObject(double value);
    TclObject(long value);
    TclObject(const TclObject& object);
    TclObject(Tcl_Obj* obj);
    ~TclObject();

    // Assignment
    TclObject& operator=(const TclObject& object);

    // List methods
    void lappend(const TclObject& object);
    TclObject lindex(int index);
    TclObject operator[](int index);
    int llength();

    // Conversions
    QString toString();
    double toDouble();
    long toLong();
    QStringList toStringList();

protected:
    Tcl_Obj* _object;
};

#endif
