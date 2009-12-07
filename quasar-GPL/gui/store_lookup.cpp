// $Id: store_lookup.cpp,v 1.12 2004/02/16 09:24:16 bpepers Exp $
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

#include "store_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "store_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "store_master.h"

#include <qlabel.h>
#include <qlayout.h>

StoreLookup::StoreLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "StoreLookup"), activeOnly(true),
      sellOnly(false)
{
    _label->setText(tr("Name:"));
    _text->setLength(30);
    _list->addTextColumn(tr("Name"), 30);
    _list->addTextColumn(tr("Number"), 10);
    setCaption(tr("Store Lookup"));
}

StoreLookup::StoreLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "StoreLookup"), activeOnly(true),
      sellOnly(false)
{
    _label->setText(tr("Name:"));
    _text->setLength(30);
    _list->addTextColumn(tr("Name"), 30);
    _list->addTextColumn(tr("Number"), 10);
    setCaption(tr("Store Lookup"));
}

StoreLookup::~StoreLookup()
{
}

QString
StoreLookup::lookupById(Id store_id)
{
    Store loc;
    if (store_id != INVALID_ID && _quasar->db()->lookup(store_id, loc))
	return loc.name();
    return "";
}

vector<DataPair>
StoreLookup::lookupByText(const QString& text)
{
    StoreSelect conditions;
    if (!text.isEmpty())
	conditions.name = text + "%";
    conditions.sellOnly = sellOnly;
    conditions.activeOnly = activeOnly;
    vector<Store> stores;

    _quasar->db()->select(stores, conditions);
    if (stores.size() == 0 && !text.isEmpty()) {
	conditions.name = "";
	conditions.number = text;
	_quasar->db()->select(stores, conditions);
    }

    vector<DataPair> data;
    for (unsigned int i = 0; i < stores.size(); ++i) {
	data.push_back(DataPair(stores[i].id(), stores[i].name()));
    }

    return data;
}

void
StoreLookup::refresh()
{
    StoreSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.sellOnly = sellOnly;
    conditions.activeOnly = activeOnly;

    vector<Store> stores;
    _quasar->db()->select(stores, conditions);

    _list->clear();
    for (unsigned int i = 0; i < stores.size(); ++i) {
	Store& store = stores[i];

	ListViewItem* item = new ListViewItem(_list, store.id());
	item->setText(0, store.name());
	item->setText(1, store.number());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
StoreLookup::slotNew()
{
    StoreMaster* window = new StoreMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
StoreLookup::slotEdit(Id id)
{
    StoreMaster* window = new StoreMaster(_main, id);
    window->show();
    reject();
    return window;
}
