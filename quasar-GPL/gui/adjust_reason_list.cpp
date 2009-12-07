// $Id: adjust_reason_list.cpp,v 1.2 2005/01/09 20:22:28 bpepers Exp $
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

#include "adjust_reason_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "adjust_reason_select.h"
#include "adjust_reason_master.h"
#include "grid.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

AdjustReasonList::AdjustReasonList(MainWindow* main)
    : ActiveList(main, "AdjustReasonList")
{
    _helpSource = "adjust_reason_list.html";

    _list->addTextColumn(tr("Name"), 14);
    _list->addTextColumn(tr("Number"), 10);
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Adjustment Reason List"));
    finalize();
}

AdjustReasonList::~AdjustReasonList()
{
}

bool
AdjustReasonList::isActive(Id reason_id)
{
    AdjustReason reason;
    _quasar->db()->lookup(reason_id, reason);
    return reason.isActive();
}

void
AdjustReasonList::setActive(Id reason_id, bool active)
{
    if (reason_id == INVALID_ID) return;

    AdjustReason reason;
    _quasar->db()->lookup(reason_id, reason);

    AdjustReason orig = reason;
    _quasar->db()->setActive(reason, active);
    _quasar->db()->update(orig, reason);
}

void
AdjustReasonList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::ADJUST_REASON)
	slotRefresh();
}

void
AdjustReasonList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id reason_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    AdjustReasonSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<AdjustReason> reasons;
    _quasar->db()->select(reasons, conditions);

    for (unsigned int i = 0; i < reasons.size(); ++i) {
	AdjustReason& reason = reasons[i];

	ListViewItem* lvi = new ListViewItem(_list, reason.id());
	lvi->setText(0, reason.name());
	lvi->setText(1, reason.number());
	if (showInactive) lvi->setValue(2, !reason.isActive());
	if (reason.id() == reason_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
AdjustReasonList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Adjustment Reasons"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
AdjustReasonList::performNew()
{
    AdjustReasonMaster* master = new AdjustReasonMaster(_main, INVALID_ID);
    master->show();
}

void
AdjustReasonList::performEdit()
{
    Id reason_id = currentId();
    AdjustReasonMaster* master = new AdjustReasonMaster(_main, reason_id);
    master->show();
}
