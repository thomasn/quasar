// $Id: charge_list.cpp,v 1.12 2004/01/31 01:50:30 arandell Exp $
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

#include "charge_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "charge_select.h"
#include "charge_master.h"
#include "grid.h"

#include <qapplication.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

ChargeList::ChargeList(MainWindow* main)
    : ActiveList(main, "ChargeList")
{
    _helpSource = "charge_list.html";

    _list->addTextColumn(tr("Name"), 20);
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Charge List"));
    finalize();
}

ChargeList::~ChargeList()
{
}

bool
ChargeList::isActive(Id charge_id)
{
    Charge charge;
    _quasar->db()->lookup(charge_id, charge);
    return charge.isActive();
}

void
ChargeList::setActive(Id charge_id, bool active)
{
    if (charge_id == INVALID_ID) return;

    Charge charge;
    _quasar->db()->lookup(charge_id, charge);

    Charge orig = charge;
    _quasar->db()->setActive(charge, active);
    _quasar->db()->update(orig, charge);
}

void
ChargeList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::CHARGE)
	slotRefresh();
}

void
ChargeList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id charge_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    ChargeSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<Charge> charges;
    _quasar->db()->select(charges, conditions);

    for (unsigned int i = 0; i < charges.size(); i++) {
	Charge& charge = charges[i];

	ListViewItem* lvi = new ListViewItem(_list, charge.id());
	lvi->setValue(0, charge.name());
	if (showInactive) lvi->setValue(1, !charge.isActive());

	if (charge.id() == charge_id)
	    current = lvi;
    }

    if (current == NULL)
	current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
ChargeList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Charges"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
ChargeList::performNew()
{
    ChargeMaster* master = new ChargeMaster(_main, INVALID_ID);
    master->show();
}

void
ChargeList::performEdit()
{
    Id charge_id = currentId();
    ChargeMaster* master = new ChargeMaster(_main, charge_id);
    master->show();
}
