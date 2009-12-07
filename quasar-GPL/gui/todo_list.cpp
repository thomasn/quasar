// $Id: todo_list.cpp,v 1.26 2004/01/31 01:50:31 arandell Exp $
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

#include "todo_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "todo_select.h"
#include "todo_master.h"
#include "date_valcon.h"
#include "grid.h"

#include <qapplication.h>
#include <qpopupmenu.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

TodoList::TodoList(MainWindow* main)
    : ActiveList(main, "TodoList")
{
    _helpSource = "todo_list.html";

    _list->addTextColumn(tr("Description"), 40);
    _list->addDateColumn(tr("Remind On"));
    _list->addCheckColumn(tr("Done"));
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    _inactive->setText(tr("Show Done?"));

    setCaption(tr("Todo List"));
    finalize();
}

TodoList::~TodoList()
{
}

void
TodoList::slotActivities()
{
    Id id = currentId();

    _activities->clear();
    if (id != INVALID_ID) {
	if (isActive(id))
	    _activities->insertItem(tr("Set done"), this,
				    SLOT(slotSetInactive()));
	else
	    _activities->insertItem(tr("Set undone"), this,
				    SLOT(slotSetActive()));
    } else {
	int menu_id = _activities->insertItem(tr("Set done"));
	_activities->setItemEnabled(menu_id, false);
    }
}

bool
TodoList::isActive(Id todo_id)
{
    Todo todo;
    _quasar->db()->lookup(todo_id, todo);
    return todo.isActive();
}

void
TodoList::setActive(Id todo_id, bool active)
{
    if (todo_id == INVALID_ID) return;

    Todo todo;
    _quasar->db()->lookup(todo_id, todo);

    Todo orig = todo;
    _quasar->db()->setActive(todo, active);
    _quasar->db()->update(orig, todo);
}

void
TodoList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::TODO)
	slotRefresh();
}

void
TodoList::performRefresh()
{
    Id todo_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    TodoSelect conditions;
    conditions.activeOnly = !_inactive->isChecked();
    vector<Todo> todos;
    _quasar->db()->select(todos, conditions);

    for (unsigned int i = 0; i < todos.size(); i++) {
	Todo& todo = todos[i];

	ListViewItem* lvi = new ListViewItem(_list, todo.id());
	lvi->setValue(0, todo.description());
	lvi->setValue(1, todo.remindOn());
	lvi->setValue(2, !todo.isActive());

	if (todo.id() == todo_id)
	    current = lvi;
    }

    if (current == NULL)
	current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
TodoList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("To Do List"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
TodoList::performNew()
{
    TodoMaster* master = new TodoMaster(_main, INVALID_ID);
    master->show();
}

void
TodoList::performEdit()
{
    Id todo_id = currentId();
    TodoMaster* master = new TodoMaster(_main, todo_id);
    master->show();
}
