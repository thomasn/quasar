// $Id: expense_lookup.cpp,v 1.9 2004/12/06 09:24:15 bpepers Exp $
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

#include "expense_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "expense_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "expense_master.h"

#include <qlabel.h>
#include <qlayout.h>

ExpenseLookup::ExpenseLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "ExpenseLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    setCaption(tr("Expense Lookup"));
}

ExpenseLookup::ExpenseLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "ExpenseLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    setCaption(tr("Expense Lookup"));
}

ExpenseLookup::~ExpenseLookup()
{
}

QString
ExpenseLookup::lookupById(Id expense_id)
{
    Expense expense;
    if (expense_id != INVALID_ID && _quasar->db()->lookup(expense_id, expense))
	return expense.name();
    return "";
}

vector<DataPair>
ExpenseLookup::lookupByText(const QString& text)
{
    ExpenseSelect conditions;
    if (!text.isEmpty())
	conditions.name = text + "%";
    conditions.activeOnly = activeOnly;

    vector<Expense> expenses;
    _quasar->db()->select(expenses, conditions);
    if (expenses.size() == 0 && !text.isEmpty()) {
	conditions.name = "";
	conditions.number = text;
	_quasar->db()->select(expenses, conditions);
    }

    vector<DataPair> data;
    for (unsigned int i = 0; i < expenses.size(); ++i) {
	data.push_back(DataPair(expenses[i].id(), expenses[i].name()));
    }

    return data;
}

void
ExpenseLookup::refresh()
{
    ExpenseSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.activeOnly = activeOnly;

    vector<Expense> expenses;
    _quasar->db()->select(expenses, conditions);

    _list->clear();
    for (unsigned int i = 0; i < expenses.size(); ++i) {
	Expense& expense = expenses[i];

	ListViewItem* item = new ListViewItem(_list, expense.id());
	item->setText(0, expense.name());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
ExpenseLookup::slotNew()
{
    ExpenseMaster* window = new ExpenseMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
ExpenseLookup::slotEdit(Id id)
{
    ExpenseMaster* window = new ExpenseMaster(_main, id);
    window->show();
    reject();
    return window;
}
