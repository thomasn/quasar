// $Id: pat_group_lookup.cpp,v 1.5 2004/12/06 09:24:15 bpepers Exp $
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

#include "pat_group_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "pat_group_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "pat_group_master.h"

#include <qlabel.h>
#include <qlayout.h>

PatGroupLookup::PatGroupLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "PatGroupLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    _list->addTextColumn(tr("Number"), 12);
    setCaption(tr("Patronage Group Lookup"));
}

PatGroupLookup::PatGroupLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "PatGroupLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    _list->addTextColumn(tr("Number"), 12);
    setCaption(tr("Patronage Group Lookup"));
}

PatGroupLookup::~PatGroupLookup()
{
}

QString
PatGroupLookup::lookupById(Id group_id)
{
    PatGroup group;
    if (group_id != INVALID_ID && _quasar->db()->lookup(group_id, group))
	return group.name();
    return "";
}

vector<DataPair>
PatGroupLookup::lookupByText(const QString& text)
{
    PatGroupSelect conditions;
    if (!text.isEmpty())
	conditions.name = text + "%";
    conditions.activeOnly = activeOnly;

    vector<PatGroup> groups;
    _quasar->db()->select(groups, conditions);
    if (groups.size() == 0 && !text.isEmpty()) {
	conditions.name = "";
	conditions.number = text;
	_quasar->db()->select(groups, conditions);
    }

    vector<DataPair> data;
    for (unsigned int i = 0; i < groups.size(); ++i) {
	data.push_back(DataPair(groups[i].id(), groups[i].name()));
    }

    return data;
}

void
PatGroupLookup::refresh()
{
    PatGroupSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.activeOnly = activeOnly;

    vector<PatGroup> groups;
    _quasar->db()->select(groups, conditions);

    _list->clear();
    for (unsigned int i = 0; i < groups.size(); ++i) {
	PatGroup& group = groups[i];

	ListViewItem* item = new ListViewItem(_list, group.id());
	item->setValue(0, group.name());
	item->setValue(1, group.number());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
PatGroupLookup::slotNew()
{
    PatGroupMaster* window = new PatGroupMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
PatGroupLookup::slotEdit(Id id)
{
    PatGroupMaster* window = new PatGroupMaster(_main, id);
    window->show();
    reject();
    return window;
}
