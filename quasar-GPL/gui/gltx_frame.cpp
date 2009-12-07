// $Id: gltx_frame.cpp,v 1.11 2005/01/31 23:28:32 bpepers Exp $
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

#include "gltx_frame.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "gltx.h"
#include "station.h"
#include "time_edit.h"
#include "date_popup.h"
#include "lookup_edit.h"
#include "store_lookup.h"
#include "station_lookup.h"
#include "employee_lookup.h"
#include "gltx_lookup.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qlayout.h>

GltxFrame::GltxFrame(MainWindow* main, const QString& numText, QWidget* parent)
    : QGroupBox(parent, "GltxFrame"), _main(main), _quasar(main->quasar())
{
    numberLabel = new QLabel(numText, this);
    number = new LineEdit(14, this);
    numberLabel->setBuddy(number);

    QLabel* dateLabel = new QLabel(tr("&Date:"), this);
    date = new DatePopup(this);
    dateLabel->setBuddy(date);

    QLabel* storeLabel = new QLabel(tr("Store:"), this);
    storeLookup = new StoreLookup(_main, parent);
    store = new LookupEdit(storeLookup, this);
    store->setLength(20);
    storeLabel->setBuddy(store);

    QLabel* referenceLabel = new QLabel(tr("Reference:"), this);
    reference = new LineEdit(9, this);
    referenceLabel->setBuddy(reference);

    QLabel* timeLabel = new QLabel(tr("&Time:"), this);
    time = new TimeEdit(this);
    timeLabel->setBuddy(time);

    QLabel* stationLabel = new QLabel(tr("Station:"), this);
    station = new LookupEdit(new StationLookup(_main, parent), this);
    station->setLength(20);
    stationLabel->setBuddy(station);

    QLabel* shiftLabel = new QLabel(tr("Shift:"), this);
    shift = new LookupEdit(new GltxLookup(_main, parent, DataObject::SHIFT),
			    this);
    shift->setSizeInfo(8, '9');
    shift->setFocusPolicy(ClickFocus);
    shiftLabel->setBuddy(shift);

    QLabel* employeeLabel = new QLabel(tr("Employee:"), this);
    employee = new LookupEdit(new EmployeeLookup(_main, parent), this);
    employee->setLength(20);
    employeeLabel->setBuddy(employee);

    memoLabel = new QLabel(tr("&Memo:"), this);
    memo = new LineEdit(this);
    memo->setMaxLength(40);
    memo->setMinimumWidth(memo->fontMetrics().width('x') * 40);
    memoLabel->setBuddy(memo);

    grid = new QGridLayout(this, 1, 1, frameWidth() * 2);
    grid->setSpacing(3);
    grid->setMargin(6);
    grid->setColStretch(2, 1);
    grid->addColSpacing(2, 10);
    grid->setColStretch(5, 1);
    grid->addColSpacing(5, 10);
    grid->addWidget(numberLabel, 1, 0);
    grid->addWidget(number, 1, 1);
    grid->addWidget(dateLabel, 1, 3);
    grid->addWidget(date, 1, 4);
    grid->addWidget(storeLabel, 1, 6);
    grid->addWidget(store, 1, 7);
    grid->addWidget(referenceLabel, 2, 0);
    grid->addWidget(reference, 2, 1);
    grid->addWidget(timeLabel, 2, 3);
    grid->addWidget(time, 2, 4);
    grid->addWidget(stationLabel, 2, 6);
    grid->addWidget(station, 2, 7);
    grid->addWidget(shiftLabel, 3, 3);
    grid->addWidget(shift, 3, 4);
    grid->addWidget(employeeLabel, 3, 6);
    grid->addWidget(employee, 3, 7);
    grid->addWidget(memoLabel, 9, 0);
    grid->addMultiCellWidget(memo, 9, 9, 1, 7);

    // Hide store if only one possible
    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	store->hide();
    }

    setFrameStyle(QFrame::Raised | QFrame::Panel);
    memo->setFocus();
}

GltxFrame::~GltxFrame()
{
}

void
GltxFrame::setTitle(const QString& title)
{
    setFrameStyle(Box | Sunken);
    grid->addRowSpacing(0, fontMetrics().height());
    QGroupBox::setTitle(title);
}

void
GltxFrame::hideMemo()
{
    memoLabel->hide();
    memo->hide();
}

QWidget*
GltxFrame::firstField()
{
    return memo;
}

void
GltxFrame::getData(Gltx& gltx)
{
    gltx.setNumber(number->text());
    gltx.setPostDate(date->getDate());
    gltx.setPostTime(time->getTime());
    gltx.setReference(reference->text());
    gltx.setMemo(memo->text());
    gltx.setStoreId(store->getId());
    gltx.setStationId(station->getId());
    gltx.setEmployeeId(employee->getId());
    gltx.setShiftId(shift->getId());
}

void
GltxFrame::setData(const Gltx& gltx)
{
    number->setText(gltx.number());
    date->setDate(gltx.postDate());
    time->setTime(gltx.postTime());
    reference->setText(gltx.reference());
    memo->setText(gltx.memo());
    store->setId(gltx.storeId());
    station->setId(gltx.stationId());
    employee->setId(gltx.employeeId());
    shift->setId(gltx.shiftId());
}

void
GltxFrame::defaultData(Gltx& gltx)
{
    gltx.setPostDate(QDate::currentDate());
    gltx.setPostTime(QTime::currentTime());
    gltx.setStoreId(_quasar->defaultStore());
    gltx.setStationId(_quasar->defaultStation());
    gltx.setEmployeeId(_quasar->defaultEmployee());
}

void
GltxFrame::cloneData(Gltx& gltx)
{
    gltx.setId(INVALID_ID);
    gltx.setNumber("#");
    gltx.setShiftId(INVALID_ID);
    gltx.setPrinted(false);
    gltx.payments().clear();
    _quasar->db()->setActive(gltx, true);
}
