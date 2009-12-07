// $Id: user_db.cpp,v 1.6 2005/03/01 19:59:42 bpepers Exp $
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

#include "user.h"
#include "user_select.h"

// Create a User
bool
QuasarDB::create(User& user)
{
    if (!validate(user)) return false;

    QString cmd = insertCmd("users", "user_id", "name,passwd,store_id,"
			    "employee_id,security_id,screen");
    Stmt stmt(_connection, cmd);

    insertData(user, stmt);
    stmtSetString(stmt, user.name().lower());
    stmtSetString(stmt, user.password());
    stmtSetId(stmt, user.defaultStore());
    stmtSetId(stmt, user.defaultEmployee());
    stmtSetId(stmt, user.securityType());
    stmtSetString(stmt, user.screen());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, user);
    return true;
}

// Delete a user
bool
QuasarDB::remove(const User& user)
{
    if (user.id() == INVALID_ID) return false;
    if (!removeData(user, "users", "user_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, user);
    return true;
}

// Update a User
bool
QuasarDB::update(const User& orig, User& user)
{
    if (orig.id() == INVALID_ID || user.id() == INVALID_ID) return false;
    if (!validate(user)) return false;

    // Update the users table
    QString cmd = updateCmd("users", "user_id", "name,passwd,store_id,"
			    "employee_id,security_id,screen");
    Stmt stmt(_connection, cmd);

    updateData(orig, user, stmt);
    stmtSetString(stmt, user.name().lower());
    stmtSetString(stmt, user.password());
    stmtSetId(stmt, user.defaultStore());
    stmtSetId(stmt, user.defaultEmployee());
    stmtSetId(stmt, user.securityType());
    stmtSetString(stmt, user.screen());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a User.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id user_id, User& user)
{
    if (user_id == INVALID_ID) return false;
    UserSelect conditions;
    vector<User> users;

    conditions.id = user_id;
    if (!select(users, conditions)) return false;
    if (users.size() != 1) return false;

    user = users[0];
    return true;
}

// Lookup a User by its name
bool
QuasarDB::lookup(const QString& name, User& user)
{
    if (name.isEmpty()) return false;
    UserSelect conditions;
    vector<User> users;

    conditions.name = name;
    if (!select(users, conditions)) return false;
    if (users.size() != 1) return false;

    user = users[0];
    return true;
}

// Returns a vector of User's.
bool
QuasarDB::select(vector<User>& users, const UserSelect& conditions)
{
    users.clear();

    QString cmd = selectCmd("users", "user_id", "name,passwd,store_id,"
			    "employee_id,security_id,screen", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	User user;
	int next = 1;
	selectData(user, stmt, next);
	user.setName(stmtGetString(stmt, next++));
	user.setPassword(stmtGetString(stmt, next++));
	user.setDefaultStore(stmtGetId(stmt, next++));
	user.setDefaultEmployee(stmtGetId(stmt, next++));
	user.setSecurityType(stmtGetId(stmt, next++));
	user.setScreen(stmtGetString(stmt, next++));
	users.push_back(user);
    }

    commit();
    return true;
}

// Validate user information
bool
QuasarDB::validate(const User& user)
{
    if (!validate((DataObject&)user)) return false;

    if (user.name().stripWhiteSpace().isEmpty())
	return error("Blank user name");

    return true;
}

bool
QuasarDB::users(vector<QString>& users)
{
    users.clear();

    QString cmd = "select name from users order by name";
    Stmt stmt(_connection, cmd);
    if (!execute(stmt)) return false;
    while (stmt.next())
	users.push_back(stmt.getString(1));
    commit();

    return true;
}
