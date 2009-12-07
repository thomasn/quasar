// $Id: tender_transfer.cpp,v 1.23 2005/03/13 22:10:02 bpepers Exp $
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

#include "tender_transfer.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "tender_select.h"
#include "tender.h"
#include "account.h"
#include "gltx_frame.h"
#include "table.h"
#include "money_edit.h"
#include "lookup_edit.h"
#include "tender_lookup.h"
#include "account_lookup.h"
#include "store_lookup.h"
#include "station_lookup.h"
#include "employee_lookup.h"
#include "gltx_lookup.h"
#include "date_popup.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpopupmenu.h>
#include <qvbox.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qlayout.h>
#include <qmessagebox.h>

TenderTransfer::TenderTransfer(MainWindow* main, Id transfer_id)
    : DataWindow(main, "TenderTransfer", transfer_id)
{
    _helpSource = "tender_transfer.html";

    // Get the company for deposit info
    _quasar->db()->lookup(_company);

    // Create widgets
    _gltxFrame = new GltxFrame(main, tr("Transfer No."), _frame);
    _gltxFrame->setTitle(tr("From"));

    _tenders = new Table(_frame);
    _tenders->setVScrollBarMode(QScrollView::AlwaysOn);
    _tenders->setLeftMargin(fontMetrics().width("99999"));
    _tenders->setDisplayRows(6);
    connect(_tenders, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(cellChanged(int,int,Variant)));
    connect(_tenders, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(focusNext(bool&,int&,int&,int)));
    connect(_tenders, SIGNAL(rowDeleted(int)), SLOT(recalculate()));

    // Lookups
    LookupWindow* tenderLookup = new TenderLookup(_main, this);

    // Add columns
    new LookupColumn(_tenders, tr("Tender"), 20, tenderLookup);
    new MoneyColumn(_tenders, tr("Amount"));
    new NumberColumn(_tenders, tr("Rate"), 3);
    new MoneyColumn(_tenders, tr("Total"));

    // Add editors
    new LookupEditor(_tenders, 0, new LookupEdit(tenderLookup, _tenders));
    new NumberEditor(_tenders, 1, new MoneyEdit(_tenders));

    QGroupBox* to = new QGroupBox(tr("To"), _frame);

    QLabel* toNumberLabel = new QLabel(tr("Transfer No."), to);
    _toNumber = new LineEdit(9, to);
    toNumberLabel->setBuddy(_toNumber);

    QLabel* toStoreLabel = new QLabel(tr("Store:"), to);
    _toStore = new LookupEdit(new StoreLookup(_main, this), to);
    _toStore->setLength(30);
    toStoreLabel->setBuddy(_toStore);

    QLabel* toShiftLabel = new QLabel(tr("Shift:"), to);
    _toShift = new LookupEdit(new GltxLookup(_main, this, DataObject::SHIFT),
			       to);
    _toShift->setLength(10);
    _toShift->setFocusPolicy(ClickFocus);
    toShiftLabel->setBuddy(_toShift);

    QLabel* toStationLabel = new QLabel(tr("Station:"), to);
    _toStation = new LookupEdit(new StationLookup(_main, this), to);
    _toStation->setLength(30);
    toStationLabel->setBuddy(_toStation);

    QLabel* toEmployeeLabel = new QLabel(tr("Employee:"), to);
    _toEmployee = new LookupEdit(new EmployeeLookup(_main, this), to);
    _toEmployee->setLength(30);
    toEmployeeLabel->setBuddy(_toEmployee);

    QGridLayout* toGrid = new QGridLayout(to, 1, 1, to->frameWidth() * 2);
    toGrid->setSpacing(3);
    toGrid->setMargin(6);
    toGrid->setColStretch(2, 1);
    toGrid->addColSpacing(2, 10);
    toGrid->addRowSpacing(0, to->fontMetrics().height());
    toGrid->addWidget(toNumberLabel, 1, 0);
    toGrid->addWidget(_toNumber, 1, 1, AlignLeft | AlignVCenter);
    toGrid->addWidget(toStoreLabel, 1, 3);
    toGrid->addWidget(_toStore, 1, 4, AlignLeft | AlignVCenter);
    toGrid->addWidget(toShiftLabel, 2, 0);
    toGrid->addWidget(_toShift, 2, 1, AlignLeft | AlignVCenter);
    toGrid->addWidget(toStationLabel, 2, 3);
    toGrid->addWidget(_toStation, 2, 4, AlignLeft | AlignVCenter);
    toGrid->addWidget(toEmployeeLabel, 3, 3);
    toGrid->addWidget(_toEmployee, 3, 4, AlignLeft | AlignVCenter);

    QLabel* accountLabel = new QLabel(tr("Transfer Account:"), _frame);
    AccountLookup* lookup = new AccountLookup(main, this, Account::Expense);
    _account = new LookupEdit(lookup, _frame);
    _account->setLength(30);
    accountLabel->setBuddy(_account);

    QLabel* totalLabel = new QLabel(tr("Transfer Amount:"), _frame);
    _total = new MoneyEdit(_frame);
    _total->setLength(14);
    _total->setFocusPolicy(NoFocus);
    totalLabel->setBuddy(_total);

    _inactive->setText(tr("Voided?"));

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(2, 1);
    grid->setRowStretch(1, 1);
    grid->addMultiCellWidget(_gltxFrame, 0, 0, 0, 4);
    grid->addMultiCellWidget(_tenders, 1, 1, 0, 4);
    grid->addMultiCellWidget(to, 2, 2, 0, 4);
    grid->addWidget(accountLabel, 3, 0);
    grid->addWidget(_account, 3, 1, AlignLeft | AlignVCenter);
    grid->addWidget(totalLabel, 3, 3);
    grid->addWidget(_total, 3, 4, AlignLeft | AlignVCenter);

    setCaption(tr("Tender Transfer"));
    finalize();
}

TenderTransfer::~TenderTransfer()
{
}

void
TenderTransfer::setStore(Id store_id)
{
    _gltxFrame->store->setId(store_id);
}

void
TenderTransfer::setStation(Id station_id)
{
    _gltxFrame->station->setId(station_id);
}

void
TenderTransfer::setEmployee(Id employee_id)
{
    _gltxFrame->employee->setId(employee_id);
}

void
TenderTransfer::setDate(QDate date)
{
    if (date.isNull())
	date = QDate::currentDate();
    _gltxFrame->date->setDate(date);
}

void
TenderTransfer::oldItem()
{
    _quasar->db()->lookup(_id, _orig);
    _quasar->db()->lookup(_orig.linkId(), _origLink);

    // Switch to show consistantly
    if (_orig.number() > _origLink.number()) {
	TenderAdjust temp = _orig;
	_orig = _origLink;
	_origLink = temp;
    }

    _curr = _orig;
    _link = _origLink;
    _firstField = _gltxFrame->firstField();
}

void
TenderTransfer::newItem()
{
    TenderAdjust blank;
    _orig = blank;
    _gltxFrame->defaultData(_orig);
    _orig.setAccountId(_company.transferAccount());

    _origLink = _orig;
    _origLink.setStationId(INVALID_ID);
    _origLink.setEmployeeId(INVALID_ID);

    _curr = _orig;
    _link = _origLink;
    _firstField = _gltxFrame->firstField();
}

void
TenderTransfer::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _quasar->db()->lookup(_curr.linkId(), _link);
    _gltxFrame->cloneData(_curr);
    _gltxFrame->cloneData(_link);
    dataToWidget();
}

bool
TenderTransfer::fileItem()
{
    if (_curr.number() != "#")
	if (checkGltxUsed(_curr.number(), DataObject::TEND_ADJUST, _curr.id()))
	    return false;
    if (_link.number() != "#")
	if (checkGltxUsed(_link.number(), DataObject::TEND_ADJUST, _link.id()))
	    return false;

    // Setup linked transaction which is reverse
    _link.setReference(_curr.reference());
    _link.setPostDate(_curr.postDate());
    _link.setPostTime(_curr.postTime());
    _link.setMemo(_curr.memo());
    _link.accounts() = _curr.accounts();
    _link.tenders() = _curr.tenders();
    unsigned int i;
    for (i = 0; i < _link.tenders().size(); ++i) {
	_link.tenders()[i].amount *= -1;
	_link.tenders()[i].conv_amt *= -1;

	Tender tender;
	_quasar->db()->lookup(_link.tenders()[i].tender_id, tender);

	Id account_id = tender.accountId();
	if (_company.shiftMethod() == Company::BY_STATION) {
	    if (_link.stationId() == _company.safeStation())
		if (tender.safeId() != INVALID_ID)
		    account_id = tender.safeId();
	} else {
	    if (_link.employeeId() == _company.safeEmployee())
		if (tender.safeId() != INVALID_ID)
		    account_id = tender.safeId();
	}

	_link.accounts()[i].account_id = account_id;
	_link.accounts()[i].amount *= -1;
    }
    _link.accounts()[_link.tenders().size()].amount *= -1;

    if (_orig.id() == INVALID_ID) {
	if (!_quasar->db()->create(_curr))
	    return false;
	if (!_quasar->db()->create(_link)) {
	    _quasar->db()->remove(_curr);
	    return false;
	}
	if (!_quasar->db()->linkTx(_curr.id(), _link.id())) {
	    _quasar->db()->remove(_curr);
	    _quasar->db()->remove(_link);
	    return false;
	}
    } else {
	if (!_quasar->db()->update(_orig, _curr))
	    return false;
	if (!_quasar->db()->update(_origLink, _link)) {
	    _quasar->db()->update(_curr, _orig);
	    return false;
	}
    }

    if (_company.transferAccount() == INVALID_ID) {
	Company orig = _company;
	_company.setTransferAccount(_curr.accountId());
	_quasar->db()->update(orig, _company);
    }

    _orig = _curr;
    _origLink = _link;
    _id = _curr.id();

    return true;
}

bool
TenderTransfer::deleteItem()
{
    if (!_quasar->db()->remove(_curr))
	return false;
    if (!_quasar->db()->remove(_link)) {
	_quasar->db()->create(_curr);
	_quasar->db()->linkTx(_curr.id(), _link.id());
	return false;
    }
    return true;
}

void
TenderTransfer::restoreItem()
{
    _curr = _orig;
    _link = _origLink;
}

void
TenderTransfer::cloneItem()
{
    TenderTransfer* clone = new TenderTransfer(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
TenderTransfer::isChanged()
{
    return _curr != _orig || _link != _origLink;
}

// Set the widgets from the data object.
void 
TenderTransfer::dataToWidget()
{
    _gltxFrame->setData(_curr);
    _toNumber->setText(_link.number());
    _toShift->setId(_link.shiftId());
    _toStore->setId(_link.storeId());
    _toStation->setId(_link.stationId());
    _toEmployee->setId(_link.employeeId());
    _account->setId(_curr.accountId());
    _inactive->setChecked(!_curr.isActive());

    // Load the tenders
    _tenders->clear();
    for (unsigned int i = 0; i < _curr.tenders().size(); ++i) {
	const TenderLine& line = _curr.tenders()[i];
	if (line.voided) continue;

	VectorRow* row = new VectorRow(_tenders->columns());
	row->setValue(0, line.tender_id);
	row->setValue(1, -line.conv_amt);
	row->setValue(2, line.conv_rate);
	row->setValue(3, -line.amount);
	_tenders->appendRow(row);
    }
    _tenders->appendRow(new VectorRow(_tenders->columns()));

    recalculate();
}

// Set the data object from the widgets.
void
TenderTransfer::widgetToData()
{
    _gltxFrame->getData(_curr);
    _link.setNumber(_toNumber->text());
    _link.setShiftId(_toShift->getId());
    _link.setStoreId(_toStore->getId());
    _link.setStationId(_toStation->getId());
    _link.setEmployeeId(_toEmployee->getId());
    _curr.setAccountId(_account->getId());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());
    _quasar->db()->setActive(_link, !_inactive->isChecked());

    vector<AccountLine>& accounts = _curr.accounts();
    accounts.clear();

    // Add the tenders
    _curr.tenders().clear();
    fixed total_amt = 0.0;
    for (int row = 0; row < _tenders->rows(); ++row) {
	Id tender_id = _tenders->cellValue(row, 0).toId();
	if (tender_id == INVALID_ID) continue;
	fixed conv_amt = _tenders->cellValue(row, 1).toFixed();
	fixed conv_rate = _tenders->cellValue(row, 2).toFixed();
	fixed amount = _tenders->cellValue(row, 3).toFixed();

	Tender tender;
	_quasar->db()->lookup(tender_id, tender);

	TenderLine line;
	line.tender_id = tender_id;
	line.amount = -amount;
	line.conv_rate = conv_rate;
	line.conv_amt = -conv_amt;
	_curr.tenders().push_back(line);

	Id account_id = tender.accountId();
	if (_company.shiftMethod() == Company::BY_STATION) {
	    if (_curr.stationId() == _company.safeStation())
		if (tender.safeId() != INVALID_ID)
		    account_id = tender.safeId();
	} else {
	    if (_curr.employeeId() == _company.safeEmployee())
		if (tender.safeId() != INVALID_ID)
		    account_id = tender.safeId();
	}

	accounts.push_back(AccountLine(account_id, -amount));
	total_amt += amount;
    }

    // Add the expense account
    Id expense_id = _account->getId();
    if (expense_id != INVALID_ID)
	accounts.push_back(AccountLine(expense_id, total_amt));
}

void
TenderTransfer::cellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _tenders->rows() - 1 && col == 0) {
	Id id = _tenders->cellValue(row, col).toId();
	if (id != INVALID_ID)
	    _tenders->appendRow(new VectorRow(_tenders->columns()));
    }

    // Check for other changes
    Id tender_id;
    switch (col) {
    case 0: // tender_id
	tender_id = _tenders->cellValue(row, 0).toId();
	if (tender_id != INVALID_ID) {
	    Tender tender;
	    _quasar->db()->lookup(tender_id, tender);

	    _tenders->setCellValue(row, 1, 0.0);
	    _tenders->setCellValue(row, 2, tender.convertRate());
	    _tenders->setCellValue(row, 3, 0.0);
	} else {
	    _tenders->setCellValue(row, 1, "");
	    _tenders->setCellValue(row, 2, "");
	    _tenders->setCellValue(row, 3, "");
	}
	break;
    case 1: // amount
	if (!_tenders->cellValue(row, 2).toString().isEmpty()) {
	    fixed amount = _tenders->cellValue(row, 1).toFixed();
	    fixed rate = _tenders->cellValue(row, 2).toFixed();
	    _tenders->setCellValue(row, 3, amount * rate);
	}
	break;
    }

    recalculate();
}

void
TenderTransfer::focusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _tenders->currentRow();
    int col = _tenders->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id id = _tenders->cellValue(row, col).toId();
	if (id == INVALID_ID && row == _tenders->rows() - 1) {
	    leave = true;
	}
    } else if (type == Table::MoveNext && col == 1) {
	newCol = 0;
	newRow = row + 1;
    } else if (type == Table::MovePrev && col == 0) {
	if (row > 0) {
	    newCol = 1;
	    newRow = row - 1;
	}
    }
}

void
TenderTransfer::recalculate()
{
    fixed total = 0.0;
    for (int i = 0; i < _tenders->rows(); ++i) {
	Id tender_id = _tenders->cellValue(i, 0).toId();
	if (tender_id == INVALID_ID) continue;

	total += _tenders->cellValue(i, 3).toFixed();
    }

    if (total == 0.0)
	_total->clear();
    else
	_total->setFixed(-total);
}
