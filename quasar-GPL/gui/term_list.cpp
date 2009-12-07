// $Id: term_list.cpp,v 1.10 2004/01/31 01:50:31 arandell Exp $
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

#include "term_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "term_select.h"
#include "term_master.h"
#include "grid.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

TermList::TermList(MainWindow* main)
    : ActiveList(main, "TermList")
{
    _helpSource = "term_list.html";

    _list->addTextColumn(tr("Name"), 20);
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Terms List"));
    finalize();
}

TermList::~TermList()
{
}

bool
TermList::isActive(Id term_id)
{
    Term term;
    _quasar->db()->lookup(term_id, term);
    return term.isActive();
}

void
TermList::setActive(Id term_id, bool active)
{
    if (term_id == INVALID_ID) return;

    Term term;
    _quasar->db()->lookup(term_id, term);

    Term orig = term;
    _quasar->db()->setActive(term, active);
    _quasar->db()->update(orig, term);
}

void
TermList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::TERM)
	slotRefresh();
}

void
TermList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id term_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    TermSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<Term> terms;
    _quasar->db()->select(terms, conditions);

    for (unsigned int i = 0; i < terms.size(); ++i) {
	Term& term = terms[i];

	ListViewItem* lvi = new ListViewItem(_list, term.id());
	lvi->setValue(0, term.name());
	if (showInactive) lvi->setValue(1, !term.isActive());
	if (term.id() == term_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
TermList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Terms"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
TermList::performNew()
{
    TermMaster* master = new TermMaster(_main, INVALID_ID);
    master->show();
}

void
TermList::performEdit()
{
    Id term_id = currentId();
    TermMaster* master = new TermMaster(_main, term_id);
    master->show();
}
