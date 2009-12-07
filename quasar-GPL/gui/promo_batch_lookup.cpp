// $Id: promo_batch_lookup.cpp,v 1.1 2004/10/15 05:18:17 bpepers Exp $
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

#include "promo_batch_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "promo_batch_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "promo_batch_master.h"

#include <qlabel.h>
#include <qlayout.h>

PromoBatchLookup::PromoBatchLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "PromoBatchLookup"), activeOnly(true)
{
    _label->setText(tr("Number:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Number"), 10);
    _list->addTextColumn(tr("Description"), 30);
    setCaption(tr("Promotion Lookup"));
}

PromoBatchLookup::PromoBatchLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "PromoBatchLookup"), activeOnly(true)
{
    _label->setText(tr("Number:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Number"), 10);
    _list->addTextColumn(tr("Description"), 30);
    setCaption(tr("Promotion Lookup"));
}

PromoBatchLookup::~PromoBatchLookup()
{
}

QString
PromoBatchLookup::lookupById(Id batch_id)
{
    PromoBatch batch;
    if (batch_id != INVALID_ID && _quasar->db()->lookup(batch_id, batch))
	return batch.number();
    return "";
}

vector<DataPair>
PromoBatchLookup::lookupByText(const QString& text)
{
    PromoBatchSelect conditions;
    conditions.number = text + "%";
    conditions.activeOnly = activeOnly;

    vector<PromoBatch> batches;
    _quasar->db()->select(batches, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < batches.size(); ++i) {
	data.push_back(DataPair(batches[i].id(), batches[i].number()));
    }

    return data;
}

void
PromoBatchLookup::refresh()
{
    PromoBatchSelect conditions;
    if (!text().isEmpty())
	conditions.number = text() + "%";
    conditions.activeOnly = activeOnly;

    vector<PromoBatch> batches;
    _quasar->db()->select(batches, conditions);

    _list->clear();
    for (unsigned int i = 0; i < batches.size(); ++i) {
	PromoBatch& batch = batches[i];

	ListViewItem* item = new ListViewItem(_list, batch.id());
	item->setText(0, batch.number());
	item->setText(1, batch.description());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
PromoBatchLookup::slotNew()
{
    PromoBatchMaster* window = new PromoBatchMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
PromoBatchLookup::slotEdit(Id id)
{
    PromoBatchMaster* window = new PromoBatchMaster(_main, id);
    window->show();
    reject();
    return window;
}
