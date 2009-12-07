// $Id: pat_group_list.cpp,v 1.6 2004/01/31 01:50:30 arandell Exp $
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

#include "pat_group_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "pat_group_select.h"
#include "pat_group_master.h"
#include "grid.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

PatGroupList::PatGroupList(MainWindow* main)
    : ActiveList(main, "PatGroupList")
{
    _helpSource = "pat_group_list.html";

    _list->addTextColumn(tr("Name"), 20);
    _list->addTextColumn(tr("Number"), 12);
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Patronage Group List"));
    finalize();
}

PatGroupList::~PatGroupList()
{
}

bool
PatGroupList::isActive(Id group_id)
{
    PatGroup group;
    _quasar->db()->lookup(group_id, group);
    return group.isActive();
}

void
PatGroupList::setActive(Id group_id, bool active)
{
    if (group_id == INVALID_ID) return;

    PatGroup group;
    _quasar->db()->lookup(group_id, group);

    PatGroup orig = group;
    _quasar->db()->setActive(group, active);
    _quasar->db()->update(orig, group);
}

void
PatGroupList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::GROUP)
	slotRefresh();
}

void
PatGroupList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id group_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    PatGroupSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<PatGroup> groups;
    _quasar->db()->select(groups, conditions);

    for (unsigned int i = 0; i < groups.size(); ++i) {
	PatGroup& group = groups[i];

	ListViewItem* lvi = new ListViewItem(_list, group.id());
	lvi->setText(0, group.name());
	lvi->setText(1, group.number());
	if (showInactive) lvi->setValue(2, !group.isActive());
	if (group.id() == group_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
PatGroupList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Patronage Groups"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
PatGroupList::performNew()
{
    PatGroupMaster* master = new PatGroupMaster(_main, INVALID_ID);
    master->show();
}

void
PatGroupList::performEdit()
{
    Id group_id = currentId();
    PatGroupMaster* master = new PatGroupMaster(_main, group_id);
    master->show();
}
