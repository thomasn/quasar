// $Id: conn.cpp,v 1.1 2005/03/01 19:45:35 bpepers Exp $
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

#include "conn.h"

Conn::Conn(Driver* driver)
{
    _conn = driver->allocConnection();
    _conn->incrRefCount();
}

Conn::Conn(const Conn& rhs)
{
    _conn = rhs._conn;
    _conn->incrRefCount();
}

Conn::~Conn()
{
    _conn->decrRefCount();
    if (_conn->refCount() <= 0)
	delete _conn;
}

Conn&
Conn::operator=(const Conn& rhs)
{
    rhs._conn->incrRefCount();
    _conn->decrRefCount();
    if (_conn->refCount() <= 0)
	delete _conn;
    _conn = rhs._conn;
    return *this;
}

bool
Conn::setAutoCommit(bool autoCommit)
{
    return _conn->setAutoCommit(autoCommit);
}

bool
Conn::getAutoCommit() const
{
    return _conn->getAutoCommit();
}

bool
Conn::setIsolationLevel(IsoLevel level)
{
    return _conn->setIsolationLevel(level);
}

IsoLevel
Conn::getIsolationLevel() const
{
    return _conn->getIsolationLevel();
}

bool
Conn::connect(const QString& database)
{
    return _conn->connect(database);
}

bool
Conn::dbaConnect(const QString& database)
{
    return _conn->dbaConnect(database);
}

void
Conn::disconnect()
{
    _conn->disconnect();
}

bool
Conn::connected() const
{
    return _conn->connected();
}

bool
Conn::commit()
{
    return _conn->commit();
}

bool
Conn::rollback()
{
    return _conn->rollback();
}

bool
Conn::create(const TableDefn& table)
{
    return _conn->create(table);
}

bool
Conn::create(const ConstraintDefn& constraint)
{
    return _conn->create(constraint);
}

bool
Conn::create(const IndexDefn& index)
{
    return _conn->create(index);
}

bool
Conn::create(const QValueList<TableDefn>& tables)
{
    return _conn->create(tables);
}

QString
Conn::lastError() const
{
    return _conn->lastError();
}
