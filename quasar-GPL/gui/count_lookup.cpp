// $Id: count_lookup.cpp,v 1.1 2004/09/26 21:49:56 bpepers Exp $
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

#include "count_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "count_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "count_master.h"

#include <qlabel.h>
#include <qlayout.h>

CountLookup::CountLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "CountLookup"), activeOnly(true)
{
    _label->setText(tr("Number:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Number"), 10);
    _list->addTextColumn(tr("Description"), 30);
    setCaption(tr("Count Lookup"));
}

CountLookup::CountLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "CountLookup"), activeOnly(true)
{
    _label->setText(tr("Number:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Number"), 10);
    _list->addTextColumn(tr("Description"), 30);
    setCaption(tr("Count Lookup"));
}

CountLookup::~CountLookup()
{
}

QString
CountLookup::lookupById(Id count_id)
{
    Count count;
    if (count_id != INVALID_ID && _quasar->db()->lookup(count_id, count))
	return count.number();
    return "";
}

vector<DataPair>
CountLookup::lookupByText(const QString& text)
{
    CountSelect conditions;
    if (!text.isEmpty())
	conditions.number = text + "%";
    conditions.activeOnly = activeOnly;

    vector<Count> counts;
    _quasar->db()->select(counts, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < counts.size(); ++i) {
	data.push_back(DataPair(counts[i].id(), counts[i].number()));
    }

    return data;
}

void
CountLookup::refresh()
{
    CountSelect conditions;
    if (!text().isEmpty())
	conditions.number = text() + "%";
    conditions.activeOnly = activeOnly;

    vector<Count> counts;
    _quasar->db()->select(counts, conditions);

    _list->clear();
    for (unsigned int i = 0; i < counts.size(); ++i) {
	Count& count = counts[i];

	ListViewItem* item = new ListViewItem(_list, count.id());
	item->setText(0, count.number());
	item->setText(1, count.description());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
CountLookup::slotNew()
{
    CountMaster* window = new CountMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
CountLookup::slotEdit(Id id)
{
    CountMaster* window = new CountMaster(_main, id);
    window->show();
    reject();
    return window;
}
