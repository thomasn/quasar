// $Id: tender_adjustment.cpp,v 1.20 2005/03/13 23:13:45 bpepers Exp $
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

#include "tender_adjustment.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "company.h"
#include "tender_select.h"
#include "tender.h"
#include "account.h"
#include "gltx_frame.h"
#include "table.h"
#include "money_edit.h"
#include "lookup_edit.h"
#include "store_lookup.h"
#include "gltx_lookup.h"
#include "tender_lookup.h"
#include "account_lookup.h"
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

TenderAdjustment::TenderAdjustment(MainWindow* main, Id adjustment_id)
    : DataWindow(main, "TenderAdjustment", adjustment_id)
{
    _helpSource = "tender_adjustment.html";

    // Create widgets
    _gltxFrame = new GltxFrame(main, tr("Adjustment No."), _frame);

    _tenders = new Table(_frame);
    _tenders->setVScrollBarMode(QScrollView::AlwaysOn);
    _tenders->setLeftMargin(fontMetrics().width("99999"));
    _tenders->setDisplayRows(6);
    connect(_tenders, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(cellChanged(int,int,Variant)));
    connect(_tenders, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(focusNext(bool&,int&,int&,int)));

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

    QLabel* accountLabel = new QLabel(tr("Expense Account:"), _frame);
    AccountLookup* lookup = new AccountLookup(main, this);
    _account = new LookupEdit(lookup, _frame);
    _account->setLength(30);
    accountLabel->setBuddy(_account);

    QLabel* totalLabel = new QLabel(tr("Expense Amount:"), _frame);
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
    grid->addMultiCellWidget(_gltxFrame, 0, 0, 0, 2);
    grid->addMultiCellWidget(_tenders, 1, 1, 0, 2);
    grid->addWidget(accountLabel, 2, 0);
    grid->addWidget(_account, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(totalLabel, 3, 0);
    grid->addWidget(_total, 3, 1, AlignLeft | AlignVCenter);

    setCaption(tr("Tender Adjustment"));
    finalize();
}

TenderAdjustment::~TenderAdjustment()
{
}

void
TenderAdjustment::setStore(Id store_id)
{
    _gltxFrame->store->setId(store_id);
}

void
TenderAdjustment::setStation(Id station_id)
{
    _gltxFrame->station->setId(station_id);
}

void
TenderAdjustment::setEmployee(Id employee_id)
{
    _gltxFrame->employee->setId(employee_id);
}

void
TenderAdjustment::setDate(QDate date)
{
    if (date.isNull())
	date = QDate::currentDate();
    _gltxFrame->date->setDate(date);
}

void
TenderAdjustment::oldItem()
{
    _quasar->db()->lookup(_id, _orig);
    _curr = _orig;
    _firstField = _gltxFrame->firstField();
}

void
TenderAdjustment::newItem()
{
    TenderAdjust blank;
    _orig = blank;
    _gltxFrame->defaultData(_orig);

    _curr = _orig;
    _firstField = _gltxFrame->firstField();
}

void
TenderAdjustment::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _gltxFrame->cloneData(_curr);
    dataToWidget();
}

bool
TenderAdjustment::fileItem()
{
    if (_curr.number() != "#")
	if (checkGltxUsed(_curr.number(), DataObject::TEND_ADJUST, _curr.id()))
	    return false;

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
TenderAdjustment::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
TenderAdjustment::restoreItem()
{
    _curr = _orig;
}

void
TenderAdjustment::cloneItem()
{
    TenderAdjustment* clone = new TenderAdjustment(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
TenderAdjustment::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
TenderAdjustment::dataToWidget()
{
    _gltxFrame->setData(_curr);
    _account->setId(_curr.accountId());
    _inactive->setChecked(!_curr.isActive());

    // Load the tenders
    _tenders->clear();
    for (unsigned int i = 0; i < _curr.tenders().size(); ++i) {
	const TenderLine& line = _curr.tenders()[i];
	if (line.voided) continue;

	VectorRow* row = new VectorRow(_tenders->columns());
	row->setValue(0, line.tender_id);
	row->setValue(1, line.conv_amt);
	row->setValue(2, line.conv_rate);
	row->setValue(3, line.amount);
	_tenders->appendRow(row);
    }
    _tenders->appendRow(new VectorRow(_tenders->columns()));

    recalculate();
}

// Set the data object from the widgets.
void
TenderAdjustment::widgetToData()
{
    _gltxFrame->getData(_curr);
    _curr.setAccountId(_account->getId());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    Id store_id = _curr.storeId();
    vector<AccountLine>& accounts = _curr.accounts();
    accounts.clear();

    Company company;
    _quasar->db()->lookup(company);

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
	line.amount = amount;
	line.conv_rate = conv_rate;
	line.conv_amt = conv_amt;
	_curr.tenders().push_back(line);

	Id account_id = tender.accountId();
	if (_curr.stationId() == company.safeStation())
	    if (tender.safeId() != INVALID_ID)
		account_id = tender.safeId();
	if (_curr.employeeId() == company.safeEmployee())
	    if (tender.safeId() != INVALID_ID)
		account_id = tender.safeId();

	accounts.push_back(AccountLine(account_id, amount));
	total_amt += amount;
    }

    // Add the expense account
    Id expense_id = _account->getId();
    if (expense_id != INVALID_ID)
	accounts.push_back(AccountLine(expense_id, -total_amt));
}

void
TenderAdjustment::cellChanged(int row, int col, Variant)
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
	    fixed conv_amt = _tenders->cellValue(row, 1).toFixed();
	    fixed rate = _tenders->cellValue(row, 2).toFixed();
	    fixed amount = conv_amt * rate;
	    amount.moneyRound();
	    _tenders->setCellValue(row, 3, amount);
	}
	break;
    }

    recalculate();
}

void
TenderAdjustment::focusNext(bool& leave, int& newRow, int& newCol, int type)
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
TenderAdjustment::recalculate()
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
