// $Id: dept_list.cpp,v 1.38 2004/11/10 10:36:52 bpepers Exp $
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

#include "dept_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "dept_select.h"
#include "dept_master.h"
#include "grid.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

DeptList::DeptList(MainWindow* main)
    : ActiveList(main, "DeptList")
{
    _helpSource = "dept_list.html";

    _list->addTextColumn(tr("Name"), 30);
    _list->addTextColumn(tr("Number"), 10);
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Department List"));
    finalize();
}

DeptList::~DeptList()
{
}

bool
DeptList::isActive(Id dept_id)
{
    Dept dept;
    _quasar->db()->lookup(dept_id, dept);
    return dept.isActive();
}

void
DeptList::setActive(Id dept_id, bool active)
{
    if (dept_id == INVALID_ID) return;

    Dept dept;
    _quasar->db()->lookup(dept_id, dept);

    Dept orig = dept;
    _quasar->db()->setActive(dept, active);
    _quasar->db()->update(orig, dept);
}

void
DeptList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::DEPT)
	slotRefresh();
}

void
DeptList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id dept_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    DeptSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<Dept> depts;
    _quasar->db()->select(depts, conditions);

    for (unsigned int i = 0; i < depts.size(); ++i) {
	Dept& dept = depts[i];

	ListViewItem* lvi = new ListViewItem(_list, dept.id());
	lvi->setText(0, dept.name());
	lvi->setText(1, dept.number());
	if (showInactive) lvi->setValue(2, !dept.isActive());
	if (dept.id() == dept_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
DeptList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Departments"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
DeptList::performNew()
{
    DeptMaster* master = new DeptMaster(_main, INVALID_ID);
    master->show();
}

void
DeptList::performEdit()
{
    Id dept_id = currentId();
    DeptMaster* master = new DeptMaster(_main, dept_id);
    master->show();
}
