// $Id: cash_reconcile.cpp,v 1.32 2005/03/13 23:13:45 bpepers Exp $
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

#include "cash_reconcile.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "report_tx.h"
#include "shift_view.h"
#include "tx_summary.h"
#include "count_tender.h"
#include "bank_deposit.h"
#include "tender_adjustment.h"
#include "tender_transfer.h"
#include "company.h"
#include "shift.h"
#include "tender_count.h"
#include "station.h"
#include "employee.h"
#include "station_select.h"
#include "employee_select.h"
#include "date_popup.h"
#include "lookup_edit.h"
#include "store_lookup.h"
#include "station_lookup.h"
#include "employee_lookup.h"
#include "account_lookup.h"
#include "list_view_item.h"
#include "shift_select.h"
#include "tender_count_select.h"
#include "tender_adjust.h"
#include "money_edit.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <algorithm>

CashReconcile::CashReconcile(MainWindow* main)
    : QuasarWindow(main, "CashReconcile")
{
    _helpSource = "cash_reconcile.html";

    Company company;
    _quasar->db()->lookup(_company);

    QFrame* frame = new QFrame(this);

    QLabel* dateLabel = new QLabel(tr("Date:"), frame);
    _date = new DatePopup(frame);
    dateLabel->setBuddy(_date);
    connect(_date, SIGNAL(validData()), SLOT(slotRefresh()));

    QLabel* storeLabel = new QLabel(tr("Store:"), frame);
    _store = new LookupEdit(new StoreLookup(main, this), frame);
    _store->setLength(30);
    storeLabel->setBuddy(_store);
    connect(_store, SIGNAL(validData()), SLOT(slotRefresh()));

    _lines = new ListView(frame);
    _lines->addTextColumn(tr("Number"), 10, AlignRight);
    if (_company.shiftMethod() == Company::BY_STATION)
	_lines->addTextColumn(tr("Station"), 20);
    else
	_lines->addTextColumn(tr("Employee"), 20);
    _lines->addNumberColumn(tr("Not Rungoff"));
    _lines->addNumberColumn(tr("Shifts"));
    _lines->addNumberColumn(tr("Counts"));
    _lines->setAllColumnsShowFocus(true);
    _lines->setShowSortIndicator(true);
    connect(_lines, SIGNAL(selectionChanged()), this,
	    SLOT(slotLineChanged()));
    connect(_lines, SIGNAL(doubleClicked(QListViewItem*)), this,
	    SLOT(slotLineSelected()));

    QFrame* mid = new QFrame(frame);

    _tabs = new QTabWidget(mid);
    _info = new QFrame(_tabs);
    _setup = new QFrame(_tabs);
    _tabs->addTab(_info, tr("Shifts && Counts"));
    _tabs->addTab(_setup, tr("Setup"));

    _shiftList = new ListView(_info);
    _shiftList->addTextColumn(tr("Shift #"), 10, AlignRight);
    _shiftList->addDateColumn(tr("Date"));
    _shiftList->setAllColumnsShowFocus(true);
    _shiftList->setShowSortIndicator(true);
    _shiftList->setResizeMode(QListView::AllColumns);
    connect(_shiftList, SIGNAL(doubleClicked(QListViewItem*)), this,
	    SLOT(slotShiftSelected()));

    _countList = new ListView(_info);
    _countList->addTextColumn(tr("Count #"), 10, AlignRight);
    _countList->addDateColumn(tr("Date"));
    _countList->setAllColumnsShowFocus(true);
    _countList->setShowSortIndicator(true);
    _countList->setResizeMode(QListView::AllColumns);
    connect(_countList, SIGNAL(doubleClicked(QListViewItem*)), this,
	    SLOT(slotCountSelected()));

    QGridLayout* infoGrid = new QGridLayout(_info);
    infoGrid->setSpacing(3);
    infoGrid->setMargin(3);
    infoGrid->setRowStretch(0, 1);
    infoGrid->setColStretch(0, 1);
    infoGrid->setColStretch(1, 1);
    infoGrid->addWidget(_shiftList, 0, 0);
    infoGrid->addWidget(_countList, 0, 1);

    QLabel* safeStoreLabel = new QLabel(tr("Safe Store:"), _setup);
    _safeStore = new LookupEdit(new StoreLookup(main, this), _setup);
    _safeStore->setLength(30);
    safeStoreLabel->setBuddy(_safeStore);

    QLabel* safeIdLabel;
    if (_company.shiftMethod() == Company::BY_STATION) {
	safeIdLabel = new QLabel(tr("Safe Station:"), _setup);
	_safeId = new LookupEdit(new StationLookup(main, this), _setup);
    } else {
	safeIdLabel = new QLabel(tr("Safe Employee:"), _setup);
	_safeId = new LookupEdit(new EmployeeLookup(main, this), _setup);
    }
    _safeId->setLength(30);
    safeIdLabel->setBuddy(_safeId);

    QLabel* adjustLabel = new QLabel(tr("Over/Short Account:"), _setup);
    _adjust = new LookupEdit(new AccountLookup(main, this), _setup);
    _adjust->setLength(30);
    adjustLabel->setBuddy(_adjust);

    QLabel* transferLabel = new QLabel(tr("Transfer Account:"), _setup);
    _transfer = new LookupEdit(new AccountLookup(main, this), _setup);
    _transfer->setLength(30);
    transferLabel->setBuddy(_transfer);

    QGridLayout* setupGrid = new QGridLayout(_setup);
    setupGrid->setSpacing(3);
    setupGrid->setMargin(3);
    setupGrid->setRowStretch(4, 1);
    setupGrid->setColStretch(2, 1);
    setupGrid->addWidget(safeStoreLabel, 0, 0);
    setupGrid->addWidget(_safeStore, 0, 1);
    setupGrid->addWidget(safeIdLabel, 1, 0);
    setupGrid->addWidget(_safeId, 1, 1);
    setupGrid->addWidget(adjustLabel, 2, 0);
    setupGrid->addWidget(_adjust, 2, 1);
    setupGrid->addWidget(transferLabel, 3, 0);
    setupGrid->addWidget(_transfer, 3, 1);

    QFrame* txb = new QFrame(mid);
    _shiftClose = new QPushButton(tr("Ringoff"), txb);
    _countCreate = new QPushButton(tr("Create Count"), txb);
    _bankDeposit = new QPushButton(tr("Bank Deposit"), txb);
    QPushButton* adjust = new QPushButton(tr("Adjustment"), txb);
    QPushButton* transfer = new QPushButton(tr("Transfer"), txb);

    connect(_shiftClose, SIGNAL(clicked()), SLOT(slotShiftClose()));
    connect(_countCreate, SIGNAL(clicked()), SLOT(slotCreateCount()));
    connect(_bankDeposit, SIGNAL(clicked()), SLOT(slotBankDeposit()));
    connect(adjust, SIGNAL(clicked()), SLOT(slotTenderAdjust()));
    connect(transfer, SIGNAL(clicked()), SLOT(slotTenderTransfer()));

    QGridLayout* txbGrid = new QGridLayout(txb);
    txbGrid->setSpacing(3);
    txbGrid->setMargin(3);
    txbGrid->setRowStretch(0, 1);
    txbGrid->addRowSpacing(4, 12);
    txbGrid->addWidget(_shiftClose, 1, 0);
    txbGrid->addWidget(_countCreate, 2, 0);
    txbGrid->addWidget(_bankDeposit, 3, 0);
    txbGrid->addWidget(adjust, 5, 0);
    txbGrid->addWidget(transfer, 6, 0);

    QGridLayout* midGrid = new QGridLayout(mid);
    midGrid->setSpacing(3);
    midGrid->setMargin(3);
    midGrid->setColStretch(0, 1);
    midGrid->addColSpacing(1, 10);
    midGrid->addWidget(_tabs, 0, 0);
    midGrid->addWidget(txb, 0, 2);

    QFrame* buttons = new QFrame(frame);
    QPushButton* refresh = new QPushButton(tr("Refresh"), buttons);
    QPushButton* summary = new QPushButton(tr("Summary"), buttons);
    _reconcile = new QPushButton(tr("Reconcile"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);

    connect(refresh, SIGNAL(clicked()), SLOT(slotRefresh()));
    connect(summary, SIGNAL(clicked()), SLOT(slotSummary()));
    connect(_reconcile, SIGNAL(clicked()), SLOT(slotReconcile()));
    connect(cancel, SIGNAL(clicked()), SLOT(close()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(2, 1);
    buttonGrid->addWidget(refresh, 0, 0);
    buttonGrid->addWidget(summary, 0, 1);
    buttonGrid->addWidget(_reconcile, 0, 3);
    buttonGrid->addWidget(cancel, 0, 4);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(3);
    grid->setRowStretch(2, 1);
    grid->setColStretch(2, 1);
    grid->addWidget(dateLabel, 0, 0);
    grid->addWidget(_date, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(storeLabel, 1, 0);
    grid->addWidget(_store, 1, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_lines, 2, 2, 0, 2);
    grid->addMultiCellWidget(mid, 3, 3, 0, 2);
    grid->addMultiCellWidget(buttons, 4, 4, 0, 2);

    _date->setDate(QDate::currentDate());
    _store->setId(_quasar->defaultStore());
    _safeStore->setId(_company.safeStore());
    if (_company.shiftMethod() == Company::BY_STATION)
	_safeId->setId(_company.safeStation());
    else
	_safeId->setId(_company.safeEmployee());
    _adjust->setId(_company.overShortAccount());
    _transfer->setId(_company.transferAccount());
    _lines->setFocus();
    slotRefresh();

    setCentralWidget(frame);
    setCaption(tr("Cash Reconcile"));
    finalize();
}

CashReconcile::~CashReconcile()
{
}

void
CashReconcile::slotLineChanged()
{
    _shifts.clear();
    _counts.clear();
    _shiftList->clear();
    _countList->clear();

    ListViewItem* item = _lines->selectedItem();
    if (item == NULL) {
	_shiftClose->setEnabled(false);
	_countCreate->setEnabled(false);
	_bankDeposit->setEnabled(false);
	_reconcile->setEnabled(false);
	return;
    }

    ShiftSelect shiftSelect;
    shiftSelect.store_id = _store->getId();
    if (stationId() == INVALID_ID && employeeId() == INVALID_ID) {
	if (_company.shiftMethod() == Company::BY_STATION)
	    shiftSelect.blankStation = true;
	else
	    shiftSelect.blankEmployee = true;
    } else {
	shiftSelect.station_id = stationId();
	shiftSelect.employee_id = employeeId();
    }
    shiftSelect.end_date = _date->getDate();
    shiftSelect.activeOnly = true;
    shiftSelect.unclosed = true;
    _quasar->db()->select(_shifts, shiftSelect);

    TenderCountSelect countSelect;
    countSelect.store_id = shiftSelect.store_id;
    countSelect.blankStation = shiftSelect.blankStation;
    countSelect.blankEmployee = shiftSelect.blankEmployee;
    countSelect.station_id = shiftSelect.station_id;
    countSelect.employee_id = shiftSelect.employee_id;
    countSelect.end_date = shiftSelect.end_date;
    countSelect.activeOnly = true;
    countSelect.unclosed = true;
    _quasar->db()->select(_counts, countSelect);

    unsigned int i;
    for (i = 0; i < _shifts.size(); ++i) {
	const Shift& shift = _shifts[i];

	ListViewItem* item = new ListViewItem(_shiftList, shift.id());
	item->setValue(0, shift.number());
	item->setValue(1, shift.postDate());
    }

    for (i = 0; i < _counts.size(); ++i) {
	const TenderCount& count = _counts[i];

	ListViewItem* item = new ListViewItem(_countList, count.id());
	item->setValue(0, count.number());
	item->setValue(1, count.date());
    }

    int openCnt = item->value(2).toInt();
    int shiftCnt = item->value(3).toInt();
    int countCnt = item->value(4).toInt();

    _shiftClose->setEnabled(openCnt != 0);
    _countCreate->setEnabled(shiftCnt != 0);
    _bankDeposit->setEnabled(item->id == _safeId->getId() && (openCnt != 0 || shiftCnt != 0));
    _reconcile->setEnabled(shiftCnt != 0 || countCnt != 0);
}

void
CashReconcile::slotLineSelected()
{
    ReportTx* master = new ReportTx(_main);
    master->setStoreId(_store->getId());
    master->setStationId(stationId());
    master->setEmployeeId(employeeId());
    master->setDates(_date->getDate(), _date->getDate());
    master->refresh();
    master->show();
}

void
CashReconcile::slotShiftSelected()
{
    ListViewItem* item = _shiftList->currentItem();
    if (item == NULL) return;

    ShiftView* master = new ShiftView(_main, item->id);
    master->show();
}

void
CashReconcile::slotCountSelected()
{
    ListViewItem* item = _countList->currentItem();
    if (item == NULL) return;

    CountTender* master = new CountTender(_main, item->id);
    master->show();
    connect(master, SIGNAL(created(Id)), SLOT(slotRefresh()));
    connect(master, SIGNAL(updated(Id)), SLOT(slotRefresh()));
    connect(master, SIGNAL(deleted(Id)), SLOT(slotRefresh()));
}

void
CashReconcile::slotShiftClose()
{
    QDate date = _date->getDate();
    Id station_id = stationId();
    Id employee_id = employeeId();

    // Posting time is current time if closing for today or 11:59:59 PM
    QTime time = QTime::currentTime();
    if (date != QDate::currentDate())
	time = QTime(23, 59, 59);

    QString name;
    if (station_id != INVALID_ID) {
	Station station;
	_quasar->db()->lookup(station_id, station);
	name = station.name();
    } else if (employee_id != INVALID_ID) {
	Employee employee;
	_quasar->db()->lookup(employee_id, employee);
	name = employee.nameFL();
    } else {
	name = "<None>";
    }
    if (name.isEmpty()) name = tr("<blank>");

    QString message = "Are you sure you want to ringoff \n\"" + name + "\"";
    int choice = QMessageBox::warning(this, tr("Ringoff?"), message,
				      tr("Yes"), tr("No"));
    if (choice != 0) return;

    // Create the shift
    Shift shift;
    shift.setStoreId(_store->getId());
    shift.setStationId(station_id);
    shift.setEmployeeId(employee_id);
    shift.setPostDate(date);
    shift.setPostTime(time);
    if (!_quasar->db()->create(shift)) {
	message = tr("Failed creating shift close transaction");
	QMessageBox::critical(this, tr("Error"), message);
	return;
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

void
CashReconcile::slotCreateCount()
{
    CountTender* master = new CountTender(_main);
    master->setStoreId(_store->getId());
    master->setStationId(stationId());
    master->setEmployeeId(employeeId());
    master->setDate(_date->getDate());
    master->show();
    connect(master, SIGNAL(created(Id)), SLOT(slotRefresh()));
}

void
CashReconcile::slotBankDeposit()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    BankDeposit* master = new BankDeposit(_main);
    master->setShifts(_shifts);
    master->setDate(_date->getDate());
    master->show();
    connect(master, SIGNAL(depositDone()), SLOT(slotRefresh()));

    QApplication::restoreOverrideCursor();
}

void
CashReconcile::slotTenderAdjust()
{
    TenderAdjustment* master = new TenderAdjustment(_main);
    master->setStore(_store->getId());
    master->setStation(stationId());
    master->setEmployee(employeeId());
    master->setDate(_date->getDate());
    master->show();
    connect(master, SIGNAL(created(Id)), SLOT(slotRefresh()));
}

void
CashReconcile::slotTenderTransfer()
{
    TenderTransfer* master = new TenderTransfer(_main);
    master->setStore(_store->getId());
    master->setStation(stationId());
    master->setEmployee(employeeId());
    master->setDate(_date->getDate());
    master->show();
    connect(master, SIGNAL(created(Id)), SLOT(slotRefresh()));
}

void
CashReconcile::slotSummary()
{
    QWidget* window = _main->findWindow("TxSummary");
    if (window == NULL) {
	window = new TxSummary(_main);
	window->show();
    }
}

void
CashReconcile::slotRefresh()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    ListViewItem* currentItem = _lines->selectedItem();
    Id currentId = (currentItem != NULL) ? currentItem->id : INVALID_ID;
    _lines->clear();

    if (_company.shiftMethod() == Company::BY_STATION) {
	StationSelect conditions;
	conditions.activeOnly = true;
	vector<Station> stations;
	_quasar->db()->select(stations, conditions);

	for (unsigned int i = 0; i < stations.size(); ++i) {
	    const Station& station = stations[i];
	    int transactions = 0;
	    int shifts = 0;
	    vector<TenderCount> counts;

	    GltxSelect gltxCond;
	    gltxCond.store_id = _store->getId();
	    gltxCond.station_id = station.id();
	    gltxCond.end_date = _date->getDate();
	    gltxCond.unclosed = true;
	    gltxCond.cashrecOnly = true;
	    _quasar->db()->count(transactions, gltxCond);

	    gltxCond.type = DataObject::SHIFT;
	    _quasar->db()->count(shifts, gltxCond);

	    TenderCountSelect countCond;
	    countCond.store_id = _store->getId();
	    countCond.station_id = station.id();
	    countCond.end_date = _date->getDate();
	    countCond.activeOnly = true;
	    countCond.unclosed = true;
	    _quasar->db()->select(counts, countCond);

	    ListViewItem* item = new ListViewItem(_lines, station.id());
	    item->setValue(0, station.number());
	    item->setValue(1, station.name());
	    item->setValue(2, transactions - shifts);
	    item->setValue(3, shifts);
	    item->setValue(4, counts.size());

	    if (station.id() == currentId)
		_lines->setSelected(item, true);
	}

	int transactions = 0;
	int shifts = 0;
	vector<TenderCount> counts;

	GltxSelect gltxCond;
	gltxCond.store_id = _store->getId();
	gltxCond.blankStation = true;
	gltxCond.end_date = _date->getDate();
	gltxCond.unclosed = true;
	gltxCond.cashrecOnly = true;
	_quasar->db()->count(transactions, gltxCond);

	gltxCond.type = DataObject::SHIFT;
	_quasar->db()->count(shifts, gltxCond);

	TenderCountSelect countCond;
	countCond.store_id = _store->getId();
	countCond.blankStation = true;
	countCond.end_date = _date->getDate();
	countCond.activeOnly = true;
	countCond.unclosed = true;
	_quasar->db()->select(counts, countCond);

	if (transactions != 0 || shifts != 0 || counts.size() != 0) {
	    ListViewItem* item = new ListViewItem(_lines);
	    item->setValue(0, "");
	    item->setValue(1, "<None>");
	    item->setValue(2, transactions - shifts);
	    item->setValue(3, shifts);
	    item->setValue(4, counts.size());
	}
    } else {
	EmployeeSelect conditions;
	conditions.activeOnly = true;
	vector<Employee> employees;
	_quasar->db()->select(employees, conditions);

	for (unsigned int i = 0; i < employees.size(); ++i) {
	    const Employee& employee = employees[i];
	    int transactions = 0;
	    int shifts = 0;
	    vector<TenderCount> counts;

	    GltxSelect gltxCond;
	    gltxCond.store_id = _store->getId();
	    gltxCond.employee_id = employee.id();
	    gltxCond.end_date = _date->getDate();
	    gltxCond.unclosed = true;
	    gltxCond.cashrecOnly = true;
	    _quasar->db()->count(transactions, gltxCond);

	    gltxCond.type = DataObject::SHIFT;
	    _quasar->db()->count(shifts, gltxCond);

	    TenderCountSelect countCond;
	    countCond.store_id = _store->getId();
	    countCond.employee_id = employee.id();
	    countCond.end_date = _date->getDate();
	    countCond.activeOnly = true;
	    countCond.unclosed = true;
	    _quasar->db()->select(counts, countCond);

	    ListViewItem* item = new ListViewItem(_lines, employee.id());
	    item->setValue(0, employee.number());
	    item->setValue(1, employee.name());
	    item->setValue(2, transactions - shifts);
	    item->setValue(3, shifts);
	    item->setValue(4, counts.size());

	    if (employee.id() == currentId)
		_lines->setSelected(item, true);
	}

	int transactions = 0;
	int shifts = 0;
	vector<TenderCount> counts;

	GltxSelect gltxCond;
	gltxCond.store_id = _store->getId();
	gltxCond.blankEmployee = true;
	gltxCond.end_date = _date->getDate();
	gltxCond.unclosed = true;
	gltxCond.cashrecOnly = true;
	_quasar->db()->count(transactions, gltxCond);

	gltxCond.type = DataObject::SHIFT;
	_quasar->db()->count(shifts, gltxCond);

	TenderCountSelect countCond;
	countCond.store_id = _store->getId();
	countCond.blankEmployee = true;
	countCond.end_date = _date->getDate();
	countCond.activeOnly = true;
	countCond.unclosed = true;
	_quasar->db()->select(counts, countCond);

	if (transactions != 0 || shifts != 0 || counts.size() != 0) {
	    ListViewItem* item = new ListViewItem(_lines);
	    item->setValue(0, "");
	    item->setValue(1, "<None>");
	    item->setValue(2, transactions - shifts);
	    item->setValue(3, shifts);
	    item->setValue(4, counts.size());
	}
    }

    if (_lines->selectedItem() == NULL && _lines->firstChild() != NULL)
	_lines->setSelected(_lines->firstChild(), true);
    if (_lines->selectedItem() != NULL)
	_lines->ensureItemVisible(_lines->selectedItem());

    QApplication::restoreOverrideCursor();
}

// Used to sort summary tenders by menu number
static bool operator<(const SummaryLine& lhs, const SummaryLine& rhs) {
    return lhs.tender.menuNumber() < rhs.tender.menuNumber();
}

void
CashReconcile::slotReconcile()
{
    if (_lines->selectedItem() == NULL) {
	QString message;
	if (_company.shiftMethod() == Company::BY_STATION) {
	    message = tr("A station must be picked");
	} else {
	    message = tr("An employee must be picked");
	}
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    Id station_id = stationId();
    Id employee_id = employeeId();
    Id store_id = _store->getId();
    Id safe_store_id = _safeStore->getId();
    Id safe_id = _safeId->getId();
    Id adjust_id = _adjust->getId();
    Id transfer_id = _transfer->getId();

    Id safe_station_id;
    Id safe_employee_id;
    if (_company.shiftMethod() == Company::BY_STATION)
	safe_station_id = safe_id;
    else
	safe_employee_id = safe_id;

    if (store_id == INVALID_ID) {
	QString message = tr("A store must be entered");
	QMessageBox::critical(this, tr("Error"), message);
	_store->setFocus();
	return;
    }
    if (safe_store_id == INVALID_ID) {
	QString message = tr("A safe store must be entered");
	QMessageBox::critical(this, tr("Error"), message);
	_tabs->showPage(_setup);
	_safeStore->setFocus();
	return;
    }
    if (safe_id == INVALID_ID) {
	QString message = tr("A safe must be entered");
	QMessageBox::critical(this, tr("Error"), message);
	_tabs->showPage(_setup);
	_safeId->setFocus();
	return;
    }
    if (adjust_id == INVALID_ID) {
	QString message = tr("An over/short account must be entered");
	QMessageBox::critical(this, tr("Error"), message);
	_tabs->showPage(_setup);
	_adjust->setFocus();
	return;
    }
    if (transfer_id == INVALID_ID) {
	QString message = tr("A transfer account must be entered");
	QMessageBox::critical(this, tr("Error"), message);
	_tabs->showPage(_setup);
	_transfer->setFocus();
	return;
    }
    if (_shifts.size() == 0) {
	QString message = tr("You must have at least one shift");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QDate date = _date->getDate();
    if (date.isNull()) {
	QString message = tr("The date to reconcile is required");
	QMessageBox::critical(this, tr("Error"), message);
	_date->setFocus();
	return;
    }

    // Posting time is current time if closing for today or 11:59:59 PM
    QTime time = QTime::currentTime();
    if (date != QDate::currentDate())
	time = QTime(23, 59, 59);

    vector<Shift> shifts = _shifts;
    vector<TenderCount> counts = _counts;

    if (shifts.size() > 1 || counts.size() > 1) {
	// TODO: ask which shifts/counts to use
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Id shift_id = shifts[0].id();
    _tenders.clear();
    _summary.clear();

    // Process shifts and add tenders to summary
    unsigned int i;
    for (i = 0; i < shifts.size(); ++i) {
	const Shift& shift = shifts[i];

	GltxSelect conditions;
	conditions.shift_id = shift.id();
	conditions.activeOnly = true;
	vector<Gltx> gltxs;
	_quasar->db()->select(gltxs, conditions);

	for (unsigned int j = 0; j < gltxs.size(); ++j) {
	    const Gltx& gltx = gltxs[j];

	    for (unsigned int k = 0; k < gltx.tenders().size(); ++k) {
		Id tender_id = gltx.tenders()[k].tender_id;
		fixed amount = gltx.tenders()[k].conv_amt;
		if (gltx.tenders()[k].voided) continue;

		addShiftAmt(tender_id, amount);
	    }
	}
    }

    // Process counts and add tenders to summary
    for (i = 0; i < counts.size(); ++i) {
	const TenderCount& count = counts[i];

	for (unsigned int j = 0; j < count.tenders().size(); ++j) {
	    const TenderInfo& info = count.tenders()[j];
	    addCountAmt(info.tender_id, info.amount);
	}
    }

    // Sort tenders by menu number
    std::sort(_summary.begin(), _summary.end());

    // Prepare over/short transaction
    TenderAdjust adjustment;
    adjustment.setPostDate(date);
    adjustment.setPostTime(time);
    adjustment.setMemo(tr("Over/Short"));
    adjustment.setStoreId(store_id);
    adjustment.setStationId(station_id);
    adjustment.setEmployeeId(employee_id);
    adjustment.setShiftId(shift_id);
    adjustment.setAccountId(adjust_id);

    // Prepare transfer out transaction
    TenderAdjust transOut;
    transOut.setPostDate(date);
    transOut.setPostTime(time);
    transOut.setMemo(tr("Deposit to safe"));
    transOut.setStoreId(store_id);
    transOut.setStationId(station_id);
    transOut.setEmployeeId(employee_id);
    transOut.setShiftId(shift_id);
    transOut.setAccountId(transfer_id);

    // Memo for transfer in depends on method
    QString memo;
    if (station_id != INVALID_ID) {
	Station station;
	_quasar->db()->lookup(station_id, station);
	memo = tr("Deposit from: %1").arg(station.name());
    } else {
	Employee employee;
	_quasar->db()->lookup(employee_id, employee);
	memo = tr("Deposit from: %1").arg(employee.nameFL());
    }

    // Prepare transfer in transaction
    TenderAdjust transIn;
    transIn.setPostDate(date);
    transIn.setPostTime(time);
    transIn.setMemo(memo);
    transIn.setStoreId(safe_store_id);
    transIn.setStationId(safe_station_id);
    transIn.setEmployeeId(safe_employee_id);
    transIn.setAccountId(transfer_id);

    // Setup screen
    QDialog* dialog = new QDialog(this, "Summary", true);
    dialog->setCaption(tr("Reconcile Summary"));

    ListView* tenderList = new ListView(dialog);
    tenderList->addTextColumn(tr("Tender"), 20);
    tenderList->addMoneyColumn(tr("Shifts"));
    tenderList->addMoneyColumn(tr("Counts"));
    tenderList->addMoneyColumn(tr("Over/Short"));
    tenderList->setAllColumnsShowFocus(true);
    tenderList->setSorting(-1);

    QFrame* buttons = new QFrame(dialog);
    QPushButton* post = new QPushButton(tr("Reconcile"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);
    cancel->setDefault(true);

    connect(post, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(3);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(post, 0, 1);
    buttonGrid->addWidget(cancel, 0, 2);

    QGridLayout* grid = new QGridLayout(dialog);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(0, 1);
    grid->setColStretch(0, 1);
    grid->addWidget(tenderList, 0, 0);
    grid->addWidget(buttons, 1, 0);

    // Process summary info
    fixed shiftsTotal = 0.0;
    fixed countsTotal = 0.0;
    fixed overShort = 0.0;
    fixed transferAmt = 0.0;
    ListViewItem* last = NULL;
    for (i = 0; i < _summary.size(); ++i) {
	const Tender& tender = _summary[i].tender;
	fixed shiftsAmt = _summary[i].shiftsAmt;
	fixed countsAmt = _summary[i].countsAmt;
	fixed overShortAmt = countsAmt - shiftsAmt;

	// Add to table to show to user
	ListViewItem* item = new ListViewItem(tenderList, last);
	last = item;
	item->setValue(0, tender.name());
	item->setValue(1, shiftsAmt);
	item->setValue(2, countsAmt);
	item->setValue(3, overShortAmt);

	// Add to over/short if there is an amount
	if (overShortAmt != 0.0) {
	    TenderLine info;
	    info.tender_id = tender.id();
	    info.conv_amt = overShortAmt;
	    info.conv_rate = tender.convertRate();
	    info.amount = overShortAmt * tender.convertRate();
	    info.amount.moneyRound();
	    info.is_change = false;
	    info.voided = false;
	    adjustment.tenders().push_back(info);
	    overShort += info.amount;
	}

	// Add counted to transfer in/out
	TenderLine info;
	info.tender_id = tender.id();
	info.conv_amt = countsAmt;
	info.conv_rate = tender.convertRate();
	info.amount = countsAmt * tender.convertRate();
	info.amount.moneyRound();
	info.is_change = false;
	info.voided = false;
	transIn.tenders().push_back(info);

	transferAmt += info.amount;
	info.amount = -info.amount;
	info.conv_amt = -info.conv_amt;
	transOut.tenders().push_back(info);

	shiftsTotal += shiftsAmt;
	countsTotal += countsAmt;
    }

    // Add total line
    ListViewItem* item = new ListViewItem(tenderList, last);
    item->setValue(0, tr("Total"));
    item->setValue(1, shiftsTotal);
    item->setValue(2, countsTotal);
    item->setValue(3, countsTotal - shiftsTotal);

    // Show table to user and allow cancel or post
    QApplication::restoreOverrideCursor();
    int ch = dialog->exec();
    if (ch != QDialog::Accepted) return;

    // Check total over/short
    if (overShort > 5.0 || overShort < -5.0) {
	QString message = tr("Warning - your over/short is large.\n"
			     "Are you sure you mean to post this?");
	int ch = QMessageBox::warning(this, tr("Post?"), message,
				      QMessageBox::No, QMessageBox::Yes);
	if (ch != QMessageBox::Yes) return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Add accounts to adjustment
    adjustment.accounts().push_back(AccountLine(adjust_id, -overShort));
    for (i = 0; i < adjustment.tenders().size(); ++i) {
	Id tender_id = adjustment.tenders()[i].tender_id;
	fixed amount = adjustment.tenders()[i].amount;

	Tender tender;
	findTender(tender_id, tender);

	Id tender_acct = tender.accountId();
	if (_company.shiftMethod() == Company::BY_STATION) {
	    if (adjustment.stationId() == safe_station_id)
		if (tender.safeId() != INVALID_ID)
		    tender_acct = tender.safeId();
	} else {
	    if (adjustment.employeeId() == safe_employee_id)
		if (tender.safeId() != INVALID_ID)
		    tender_acct = tender.safeId();
	}

	adjustment.accounts().push_back(AccountLine(tender_acct, amount));
    }

    // Add accounts to transfer out
    transOut.accounts().push_back(AccountLine(transfer_id, transferAmt));
    for (i = 0; i < transOut.tenders().size(); ++i) {
	Id tender_id = transOut.tenders()[i].tender_id;
	fixed amount = transOut.tenders()[i].amount;

	Tender tender;
	findTender(tender_id, tender);

	Id tender_acct = tender.accountId();
	if (_company.shiftMethod() == Company::BY_STATION) {
	    if (transOut.stationId() == safe_station_id)
		if (tender.safeId() != INVALID_ID)
		    tender_acct = tender.safeId();
	} else {
	    if (transOut.employeeId() == safe_employee_id)
		if (tender.safeId() != INVALID_ID)
		    tender_acct = tender.safeId();
	}

	transOut.accounts().push_back(AccountLine(tender_acct, amount));
    }

    // Add accounts to transfer in
    transIn.accounts().push_back(AccountLine(transfer_id, -transferAmt));
    for (i = 0; i < transIn.tenders().size(); ++i) {
	Id tender_id = transIn.tenders()[i].tender_id;
	fixed amount = transIn.tenders()[i].amount;

	Tender tender;
	findTender(tender_id, tender);

	Id tender_acct = tender.safeId();
	if (tender_acct == INVALID_ID)
	    tender_acct = tender.accountId();

	transIn.accounts().push_back(AccountLine(tender_acct, amount));
    }

    // Post adjustment if any tenders
    if (adjustment.tenders().size() > 0) {
	QApplication::restoreOverrideCursor();
	if (!_quasar->db()->create(adjustment)) {
	    QString message = tr("Failed creating over/short transaction");
	    QMessageBox::critical(this, tr("Error"), message);
	    return;
	}
    }

    // Post transfers
    if (!_quasar->db()->create(transOut)) {
	QApplication::restoreOverrideCursor();
	QString message = tr("Failed creating transfer out transaction");
	QMessageBox::critical(this, tr("Error"), message);
	if (adjustment.id() != INVALID_ID)
	    _quasar->db()->remove(adjustment);
	return;
    }
    if (!_quasar->db()->create(transIn)) {
	QApplication::restoreOverrideCursor();
	QString message = tr("Failed creating transfer in transaction");
	QMessageBox::critical(this, tr("Error"), message);
	if (adjustment.id() != INVALID_ID)
	    _quasar->db()->remove(adjustment);
	_quasar->db()->remove(transOut);
	return;
    }

    // Link the transfers
    _quasar->db()->linkTx(transOut.id(), transIn.id());

    // Set shift in shifts
    for (int cnt = shifts.size() - 1; cnt >= 0; --cnt) {
	Shift shift = shifts[cnt];
	shift.setShiftId(shift_id);
	shift.setAdjustmentId(adjustment.id());
	shift.setTransferId(transOut.id());
	_quasar->db()->update(shifts[cnt], shift);
    }

    // Set shift in counts
    for (i = 0; i < counts.size(); ++i) {
	TenderCount count = counts[i];
	count.setShiftId(shift_id);
	_quasar->db()->update(counts[i], count);
    }

    Company orig, company;
    _quasar->db()->lookup(orig);
    company = orig;

    company.setSafeStore(safe_store_id);
    company.setSafeStation(safe_station_id);
    company.setSafeEmployee(safe_employee_id);
    company.setOverShortAccount(adjust_id);
    company.setTransferAccount(transfer_id);
    _quasar->db()->update(orig, company);

    QApplication::restoreOverrideCursor();
    slotRefresh();
}

Id
CashReconcile::stationId()
{
    ListViewItem* item = _lines->selectedItem();
    if (item == NULL)
	return INVALID_ID;

    if (_company.shiftMethod() != Company::BY_STATION)
	return INVALID_ID;

    return item->id;
}

Id
CashReconcile::employeeId()
{
    ListViewItem* item = _lines->selectedItem();
    if (item == NULL)
	return INVALID_ID;

    if (_company.shiftMethod() != Company::BY_EMPLOYEE)
	return INVALID_ID;

    return item->id;
}

void
CashReconcile::addShiftAmt(Id tender_id, fixed shift_amt)
{
    for (unsigned int i = 0; i < _summary.size(); ++i) {
	if (_summary[i].tender.id() == tender_id) {
	    _summary[i].shiftsAmt += shift_amt;
	    return;
	}
    }

    Tender tender;
    findTender(tender_id, tender);

    SummaryLine line;
    line.tender = tender;
    line.shiftsAmt = shift_amt;
    _summary.push_back(line);
}

void
CashReconcile::addCountAmt(Id tender_id, fixed count_amt)
{
    for (unsigned int i = 0; i < _summary.size(); ++i) {
	if (_summary[i].tender.id() == tender_id) {
	    _summary[i].countsAmt += count_amt;
	    return;
	}
    }

    Tender tender;
    findTender(tender_id, tender);

    SummaryLine line;
    line.tender = tender;
    line.countsAmt = count_amt;
    _summary.push_back(line);
}

bool
CashReconcile::findTender(Id tender_id, Tender& tender)
{
    for (unsigned int i = 0; i < _tenders.size(); ++i) {
	if (_tenders[i].id() == tender_id) {
	    tender = _tenders[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(tender_id, tender))
	return false;

    _tenders.push_back(tender);
    return true;
}
