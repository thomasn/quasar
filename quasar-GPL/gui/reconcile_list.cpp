// $Id: reconcile_list.cpp,v 1.12 2004/01/31 01:50:31 arandell Exp $
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

#include "reconcile_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "reconcile_select.h"
#include "reconcile_master.h"
#include "percent_valcon.h"
#include "account.h"
#include "grid.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

ReconcileList::ReconcileList(MainWindow* main)
    : ActiveList(main, "ReconcileList")
{
    _helpSource = "reconcile_list.html";

    _list->addTextColumn(tr("Account"), 30);
    _list->addDateColumn(tr("Date"));
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    _inactive->setText(tr("Show reconciled?"));

    setCaption(tr("Reconcile List"));
    finalize();
}

ReconcileList::~ReconcileList()
{
}

bool
ReconcileList::isActive(Id reconcile_id)
{
    Reconcile reconcile;
    _quasar->db()->lookup(reconcile_id, reconcile);
    return reconcile.isActive();
}

void
ReconcileList::setActive(Id reconcile_id, bool active)
{
    if (reconcile_id == INVALID_ID) return;

    Reconcile reconcile;
    _quasar->db()->lookup(reconcile_id, reconcile);

    Reconcile orig = reconcile;
    _quasar->db()->setActive(reconcile, active);
    _quasar->db()->update(orig, reconcile);
}

void
ReconcileList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::RECONCILE)
	slotRefresh();
}

void
ReconcileList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id reconcile_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    ReconcileSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<Reconcile> reconciles;
    _quasar->db()->select(reconciles, conditions);

    for (unsigned int i = 0; i < reconciles.size(); i++) {
	Reconcile& reconcile = reconciles[i];

	Account account;
	_quasar->db()->lookup(reconcile.accountId(), account);

	ListViewItem* lvi = new ListViewItem(_list, reconcile.id());
	lvi->setValue(0, account.name());
	lvi->setValue(1, reconcile.statementDate());
	if (showInactive) lvi->setValue(2, !reconcile.isActive());
	if (reconcile.id() == reconcile_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
ReconcileList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Reconciliations"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
ReconcileList::performNew()
{
    ReconcileMaster* master = new ReconcileMaster(_main, INVALID_ID);
    master->show();
}

void
ReconcileList::performEdit()
{
    Id reconcile_id = currentId();
    ReconcileMaster* master = new ReconcileMaster(_main, reconcile_id);
    master->show();
}
