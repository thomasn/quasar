// $Id: group_lookup.cpp,v 1.9 2004/02/16 09:24:16 bpepers Exp $
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

#include "group_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "group_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "group_master.h"

#include <qlabel.h>
#include <qlayout.h>

GroupLookup::GroupLookup(MainWindow* main, QWidget* parent, int matchType)
    : QuasarLookup(main, parent, "GroupLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    _list->addTextColumn(tr("Type"), 20);
    setCaption(tr("Group Lookup"));

    QLabel* typeLabel = new QLabel(tr("Type:"), _search);
    type = new ComboBox(_search);
    typeLabel->setBuddy(type);

    setTabOrder(_text, type);
    _searchGrid->addWidget(typeLabel, 1, 0);
    _searchGrid->addWidget(type, 1, 1, AlignLeft | AlignVCenter);

    type->insertItem(tr("All Types"));
    type->insertItem(tr("All Card Types"));
    type->insertItem(Group::typeName(Group::ACCOUNT));
    type->insertItem(Group::typeName(Group::ITEM));
    type->insertItem(Group::typeName(Group::CUSTOMER));
    type->insertItem(Group::typeName(Group::VENDOR));
    type->insertItem(Group::typeName(Group::EMPLOYEE));
    type->insertItem(Group::typeName(Group::PERSONAL));

    if (matchType != -1)
	type->setCurrentItem(Group::typeName(matchType));
}

GroupLookup::GroupLookup(QuasarClient* quasar, QWidget* parent, int matchType)
    : QuasarLookup(quasar, parent, "GroupLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    _list->addTextColumn(tr("Type"), 20);
    setCaption(tr("Group Lookup"));

    QLabel* typeLabel = new QLabel(tr("Type:"), _search);
    type = new ComboBox(_search);
    typeLabel->setBuddy(type);

    setTabOrder(_text, type);
    _searchGrid->addWidget(typeLabel, 1, 0);
    _searchGrid->addWidget(type, 1, 1, AlignLeft | AlignVCenter);

    type->insertItem(tr("All Types"));
    type->insertItem(tr("All Card Types"));
    type->insertItem(Group::typeName(Group::ACCOUNT));
    type->insertItem(Group::typeName(Group::ITEM));
    type->insertItem(Group::typeName(Group::CUSTOMER));
    type->insertItem(Group::typeName(Group::VENDOR));
    type->insertItem(Group::typeName(Group::EMPLOYEE));
    type->insertItem(Group::typeName(Group::PERSONAL));

    if (matchType != -1)
	type->setCurrentItem(Group::typeName(matchType));
}

GroupLookup::~GroupLookup()
{
}

QString
GroupLookup::lookupById(Id group_id)
{
    Group group;
    if (group_id != INVALID_ID && _quasar->db()->lookup(group_id, group))
	return group.name();
    return "";
}

vector<DataPair>
GroupLookup::lookupByText(const QString& text)
{
    GroupSelect conditions;
    if (!text.isEmpty())
	conditions.name = text + "%";
    conditions.type = matchType();
    conditions.activeOnly = activeOnly;

    vector<Group> groups;
    _quasar->db()->select(groups, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < groups.size(); ++i) {
	if (type->currentText() == tr("All Card Types")) {
	    switch (groups[i].type()) {
	    case Group::CUSTOMER:
	    case Group::EMPLOYEE:
	    case Group::VENDOR:
	    case Group::PERSONAL:
		break;
	    default:
		continue;
	    }
	}
	data.push_back(DataPair(groups[i].id(), groups[i].name()));
    }

    return data;
}

void
GroupLookup::refresh()
{
    GroupSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.type = matchType();
    conditions.activeOnly = activeOnly;

    vector<Group> groups;
    _quasar->db()->select(groups, conditions);

    _list->clear();
    for (unsigned int i = 0; i < groups.size(); ++i) {
	Group& group = groups[i];

	if (type->currentText() == tr("All Card Types")) {
	    switch (group.type()) {
	    case Group::CUSTOMER:
	    case Group::EMPLOYEE:
	    case Group::VENDOR:
	    case Group::PERSONAL:
		break;
	    default:
		continue;
	    }
	}

	ListViewItem* item = new ListViewItem(_list, group.id());
	item->setValue(0, group.name());
	item->setValue(1, group.typeName());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
GroupLookup::slotNew()
{
    GroupMaster* window = new GroupMaster(_main);
    window->setType(matchType());
    window->show();
    reject();
    return window;
}

QWidget*
GroupLookup::slotEdit(Id id)
{
    GroupMaster* window = new GroupMaster(_main, id);
    window->setType(matchType());
    window->show();
    reject();
    return window;
}

int
GroupLookup::matchType()
{
    if (type->currentText() == tr("All Types"))
	return -1;
    if (type->currentText() == tr("All Card Types"))
	return -1;
    return Group::type(type->currentText());
}
