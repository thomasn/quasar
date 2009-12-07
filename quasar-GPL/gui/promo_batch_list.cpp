// $Id: promo_batch_list.cpp,v 1.1 2004/10/08 08:44:32 bpepers Exp $
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

#include "promo_batch_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "promo_batch_select.h"
#include "promo_batch_master.h"
#include "grid.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

PromoBatchList::PromoBatchList(MainWindow* main)
    : ActiveList(main, "PromoBatchList")
{
    _helpSource = "promo_batch_list.html";

    _list->addTextColumn(tr("Number"), 10);
    _list->addTextColumn(tr("Description"), 40);
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Promotion Batch List"));
    finalize();
}

PromoBatchList::~PromoBatchList()
{
}

bool
PromoBatchList::isActive(Id batch_id)
{
    PromoBatch batch;
    _quasar->db()->lookup(batch_id, batch);
    return batch.isActive();
}

void
PromoBatchList::setActive(Id batch_id, bool active)
{
    if (batch_id == INVALID_ID) return;

    PromoBatch batch;
    _quasar->db()->lookup(batch_id, batch);

    PromoBatch orig = batch;
    _quasar->db()->setActive(batch, active);
    _quasar->db()->update(orig, batch);
}

void
PromoBatchList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::PROMO_BATCH)
	slotRefresh();
}

void
PromoBatchList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id batch_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    PromoBatchSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<PromoBatch> batches;
    _quasar->db()->select(batches, conditions);

    for (unsigned int i = 0; i < batches.size(); i++) {
	PromoBatch& batch = batches[i];

	ListViewItem* lvi = new ListViewItem(_list, batch.id());
	lvi->setText(0, batch.number());
	lvi->setText(1, batch.description());
	if (showInactive) lvi->setValue(2, !batch.isActive());
	if (batch.id() == batch_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
PromoBatchList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Promo Batches"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
PromoBatchList::performNew()
{
    PromoBatchMaster* master = new PromoBatchMaster(_main);
    master->show();
}

void
PromoBatchList::performEdit()
{
    Id batch_id = currentId();
    PromoBatchMaster* master = new PromoBatchMaster(_main, batch_id);
    master->show();
}
