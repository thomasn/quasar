// $Id: slip_lookup.cpp,v 1.13 2004/08/08 23:08:22 bpepers Exp $
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

#include "slip_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "slip_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "slip_master.h"

#include <qlabel.h>
#include <qlayout.h>

SlipLookup::SlipLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "SlipLookup")
{
    _label->setText(tr("Number:"));
    _text->setLength(14);
    _list->addTextColumn(tr("Number"), 14);
    _list->addDateColumn(tr("Ship Date"));
    setCaption(tr("Slip Lookup"));
}

SlipLookup::SlipLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "SlipLookup")
{
    _label->setText(tr("Number:"));
    _text->setLength(14);
    _list->addTextColumn(tr("Number"), 14);
    _list->addDateColumn(tr("Ship Date"));
    setCaption(tr("Slip Lookup"));
}

SlipLookup::~SlipLookup()
{
}

QString
SlipLookup::lookupById(Id slip_id)
{
    Slip slip;
    if (slip_id != INVALID_ID && _quasar->db()->lookup(slip_id, slip))
	return slip.number();
    return "";
}

vector<DataPair>
SlipLookup::lookupByText(const QString& text)
{
    SlipSelect conditions;
    if (!text.isEmpty())
	conditions.number = text + "%";
    conditions.vendor_id = vendor_id;
    conditions.store_id = store_id;
    conditions.activeOnly = true;

    vector<Slip> slips;
    _quasar->db()->select(slips, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < slips.size(); ++i) {
	data.push_back(DataPair(slips[i].id(), slips[i].number()));
    }

    return data;
}

void
SlipLookup::refresh()
{
    SlipSelect conditions;
    if (!text().isEmpty())
	conditions.number = text() + "%";
    conditions.vendor_id = vendor_id;
    conditions.store_id = store_id;
    conditions.activeOnly = true;

    vector<Slip> slips;
    _quasar->db()->select(slips, conditions);

    _list->clear();
    for (unsigned int i = 0; i < slips.size(); ++i) {
	Slip& slip = slips[i];

	ListViewItem* item = new ListViewItem(_list, slip.id());
	item->setValue(0, slip.number());
	item->setValue(1, slip.shipDate());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
SlipLookup::slotNew()
{
    SlipMaster* window = new SlipMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
SlipLookup::slotEdit(Id id)
{
    SlipMaster* window = new SlipMaster(_main, id);
    window->show();
    reject();
    return window;
}
