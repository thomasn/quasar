// $Id: tcl_object.cpp,v 1.1 2005/02/28 10:31:49 bpepers Exp $
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

#include "tcl_object.h"

TclObject::TclObject()
{
    _object = Tcl_NewObj();
    Tcl_IncrRefCount(_object);
}

TclObject::TclObject(const QString& value)
{
    _object = Tcl_NewStringObj(value.utf8(), -1);
    Tcl_IncrRefCount(_object);
}

TclObject::TclObject(double value)
{
    _object = Tcl_NewDoubleObj(value);
    Tcl_IncrRefCount(_object);
}

TclObject::TclObject(long value)
{
    _object = Tcl_NewLongObj(value);
    Tcl_IncrRefCount(_object);
}

TclObject::TclObject(const TclObject& object)
{
    _object = object._object;
    Tcl_IncrRefCount(_object);
}

TclObject::TclObject(Tcl_Obj* obj)
{
    _object = obj;
    Tcl_IncrRefCount(_object);
}

TclObject::~TclObject()
{
    Tcl_DecrRefCount(_object);
}

TclObject&
TclObject::operator=(const TclObject& rhs)
{
    Tcl_IncrRefCount(rhs._object);
    Tcl_DecrRefCount(_object);
    _object = rhs._object;
    return *this;
}

void
TclObject::lappend(const TclObject& object)
{
    Tcl_ListObjAppendElement(NULL, _object, object._object);
}

TclObject
TclObject::lindex(int index)
{
    Tcl_Obj* value;
    Tcl_ListObjIndex(NULL, _object, index, &value);
    return TclObject(value);
}

TclObject
TclObject::operator[](int index)
{
    return lindex(index);
}

int
TclObject::llength()
{
    int length = 0;
    Tcl_ListObjLength(NULL, _object, &length);
    return length;
}

QString
TclObject::toString()
{
    return QString::fromUtf8(Tcl_GetString(_object));
}

QStringList
TclObject::toStringList()
{
    QStringList list;
    for (int i = 0; i < llength(); ++i)
	list.append(lindex(i).toString());
    return list;
}
