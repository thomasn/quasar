// $Id: label_batch_list.cpp,v 1.1 2004/03/14 01:36:24 bpepers Exp $
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

#include "label_batch_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "label_batch_select.h"
#include "label_batch_master.h"
#include "grid.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

LabelBatchList::LabelBatchList(MainWindow* main)
    : ActiveList(main, "LabelBatchList")
{
    _helpSource = "label_batch_list.html";

    _list->addTextColumn(tr("Number"), 10);
    _list->addTextColumn(tr("Description"), 40);
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Label Batch List"));
    finalize();
}

LabelBatchList::~LabelBatchList()
{
}

bool
LabelBatchList::isActive(Id batch_id)
{
    LabelBatch batch;
    _quasar->db()->lookup(batch_id, batch);
    return batch.isActive();
}

void
LabelBatchList::setActive(Id batch_id, bool active)
{
    if (batch_id == INVALID_ID) return;

    LabelBatch batch;
    _quasar->db()->lookup(batch_id, batch);

    LabelBatch orig = batch;
    _quasar->db()->setActive(batch, active);
    _quasar->db()->update(orig, batch);
}

void
LabelBatchList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::LABEL_BATCH)
	slotRefresh();
}

void
LabelBatchList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id batch_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    LabelBatchSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<LabelBatch> batches;
    _quasar->db()->select(batches, conditions);

    for (unsigned int i = 0; i < batches.size(); i++) {
	LabelBatch& batch = batches[i];

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
LabelBatchList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Label Batches"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
LabelBatchList::performNew()
{
    LabelBatchMaster* master = new LabelBatchMaster(_main);
    master->show();
}

void
LabelBatchList::performEdit()
{
    Id batch_id = currentId();
    LabelBatchMaster* master = new LabelBatchMaster(_main, batch_id);
    master->show();
}
