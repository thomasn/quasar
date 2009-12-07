// $Id: expense_list.cpp,v 1.14 2004/01/31 01:50:30 arandell Exp $
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

#include "expense_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "expense_select.h"
#include "expense_master.h"
#include "grid.h"

#include <qapplication.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

ExpenseList::ExpenseList(MainWindow* main)
    : ActiveList(main, "ExpenseList")
{
    _helpSource = "expense_list.html";

    _list->addTextColumn(tr("Name"), 20);
    _list->addTextColumn(tr("Number"), 8);
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Expense List"));
    finalize();
}

ExpenseList::~ExpenseList()
{
}

bool
ExpenseList::isActive(Id expense_id)
{
    Expense expense;
    _quasar->db()->lookup(expense_id, expense);
    return expense.isActive();
}

void
ExpenseList::setActive(Id expense_id, bool active)
{
    if (expense_id == INVALID_ID) return;

    Expense expense;
    _quasar->db()->lookup(expense_id, expense);

    Expense orig = expense;
    _quasar->db()->setActive(expense, active);
    _quasar->db()->update(orig, expense);
}

void
ExpenseList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::EXPENSE)
	slotRefresh();
}

void
ExpenseList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id expense_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    ExpenseSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<Expense> expenses;
    _quasar->db()->select(expenses, conditions);

    for (unsigned int i = 0; i < expenses.size(); i++) {
	Expense& expense = expenses[i];

	ListViewItem* lvi = new ListViewItem(_list, expense.id());
	lvi->setValue(0, expense.name());
	lvi->setValue(1, expense.number());
	if (showInactive) lvi->setValue(2, !expense.isActive());

	if (expense.id() == expense_id)
	    current = lvi;
    }

    if (current == NULL)
	current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
ExpenseList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Expenses"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
ExpenseList::performNew()
{
    ExpenseMaster* master = new ExpenseMaster(_main, INVALID_ID);
    master->show();
}

void
ExpenseList::performEdit()
{
    Id expense_id = currentId();
    ExpenseMaster* master = new ExpenseMaster(_main, expense_id);
    master->show();
}
