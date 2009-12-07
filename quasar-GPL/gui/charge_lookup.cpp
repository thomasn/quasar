// $Id: charge_lookup.cpp,v 1.10 2004/02/18 20:51:02 bpepers Exp $
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

#include "charge_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "charge_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "charge_master.h"

#include <qlabel.h>
#include <qlayout.h>

ChargeLookup::ChargeLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "ChargeLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    setCaption(tr("Charge Lookup"));
}

ChargeLookup::ChargeLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "ChargeLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    setCaption(tr("Charge Lookup"));
}

ChargeLookup::~ChargeLookup()
{
}

QString
ChargeLookup::lookupById(Id charge_id)
{
    Charge charge;
    if (charge_id != INVALID_ID && _quasar->db()->lookup(charge_id, charge))
	return charge.name();
    return "";
}

vector<DataPair>
ChargeLookup::lookupByText(const QString& text)
{
    ChargeSelect conditions;
    if (!text.isEmpty())
	conditions.name = text + "%";
    conditions.activeOnly = activeOnly;

    vector<Charge> charges;
    _quasar->db()->select(charges, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < charges.size(); ++i) {
	data.push_back(DataPair(charges[i].id(), charges[i].name()));
    }

    return data;
}

void
ChargeLookup::refresh()
{
    ChargeSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.activeOnly = activeOnly;

    vector<Charge> charges;
    _quasar->db()->select(charges, conditions);

    _list->clear();
    for (unsigned int i = 0; i < charges.size(); ++i) {
	Charge& charge = charges[i];

	ListViewItem* item = new ListViewItem(_list, charge.id());
	item->setText(0, charge.name());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
ChargeLookup::slotNew()
{
    ChargeMaster* window = new ChargeMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
ChargeLookup::slotEdit(Id id)
{
    ChargeMaster* window = new ChargeMaster(_main, id);
    window->show();
    reject();
    return window;
}
