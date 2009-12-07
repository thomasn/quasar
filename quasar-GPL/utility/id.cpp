// $Id: id.cpp,v 1.8 2005/04/12 07:38:03 bpepers Exp $
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

#include "id.h"
#include <assert.h>

static fixed defaultSystemId = 0;
#ifdef __GNUC__
static int64_t systemStart = 1000000000000LL;
#else
static int64_t systemStart = 1000000000000;
#endif

Id::Id()
{
    setNull();
}

Id::~Id()
{
}

fixed
Id::systemId() const
{
    if (isNull()) return -1;
    return _id / systemStart;
}

fixed
Id::objectId() const
{
    if (isNull()) return -1;
    return _id % systemStart;
}

bool
Id::isNull() const
{
    return _id == -1;
}

void
Id::setNull()
{
    _id = -1;
}

QString
Id::toString() const
{
    fixed value = _id;
    return value.toString();
}

bool
Id::fromString(const QString& text)
{
    bool ok;
    double value = text.toDouble(&ok);
    if (!ok) return false;
    _id = int64_t(value);
    return true;
}

Id
Id::fromStringStatic(const QString& text)
{
    Id id;
    if (!id.fromString(text))
	qWarning("Invalid id: " + text);
    return id;
}

void
Id::setSystemId(fixed id)
{
    assert(id < 1000);
    if (isNull())
	_id = (id * systemStart).toInt64();
    else
	_id = (id * systemStart + objectId()).toInt64();
}

void
Id::setObjectId(fixed id)
{
    if (isNull())
	_id = (defaultSystemId * systemStart + id).toInt64();
    else
	_id = (systemId() * systemStart + id).toInt64();
}

bool
Id::operator==(const Id& rhs) const
{
    if (_id != rhs._id) return false;
    return true;
}

bool
Id::operator!=(const Id& rhs) const
{
    return !(*this == rhs);
}

bool
Id::operator<(const Id& rhs) const
{
    return _id < rhs._id;
}

bool
Id::operator<=(const Id& rhs) const
{
    return _id <= rhs._id;
}

bool
Id::operator>(const Id& rhs) const
{
    return _id > rhs._id;
}

bool
Id::operator>=(const Id& rhs) const
{
    return _id >= rhs._id;
}

QString
Id::sqlDataType()
{
    return "numeric(18,0)";
}

void
Id::setDefaultSystemId(fixed id)
{
    defaultSystemId = id;
}
