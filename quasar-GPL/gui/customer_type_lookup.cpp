// $Id: customer_type_lookup.cpp,v 1.6 2004/02/18 20:51:02 bpepers Exp $
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

#include "customer_type_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "customer_type_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "customer_type_master.h"

#include <qlabel.h>
#include <qlayout.h>

CustomerTypeLookup::CustomerTypeLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "CustomerTypeLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    setCaption(tr("Customer Type Lookup"));
}

CustomerTypeLookup::CustomerTypeLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "CustomerTypeLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    setCaption(tr("Customer Type Lookup"));
}

CustomerTypeLookup::~CustomerTypeLookup()
{
}

QString
CustomerTypeLookup::lookupById(Id type_id)
{
    CustomerType type;
    if (type_id != INVALID_ID && _quasar->db()->lookup(type_id, type))
	return type.name();
    return "";
}

vector<DataPair>
CustomerTypeLookup::lookupByText(const QString& text)
{
    CustomerTypeSelect conditions;
    conditions.name = text + "%";
    conditions.activeOnly = activeOnly;

    vector<CustomerType> types;
    _quasar->db()->select(types, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < types.size(); ++i) {
	data.push_back(DataPair(types[i].id(), types[i].name()));
    }

    return data;
}

void
CustomerTypeLookup::refresh()
{
    CustomerTypeSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.activeOnly = activeOnly;

    vector<CustomerType> types;
    _quasar->db()->select(types, conditions);

    _list->clear();
    for (unsigned int i = 0; i < types.size(); ++i) {
	CustomerType& type = types[i];

	ListViewItem* item = new ListViewItem(_list, type.id());
	item->setText(0, type.name());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
CustomerTypeLookup::slotNew()
{
    CustomerTypeMaster* window = new CustomerTypeMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
CustomerTypeLookup::slotEdit(Id id)
{
    CustomerTypeMaster* window = new CustomerTypeMaster(_main, id);
    window->show();
    reject();
    return window;
}
