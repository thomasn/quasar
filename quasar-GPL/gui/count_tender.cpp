// $Id: count_tender.cpp,v 1.16 2005/03/22 20:08:29 bpepers Exp $
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

#include "count_tender.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "company.h"
#include "table.h"
#include "date_popup.h"
#include "lookup_edit.h"
#include "money_edit.h"
#include "station_lookup.h"
#include "employee_lookup.h"
#include "store_lookup.h"
#include "tender_select.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <algorithm>

// Used to sort tenders by menu number
static bool operator<(const Tender& lhs, const Tender& rhs) {
    return lhs.menuNumber() < rhs.menuNumber();
}

CountTender::CountTender(MainWindow* main, Id count_id)
    : DataWindow(main, "CountTender", count_id)
{
    _helpSource = "count_tender.html";

    QFrame* top = new QFrame(_frame);
    top->setFrameStyle(QFrame::Raised | QFrame::Panel);

    QLabel* numberLabel = new QLabel(tr("Count No.:"), top);
    _number = new LineEdit(top);
    _number->setLength(14, '9');
    numberLabel->setBuddy(_number);

    QLabel* dateLabel = new QLabel(tr("Date:"), top);
    _date = new DatePopup(top);
    dateLabel->setBuddy(_date);

    QLabel* stationLabel = new QLabel(tr("Station:"), top);
    _station = new LookupEdit(new StationLookup(main, this), top);
    _station->setLength(30);
    stationLabel->setBuddy(_station);

    QLabel* employeeLabel = new QLabel(tr("Employee:"), top);
    _employee = new LookupEdit(new EmployeeLookup(main, this), top);
    _employee->setLength(30);
    employeeLabel->setBuddy(_employee);

    QLabel* storeLabel = new QLabel(tr("Store:"), top);
    _store = new LookupEdit(new StoreLookup(_main, this), top);
    _store->setFocusPolicy(ClickFocus);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(3);
    topGrid->setMargin(3);
    topGrid->setColStretch(2, 1);
    topGrid->addWidget(numberLabel, 0, 0);
    topGrid->addWidget(_number, 0, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(dateLabel, 0, 3);
    topGrid->addWidget(_date, 0, 4, AlignLeft | AlignVCenter);
    topGrid->addWidget(stationLabel, 1, 0);
    topGrid->addWidget(_station, 1, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(employeeLabel, 2, 0);
    topGrid->addWidget(_employee, 2, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(storeLabel, 3, 0);
    topGrid->addWidget(_store, 3, 1, AlignLeft | AlignVCenter);

    QFrame* mid = new QFrame(_frame);
    mid->setFrameStyle(QFrame::Raised | QFrame::Panel);

    _tenders = new Table(mid);
    _tenders->setVScrollBarMode(QScrollView::AlwaysOn);
    _tenders->setLeftMargin(fontMetrics().width("99999"));
    _tenders->setDisplayRows(10);
    connect(_tenders, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(cellChanged(int,int,Variant)));
    connect(_tenders, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(focusNext(bool&,int&,int&,int)));
    connect(_tenders, SIGNAL(rowDeleted(int)), SLOT(recalculate()));

    // Add columns
    new TextColumn(_tenders, tr("Tender"), 30);
    new MoneyColumn(_tenders, tr("Amount"), 10);

    // Add editors
    new NumberEditor(_tenders, 1, new MoneyEdit(_tenders));

    QGridLayout* midGrid = new QGridLayout(mid);
    midGrid->setSpacing(3);
    midGrid->setMargin(3);
    midGrid->setColStretch(0, 1);
    midGrid->addWidget(_tenders, 0, 0);

    QFrame* bot = new QFrame(_frame);
    bot->setFrameStyle(QFrame::Raised | QFrame::Panel);

    QLabel* totalLabel = new QLabel(tr("Tender Total:"), bot);
    _total = new MoneyEdit(bot);
    _total->setFocusPolicy(NoFocus);

    QGridLayout* botGrid = new QGridLayout(bot);
    botGrid->setSpacing(3);
    botGrid->setMargin(3);
    botGrid->setColStretch(0, 1);
    botGrid->addWidget(totalLabel, 0, 0, AlignRight | AlignVCenter);
    botGrid->addWidget(_total, 0, 1);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(1, 1);
    grid->addWidget(top, 0, 0);
    grid->addWidget(mid, 1, 0);
    grid->addWidget(bot, 2, 0);

    Company company;
    _quasar->db()->lookup(company);

    if (company.shiftMethod() == Company::BY_STATION) {
	employeeLabel->hide();
	_employee->hide();
    } else {
	stationLabel->hide();
	_station->hide();
    }

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    TenderSelect conditions;
    conditions.activeOnly = true;
    _quasar->db()->select(_tenderList, conditions);
    std::sort(_tenderList.begin(), _tenderList.end());

    for (unsigned int i = 0; i < _tenderList.size(); ++i) {
	VectorRow* row = new VectorRow(_tenders->columns());
	row->setValue(0, _tenderList[i].name());
	row->setValue(1, "");
	_tenders->appendRow(row);
    }
    _tenders->setCurrentCell(0, 1);
    qApp->processEvents();
    _tenders->verticalScrollBar()->setValue(0);

    setCaption(tr("Count Tenders"));
    finalize();
}

CountTender::~CountTender()
{
}

void
CountTender::setStoreId(Id store_id)
{
    _store->setId(store_id);
}

void
CountTender::setStationId(Id station_id)
{
    _station->setId(station_id);
    _number->deselect();
    _tenders->setFocus();
}

void
CountTender::setEmployeeId(Id employee_id)
{
    _employee->setId(employee_id);
    _number->deselect();
    _tenders->setFocus();
}

void
CountTender::setDate(QDate date)
{
    if (date.isNull())
	date = QDate::currentDate();
    _date->setDate(date);
}

void
CountTender::setTender(Id tender_id, fixed amount)
{
    for (unsigned int i = 0; i < _tenderList.size(); ++i) {
	if (_tenderList[i].id() == tender_id) {
	    _tenders->setCellValue(i, 1, amount);
	    break;
	}
    }
}

void
CountTender::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _tenders;
}

void
CountTender::newItem()
{
    TenderCount blank;
    _orig = blank;
    _orig.setNumber("#");
    _orig.setDate(QDate::currentDate());
    _orig.setStoreId(_quasar->defaultStore());

    _curr = _orig;
    _firstField = _number;
}

void
CountTender::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _curr.setNumber("#");
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
}

bool
CountTender::fileItem()
{
    if (_orig.id() == INVALID_ID) {
	if (!_quasar->db()->create(_curr)) return false;
    } else {
	if (!_quasar->db()->update(_orig, _curr)) return false;
    }

    _orig = _curr;
    _id = _curr.id();

    return true;
}

bool
CountTender::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
CountTender::restoreItem()
{
    _curr = _orig;
}

void
CountTender::cloneItem()
{
    CountTender* clone = new CountTender(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
CountTender::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
CountTender::dataToWidget()
{
    _number->setText(_curr.number());
    _date->setDate(_curr.date());
    _station->setId(_curr.stationId());
    _employee->setId(_curr.employeeId());
    _store->setId(_curr.storeId());
    _inactive->setChecked(!_curr.isActive());

    // Load the tenders
    for (unsigned int i = 0; i < _curr.tenders().size(); ++i) {
	Id tender_id = _curr.tenders()[i].tender_id;
	fixed amount = _curr.tenders()[i].amount;

	for (int row = 0; row < _tenders->rows(); ++row) {
	    if (_tenderList[row].id() == tender_id) {
		_tenders->setCellValue(row, 1, amount);
		break;
	    }
	}
    }

    recalculate();
}

// Set the data object from the widgets.
void
CountTender::widgetToData()
{
    _curr.setNumber(_number->text());
    _curr.setDate(_date->getDate());
    _curr.setStationId(_station->getId());
    _curr.setEmployeeId(_employee->getId());
    _curr.setStoreId(_store->getId());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    // Process all the tenders
    _curr.tenders().clear();
    for (int i = 0; i < _tenders->rows(); ++i) {
	Id tender_id = _tenderList[i].id();
	fixed amount = _tenders->cellValue(i, 1).toFixed();
	if (amount == 0.0) continue;

	TenderInfo info;
	info.tender_id = tender_id;
	info.amount = amount;
	_curr.tenders().push_back(info);
    }
}

void
CountTender::cellChanged(int, int, Variant)
{
    recalculate();
}

void
CountTender::focusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _tenders->currentRow();

    newCol = 1;
    if (type == Table::MovePrev) {
	if (row == 0)
	    leave = true;
	else
	    newRow = row - 1;
    }
}

void
CountTender::recalculate()
{
    fixed total = 0.0;
    for (int row = 0; row < _tenders->rows(); ++row)
	total += _tenders->cellValue(row, 1).toFixed();

    _total->setFixed(total);
}
