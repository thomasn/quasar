// $Id: tender_list.cpp,v 1.35 2004/01/31 01:50:31 arandell Exp $
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

#include "tender_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "tender_select.h"
#include "tender_master.h"
#include "grid.h"

#include <qapplication.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

TenderList::TenderList(MainWindow* main)
    : ActiveList(main, "TenderList")
{
    _helpSource = "tender_list.html";

    _list->addTextColumn(tr("Name"), 20);
    _list->addTextColumn(tr("Type"), 10);
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Tender List"));
    finalize();
    slotRefresh();
}

TenderList::~TenderList()
{
}

bool
TenderList::isActive(Id tender_id)
{
    Tender tender;
    _quasar->db()->lookup(tender_id, tender);
    return tender.isActive();
}

void
TenderList::setActive(Id tender_id, bool active)
{
    if (tender_id == INVALID_ID) return;

    Tender tender;
    _quasar->db()->lookup(tender_id, tender);

    Tender orig = tender;
    _quasar->db()->setActive(tender, active);
    _quasar->db()->update(orig, tender);
}

void
TenderList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::TENDER)
	slotRefresh();
}

void
TenderList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id tender_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    TenderSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<Tender> tenders;
    _quasar->db()->select(tenders, conditions);

    for (unsigned int i = 0; i < tenders.size(); i++) {
	Tender& tender = tenders[i];

	ListViewItem* lvi = new ListViewItem(_list, tender.id());
	lvi->setValue(0, tender.name());
	lvi->setValue(1, tender.typeName());
	if (showInactive) lvi->setValue(2, !tender.isActive());

	if (tender.id() == tender_id)
	    current = lvi;
    }

    if (current == NULL)
	current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
TenderList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Tenders"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
TenderList::performNew()
{
    TenderMaster* master = new TenderMaster(_main, INVALID_ID);
    master->show();
}

void
TenderList::performEdit()
{
    Id tender_id = currentId();
    TenderMaster* master = new TenderMaster(_main, tender_id);
    master->show();
}
