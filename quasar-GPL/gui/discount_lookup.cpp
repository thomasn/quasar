// $Id: discount_lookup.cpp,v 1.10 2004/02/18 20:51:02 bpepers Exp $
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

#include "discount_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "discount_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "discount_master.h"

#include <qlabel.h>
#include <qlayout.h>

DiscountLookup::DiscountLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "DiscountLookup"), activeOnly(true),
      lineOnly(false), txOnly(false)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    setCaption(tr("Discount Lookup"));
}

DiscountLookup::DiscountLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "DiscountLookup"), activeOnly(true),
      lineOnly(false), txOnly(false)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    setCaption(tr("Discount Lookup"));
}

DiscountLookup::~DiscountLookup()
{
}

QString
DiscountLookup::lookupById(Id disc_id)
{
    Discount discount;
    if (disc_id != INVALID_ID && _quasar->db()->lookup(disc_id, discount))
	return discount.name();
    return "";
}

vector<DataPair>
DiscountLookup::lookupByText(const QString& text)
{
    DiscountSelect conditions;
    if (!text.isEmpty())
	conditions.name = text + "%";
    conditions.lineOnly = lineOnly;
    conditions.txOnly = txOnly;
    conditions.activeOnly = activeOnly;

    vector<Discount> discounts;
    _quasar->db()->select(discounts, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < discounts.size(); ++i) {
	data.push_back(DataPair(discounts[i].id(), discounts[i].name()));
    }

    return data;
}

void
DiscountLookup::refresh()
{
    DiscountSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.lineOnly = lineOnly;
    conditions.txOnly = txOnly;
    conditions.activeOnly = activeOnly;

    vector<Discount> discounts;
    _quasar->db()->select(discounts, conditions);

    _list->clear();
    for (unsigned int i = 0; i < discounts.size(); ++i) {
	Discount& discount = discounts[i];

	ListViewItem* item = new ListViewItem(_list, discount.id());
	item->setText(0, discount.name());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
DiscountLookup::slotNew()
{
    DiscountMaster* window = new DiscountMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
DiscountLookup::slotEdit(Id id)
{
    DiscountMaster* window = new DiscountMaster(_main, id);
    window->show();
    reject();
    return window;
}
