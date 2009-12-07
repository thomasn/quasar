// $Id: shift_list.cpp,v 1.9 2004/01/31 01:50:31 arandell Exp $
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

#include "shift_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "cash_reconcile.h"
#include "tender_adjustment.h"
#include "tender_transfer.h"
#include "shift.h"
#include "station.h"
#include "employee.h"
#include "shift_select.h"
#include "shift_close.h"
#include "shift_view.h"
#include "lookup_edit.h"
#include "store_lookup.h"
#include "grid.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qcheckbox.h>
#include <qlayout.h>

ShiftList::ShiftList(MainWindow* main)
    : ActiveList(main, "ShiftList")
{
    _helpSource = "shift_list.html";

    QPopupMenu* process = new QPopupMenu(this);
    process->insertItem(tr("Cash Reconciliation"), this,
			SLOT(slotCashReconcile()));
    process->insertItem(tr("Tender Adjustment"), this,
			SLOT(slotTenderAdjust()));
    process->insertItem(tr("Tender Transfer"), this,
			SLOT(slotTenderTransfer()));
    menuBar()->insertItem(tr("&Process"), process, -1, 2);

    _list->addTextColumn(tr("Number"), 10);
    _list->addDateColumn(tr("Date"));
    _list->addTextColumn(tr("Station"), 20);
    _list->addTextColumn(tr("Employee"), 20);
    _list->setSorting(0);

    QLabel* storeLabel = new QLabel(tr("&Store:"), _search);
    _store = new LookupEdit(new StoreLookup(_main, this), _search);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    QGridLayout* grid = new QGridLayout(_search);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(2, 1);
    grid->addWidget(storeLabel, 0, 0);
    grid->addWidget(_store, 0, 1, AlignLeft | AlignVCenter);

    _store->setId(_quasar->defaultStore());
    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Shift List"));
    finalize();
    _store->setFocus();
}

ShiftList::~ShiftList()
{
}

void
ShiftList::slotTenderAdjust()
{
    TenderAdjustment* master = new TenderAdjustment(_main);
    master->show();
}

void
ShiftList::slotTenderTransfer()
{
    TenderTransfer* master = new TenderTransfer(_main);
    master->show();
}

void
ShiftList::slotCashReconcile()
{
    CashReconcile* master = new CashReconcile(_main);
    master->show();
}

bool
ShiftList::isActive(Id shift_id)
{
    Shift shift;
    _quasar->db()->lookup(shift_id, shift);
    return shift.isActive();
}

void
ShiftList::setActive(Id shift_id, bool active)
{
    if (shift_id == INVALID_ID) return;

    Shift shift;
    _quasar->db()->lookup(shift_id, shift);

    Shift orig = shift;
    _quasar->db()->setActive(shift, active);
    _quasar->db()->update(orig, shift);
}

void
ShiftList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::SHIFT)
	slotRefresh();
}

void
ShiftList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id shift_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    ShiftSelect conditions;
    conditions.activeOnly = !showInactive;
    conditions.store_id = _store->getId();
    conditions.unlinked = true;
    vector<Shift> shifts;
    _quasar->db()->select(shifts, conditions);

    for (unsigned int i = 0; i < shifts.size(); ++i) {
	Shift& shift = shifts[i];

	Station station;
	_quasar->db()->lookup(shift.stationId(), station);

	Employee employee;
	_quasar->db()->lookup(shift.employeeId(), employee);

	ListViewItem* lvi = new ListViewItem(_list, shift.id());
	lvi->setValue(0, shift.number());
	lvi->setValue(1, shift.postDate());
	lvi->setValue(2, station.name());
	lvi->setValue(3, employee.nameFL());
	if (showInactive) lvi->setValue(4, !shift.isActive());
	if (shift.id() == shift_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
ShiftList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Shift List"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
ShiftList::performNew()
{
    ShiftClose* master = new ShiftClose(_main);
    master->show();
}

void
ShiftList::performEdit()
{
    Id shift_id = currentId();
    ShiftView* master = new ShiftView(_main, shift_id);
    master->show();
}
