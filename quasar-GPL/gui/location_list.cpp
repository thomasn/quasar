// $Id: location_list.cpp,v 1.16 2004/01/31 01:50:30 arandell Exp $
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

#include "location_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "location_select.h"
#include "location_master.h"
#include "lookup_edit.h"
#include "store_lookup.h"
#include "grid.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>

LocationList::LocationList(MainWindow* main)
    : ActiveList(main, "LocationList")
{
    _helpSource = "location_list.html";

    _list->addTextColumn(tr("Name"), 30);
    _list->addTextColumn(tr("Store"), 30);
    _list->setSorting(0);

    QLabel* storeLabel = new QLabel(tr("&Store:"), _search);
    _store = new LookupEdit(new StoreLookup(_main, this), _search);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    QGridLayout* grid = new QGridLayout(_search);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(2, 1);
    grid->addWidget(storeLabel, 0, 0);
    grid->addWidget(_store, 0, 1, AlignLeft | AlignVCenter);

    _store->setId(_quasar->defaultStore());
    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Location List"));
    finalize(false);
    _store->setFocus();
}

LocationList::~LocationList()
{
}

bool
LocationList::isActive(Id location_id)
{
    Location location;
    _quasar->db()->lookup(location_id, location);
    return location.isActive();
}

void
LocationList::setActive(Id location_id, bool active)
{
    if (location_id == INVALID_ID) return;

    Location location;
    _quasar->db()->lookup(location_id, location);

    Location orig = location;
    _quasar->db()->setActive(location, active);
    _quasar->db()->update(orig, location);
}

void
LocationList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::LOCATION)
	slotRefresh();
}

void
LocationList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id location_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    LocationSelect conditions;
    conditions.activeOnly = !showInactive;
    conditions.store_id = _store->getId();
    vector<Location> locations;
    _quasar->db()->select(locations, conditions);

    for (unsigned int i = 0; i < locations.size(); ++i) {
	Location& location = locations[i];

	Store store;
	findStore(location.storeId(), store);

	ListViewItem* lvi = new ListViewItem(_list, location.id());
	lvi->setValue(0, location.name());
	lvi->setValue(1, store.name());
	if (showInactive) lvi->setValue(2, !location.isActive());
	if (location.id() == location_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
LocationList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Locations"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
LocationList::performNew()
{
    LocationMaster* master = new LocationMaster(_main, INVALID_ID);
    master->show();
}

void
LocationList::performEdit()
{
    Id location_id = currentId();
    LocationMaster* master = new LocationMaster(_main, location_id);
    master->show();
}

bool
LocationList::findStore(Id store_id, Store& store)
{
    for (unsigned int i = 0; i < _stores.size(); ++i) {
	if (_stores[i].id() == store_id) {
	    store = _stores[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(store_id, store))
	return false;

    _stores.push_back(store);
    return true;
}
