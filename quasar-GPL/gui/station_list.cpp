// $Id: station_list.cpp,v 1.7 2004/01/31 01:50:31 arandell Exp $
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

#include "station_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "station_select.h"
#include "station_master.h"
#include "grid.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

StationList::StationList(MainWindow* main)
    : ActiveList(main, "StationList")
{
    _helpSource = "station_list.html";

    _list->addTextColumn(tr("Name"), 20);
    _list->addTextColumn(tr("Number"), 10);
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Station List"));
    finalize();
}

StationList::~StationList()
{
}

bool
StationList::isActive(Id station_id)
{
    Station station;
    _quasar->db()->lookup(station_id, station);
    return station.isActive();
}

void
StationList::setActive(Id station_id, bool active)
{
    if (station_id == INVALID_ID) return;

    Station station;
    _quasar->db()->lookup(station_id, station);

    Station orig = station;
    _quasar->db()->setActive(station, active);
    _quasar->db()->update(orig, station);
}

void
StationList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::STATION)
	slotRefresh();
}

void
StationList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id station_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    StationSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<Station> stations;
    _quasar->db()->select(stations, conditions);

    for (unsigned int i = 0; i < stations.size(); ++i) {
	Station& station = stations[i];

	ListViewItem* lvi = new ListViewItem(_list, station.id());
	lvi->setValue(0, station.name());
	lvi->setValue(1, station.number());
	if (showInactive) lvi->setValue(2, !station.isActive());
	if (station.id() == station_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
StationList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Stations"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
StationList::performNew()
{
    StationMaster* master = new StationMaster(_main, INVALID_ID);
    master->show();
}

void
StationList::performEdit()
{
    Id station_id = currentId();
    StationMaster* master = new StationMaster(_main, station_id);
    master->show();
}
