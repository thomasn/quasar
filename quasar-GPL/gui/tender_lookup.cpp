// $Id: tender_lookup.cpp,v 1.17 2004/02/16 09:24:16 bpepers Exp $
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

#include "tender_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "tender_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "tender_master.h"

#include <qlabel.h>
#include <qlayout.h>

TenderLookup::TenderLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "TenderLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    setCaption(tr("Tender Lookup"));
}

TenderLookup::TenderLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "TenderLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    setCaption(tr("Tender Lookup"));
}

TenderLookup::~TenderLookup()
{
}

QString
TenderLookup::lookupById(Id tender_id)
{
    Tender tender;
    if (tender_id != INVALID_ID && _quasar->db()->lookup(tender_id, tender))
	return tender.name();
    return "";
}

vector<DataPair>
TenderLookup::lookupByText(const QString& text)
{
    TenderSelect conditions;
    if (!text.isEmpty())
	conditions.name = text + "%";
    conditions.activeOnly = activeOnly;

    vector<Tender> tenders;
    _quasar->db()->select(tenders, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < tenders.size(); ++i) {
	data.push_back(DataPair(tenders[i].id(), tenders[i].name()));
    }

    return data;
}

void
TenderLookup::refresh()
{
    TenderSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.activeOnly = activeOnly;

    vector<Tender> tenders;
    _quasar->db()->select(tenders, conditions);

    _list->clear();
    for (unsigned int i = 0; i < tenders.size(); ++i) {
	Tender& tender = tenders[i];

	ListViewItem* item = new ListViewItem(_list, tender.id());
	item->setText(0, tender.name());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
TenderLookup::slotNew()
{
    TenderMaster* window = new TenderMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
TenderLookup::slotEdit(Id id)
{
    TenderMaster* window = new TenderMaster(_main, id);
    window->show();
    reject();
    return window;
}
