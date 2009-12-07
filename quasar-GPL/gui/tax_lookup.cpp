// $Id: tax_lookup.cpp,v 1.21 2005/06/08 07:32:25 bpepers Exp $
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

#include "tax_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "tax_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "tax_master.h"

#include <qlabel.h>
#include <qlayout.h>

TaxLookup::TaxLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "TaxLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    setCaption(tr("Tax Lookup"));
}

TaxLookup::TaxLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "TaxLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    setCaption(tr("Tax Lookup"));
}

TaxLookup::~TaxLookup()
{
}

QString
TaxLookup::lookupById(Id tax_id)
{
    Tax tax;
    if (tax_id != INVALID_ID && _quasar->db()->lookup(tax_id, tax))
	return tax.name();
    return "";
}

vector<DataPair>
TaxLookup::lookupByText(const QString& text)
{
    TaxSelect conditions;
    if (!text.isEmpty())
	conditions.name = text + "%";
    conditions.activeOnly = activeOnly;

    vector<Tax> taxes;
    _quasar->db()->select(taxes, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < taxes.size(); ++i) {
	data.push_back(DataPair(taxes[i].id(), taxes[i].name()));
    }

    return data;
}

void
TaxLookup::refresh()
{
    TaxSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.activeOnly = activeOnly;

    vector<Tax> taxes;
    _quasar->db()->select(taxes, conditions);

    _list->clear();
    for (unsigned int i = 0; i < taxes.size(); ++i) {
	Tax& tax = taxes[i];

	ListViewItem* item = new ListViewItem(_list, tax.id());
	item->setText(0, tax.name());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
TaxLookup::slotNew()
{
    TaxMaster* window = new TaxMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
TaxLookup::slotEdit(Id id)
{
    TaxMaster* window = new TaxMaster(_main, id);
    window->show();
    reject();
    return window;
}
