// $Id: subdept_list.cpp,v 1.37 2004/11/10 10:36:52 bpepers Exp $
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

#include "subdept_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "dept.h"
#include "subdept_select.h"
#include "subdept_master.h"
#include "grid.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

SubdeptList::SubdeptList(MainWindow* main)
    : ActiveList(main, "SubdeptList")
{
    _helpSource = "subdept_list.html";

    _list->addTextColumn(tr("Name"), 30);
    _list->addTextColumn(tr("Number"), 10);
    _list->addTextColumn(tr("Department"), 30);
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Subdepartment List"));
    finalize();
}

SubdeptList::~SubdeptList()
{
}

bool
SubdeptList::isActive(Id subdept_id)
{
    Subdept subdept;
    _quasar->db()->lookup(subdept_id, subdept);
    return subdept.isActive();
}

void
SubdeptList::setActive(Id subdept_id, bool active)
{
    if (subdept_id == INVALID_ID) return;

    Subdept subdept;
    _quasar->db()->lookup(subdept_id, subdept);

    Subdept orig = subdept;
    _quasar->db()->setActive(subdept, active);
    _quasar->db()->update(orig, subdept);
}

void
SubdeptList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::SUBDEPT)
	slotRefresh();
}

void
SubdeptList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id subdept_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    SubdeptSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<Subdept> subdepts;
    _quasar->db()->select(subdepts, conditions);

    for (unsigned int i = 0; i < subdepts.size(); i++) {
	Subdept& subdept = subdepts[i];

	Dept dept;
	_quasar->db()->lookup(subdept.deptId(), dept);

	ListViewItem* lvi = new ListViewItem(_list, subdept.id());
	lvi->setText(0, subdept.name());
	lvi->setText(1, subdept.number());
	lvi->setText(2, dept.name());
	if (showInactive) lvi->setValue(3, !subdept.isActive());
	if (subdept.id() == subdept_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
SubdeptList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Subdepartments"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
SubdeptList::performNew()
{
    SubdeptMaster* master = new SubdeptMaster(_main, INVALID_ID);
    master->show();
}

void
SubdeptList::performEdit()
{
    Id subdept_id = currentId();
    SubdeptMaster* master = new SubdeptMaster(_main, subdept_id);
    master->show();
}
