// $Id: stmt.cpp,v 1.1 2005/03/01 19:45:35 bpepers Exp $
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

#include "stmt.h"

Stmt::Stmt(Connection* conn)
{
    _stmt = conn->allocStatement();
    _stmt->incrRefCount();
}

Stmt::Stmt(Connection* conn, const QString& command)
{
    _stmt = conn->allocStatement(command);
    _stmt->incrRefCount();
}

Stmt::Stmt(const Stmt& rhs)
{
    _stmt = rhs._stmt;
    _stmt->incrRefCount();
}

Stmt::~Stmt()
{
    _stmt->decrRefCount();
    if (_stmt->refCount() <= 0)
	delete _stmt;
}

Stmt&
Stmt::operator=(const Stmt& rhs)
{
    rhs._stmt->incrRefCount();
    _stmt->decrRefCount();
    if (_stmt->refCount() <= 0)
	delete _stmt;
    _stmt = rhs._stmt;
    return *this;
}

bool
Stmt::setCommand(const QString& command)
{
    return _stmt->setCommand(command);
}

QString
Stmt::getCommand() const
{
    return _stmt->getCommand();
}

int
Stmt::paramCount()
{
    return _stmt->paramCount();
}

void
Stmt::setNull(int param)
{
    _stmt->setNull(param);
}

void
Stmt::setValue(int param, Variant value)
{
    _stmt->setValue(param, value);
}

void
Stmt::setString(int param, const QString& value)
{
    _stmt->setString(param, value);
}

void
Stmt::setDouble(int param, double value)
{
    _stmt->setDouble(param, value);
}

void
Stmt::setLong(int param, long value)
{
    _stmt->setLong(param, value);
}

void
Stmt::setDate(int param, QDate value)
{
    _stmt->setDate(param, value);
}

void
Stmt::setTime(int param, QTime value)
{
    _stmt->setTime(param, value);
}

bool
Stmt::execute()
{
    return _stmt->execute();
}

bool
Stmt::next()
{
    return _stmt->next();
}

bool
Stmt::cancel()
{
    return _stmt->cancel();
}

int
Stmt::getUpdateCount()
{
    return _stmt->getUpdateCount();
}

int
Stmt::columnCount()
{
    return _stmt->columnCount();
}

int
Stmt::findColumn(const QString& name)
{
    return _stmt->findColumn(name);
}

QString
Stmt::columnName(int column)
{
    return _stmt->columnName(column);
}

bool
Stmt::isNull(int column)
{
    return _stmt->isNull(column);
}

Variant
Stmt::getValue(int column)
{
    return _stmt->getValue(column);
}

QString
Stmt::getString(int column)
{
    return _stmt->getString(column);
}

double
Stmt::getDouble(int column)
{
    return _stmt->getDouble(column);
}

long
Stmt::getLong(int column)
{
    return _stmt->getLong(column);
}

QDate
Stmt::getDate(int column)
{
    return _stmt->getDate(column);
}

QTime
Stmt::getTime(int column)
{
    return _stmt->getTime(column);
}

QString
Stmt::lastError() const
{
    return _stmt->lastError();
}
