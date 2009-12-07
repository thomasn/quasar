// $Id: adjust_reason_lookup.cpp,v 1.2 2005/01/09 20:22:28 bpepers Exp $
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

#include "adjust_reason_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "adjust_reason_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "adjust_reason_master.h"

#include <qlabel.h>
#include <qlayout.h>

AdjustReasonLookup::AdjustReasonLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "AdjustReasonLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(14);
    _list->addTextColumn(tr("Name"), 14);
    _list->addTextColumn(tr("Number"), 10);
    setCaption(tr("Reason Lookup"));
}

AdjustReasonLookup::AdjustReasonLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "AdjustReasonLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(14);
    _list->addTextColumn(tr("Name"), 14);
    _list->addTextColumn(tr("Number"), 10);
    setCaption(tr("Reason Lookup"));
}

AdjustReasonLookup::~AdjustReasonLookup()
{
}

QString
AdjustReasonLookup::lookupById(Id reason_id)
{
    AdjustReason reason;
    if (reason_id != INVALID_ID && _quasar->db()->lookup(reason_id, reason))
	return reason.name();
    return "";
}

vector<DataPair>
AdjustReasonLookup::lookupByText(const QString& text)
{
    AdjustReasonSelect conditions;
    if (!text.isEmpty())
	conditions.name = text + "%";
    conditions.activeOnly = activeOnly;

    vector<AdjustReason> reasons;
    _quasar->db()->select(reasons, conditions);
    if (reasons.size() == 0 && !text.isEmpty()) {
	conditions.name = "";
	conditions.number = text;
	_quasar->db()->select(reasons, conditions);
    }

    vector<DataPair> data;
    for (unsigned int i = 0; i < reasons.size(); ++i) {
	data.push_back(DataPair(reasons[i].id(), reasons[i].name()));
    }

    return data;
}

void
AdjustReasonLookup::refresh()
{
    AdjustReasonSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.activeOnly = activeOnly;

    vector<AdjustReason> reasons;
    _quasar->db()->select(reasons, conditions);

    _list->clear();
    for (unsigned int i = 0; i < reasons.size(); ++i) {
	AdjustReason& reason = reasons[i];

	ListViewItem* item = new ListViewItem(_list, reason.id());
	item->setValue(0, reason.name());
	item->setValue(1, reason.number());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
AdjustReasonLookup::slotNew()
{
    AdjustReasonMaster* window = new AdjustReasonMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
AdjustReasonLookup::slotEdit(Id id)
{
    AdjustReasonMaster* window = new AdjustReasonMaster(_main, id);
    window->show();
    reject();
    return window;
}
