// $Id: todo_db.cpp,v 1.18 2005/03/01 19:59:42 bpepers Exp $
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

#include "todo.h"
#include "todo_select.h"

// Create a Todo
bool
QuasarDB::create(Todo& todo)
{
    if (!validate(todo)) return false;

    QString cmd = insertCmd("todo", "todo_id", "note,remind_on");
    Stmt stmt(_connection, cmd);

    insertData(todo, stmt);
    stmtSetString(stmt, todo.note());
    stmtSetDate(stmt, todo.remindOn());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, todo);
    return true;
}

// Delete a Todo
bool
QuasarDB::remove(const Todo& todo)
{
    if (todo.id() == INVALID_ID) return false;
    if (!removeData(todo, "todo", "todo_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, todo);
    return true;
}

// Update a Todo
bool
QuasarDB::update(const Todo& orig, Todo& todo)
{
    if (orig.id() == INVALID_ID || todo.id() == INVALID_ID) return false;
    if (!validate(todo)) return false;

    // Update the todo table
    QString cmd = updateCmd("todo", "todo_id", "note,remind_on");
    Stmt stmt(_connection, cmd);

    updateData(orig, todo, stmt);
    stmtSetString(stmt, todo.note());
    stmtSetDate(stmt, todo.remindOn());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Todo.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id todo_id, Todo& todo)
{
    if (todo_id == INVALID_ID) return false;
    TodoSelect conditions;
    vector<Todo> todos;

    conditions.id = todo_id;
    if (!select(todos, conditions)) return false;
    if (todos.size() != 1) return false;

    todo = todos[0];
    return true;
}

// Returns a vector of Todo's.
bool
QuasarDB::select(vector<Todo>& todos, const TodoSelect& conditions)
{
    todos.clear();

    QString cmd = selectCmd("todo", "todo_id", "note,remind_on",
			    conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Todo todo;
	int next = 1;
	selectData(todo, stmt, next);
	todo.setNote(stmtGetString(stmt, next++));
	todo.setRemindOn(stmtGetDate(stmt, next++));
	todos.push_back(todo);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Todo& todo)
{
    if (!validate((DataObject&)todo)) return false;

    if (todo.note().stripWhiteSpace().isEmpty())
	return error("Blank note");

    return true;
}
