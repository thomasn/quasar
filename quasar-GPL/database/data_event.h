// $Id: data_event.h,v 1.5 2004/01/30 23:16:36 arandell Exp $
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

#ifndef DATA_EVENT_H
#define DATA_EVENT_H

#include "data_object.h"

class DataEvent {
public:
    enum Type { Insert, Delete, Update };

    DataEvent(Type type, DataObject::DataType dataType, Id id=INVALID_ID)
	: _type(type), _dataType(dataType), _id(id) {}
    virtual ~DataEvent() {}

    Type type() const				{ return _type; }
    DataObject::DataType dataType() const	{ return _dataType; }
    Id id() const				{ return _id; }

    void setType(Type type)			{ _type = type; }
    void setDataType(DataObject::DataType type)	{ _dataType = type; }
    void setId(Id id)				{ _id = id; }

protected:
    Type _type;
    DataObject::DataType _dataType;
    Id _id;
};

#endif // DATA_EVENT_H
