// $Id: store_list.cpp,v 1.14 2005/03/15 15:34:35 bpepers Exp $
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

#include "store_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "store_select.h"
#include "store_master.h"
#include "company.h"
#include "grid.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

StoreList::StoreList(MainWindow* main)
    : ActiveList(main, "StoreList")
{
    _helpSource = "store_list.html";

    _list->addTextColumn(tr("Name"), 30);
    _list->addTextColumn(tr("Number"), 10);
    _list->addCheckColumn(tr("Sell?"));
    _list->addTextColumn(tr("Company"), 30);
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Store List"));
    finalize();
}

StoreList::~StoreList()
{
}

bool
StoreList::isActive(Id store_id)
{
    Store store;
    _quasar->db()->lookup(store_id, store);
    return store.isActive();
}

void
StoreList::setActive(Id store_id, bool active)
{
    if (store_id == INVALID_ID) return;

    Store store;
    _quasar->db()->lookup(store_id, store);

    Store orig = store;
    _quasar->db()->setActive(store, active);
    _quasar->db()->update(orig, store);
}

void
StoreList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::STORE)
	slotRefresh();
}

void
StoreList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id store_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    StoreSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<Store> stores;
    _quasar->db()->select(stores, conditions);

    for (unsigned int i = 0; i < stores.size(); ++i) {
	const Store& store = stores[i];

	Company company;
	_db->lookup(store.companyId(), company);

	ListViewItem* lvi = new ListViewItem(_list, store.id());
	lvi->setValue(0, store.name());
	lvi->setValue(1, store.number());
	lvi->setValue(2, store.canSell());
	lvi->setValue(3, company.name());
	if (showInactive) lvi->setValue(4, !store.isActive());
	if (store.id() == store_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
StoreList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Stores"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
StoreList::performNew()
{
    StoreMaster* master = new StoreMaster(_main, INVALID_ID);
    master->show();
}

void
StoreList::performEdit()
{
    Id store_id = currentId();
    StoreMaster* master = new StoreMaster(_main, store_id);
    master->show();
}
