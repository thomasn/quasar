// $Id: select.cpp,v 1.11 2004/01/30 23:16:36 arandell Exp $
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

#include "select.h"
#include <qregexp.h>

Select::Select()
{
    id = INVALID_ID;
    external_id = -1;
    activeOnly = false;
}

Select::~Select()
{
}

QString
Select::where() const
{
    QString where = "";

    if (external_id != -1)
	addStringCondition(where, "external_id", external_id.toString());
    if (activeOnly)
	addCondition(where, "inactive_on is null");

    return where;
}

void
Select::addCondition(QString& where, const QString& condition) const
{
    if (where.isEmpty())
	where = " where " + condition;
    else
	where += " and " + condition;
}

void
Select::addStringCondition(QString& where, const QString& field,
			   const QString& value, int len) const
{
    QString sql = value;

    if (len != 0 && int(sql.length()) > len)
	sql = sql.left(len);

    sql.replace(QRegExp("'"), "''");
    sql = "'" + sql + "'";

    if (value.right(1) == "%")
	addCondition(where, field + " like " + sql);
    else
	addCondition(where, field + " = " + sql);
}

void
Select::addIdCondition(QString& where, const QString& field, Id id) const
{
    if (id == INVALID_ID)
	addCondition(where, field + " is null");
    else
	addStringCondition(where, field, id.toString());
}
