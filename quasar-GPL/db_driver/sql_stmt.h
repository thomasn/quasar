// $Id: sql_stmt.h,v 1.5 2005/03/01 19:48:42 bpepers Exp $
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

#ifndef SQL_STMT_H
#define SQL_STMT_H

#include "variant.h"
#include "db_driver.h"

// Get database data
extern void stmtSetId(Statement* stmt, Id id);
extern void stmtSetInt(Statement* stmt, long value);
extern void stmtSetString(Statement* stmt, const QString& value);
extern void stmtSetDate(Statement* stmt, QDate date);
extern void stmtSetTime(Statement* stmt, QTime time);
extern void stmtSetBool(Statement* stmt, bool value);
extern void stmtSetFixed(Statement* stmt, fixed value);

// Set database data
extern Id stmtGetId(Statement* stmt, int field);
extern long stmtGetInt(Statement* stmt, int field);
extern QString stmtGetString(Statement* stmt, int field);
extern QDate stmtGetDate(Statement* stmt, int field);
extern QTime stmtGetTime(Statement* stmt, int field);
extern bool stmtGetBool(Statement* stmt, int field);
extern fixed stmtGetFixed(Statement* stmt, int field);

#endif // SQL_STMT_H
