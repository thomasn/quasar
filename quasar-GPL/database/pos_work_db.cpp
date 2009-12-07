// $Id: pos_work_db.cpp,v 1.2 2005/03/01 19:59:42 bpepers Exp $
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

#include "pos_work.h"
#include "pos_work_select.h"

// Create a new pos_work
bool
QuasarDB::create(const PosWork& work)
{
    QString cmd = "insert into pos_work (work_type,tx_id,status,store_id,"
	"station_id,employee_id) values (?,?,?,?,?,?)";
    Stmt stmt(_connection, cmd);

    stmtSetString(stmt, work.type());
    stmtSetString(stmt, work.id());
    stmtSetString(stmt, work.status());
    stmtSetId(stmt, work.storeId());
    stmtSetId(stmt, work.stationId());
    stmtSetId(stmt, work.employeeId());

    if (!execute(stmt)) return false;

    commit();
    return true;
}

// Remove a pos_work
bool
QuasarDB::remove(const PosWork& work)
{
    QString cmd = "delete from pos_work where work_type=? and tx_id=?";
    Stmt stmt(_connection, cmd);

    stmtSetString(stmt, work.type());
    stmtSetString(stmt, work.id());
    if (!execute(stmt)) return false;

    commit();
    return true;
}

// Set the work status
bool
QuasarDB::setStatus(PosWork& work, const QString& status)
{
    QString cmd = "update pos_work set status=? where work_type=? "
	"and tx_id=? and status=?";
    Stmt stmt(_connection, cmd);

    stmtSetString(stmt, status);
    stmtSetString(stmt, work.type());
    stmtSetString(stmt, work.id());
    stmtSetString(stmt, work.status());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    work.setStatus(status);
    commit();
    return true;
}

// Read in a pos_work record
bool
QuasarDB::lookup(const QString& type, const QString& id, PosWork& work)
{
    bool result = false;

    QString cmd = "select status,store_id,station_id,employee_id from "
	"pos_work where work_type=? and tx_id=?";
    Stmt stmt(_connection, cmd);
    stmtSetString(stmt, type);
    stmtSetString(stmt, id);
    if (!execute(stmt)) return false;
    if (stmt.next()) {
	work.setType(type);
	work.setId(id);
	work.setStatus(stmtGetString(stmt, 1));
	work.setStoreId(stmtGetId(stmt, 2));
	work.setStationId(stmtGetId(stmt, 3));
	work.setEmployeeId(stmtGetId(stmt, 4));
	result = true;
    }

    commit();
    return result;
}

// Returns a vector of pos_work
bool
QuasarDB::select(vector<PosWork>& works, const PosWorkSelect& conditions)
{
    works.clear();

    QString cmd = "select work_type,tx_id,status,store_id,station_id,"
	"employee_id from pos_work";
    cmd += conditions.where();
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	PosWork work;
	int next = 1;
	work.setType(stmtGetString(stmt, next++));
	work.setId(stmtGetString(stmt, next++));
	work.setStatus(stmtGetString(stmt, next++));
	work.setStoreId(stmtGetId(stmt, next++));
	work.setStationId(stmtGetId(stmt, next++));
	work.setEmployeeId(stmtGetId(stmt, next++));
	works.push_back(work);
    }

    commit();
    return true;
}
