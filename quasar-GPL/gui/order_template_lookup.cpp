// $Id: order_template_lookup.cpp,v 1.6 2004/02/18 20:51:02 bpepers Exp $
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

#include "order_template_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "order_template_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "order_template_master.h"

#include <qlabel.h>
#include <qlayout.h>

OrderTemplateLookup::OrderTemplateLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "OrderTemplateLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 30);
    setCaption(tr("Order Template Lookup"));
}

OrderTemplateLookup::OrderTemplateLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "OrderTemplateLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 30);
    setCaption(tr("Order Template Lookup"));
}

OrderTemplateLookup::~OrderTemplateLookup()
{
}

QString
OrderTemplateLookup::lookupById(Id temp_id)
{
    OrderTemplate temp;
    if (temp_id != INVALID_ID && _quasar->db()->lookup(temp_id, temp))
	return temp.name();
    return "";
}

vector<DataPair>
OrderTemplateLookup::lookupByText(const QString& text)
{
    OrderTemplateSelect conditions;
    if (!text.isEmpty())
	conditions.name = text + "%";
    conditions.activeOnly = activeOnly;

    vector<OrderTemplate> templates;
    _quasar->db()->select(templates, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < templates.size(); ++i) {
	data.push_back(DataPair(templates[i].id(), templates[i].name()));
    }

    return data;
}

void
OrderTemplateLookup::refresh()
{
    OrderTemplateSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.activeOnly = activeOnly;

    vector<OrderTemplate> templates;
    _quasar->db()->select(templates, conditions);

    _list->clear();
    for (unsigned int i = 0; i < templates.size(); ++i) {
	OrderTemplate& temp = templates[i];

	ListViewItem* item = new ListViewItem(_list, temp.id());
	item->setText(0, temp.name());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
OrderTemplateLookup::slotNew()
{
    OrderTemplateMaster* window = new OrderTemplateMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
OrderTemplateLookup::slotEdit(Id id)
{
    OrderTemplateMaster* window = new OrderTemplateMaster(_main, id);
    window->show();
    reject();
    return window;
}
