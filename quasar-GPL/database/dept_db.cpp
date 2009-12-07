// $Id: dept_db.cpp,v 1.21 2005/03/01 19:59:42 bpepers Exp $
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

#include "quasar_db.h"

#include "dept.h"
#include "dept_select.h"

// Create a Dept
bool
QuasarDB::create(Dept& dept)
{
    if (!validate(dept)) return false;

    QString cmd = insertCmd("dept", "dept_id", "name,number,pat_points");
    Stmt stmt(_connection, cmd);

    insertData(dept, stmt);
    stmtSetString(stmt, dept.name());
    stmtSetString(stmt, dept.number());
    stmtSetFixed(stmt, dept.patPoints());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, dept);
    return true;
}

// Delete a Dept
bool
QuasarDB::remove(const Dept& dept)
{
    if (dept.id() == INVALID_ID) return false;
    if (!removeData(dept, "dept", "dept_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, dept);
    return true;
}

// Update a Dept
bool
QuasarDB::update(const Dept& orig, Dept& dept)
{
    if (orig.id() == INVALID_ID || dept.id() == INVALID_ID) return false;
    if (!validate(dept)) return false;

    // Update the dept table
    QString cmd = updateCmd("dept", "dept_id", "name,number,pat_points");
    Stmt stmt(_connection, cmd);

    updateData(orig, dept, stmt);
    stmtSetString(stmt, dept.name());
    stmtSetString(stmt, dept.number());
    stmtSetFixed(stmt, dept.patPoints());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Dept.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id dept_id, Dept& dept)
{
    if (dept_id == INVALID_ID) return false;
    DeptSelect conditions;
    vector<Dept> depts;

    conditions.id = dept_id;
    if (!select(depts, conditions)) return false;
    if (depts.size() != 1) return false;

    dept = depts[0];
    return true;
}

// Lookup a Dept.  This is just a shortcut to using select.
bool
QuasarDB::lookup(const QString& number, Dept& dept)
{
    if (number.isEmpty()) return false;
    DeptSelect conditions;
    vector<Dept> depts;

    conditions.number = number;
    if (!select(depts, conditions)) return false;
    if (depts.size() != 1) return false;

    dept = depts[0];
    return true;
}

// Returns a vector of Dept's.  Returns depts sorted by name.
bool
QuasarDB::select(vector<Dept>& depts, const DeptSelect& conditions)
{
    depts.clear();

    QString cmd = selectCmd("dept", "dept_id", "name,number,pat_points",
			    conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Dept dept;
	int next = 1;
	selectData(dept, stmt, next);
	dept.setName(stmtGetString(stmt, next++));
	dept.setNumber(stmtGetString(stmt, next++));
	dept.setPatPoints(stmtGetFixed(stmt, next++));
	depts.push_back(dept);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Dept& dept)
{
    if (!validate((DataObject&)dept)) return false;

    if (dept.name().stripWhiteSpace().isEmpty())
	return error("Blank department name");

    if (dept.patPoints() < 0.0)
	return error("Negative patronage points are not allowed");

    return true;
}
