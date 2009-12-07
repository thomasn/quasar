// $Id: sql_stmt.cpp,v 1.7 2005/03/01 19:48:42 bpepers Exp $
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

#include "sql_stmt.h"

void
stmtSetId(Statement* stmt, Id id)
{
    if (id == INVALID_ID)
	stmt->setNull(-1);
    else
	stmt->setString(-1, id.toString());
}

Id
stmtGetId(Statement* stmt, int column)
{
    if (stmt->isNull(column))
	return INVALID_ID;
    return Id::fromStringStatic(stmt->getString(column));
}

void
stmtSetInt(Statement* stmt, long value)
{
    stmt->setLong(-1, value);
}

long
stmtGetInt(Statement* stmt, int column)
{
    return stmt->getLong(column);
}

void
stmtSetString(Statement* stmt, const QString& value)
{
    stmt->setString(-1, value);
}

QString
stmtGetString(Statement* stmt, int column)
{
    return stmt->getString(column);
}

void
stmtSetDate(Statement* stmt, QDate date)
{
    stmt->setDate(-1, date);
}

QDate
stmtGetDate(Statement* stmt, int column)
{
    return stmt->getDate(column);
}

void
stmtSetTime(Statement* stmt, QTime time)
{
    stmt->setTime(-1, time);
}

QTime
stmtGetTime(Statement* stmt, int column)
{
    return stmt->getTime(column);
}

void
stmtSetBool(Statement* stmt, bool value)
{
    stmtSetString(stmt, value ? "Y" : "N");
}

bool
stmtGetBool(Statement* stmt, int column)
{
    QString value = stmtGetString(stmt, column);
    return value == "Y";
}

void
stmtSetFixed(Statement* stmt, fixed value)
{
    stmt->setDouble(-1, value.toDouble());
}

fixed
stmtGetFixed(Statement* stmt, int column)
{
    return stmt->getDouble(column);
}
