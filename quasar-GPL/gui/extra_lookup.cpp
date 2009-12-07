// $Id: extra_lookup.cpp,v 1.11 2004/01/31 01:50:30 arandell Exp $
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

#include "extra_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "extra.h"
#include "extra_select.h"
#include "list_view_item.h"
#include "line_edit.h"

#include <qlabel.h>
#include <qlayout.h>

ExtraLookup::ExtraLookup(MainWindow* main, QWidget* parent, const QString& tbl)
    : QuasarLookup(main, parent, "ExtraLookup"), activeOnly(true)
{
    _label->setText(tr("Data Name:"));
    _text->setLength(30);
    _list->addTextColumn(tr("Data Name"), 30);
    _list->addTextColumn(tr("Table Name"), 16);
    setCaption(tr("Extra Data Lookup"));
    setAllowNew(false);
    setAllowEdit(false);

    QLabel* tableLabel = new QLabel(tr("Table Name:"), _search);
    table = new LineEdit(_search);
    table->setLength(16);
    table->setEnabled(false);
    table->setText(tbl);
    tableLabel->setBuddy(table);

    setTabOrder(_text, table);
    _searchGrid->addWidget(tableLabel, 1, 0);
    _searchGrid->addWidget(table, 1, 1, AlignLeft | AlignVCenter);
}

ExtraLookup::~ExtraLookup()
{
}

QString
ExtraLookup::lookupById(Id extra_id)
{
    Extra extra;
    if (extra_id != INVALID_ID && _quasar->db()->lookup(extra_id, extra))
	return extra.name();
    return "";
}

vector<DataPair>
ExtraLookup::lookupByText(const QString& text)
{
    ExtraSelect conditions;
    if (!text.isEmpty())
	conditions.name = text + "%";
    conditions.table = table->text();
    conditions.activeOnly = activeOnly;

    vector<Extra> extras;
    _quasar->db()->select(extras, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < extras.size(); ++i) {
	data.push_back(DataPair(extras[i].id(), extras[i].name()));
    }

    return data;
}

void
ExtraLookup::refresh()
{
    ExtraSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.table = table->text();
    conditions.activeOnly = activeOnly;

    vector<Extra> extras;
    _quasar->db()->select(extras, conditions);

    _list->clear();
    for (unsigned int i = 0; i < extras.size(); ++i) {
	Extra& extra = extras[i];

	ListViewItem* item = new ListViewItem(_list, extra.id());
	item->setValue(0, extra.name());
	item->setValue(1, extra.table());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
ExtraLookup::slotNew()
{
    return NULL;
}

QWidget*
ExtraLookup::slotEdit(Id)
{
    return NULL;
}
