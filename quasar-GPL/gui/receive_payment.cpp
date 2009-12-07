// $Id: receive_payment.cpp,v 1.19 2004/08/04 09:14:23 bpepers Exp $
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

#include "receive_payment.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "cheque_vendor.h"
#include "receive.h"
#include "vendor.h"
#include "receive_select.h"
#include "table.h"
#include "date_popup.h"
#include "id_edit.h"
#include "money_edit.h"
#include "check_box.h"
#include "lookup_edit.h"
#include "vendor_lookup.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <algorithm>

ReceivePayment::ReceivePayment(QWidget* parent, MainWindow* main, Id id)
    : QDialog(parent, "ReceivePayment", true)
{
    _main = main;
    _quasar = main->quasar();

    QLabel* numberLabel = new QLabel(tr("Invoice No.:"), this);
    _number = new LineEdit(14, this);
    _number->setFocusPolicy(NoFocus);
    numberLabel->setBuddy(_number);

    QLabel* dateLabel = new QLabel(tr("Date:"), this);
    _date = new DatePopup(this);
    _date->setFocusPolicy(NoFocus);
    dateLabel->setBuddy(_date);

    QLabel* totalLabel = new QLabel(tr("Total:"), this);
    _total = new MoneyEdit(this);
    _total->setFocusPolicy(NoFocus);
    totalLabel->setBuddy(_total);

    QLabel* paidLabel = new QLabel(tr("Paid:"), this);
    _paid = new MoneyEdit(this);
    _paid->setFocusPolicy(NoFocus);
    paidLabel->setBuddy(_paid);

    QLabel* vendorLabel = new QLabel(tr("Vendor:"), this);
    _vendor = new LookupEdit(new VendorLookup(main, this), this);
    _vendor->setLength(30);
    _vendor->setFocusPolicy(NoFocus);
    vendorLabel->setBuddy(_vendor);

    QLabel* balanceLabel = new QLabel(tr("Balance:"), this);
    _balance = new MoneyEdit(this);
    _balance->setFocusPolicy(NoFocus);
    balanceLabel->setBuddy(_balance);

    QLabel* memoLabel = new QLabel(tr("Memo:"), this);
    _memo = new LineEdit(this);
    _memo->setLength(40);
    _memo->setFocusPolicy(NoFocus);
    memoLabel->setBuddy(_memo);

    QPushButton* autoAlloc = new QPushButton(tr("Auto Allocate"), this);
    connect(autoAlloc, SIGNAL(clicked()), SLOT(slotAutoAlloc()));

    _table = new Table(this);
    _table->setVScrollBarMode(QScrollView::AlwaysOn);
    _table->setDataColumns(8);
    connect(_table, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(cellChanged(int,int,Variant)));
    connect(_table, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(focusNext(bool&,int&,int&,int)));

    // Edit widgets
    CheckBox* check = new CheckBox(_table);
    connect(check, SIGNAL(toggled(bool)), SLOT(recalculate()));

    // Add columns
    new TextColumn(_table, tr("Type"), 18);
    new TextColumn(_table, tr("Number"), 10);
    new DateColumn(_table, tr("Date"));
    new MoneyColumn(_table, tr("Total"));
    new MoneyColumn(_table, tr("Remaining"));
    new MoneyColumn(_table, tr("Amount"));
    new CheckColumn(_table, tr("Pay?"));

    // Add editors
    new NumberEditor(_table, 5, new MoneyEdit(_table));
    new CheckEditor(_table, 6, check);

    QFrame* buttons = new QFrame(this);
    QPushButton* quickCheque = new QPushButton(tr("Quick Cheque"), buttons);
    QPushButton* ok = new QPushButton(tr("&OK"), buttons);
    QPushButton* cancel = new QPushButton(tr("&Cancel"), buttons);

    quickCheque->setMinimumSize(quickCheque->sizeHint());
    ok->setMinimumSize(cancel->sizeHint());
    cancel->setMinimumSize(cancel->sizeHint());
    connect(quickCheque, SIGNAL(clicked()), SLOT(slotQuickCheque()));
    connect(ok, SIGNAL(clicked()), SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), SLOT(reject()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(quickCheque, 0, 0, AlignLeft | AlignVCenter);
    buttonGrid->addWidget(ok, 0, 1, AlignRight | AlignVCenter);
    buttonGrid->addWidget(cancel, 0, 2, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(this);
    grid->setSpacing(4);
    grid->setMargin(4);
    grid->setColStretch(2, 1);
    grid->addWidget(numberLabel, 0, 0);
    grid->addWidget(_number, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(vendorLabel, 0, 3);
    grid->addWidget(_vendor, 0, 4, AlignLeft | AlignVCenter);
    grid->addWidget(dateLabel, 1, 0);
    grid->addWidget(_date, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(balanceLabel, 1, 3);
    grid->addWidget(_balance, 1, 4, AlignLeft | AlignVCenter);
    grid->addWidget(totalLabel, 2, 0);
    grid->addWidget(_total, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(paidLabel, 3, 0);
    grid->addWidget(_paid, 3, 1, AlignLeft | AlignVCenter);
    grid->addWidget(memoLabel, 4, 0);
    grid->addMultiCellWidget(_memo, 4, 4, 1, 3, AlignLeft | AlignVCenter);
    grid->addWidget(autoAlloc, 4, 4, AlignRight | AlignVCenter);
    grid->addMultiCellWidget(_table, 5, 5, 0, 4);
    grid->addMultiCellWidget(buttons, 6, 6, 0, 4);

    // Lookup receiving and set sign
    _quasar->db()->lookup(id, _receive);
    _sign = (_receive.cardTotal() < 0.0) ? -1.0 : 1.0;

    // Load receive info
    _number->setText(_receive.number());
    _vendor->setId(_receive.cardId());
    _date->setDate(_receive.postDate());
    _total->setFixed(_receive.total() * _sign);
    _memo->setText(_receive.memo());

    // Load vendor info
    fixed balance = _quasar->db()->cardBalance(_receive.cardId());
    _balance->setFixed(balance);

    // Load transactions
    GltxSelect conditions;
    conditions.card_id = _receive.cardId();
    conditions.unpaid = true;
    conditions.activeOnly = true;
    vector<Gltx> gltxs;
    _quasar->db()->select(gltxs, conditions);

    // Load ones that have been paid
    for (unsigned int i = 0; i < _receive.payments().size(); ++i) {
	const PaymentLine& line = _receive.payments()[i];

	bool found = false;
	for (unsigned int j = 0; j < gltxs.size(); ++j)
	    if (gltxs[j].id() == line.gltx_id)
		found = true;

	if (!found) {
	    Gltx gltx;
	    _quasar->db()->lookup(line.gltx_id, gltx);
	    gltxs.push_back(gltx);
	}
    }

    // Sort by date
    std::sort(gltxs.begin(), gltxs.end());

    for (unsigned int i = 0; i < gltxs.size(); ++i) {
	const Gltx& gltx = gltxs[i];

	// Skip same sign or zero transactions
	if (_receive.cardTotal() > 0.0 && gltx.cardTotal() > 0.0) continue;
	if (_receive.cardTotal() < 0.0 && gltx.cardTotal() < 0.0) continue;
	if (gltx.cardTotal() == 0.0) continue;

	// Add row
	VectorRow* row = new VectorRow(_table->columns() + 1);
	row->setValue(0, gltx.dataTypeName());
	row->setValue(1, gltx.number());
	row->setValue(2, DateValcon().format(gltx.postDate()));
	row->setValue(3, gltx.cardTotal().abs());
	row->setValue(4, (gltx.cardTotal() - gltx.paymentTotal()).abs());
	row->setValue(5, 0.0);
	row->setValue(6, false);
	row->setValue(7, gltx.id());
	_table->appendRow(row);
    }

    // Set current allocations
    const vector<PaymentLine>& lines = _receive.payments();
    for (unsigned int line = 0; line < lines.size(); ++line) {
	Id gltx_id = lines[line].gltx_id;
	fixed amount = fabs(lines[line].amount);

	Gltx gltx;
	_quasar->db()->lookup(gltx_id, gltx);

	for (int row = 0; row < _table->rows(); ++row) {
	    Id row_id = _table->row(row)->value(7).toId();
	    fixed row_amt = _table->cellValue(row, 5).toDouble();
	    fixed row_remain = _table->cellValue(row, 4).toDouble();

	    if (row_id == gltx_id) {
		_table->setCellValue(row, 4, row_remain + amount);
		_table->setCellValue(row, 5, row_amt + amount);
		_table->setCellValue(row, 6, true);
	    }
	}
    }
    recalculate();

    if (_receive.isClaim()) {
	setCaption(tr("Vendor Claim Allocation"));
	numberLabel->setText(tr("Claim No.:"));
	quickCheque->setEnabled(false);
    } else {
	setCaption(tr("Vendor Invoice Payments"));
	numberLabel->setText(tr("Invoice No.:"));
	quickCheque->setText(tr("Quick Cheque"));
    }
}

ReceivePayment::~ReceivePayment()
{
}

void
ReceivePayment::slotAutoAlloc()
{
    fixed alloc = fabs(_total->getFixed() - _paid->getFixed());

    // First try to find exact match
    int row;
    for (row = 0; row < _table->rows(); ++row) {
	fixed remain = _table->cellValue(row, 4).toFixed();
	fixed amount = _table->cellValue(row, 5).toFixed();
	if (remain - amount == alloc) {
	    _table->setCellValue(row, 5, amount + alloc);
	    _table->setCellValue(row, 6, true);
	    recalculate();
	    return;
	}
    }

    // Try to allocate against oldest
    for (row = 0; row < _table->rows(); ++row) {
	fixed remain = _table->cellValue(row, 4).toFixed();
	fixed amount = _table->cellValue(row, 5).toFixed();
	if (amount == remain) continue;

	fixed use = remain - amount;
	if (use > alloc) use = alloc;

	_table->setCellValue(row, 5, amount + use);
	_table->setCellValue(row, 6, true);
	alloc -= use;
	if (alloc == 0.0) break;
    }
    recalculate();
}

void
ReceivePayment::slotQuickCheque()
{
    accept();
    ChequeVendor* window = new ChequeVendor(_main);
    window->payReceiving(_receive.id());
    window->show();
}

void
ReceivePayment::recalculate()
{
    fixed total = _total->getFixed();
    fixed amount = 0.0;

    for (int row = 0; row < _table->rows(); ++row) {
	bool pay = _table->cellValue(row, 6).toBool();
	if (pay) {
	    fixed payAmt = _table->cellValue(row, 5).toDouble();
	    if (payAmt == 0.0) {
		payAmt = _table->cellValue(row, 4).toDouble();
		if (amount + payAmt > total)
		    payAmt = total - amount;
		_table->setCellValue(row, 5, payAmt);
	    }
	    amount += payAmt;
	} else {
	    _table->setCellValue(row, 5, 0.0);
	}
    }

    _paid->setFixed(amount);
}

void
ReceivePayment::accept()
{
    fixed total = _total->getFixed();
    fixed paid = _paid->getFixed();
    if (paid > total) {
	qApp->beep();
	return;
    }

    // Load the lines from the table
    Receive orig = _receive;
    vector<PaymentLine>& payments = _receive.payments();
    payments.clear();
    for (int row = 0; row < _table->rows(); ++row) {
	Id gltx_id = _table->row(row)->value(7).toId();
	fixed amount = _table->cellValue(row, 5).toDouble();

	if (amount != 0.0)
	    payments.push_back(PaymentLine(gltx_id, amount * _sign, 0.0));
    }

    // Set closed if all paid
    _receive.setClosed(_receive.cardTotal() == _receive.paymentTotal());

    if (!_quasar->db()->update(orig, _receive)) {
	qApp->beep();
	return;
    }

    QDialog::accept();
}

void
ReceivePayment::cellChanged(int row, int col, Variant)
{
    fixed amount;
    bool pay;

    switch (col) {
    case 5:
	amount = _table->cellValue(row, 5).toDouble();
	_table->setCellValue(row, 6, amount != 0.0);
	recalculate();
	break;
    case 6:
	pay = _table->cellValue(row, 6).toBool();
	if (!pay) {
	    _table->setCellValue(row, 5, 0.0);
	    recalculate();
	}
	break;
    }
}

void
ReceivePayment::focusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _table->currentRow();
    int col = _table->currentColumn();

    if (type == Table::MovePrev && row == 0)
	leave = true;
    if (type == Table::MoveNext && col == 5) {
	if (row == _table->rows() - 1) {
	    leave = true;
	} else {
	    newRow = row + 1;
	    newCol = 5;
	}
    }
    if (type == Table::MoveNext && col == 6) {
	newRow = row + 1;
	newCol = 6;
    }
    if (type == Table::MovePrev && col == 5) {
	if (row > 0) {
	    newRow = row - 1;
	    newCol = 5;
	}
    }
    if (type == Table::MovePrev && col == 6) {
	if (row > 0) {
	    newRow = row - 1;
	    newCol = 6;
	}
    }
}
