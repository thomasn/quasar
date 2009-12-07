// $Id: pat_worksheet_list.cpp,v 1.3 2004/01/31 01:50:30 arandell Exp $
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

#include "pat_worksheet_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "pat_worksheet_select.h"
#include "pat_worksheet_master.h"
#include "grid.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

PatWorksheetList::PatWorksheetList(MainWindow* main)
    : ActiveList(main, "PatWorksheetList")
{
    _helpSource = "pat_worksheet_list.html";

    _list->addTextColumn(tr("Description"), 40);
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Patronage Worksheet List"));
    finalize();
}

PatWorksheetList::~PatWorksheetList()
{
}

bool
PatWorksheetList::isActive(Id worksheet_id)
{
    PatWorksheet worksheet;
    _quasar->db()->lookup(worksheet_id, worksheet);
    return worksheet.isActive();
}

void
PatWorksheetList::setActive(Id worksheet_id, bool active)
{
    if (worksheet_id == INVALID_ID) return;

    PatWorksheet worksheet;
    _quasar->db()->lookup(worksheet_id, worksheet);

    PatWorksheet orig = worksheet;
    _quasar->db()->setActive(worksheet, active);
    _quasar->db()->update(orig, worksheet);
}

void
PatWorksheetList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::PAT_WS)
	slotRefresh();
}

void
PatWorksheetList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id worksheet_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    PatWorksheetSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<PatWorksheet> worksheets;
    _quasar->db()->select(worksheets, conditions);

    for (unsigned int i = 0; i < worksheets.size(); ++i) {
	PatWorksheet& worksheet = worksheets[i];

	ListViewItem* lvi = new ListViewItem(_list, worksheet.id());
	lvi->setText(0, worksheet.description());
	if (showInactive) lvi->setValue(1, !worksheet.isActive());
	if (worksheet.id() == worksheet_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
PatWorksheetList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Patronage Worksheets"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
PatWorksheetList::performNew()
{
    PatWorksheetMaster* master = new PatWorksheetMaster(_main, INVALID_ID);
    master->show();
}

void
PatWorksheetList::performEdit()
{
    Id worksheet_id = currentId();
    PatWorksheetMaster* master = new PatWorksheetMaster(_main, worksheet_id);
    master->show();
}
