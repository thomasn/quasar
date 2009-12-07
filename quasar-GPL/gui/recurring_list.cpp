// $Id: recurring_list.cpp,v 1.9 2004/01/31 01:50:31 arandell Exp $
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

#include "recurring_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "gltx.h"
#include "card.h"
#include "group.h"
#include "recurring_select.h"
#include "recurring_master.h"
#include "gltx_select.h"
#include "card_select.h"
#include "grid.h"

#include <qapplication.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

RecurringList::RecurringList(MainWindow* main)
    : ActiveList(main, "RecurringList")
{
    _helpSource = "recurring_list.html";

    _list->addTextColumn(tr("Type"), 20);
    _list->addTextColumn(tr("Description"), 20);
    _list->addTextColumn(tr("Card/Group"), 20);
    _list->addDateColumn(tr("Last Posted"));
    _list->addDateColumn(tr("Next Due"));
    _list->addTextColumn(tr("Overdue"), 10);
    _list->setSorting(3);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Recurring Transactions"));

    finalize();
    _new->hide();
}

RecurringList::~RecurringList()
{
}

bool
RecurringList::isActive(Id recurring_id)
{
    Recurring recurring;
    _quasar->db()->lookup(recurring_id, recurring);
    return recurring.isActive();
}

void
RecurringList::setActive(Id recurring_id, bool active)
{
    if (recurring_id == INVALID_ID) return;

    Recurring recurring;
    _quasar->db()->lookup(recurring_id, recurring);

    Recurring orig = recurring;
    _quasar->db()->setActive(recurring, active);
    _quasar->db()->update(orig, recurring);
}

void
RecurringList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::RECURRING)
	slotRefresh();
}

void
RecurringList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id recurring_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    RecurringSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<Recurring> recurrings;
    _quasar->db()->select(recurrings, conditions);

    for (unsigned int i = 0; i < recurrings.size(); ++i) {
	Recurring& recurring = recurrings[i];

	Gltx gltx;
	_quasar->db()->lookup(recurring.gltxId(), gltx);

	QString name;
	if (recurring.cardGroup() == INVALID_ID) {
	    Card card;
	    _quasar->db()->lookup(gltx.cardId(), card);
	    name = card.nameFL();
	} else {
	    Group group;
	    _quasar->db()->lookup(recurring.cardGroup(), group);
	    name = group.name();
	}

	ListViewItem* lvi = new ListViewItem(_list, recurring.id());
	lvi->setValue(0, gltx.dataTypeName());
	lvi->setValue(1, recurring.description());
	lvi->setValue(2, name);
	lvi->setValue(3, recurring.lastPosted());
	lvi->setValue(4, recurring.nextDue());
	lvi->setValue(5, recurring.overdueDays());
	if (showInactive) lvi->setValue(6, !recurring.isActive());
	if (recurring.id() == recurring_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
RecurringList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Recurring Transactions"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
RecurringList::performNew()
{
    RecurringMaster* master = new RecurringMaster(_main, INVALID_ID);
    master->show();
}

void
RecurringList::performEdit()
{
    Id recurring_id = currentId();
    RecurringMaster* master = new RecurringMaster(_main, recurring_id);
    master->show();
}
