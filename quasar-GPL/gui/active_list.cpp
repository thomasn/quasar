// $Id: active_list.cpp,v 1.14 2004/01/31 01:50:30 arandell Exp $
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

#include "active_list.h"
#include "main_window.h"
#include "quasar_client.h"

#include <qapplication.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>
#include <qpopupmenu.h>

ActiveList::ActiveList(MainWindow* main, const char* name, bool tabs)
    : ListWindow(main, name, tabs)
{
    _inactive = new QCheckBox(tr("Show Inactive?"), _extra);
    connect(_inactive, SIGNAL(toggled(bool)), SLOT(slotInactiveChanged()));

    _grid = new QGridLayout(_extra);
    _grid->addWidget(_inactive, 0, 0, AlignLeft | AlignVCenter);
}

ActiveList::~ActiveList()
{
}

void
ActiveList::slotInactiveChanged()
{
    clearLists();

    int columns = _list->columns();
    if (_inactive->isChecked()) {
	_list->addCheckColumn(tr("Inactive?"));
	resize(width() + _list->columnWidth(columns), height());
    } else {
	int column = _list->findColumn(tr("Inactive?"));
	if (column != -1) {
	    resize(width() - _list->columnWidth(column), height());
	    _list->removeColumn(column);
	}
    }
}

void
ActiveList::slotSetActive()
{
    if (currentId() == INVALID_ID) return;
    setActive(currentId(), true);
}

void
ActiveList::slotSetInactive()
{
    if (currentId() == INVALID_ID) return;
    setActive(currentId(), false);
    if (!_inactive->isChecked())
	slotRefresh();
}

void
ActiveList::slotActivities()
{
    _activities->clear();
    if (isActive(currentId()))
	_activities->insertItem(tr("Set inactive"), this,
				SLOT(slotSetInactive()));
    else
	_activities->insertItem(tr("Set active"), this,
				SLOT(slotSetActive()));
}

void
ActiveList::addToPopup(QPopupMenu* menu)
{
    menu->insertSeparator();
    if (isActive(currentId()))
	menu->insertItem(tr("Set inactive"), this, SLOT(slotSetInactive()));
    else
	menu->insertItem(tr("Set active"), this, SLOT(slotSetActive()));
}
