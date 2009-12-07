// $Id: tax_list.cpp,v 1.35 2004/01/31 01:50:31 arandell Exp $
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

#include "tax_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "tax_select.h"
#include "tax_master.h"
#include "percent_valcon.h"
#include "grid.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

TaxList::TaxList(MainWindow* main)
    : ActiveList(main, "TaxList")
{
    _helpSource = "tax_list.html";

    _list->addTextColumn(tr("Name"), 7);
    _list->addTextColumn(tr("Description"), 24);
    _list->addPercentColumn(tr("Rate"));
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Tax List"));
    finalize();
}

TaxList::~TaxList()
{
}

bool
TaxList::isActive(Id tax_id)
{
    Tax tax;
    _quasar->db()->lookup(tax_id, tax);
    return tax.isActive();
}

void
TaxList::setActive(Id tax_id, bool active)
{
    if (tax_id == INVALID_ID) return;

    Tax tax;
    _quasar->db()->lookup(tax_id, tax);

    Tax orig = tax;
    _quasar->db()->setActive(tax, active);
    _quasar->db()->update(orig, tax);
}

void
TaxList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::TAX)
	slotRefresh();
}

void
TaxList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id tax_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    TaxSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<Tax> taxes;
    _quasar->db()->select(taxes, conditions);

    for (unsigned int i = 0; i < taxes.size(); i++) {
	Tax& tax = taxes[i];
	fixed rate = _db->taxRate(tax);

	ListViewItem* lvi = new ListViewItem(_list, tax.id());
	lvi->setValue(0, tax.name());
	lvi->setValue(1, tax.description());
	lvi->setValue(2, rate);
	if (showInactive) lvi->setValue(3, !tax.isActive());
	if (tax.id() == tax_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
TaxList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Tax List"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
TaxList::performNew()
{
    TaxMaster* master = new TaxMaster(_main, INVALID_ID);
    master->show();
}

void
TaxList::performEdit()
{
    Id tax_id = currentId();
    TaxMaster* master = new TaxMaster(_main, tax_id);
    master->show();
}
