// $Id: shift_close.cpp,v 1.12 2005/01/31 23:28:32 bpepers Exp $
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

#include "shift_close.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "company.h"
#include "shift.h"
#include "shift_view.h"
#include "gltx_select.h"
#include "station.h"
#include "employee.h"
#include "lookup_edit.h"
#include "store_lookup.h"
#include "list_view_item.h"
#include "date_popup.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qtimer.h>

ShiftClose::ShiftClose(MainWindow* main)
    : QuasarWindow(main, "ShiftClose")
{
    _helpSource = "shift_close.html";

    Company company;
    _quasar->db()->lookup(company);
    _method = company.shiftMethod();

    QFrame* frame = new QFrame(this);

    QLabel* storeLabel = new QLabel(tr("Store:"), frame);
    _store = new LookupEdit(new StoreLookup(main, this), frame);
    _store->setLength(30);
    storeLabel->setBuddy(_store);
    connect(_store, SIGNAL(validData()), SLOT(slotRefresh()));

    QLabel* dateLabel = new QLabel(tr("Date:"), frame);
    _date = new DatePopup(frame);
    dateLabel->setBuddy(_date);

    _list = new ListView(frame);
    if (_method == Company::BY_STATION)
	_list->addTextColumn("Station", 30);
    else
	_list->addTextColumn("Employee", 30);
    _list->addNumberColumn("# Trans", 10);
    _list->addMoneyColumn("Amount");
    _list->setAllColumnsShowFocus(true);
    _list->setShowSortIndicator(true);
    connect(_list, SIGNAL(doubleClicked(QListViewItem*)), this,
	    SLOT(slotShift()));

    QFrame* buttons = new QFrame(frame);
    QPushButton* refresh = new QPushButton(tr("Refresh"), buttons);
    connect(refresh, SIGNAL(clicked()), SLOT(slotRefresh()));
    refresh->setMinimumSize(refresh->sizeHint());

    QPushButton* shift = new QPushButton(tr("Close Shift"), buttons);
    connect(shift, SIGNAL(clicked()), SLOT(slotShift()));
    shift->setMinimumSize(refresh->sizeHint());

    QPushButton* ok = new QPushButton(tr("&Cancel"), buttons);
    connect(ok, SIGNAL(clicked()), SLOT(close()));
    ok->setMinimumSize(refresh->sizeHint());

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(2, 1);
    buttonGrid->addWidget(refresh, 0, 0, AlignLeft | AlignVCenter);
    buttonGrid->addWidget(shift, 0, 1, AlignLeft | AlignVCenter);
    buttonGrid->addWidget(ok, 0, 2, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(3);
    grid->setColStretch(1, 1);
    grid->setRowStretch(2, 1);
    grid->addWidget(storeLabel, 0, 0);
    grid->addWidget(_store, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(dateLabel, 1, 0);
    grid->addWidget(_date, 1, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_list, 2, 2, 0, 1);
    grid->addMultiCellWidget(buttons, 3, 3, 0, 1);

    _store->setId(_quasar->defaultStore(true));
    _date->setDate(QDate::currentDate());

    setCentralWidget(frame);
    setCaption(tr("Shift Close"));
    finalize();

    QTimer::singleShot(0, this, SLOT(slotRefresh()));
}

ShiftClose::~ShiftClose()
{
}

void
ShiftClose::slotRefresh()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Load transactions
    GltxSelect conditions;
    conditions.unclosed = true;
    vector<Gltx> gltxs;
    _quasar->db()->select(gltxs, conditions);

    // Process transactions
    vector<Id> ids;
    vector<int> counts;
    vector<fixed> amounts;
    unsigned int i;
    for (i = 0; i < gltxs.size(); ++i) {
	const Gltx& gltx = gltxs[i];

	Id id;
	if (_method == Company::BY_STATION)
	    id = gltx.stationId();
	else
	    id = gltx.employeeId();

	bool found = false;
	for (unsigned int j = 0; j < ids.size(); ++j) {
	    if (ids[j] == id) {
		counts[j] += 1;
		if (gltx.isActive())
		    amounts[j] += gltx.amount();
		found = true;
		break;
	    }
	}

	if (!found) {
	    ids.push_back(id);
	    counts.push_back(1);
	    amounts.push_back(gltx.isActive() ? gltx.amount() : 0.0);
	}
    }

    // Add to list
    _list->clear();
    for (i = 0; i < ids.size(); ++i) {
	QString name;
	if (_method == Company::BY_STATION) {
	    Station station;
	    _quasar->db()->lookup(ids[i], station);
	    name = station.name();
	} else {
	    Employee employee;
	    _quasar->db()->lookup(ids[i], employee);
	    name = employee.nameFL();
	}
	if (name.isEmpty()) name = tr("<blank>");

	ListViewItem* item = new ListViewItem(_list, ids[i]);
	item->setValue(0, name);
	item->setValue(1, counts[i]);
	item->setValue(2, amounts[i]);
    }

    QApplication::restoreOverrideCursor();
}

void
ShiftClose::slotShift()
{
    ListViewItem* item = _list->currentItem();
    if (item == NULL) return;

    QDate date = _date->getDate();
    if (date.isNull()) return;

    Id id = item->id;
    QString name;

    if (_method == Company::BY_STATION) {
	Station station;
	_quasar->db()->lookup(id, station);
	name = station.name();
    } else {
	Employee employee;
	_quasar->db()->lookup(id, employee);
	name = employee.nameFL();
    }
    if (name.isEmpty()) name = tr("<blank>");

    QString message = "Are you sure you want to close the\n"
	"shift of \"" + name + "\"";
    int choice = QMessageBox::warning(this, tr("Shift Close?"), message,
				      tr("Yes"), tr("No"));
    if (choice != 0) return;

    // Create the shift
    Shift shift;
    shift.setStoreId(_store->getId());
    if (_method == Company::BY_STATION)
	shift.setStationId(id);
    else
	shift.setEmployeeId(id);
    shift.setPostDate(date);
    shift.setPostTime(QTime::currentTime());
    if (!_quasar->db()->create(shift)) {
	message = tr("Failed creating shift close transaction");
	QMessageBox::critical(this, tr("Error"), message);
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    bool result = _quasar->db()->shiftClose(shift);
    QApplication::restoreOverrideCursor();

    if (!result) {
	message = tr("Failed setting shift in transactions");
	QMessageBox::critical(this, tr("Error"), message);
    } else {
	message = "The shift for \"" + name + "\"\nhas been closed";
	QMessageBox::information(this, tr("Information"), message);
	slotRefresh();
    }
}
