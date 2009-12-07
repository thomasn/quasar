// $Id: station_lookup.cpp,v 1.9 2004/02/16 09:24:16 bpepers Exp $
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

#include "station_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "station_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "station_master.h"

#include <qlabel.h>
#include <qlayout.h>

StationLookup::StationLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "StationLookup"), activeOnly(true)

{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    _list->addTextColumn(tr("Number"), 10);
    setCaption(tr("Station Lookup"));
}

StationLookup::StationLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "StationLookup"), activeOnly(true)

{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    _list->addTextColumn(tr("Number"), 10);
    setCaption(tr("Station Lookup"));
}

StationLookup::~StationLookup()
{
}

QString
StationLookup::lookupById(Id station_id)
{
    Station station;
    if (station_id != INVALID_ID && _quasar->db()->lookup(station_id, station))
	return station.name();
    return "";
}

vector<DataPair>
StationLookup::lookupByText(const QString& text)
{
    StationSelect conditions;
    if (!text.isEmpty())
	conditions.name = text + "%";
    conditions.activeOnly = activeOnly;

    vector<Station> stations;
    _quasar->db()->select(stations, conditions);

    if (stations.size() == 0) {
	conditions.name = "";
	conditions.number = text;
	_quasar->db()->select(stations, conditions);
    }

    vector<DataPair> data;
    for (unsigned int i = 0; i < stations.size(); ++i) {
	data.push_back(DataPair(stations[i].id(), stations[i].name()));
    }

    return data;
}

void
StationLookup::refresh()
{
    StationSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.activeOnly = activeOnly;

    vector<Station> stations;
    _quasar->db()->select(stations, conditions);

    _list->clear();
    for (unsigned int i = 0; i < stations.size(); ++i) {
	Station& station = stations[i];

	ListViewItem* item = new ListViewItem(_list, station.id());
	item->setValue(0, station.name());
	item->setValue(1, station.number());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
StationLookup::slotNew()
{
    StationMaster* window = new StationMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
StationLookup::slotEdit(Id id)
{
    StationMaster* window = new StationMaster(_main, id);
    window->show();
    reject();
    return window;
}
