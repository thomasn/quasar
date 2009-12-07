// $Id: term_lookup.cpp,v 1.10 2004/02/18 20:51:02 bpepers Exp $
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

#include "term_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "term_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "term_master.h"

#include <qlabel.h>
#include <qlayout.h>

TermLookup::TermLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "TermLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    setCaption(tr("Terms Lookup"));
}

TermLookup::TermLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "TermLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    setCaption(tr("Terms Lookup"));
}

TermLookup::~TermLookup()
{
}

QString
TermLookup::lookupById(Id term_id)
{
    Term term;
    if (term_id != INVALID_ID && _quasar->db()->lookup(term_id, term))
	return term.name();
    return "";
}

vector<DataPair>
TermLookup::lookupByText(const QString& text)
{
    TermSelect conditions;
    if (!text.isEmpty())
	conditions.name = text + "%";
    conditions.activeOnly = activeOnly;

    vector<Term> terms;
    _quasar->db()->select(terms, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < terms.size(); ++i) {
	data.push_back(DataPair(terms[i].id(), terms[i].name()));
    }

    return data;
}

void
TermLookup::refresh()
{
    TermSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.activeOnly = activeOnly;

    vector<Term> terms;
    _quasar->db()->select(terms, conditions);

    _list->clear();
    for (unsigned int i = 0; i < terms.size(); ++i) {
	Term& term = terms[i];

	ListViewItem* item = new ListViewItem(_list, term.id());
	item->setText(0, term.name());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
TermLookup::slotNew()
{
    TermMaster* window = new TermMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
TermLookup::slotEdit(Id id)
{
    TermMaster* window = new TermMaster(_main, id);
    window->show();
    reject();
    return window;
}
