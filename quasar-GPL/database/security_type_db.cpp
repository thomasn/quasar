// $Id: security_type_db.cpp,v 1.8 2005/03/01 19:59:42 bpepers Exp $
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

#include "security_type.h"
#include "security_type_select.h"

// Create a SecurityType
bool
QuasarDB::create(SecurityType& type)
{
    if (!validate(type)) return false;

    QString cmd = insertCmd("security_type", "security_id", "name");
    Stmt stmt(_connection, cmd);

    insertData(type, stmt);
    stmtSetString(stmt, type.name());
    if (!execute(stmt)) return false;

    cmd = "insert into security_rules (security_id,seq_num,screen,"
	"allow_view,allow_create,allow_update,allow_delete) values "
	"(?,?,?,?,?,?,?)";
    stmt.setCommand(cmd);
    for (unsigned int i = 0; i < type.rules().size(); ++i) {
	stmtSetId(stmt, type.id());
	stmtSetInt(stmt, i);
	stmtSetString(stmt, type.rules()[i].screen);
	stmtSetBool(stmt, type.rules()[i].allowView);
	stmtSetBool(stmt, type.rules()[i].allowCreate);
	stmtSetBool(stmt, type.rules()[i].allowUpdate);
	stmtSetBool(stmt, type.rules()[i].allowDelete);
	if (!execute(stmt)) return false;
    }

    commit();
    dataSignal(DataEvent::Insert, type);
    return true;
}

// Delete a SecurityType
bool
QuasarDB::remove(const SecurityType& type)
{
    if (type.id() == INVALID_ID) return false;
    if (!removeData(type, "security_type", "security_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, type);
    return true;
}

// Update a SecurityType
bool
QuasarDB::update(const SecurityType& orig, SecurityType& type)
{
    if (orig.id() == INVALID_ID || type.id() == INVALID_ID) return false;
    if (!validate(type)) return false;

    // Update the type table
    QString cmd = updateCmd("security_type", "security_id", "name");
    Stmt stmt(_connection, cmd);

    updateData(orig, type, stmt);
    stmtSetString(stmt, type.name());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");
    if (!remove(orig, "security_rules", "security_id")) return false;

    cmd = "insert into security_rules (security_id,seq_num,screen,"
	"allow_view,allow_create,allow_update,allow_delete) values "
	"(?,?,?,?,?,?,?)";
    stmt.setCommand(cmd);
    for (unsigned int i = 0; i < type.rules().size(); ++i) {
	stmtSetId(stmt, type.id());
	stmtSetInt(stmt, i);
	stmtSetString(stmt, type.rules()[i].screen);
	stmtSetBool(stmt, type.rules()[i].allowView);
	stmtSetBool(stmt, type.rules()[i].allowCreate);
	stmtSetBool(stmt, type.rules()[i].allowUpdate);
	stmtSetBool(stmt, type.rules()[i].allowDelete);
	if (!execute(stmt)) return false;
    }

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a SecurityType.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id type_id, SecurityType& type)
{
    if (type_id == INVALID_ID) return false;
    SecurityTypeSelect conditions;
    vector<SecurityType> types;

    conditions.id = type_id;
    if (!select(types, conditions)) return false;
    if (types.size() != 1) return false;

    type = types[0];
    return true;
}

// Lookup a SecurityType.  This is just a shortcut to using select.
bool
QuasarDB::lookup(const QString& name, SecurityType& type)
{
    if (name.isEmpty()) return false;
    SecurityTypeSelect conditions;
    vector<SecurityType> types;

    conditions.name = name;
    if (!select(types, conditions)) return false;
    if (types.size() != 1) return false;

    type = types[0];
    return true;
}

// Returns a vector of SecurityTypees.  Returns SecurityTypees sorted by name.
bool
QuasarDB::select(vector<SecurityType>& types, const SecurityTypeSelect& conds)
{
    types.clear();

    QString cmd = selectCmd("security_type", "security_id", "name", conds);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	SecurityType type;
	int next = 1;
	selectData(type, stmt, next);
	type.setName(stmtGetString(stmt, next++));
	types.push_back(type);
    }

    cmd = "select screen,allow_view,allow_create,allow_update,"
	"allow_delete from security_rules where security_id = ? "
	"order by seq_num";
    Stmt stmt2(_connection, cmd);

    for (unsigned int i = 0; i < types.size(); ++i) {
	Id type_id = types[i].id();

	stmtSetId(stmt2, type_id);
	if (!execute(stmt2)) return false;
	while (stmt2.next()) {
	    SecurityRule rule;
	    rule.screen = stmtGetString(stmt2, 1);
	    rule.allowView = stmtGetBool(stmt2, 2);
	    rule.allowCreate = stmtGetBool(stmt2, 3);
	    rule.allowUpdate = stmtGetBool(stmt2, 4);
	    rule.allowDelete = stmtGetBool(stmt2, 5);
	    types[i].rules().push_back(rule);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const SecurityType& type)
{
    if (!validate((DataObject&)type)) return false;

    // Check for duplicate name
    SecurityTypeSelect conditions;
    conditions.name = type.name();
    vector<SecurityType> types;
    select(types, conditions);
    for (unsigned int i = 0; i < types.size(); ++i)
	if (types[i].id() != type.id())
	    return error("Name already used for another type");

    if (type.name().stripWhiteSpace().isEmpty())
	return error("Blank type name");

    return true;
}
