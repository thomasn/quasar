// $Id: security_type_lookup.cpp,v 1.6 2004/08/08 23:08:22 bpepers Exp $
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

#include "security_type_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "security_type_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "security_type_master.h"

#include <qlabel.h>
#include <qlayout.h>

SecurityTypeLookup::SecurityTypeLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "SecurityTypeLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(30);
    _list->addTextColumn(tr("Name"), 30);
    setCaption(tr("Security Type Lookup"));
}

SecurityTypeLookup::SecurityTypeLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "SecurityTypeLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(30);
    _list->addTextColumn(tr("Name"), 30);
    setCaption(tr("Security Type Lookup"));
}

SecurityTypeLookup::~SecurityTypeLookup()
{
}

QString
SecurityTypeLookup::lookupById(Id type_id)
{
    SecurityType type;
    if (type_id != INVALID_ID && _quasar->db()->lookup(type_id, type))
	return type.name();
    return "";
}

vector<DataPair>
SecurityTypeLookup::lookupByText(const QString& text)
{
    SecurityTypeSelect conditions;
    if (!text.isEmpty())
	conditions.name = text + "%";
    conditions.activeOnly = activeOnly;

    vector<SecurityType> types;
    _quasar->db()->select(types, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < types.size(); ++i) {
	data.push_back(DataPair(types[i].id(), types[i].name()));
    }

    return data;
}

void
SecurityTypeLookup::refresh()
{
    SecurityTypeSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.activeOnly = activeOnly;

    vector<SecurityType> types;
    _quasar->db()->select(types, conditions);

    _list->clear();
    for (unsigned int i = 0; i < types.size(); ++i) {
	SecurityType& type = types[i];

	ListViewItem* item = new ListViewItem(_list, type.id());
	item->setText(0, type.name());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
SecurityTypeLookup::slotNew()
{
    SecurityTypeMaster* window = new SecurityTypeMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
SecurityTypeLookup::slotEdit(Id id)
{
    SecurityTypeMaster* window = new SecurityTypeMaster(_main, id);
    window->show();
    reject();
    return window;
}
