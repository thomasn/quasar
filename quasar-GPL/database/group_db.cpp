// $Id: group_db.cpp,v 1.6 2005/03/01 19:59:42 bpepers Exp $
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

#include "group.h"
#include "group_select.h"

// Create a Group
bool
QuasarDB::create(Group& group)
{
    if (!validate(group)) return false;

    QString cmd = insertCmd("groups", "group_id", "name,description,"
			    "group_type");
    Stmt stmt(_connection, cmd);

    insertData(group, stmt);
    stmtSetString(stmt, group.name());
    stmtSetString(stmt, group.description());
    stmtSetInt(stmt, group.type());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, group);
    return true;
}

// Delete a Group
bool
QuasarDB::remove(const Group& group)
{
    if (group.id() == INVALID_ID) return false;
    if (!removeData(group, "groups", "group_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, group);
    return true;
}

// Update a Group
bool
QuasarDB::update(const Group& orig, Group& group)
{
    if (orig.id() == INVALID_ID || group.id() == INVALID_ID) return false;
    if (!validate(group)) return false;

    // TODO: should type change be allowed?

    // Update the group table
    QString cmd = updateCmd("groups", "group_id", "name,description,"
			    "group_type");
    Stmt stmt(_connection, cmd);

    updateData(orig, group, stmt);
    stmtSetString(stmt, group.name());
    stmtSetString(stmt, group.description());
    stmtSetInt(stmt, group.type());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Group.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id group_id, Group& group)
{
    if (group_id == INVALID_ID) return false;
    GroupSelect conditions;
    vector<Group> groups;

    conditions.id = group_id;
    if (!select(groups, conditions)) return false;
    if (groups.size() != 1) return false;

    group = groups[0];
    return true;
}

// Lookup a Group.  This is just a shortcut to using select.
bool
QuasarDB::lookup(const QString& name, Group& group)
{
    if (name.isEmpty()) return false;
    GroupSelect conditions;
    vector<Group> groups;

    conditions.name = name;
    if (!select(groups, conditions)) return false;
    if (groups.size() != 1) return false;

    group = groups[0];
    return true;
}

// Returns a vector of Group's.  Returns groups sorted by name.
bool
QuasarDB::select(vector<Group>& groups, const GroupSelect& conditions)
{
    groups.clear();

    QString cmd = selectCmd("groups", "group_id", "name,description,"
			    "group_type", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Group group;
	int next = 1;
	selectData(group, stmt, next);
	group.setName(stmtGetString(stmt, next++));
	group.setDescription(stmtGetString(stmt, next++));
	group.setType(stmtGetInt(stmt, next++));
	groups.push_back(group);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Group& group)
{
    if (!validate((DataObject&)group)) return false;

    if (group.name().stripWhiteSpace().isEmpty())
	return error("Blank group name");

    return true;
}
