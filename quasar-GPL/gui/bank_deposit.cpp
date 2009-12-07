// $Id: bank_deposit.cpp,v 1.15 2005/03/13 23:13:45 bpepers Exp $
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

#include "bank_deposit.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "tender_select.h"
#include "gltx.h"
#include "gltx_select.h"
#include "tender_adjust.h"
#include "lookup_edit.h"
#include "store_lookup.h"
#include "station_lookup.h"
#include "employee_lookup.h"
#include "tender_lookup.h"
#include "money_edit.h"
#include "table.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <algorithm>

// Used to sort tenders by menu number
static bool operator<(const Tender& lhs, const Tender& rhs) {
    return lhs.menuNumber() < rhs.menuNumber();
}

BankDeposit::BankDeposit(MainWindow* main)
    : QuasarWindow(main, "BankDeposit")
{
    _helpSource = "bank_deposit.html";

    _quasar->db()->lookup(_company);

    QFrame* frame = new QFrame(this);

    QLabel* safeStoreLabel = new QLabel(tr("Safe Store:"), frame);
    _safeStore = new LookupEdit(new StoreLookup(main, this), frame);
    _safeStore->setLength(30);
    safeStoreLabel->setBuddy(_safeStore);

    QLabel* safeIdLabel;
    if (_company.shiftMethod() == Company::BY_STATION) {
	safeIdLabel = new QLabel(tr("Safe Station:"), frame);
	_safeId = new LookupEdit(new StationLookup(main, this), frame);
    } else {
	safeIdLabel = new QLabel(tr("Safe Employee:"), frame);
	_safeId = new LookupEdit(new EmployeeLookup(main, this), frame);
    }
    _safeId->setLength(30);
    safeIdLabel->setBuddy(_safeId);

    _tenders = new Table(frame);
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
    new MoneyColumn(_tenders, tr("In Safe"), 10);
    new MoneyColumn(_tenders, tr("Amount"), 10);

    // Add editors
    new NumberEditor(_tenders, 2, new MoneyEdit(_tenders));

    QLabel* totalLabel = new QLabel(tr("Tender Total:"), frame);
    _total = new MoneyEdit(frame);
    _total->setFocusPolicy(NoFocus);

    QFrame* buttons = new QFrame(frame);

    QPushButton* refresh = new QPushButton(tr("&Refresh"), buttons);
    connect(refresh, SIGNAL(clicked()), SLOT(slotRefresh()));

    QPushButton* all = new QPushButton(tr("Deposit All"), buttons);
    connect(all, SIGNAL(clicked()), SLOT(slotAll()));

    QPushButton* ok = new QPushButton(tr("&OK"), buttons);
    connect(ok, SIGNAL(clicked()), SLOT(slotOk()));

    QPushButton* cancel = new QPushButton(tr("&Cancel"), buttons);
    connect(cancel, SIGNAL(clicked()), SLOT(close()));

    ok->setMinimumSize(cancel->sizeHint());
    cancel->setMinimumSize(cancel->sizeHint());

    QGridLayout* buttonsGrid = new QGridLayout(buttons);
    buttonsGrid->setSpacing(6);
    buttonsGrid->setMargin(6);
    buttonsGrid->setColStretch(2, 1);
    buttonsGrid->addWidget(refresh, 0, 0, AlignLeft | AlignVCenter);
    buttonsGrid->addWidget(all, 0, 1, AlignLeft | AlignVCenter);
    buttonsGrid->addWidget(cancel, 0, 2, AlignRight | AlignVCenter);
    buttonsGrid->addWidget(ok, 0, 3, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setRowStretch(2, 1);
    grid->setColStretch(1, 1);
    grid->addWidget(safeStoreLabel, 0, 0);
    grid->addWidget(_safeStore, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(safeIdLabel, 1, 0);
    grid->addWidget(_safeId, 1, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_tenders, 2, 2, 0, 2);
    grid->addMultiCellWidget(totalLabel, 3, 3, 0, 1, AlignRight|AlignVCenter);
    grid->addWidget(_total, 3, 2, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(buttons, 4, 4, 0, 2);

    _safeStore->setId(_company.safeStore());
    if (_company.shiftMethod() == Company::BY_STATION)
	_safeId->setId(_company.safeStation());
    else
	_safeId->setId(_company.safeEmployee());

    TenderSelect conditions;
    conditions.activeOnly = true;
    conditions.bankOnly = true;
    _quasar->db()->select(_tenderList, conditions);
    std::sort(_tenderList.begin(), _tenderList.end());

    unsigned int i;
    for (i = 0; i < _tenderList.size(); ++i) {
	VectorRow* row = new VectorRow(_tenders->columns());
	row->setValue(0, _tenderList[i].name());
	row->setValue(1, "");
	row->setValue(2, "");
	_tenders->appendRow(row);
    }
    _tenders->setCurrentCell(0, 2);
    qApp->processEvents();
    _tenders->verticalScrollBar()->setValue(0);
    _date = QDate::currentDate();
    _tenders->setFocus();

    slotRefresh();

    setCentralWidget(frame);
    setCaption(tr("Bank Deposit"));
    finalize();
}

BankDeposit::~BankDeposit()
{
}

void
BankDeposit::setDate(QDate date)
{
    _date = date;
    slotRefresh();
}

void
BankDeposit::setShifts(const vector<Shift>& shifts)
{
    _shifts = shifts;
}

void
BankDeposit::slotRefresh()
{
    for (int row = 0; row < _tenders->rows(); ++row) {
	_tenders->setCellValue(row, 1, "");
	_tenders->setCellValue(row, 2, "");
    }

    Id store_id = _safeStore->getId();
    Id safe_id = _safeId->getId();
    if (store_id == INVALID_ID || safe_id == INVALID_ID)
	return;

    Id station_id;
    Id employee_id;
    if (_company.shiftMethod() == Company::BY_STATION)
	station_id = safe_id;
    else
	employee_id = safe_id;

    GltxSelect select;
    select.store_id = store_id;
    select.station_id = station_id;
    select.employee_id = employee_id;
    select.end_date = _date;
    select.activeOnly = true;

    vector<Gltx> gltxs;
    if (_shifts.size() == 0) {
	select.unclosed = true;
	_quasar->db()->select(gltxs, select);
    } else {
	for (unsigned int i = 0; i < _shifts.size(); ++i) {
	    select.shift_id = _shifts[i].id();
	    vector<Gltx> shift_txs;
	    _quasar->db()->select(shift_txs, select);
	    for (unsigned int j = 0; j < shift_txs.size(); ++j)
		gltxs.push_back(shift_txs[j]);
	}
    }

    vector<Id> tender_ids;
    vector<fixed> tender_amts;
    unsigned int i;
    for (i = 0; i < gltxs.size(); ++i) {
	const Gltx& gltx = gltxs[i];
	for (unsigned int j = 0; j < gltx.tenders().size(); ++j) {
	    const TenderLine& info = gltx.tenders()[j];
	    if (info.voided) continue;

	    bool found = false;
	    for (unsigned int k = 0; k < tender_ids.size(); ++k) {
		if (tender_ids[k] != info.tender_id) continue;

		tender_amts[k] += info.conv_amt;
		found = true;
		break;
	    }

	    if (!found) {
		tender_ids.push_back(info.tender_id);
		tender_amts.push_back(info.conv_amt);
	    }
	}
    }

    for (i = 0; i < tender_ids.size(); ++i) {
	if (tender_amts[i] == 0.0)
	    continue;

	for (int row = 0; row < _tenders->rows(); ++row) {
	    if (_tenderList[row].id() == tender_ids[i]) {
		_tenders->setCellValue(row, 1, tender_amts[i]);
	    }
	}
    }
}

void
BankDeposit::slotAll()
{
    for (int row = 0; row < _tenders->rows(); ++row)
	_tenders->setCellValue(row, 2, _tenders->cellValue(row, 1));
    recalculate();
}

void
BankDeposit::slotOk()
{
    Id store_id = _safeStore->getId();
    Id safe_id = _safeId->getId();

    if (store_id == INVALID_ID) {
	QString message = tr("A safe store must be entered");
	QMessageBox::critical(this, tr("Error"), message);
	_safeStore->setFocus();
	return;
    }
    if (safe_id == INVALID_ID) {
	QString message = tr("A safe must be entered");
	QMessageBox::critical(this, tr("Error"), message);
	_safeId->setFocus();
	return;
    }

    Id station_id;
    Id employee_id;
    if (_company.shiftMethod() == Company::BY_STATION)
	station_id = safe_id;
    else
	employee_id = safe_id;

    Id shift_id;
    if (_shifts.size() != 0)
	shift_id = _shifts[0].id();

    bool allOk = true;
    for (int row = 0; row < _tenders->rows(); ++row) {
	Tender& tender = _tenderList[row];
	fixed conv_amt = _tenders->cellValue(row, 2).toFixed();
	if (conv_amt == 0.0) continue;

	Id tender_acct = tender.safeId();
	if (tender_acct == INVALID_ID)
	    tender_acct = tender.accountId();
	Id bank_id = tender.bankId();

	fixed amount = conv_amt * tender.convertRate();
	amount.moneyRound();

	TenderAdjust adjustment;
	adjustment.setPostDate(_date);
	adjustment.setPostTime(QTime::currentTime());
	adjustment.setMemo(tr("Bank deposit"));
	adjustment.setShiftId(shift_id);
	adjustment.setStoreId(store_id);
	adjustment.setStationId(station_id);
	adjustment.setEmployeeId(employee_id);
	adjustment.setAccountId(bank_id);

	TenderLine line;
	line.tender_id = tender.id();
	line.amount = -amount;
	line.conv_rate = tender.convertRate();
	line.conv_amt = -conv_amt;
	adjustment.tenders().push_back(line);

	adjustment.accounts().push_back(AccountLine(bank_id, amount));
	adjustment.accounts().push_back(AccountLine(tender_acct, -amount));

	if (!_quasar->db()->create(adjustment)) {
	    QString message = tr("Bank deposit failed");
	    QMessageBox::critical(this, tr("Error"), message);
	    allOk = false;
	    break;
	}

	_tenders->setCellValue(row, 1, "");
	_tenders->setCellValue(row, 2, "");
    }

    emit depositDone();
    if (allOk) close();
}

void
BankDeposit::cellChanged(int, int, Variant)
{
    recalculate();
}

void
BankDeposit::focusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _tenders->currentRow();

    newCol = 2;
    if (type == Table::MovePrev) {
	if (row == 0)
	    leave = true;
	else
	    newRow = row - 1;
    }
}

void
BankDeposit::recalculate()
{
    fixed total = 0.0;
    for (int row = 0; row < _tenders->rows(); ++row)
	total += _tenders->cellValue(row, 2).toFixed();

    _total->setFixed(total);
}
