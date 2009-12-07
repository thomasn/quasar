// $Id: security_type_list.cpp,v 1.7 2004/01/31 01:50:31 arandell Exp $
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

#include "security_type_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "security_type_select.h"
#include "security_type_master.h"
#include "grid.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

SecurityTypeList::SecurityTypeList(MainWindow* main)
    : ActiveList(main, "SecurityTypeList")
{
    _helpSource = "security_type_list.html";

    _list->addTextColumn(tr("Name"), 30);
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Security Type List"));
    finalize();
}

SecurityTypeList::~SecurityTypeList()
{
}

bool
SecurityTypeList::isActive(Id type_id)
{
    SecurityType type;
    _quasar->db()->lookup(type_id, type);
    return type.isActive();
}

void
SecurityTypeList::setActive(Id type_id, bool active)
{
    if (type_id == INVALID_ID) return;

    SecurityType type;
    _quasar->db()->lookup(type_id, type);

    SecurityType orig = type;
    _quasar->db()->setActive(type, active);
    _quasar->db()->update(orig, type);
}

void
SecurityTypeList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::CUST_TYPE)
	slotRefresh();
}

void
SecurityTypeList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id type_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    SecurityTypeSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<SecurityType> types;
    _quasar->db()->select(types, conditions);

    for (unsigned int i = 0; i < types.size(); i++) {
	SecurityType& type = types[i];

	ListViewItem* lvi = new ListViewItem(_list, type.id());
	lvi->setText(0, type.name());
	if (showInactive) lvi->setValue(1, !type.isActive());
	if (type.id() == type_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
SecurityTypeList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Security Types"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
SecurityTypeList::performNew()
{
    SecurityTypeMaster* master = new SecurityTypeMaster(_main);
    master->show();
}

void
SecurityTypeList::performEdit()
{
    Id type_id = currentId();
    SecurityTypeMaster* master = new SecurityTypeMaster(_main, type_id);
    master->show();
}
