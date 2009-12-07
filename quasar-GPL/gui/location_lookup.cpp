// $Id: location_lookup.cpp,v 1.14 2004/02/16 09:24:16 bpepers Exp $
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

#include "location_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "location_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "location_master.h"

#include <qlabel.h>
#include <qlayout.h>

LocationLookup::LocationLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "LocationLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(30);
    _list->addTextColumn(tr("Name"), 30);
    setCaption(tr("Location Lookup"));
}

LocationLookup::LocationLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "LocationLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(30);
    _list->addTextColumn(tr("Name"), 30);
    setCaption(tr("Location Lookup"));
}

LocationLookup::~LocationLookup()
{
}

QString
LocationLookup::lookupById(Id loc_id)
{
    Location location;
    if (loc_id != INVALID_ID && _quasar->db()->lookup(loc_id, location))
	return location.name();
    return "";
}

vector<DataPair>
LocationLookup::lookupByText(const QString& text)
{
    LocationSelect conditions;
    if (!text.isEmpty())
	conditions.name = text + "%";
    conditions.store_id = store_id;
    conditions.activeOnly = activeOnly;

    vector<Location> locations;
    _quasar->db()->select(locations, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < locations.size(); ++i) {
	data.push_back(DataPair(locations[i].id(), locations[i].name()));
    }

    return data;
}

void
LocationLookup::refresh()
{
    LocationSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.store_id = store_id;
    conditions.activeOnly = activeOnly;

    vector<Location> locations;
    _quasar->db()->select(locations, conditions);

    _list->clear();
    for (unsigned int i = 0; i < locations.size(); ++i) {
	Location& location = locations[i];

	ListViewItem* item = new ListViewItem(_list, location.id());
	item->setText(0, location.name());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
LocationLookup::slotNew()
{
    LocationMaster* window = new LocationMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
LocationLookup::slotEdit(Id id)
{
    LocationMaster* window = new LocationMaster(_main, id);
    window->show();
    reject();
    return window;
}
