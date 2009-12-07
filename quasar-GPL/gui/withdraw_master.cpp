// $Id: withdraw_master.cpp,v 1.17 2005/03/13 23:13:45 bpepers Exp $
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

#include "withdraw_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "invoice_info.h"
#include "invoice.h"
#include "account.h"
#include "tender.h"
#include "customer.h"
#include "company.h"
#include "withdraw_select.h"
#include "invoice_select.h"
#include "gltx_frame.h"
#include "table.h"
#include "date_popup.h"
#include "id_edit.h"
#include "money_edit.h"
#include "check_box.h"
#include "lookup_edit.h"
#include "account_lookup.h"
#include "tender_lookup.h"
#include "customer_lookup.h"
#include "store_lookup.h"
#include "term_lookup.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qtabwidget.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qinputdialog.h>

WithdrawMaster::WithdrawMaster(MainWindow* main, Id withdraw_id)
    : DataWindow(main, "CustomerWithdraw", withdraw_id)
{
    _helpSource = "customer_withdraw.html";

    // Layouts
    QVBoxLayout* vbox1 = new QVBoxLayout(_frame);
    vbox1->setSpacing(6);
    vbox1->setMargin(6);

    _gltxFrame = new GltxFrame(main, tr("Withdraw No."), _frame);
    _gltxFrame->hideMemo();
    vbox1->addWidget(_gltxFrame);

    QFrame* top = new QFrame(_frame);
    top->setFrameStyle(QFrame::Raised | QFrame::Panel);

    QLabel* custLabel = new QLabel(tr("Customer:"), top);
    _customer = new LookupEdit(new CustomerLookup(main, this), top);
    _customer->setLength(30);
    custLabel->setBuddy(_customer);
    connect(_customer, SIGNAL(validData()), SLOT(slotCustomerChanged()));

    QLabel* balanceLabel = new QLabel(tr("Balance:"), top);
    _balance = new MoneyEdit(top);
    _balance->setFocusPolicy(NoFocus);

    QLabel* amountLabel = new QLabel(tr("Amount:"), top);
    _amount = new MoneyEdit(top);
    amountLabel->setBuddy(_amount);

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(3);
    topGrid->setMargin(3);
    topGrid->setColStretch(2, 1);
    topGrid->addWidget(custLabel, 0, 0);
    topGrid->addWidget(_customer, 0, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(balanceLabel, 0, 3);
    topGrid->addWidget(_balance, 0, 4, AlignLeft | AlignVCenter);
    topGrid->addWidget(amountLabel, 1, 0);
    topGrid->addWidget(_amount, 1, 1, AlignLeft | AlignVCenter);
    vbox1->addWidget(top);

    QTabWidget* tabs = new QTabWidget(_frame);
    tabs->setFocusPolicy(ClickFocus);
    QFrame* tenders = new QFrame(tabs);
    QFrame* refs = new QFrame(tabs);
    tabs->addTab(tenders, tr("Tenders"));
    tabs->addTab(refs, tr("References"));
    vbox1->addWidget(tabs);

    _tenders = new Table(tenders);
    _tenders->setVScrollBarMode(QScrollView::AlwaysOn);
    _tenders->setDisplayRows(3);
    connect(_tenders, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(tenderCellChanged(int,int,Variant)));
    connect(_tenders, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(tenderFocusNext(bool&,int&,int&,int)));

    TenderLookup* tendLookup = new TenderLookup(_main, this);

    new LookupColumn(_tenders, tr("Tender"), 20, tendLookup);
    new MoneyColumn(_tenders, tr("Amount"));
    new NumberColumn(_tenders, tr("Rate"), 3);
    new MoneyColumn(_tenders, tr("Total"));

    new LookupEditor(_tenders, 0, new LookupEdit(tendLookup, _tenders));
    new NumberEditor(_tenders, 1, new MoneyEdit(_tenders));

    QGridLayout* tendGrid = new QGridLayout(tenders);
    tendGrid->setSpacing(3);
    tendGrid->setMargin(3);
    tendGrid->setRowStretch(0, 1);
    tendGrid->setColStretch(0, 1);
    tendGrid->addWidget(_tenders, 0, 0);

    _refs = new Table(refs);
    _refs->setVScrollBarMode(QScrollView::AlwaysOn);
    _refs->setDisplayRows(3);
    connect(_refs, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(referenceFocusNext(bool&,int&,int&,int)));

    new TextColumn(_refs, tr("Name"), 20);
    new TextColumn(_refs, tr("Data"), 20);
    new LineEditor(_refs, 1, new LineEdit(20, _refs));

    QGridLayout* refGrid = new QGridLayout(refs);
    refGrid->setSpacing(3);
    refGrid->setMargin(3);
    refGrid->setRowStretch(0, 1);
    refGrid->setColStretch(0, 1);
    refGrid->addWidget(_refs, 0, 0);

    _inactive->setText(tr("Voided?"));

    setCaption(tr("Customer Withdraw"));
    finalize();
}

WithdrawMaster::~WithdrawMaster()
{
}

void
WithdrawMaster::setStoreId(Id store_id)
{
    if (store_id != INVALID_ID)
	_gltxFrame->store->setId(store_id);
}

void
WithdrawMaster::setCardId(Id card_id)
{
    if (card_id != INVALID_ID) {
	_customer->setId(card_id);
	slotCustomerChanged();
    }
}

void
WithdrawMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _customer;
}

void
WithdrawMaster::newItem()
{
    Company company;
    _quasar->db()->lookup(company);

    Withdraw blank;
    _orig = blank;
    _gltxFrame->defaultData(_orig);

    _curr = _orig;
    _firstField = _customer;
}

void
WithdrawMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _gltxFrame->cloneData(_curr);
    dataToWidget();
}

bool
WithdrawMaster::fileItem()
{
    if (_curr.number() != "#")
	if (checkGltxUsed(_curr.number(), DataObject::WITHDRAW, _curr.id()))
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
WithdrawMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
WithdrawMaster::restoreItem()
{
    _curr = _orig;
}

void
WithdrawMaster::cloneItem()
{
    WithdrawMaster* clone = new WithdrawMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
WithdrawMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
WithdrawMaster::dataToWidget()
{
    _gltxFrame->setData(_curr);
    _customer->setId(_curr.cardId());
    _amount->setFixed(-_curr.total());
    _inactive->setChecked(!_curr.isActive());

    // Load the tenders
    _tenders->clear();
    const vector<TenderLine>& tenders = _curr.tenders();
    unsigned int i;
    for (i = 0; i < tenders.size(); ++i) {
	Id tender_id = tenders[i].tender_id;
	fixed conv_amt = tenders[i].conv_amt;
	fixed conv_rate = tenders[i].conv_rate;
	fixed amount = tenders[i].amount;
	bool voided = tenders[i].voided;
	if (voided) continue;

	VectorRow* row = new VectorRow(_tenders->columns());
	row->setValue(0, tender_id);
	row->setValue(1, -conv_amt);
	row->setValue(2, conv_rate);
	row->setValue(3, -amount);
	_tenders->appendRow(row);
    }
    _tenders->appendRow(new VectorRow(_tenders->columns()));

    // Load the references
    _refs->clear();
    for (i = 0; i < _curr.referenceName().size(); ++i) {
	VectorRow* row = new VectorRow(_refs->columns());
	row->setValue(0, _curr.referenceName()[i]);
	row->setValue(1, _curr.referenceData()[i]);
	_refs->appendRow(row);
    }

    Customer customer;
    _quasar->db()->lookup(_customer->getId(), customer);
    _balance->setFixed(_quasar->db()->cardBalance(customer.id()));
}

// Set the data object from the widgets.
void
WithdrawMaster::widgetToData()
{
    _gltxFrame->getData(_curr);
    _curr.setCardId(_customer->getId());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    // Setup account lines
    vector<AccountLine>& lines = _curr.accounts();
    lines.clear();
    Id store_id = _curr.storeId();

    // Add the tenders
    _curr.tenders().clear();
    int row;
    for (row = 0; row < _tenders->rows(); ++row) {
	Id tender_id = _tenders->cellValue(row, 0).toId();
	if (tender_id == INVALID_ID) continue;
	fixed conv_amt = _tenders->cellValue(row, 1).toFixed();
	fixed conv_rate = _tenders->cellValue(row, 2).toFixed();
	fixed amount = _tenders->cellValue(row, 3).toFixed();

	Tender tender;
	_quasar->db()->lookup(tender_id, tender);

	TenderLine line;
	line.tender_id = tender_id;
	line.conv_amt = -conv_amt;
	line.conv_rate = conv_rate;
	line.amount = -amount;
	_curr.tenders().push_back(line);

	lines.push_back(AccountLine(tender.accountId(), -amount));
    }

    // Add all the references
    _curr.referenceName().clear();
    _curr.referenceData().clear();
    for (row = 0; row < _refs->rows(); ++row) {
	QString name = _refs->cellValue(row, 0).toString();
	QString data = _refs->cellValue(row, 1).toString();
	if (name.isEmpty()) continue;

	_curr.referenceName().push_back(name);
	_curr.referenceData().push_back(data);
    }

    // Total up discounts and amounts
    fixed amount = _amount->getFixed();

    // Add AR line
    if (_curr.cardId() != INVALID_ID) {
	Customer customer;
	_quasar->db()->lookup(_curr.cardId(), customer);
	lines.push_back(AccountLine(customer.accountId(), amount));

	// Add card line
	_curr.cards().clear();
	_curr.cards().push_back(CardLine(_curr.cardId(), amount));
    }
}

void
WithdrawMaster::slotCustomerChanged()
{
    _balance->setFixed(0.0);
    _refs->clear();

    if (_customer->getId() != INVALID_ID) {
	Customer customer;
	_quasar->db()->lookup(_customer->getId(), customer);

	_balance->setFixed(_quasar->db()->cardBalance(customer.id()));
	for (unsigned int i = 0; i < customer.references().size(); ++i) {
	    QString name = customer.references()[i];

	    // Ask for reference
	    QString caption = tr("Reference Information");
	    QString label = tr("%1:").arg(name);
	    bool ok = false;
	    QString data = QInputDialog::getText(caption, label,
						 QLineEdit::Normal,
						 "", &ok, this);
	    if (!ok) {
		_customer->setId(INVALID_ID);
		slotCustomerChanged();
		return;
	    }

	    VectorRow* row = new VectorRow(_refs->columns());
	    row->setValue(0, name);
	    row->setValue(1, data);
	    _refs->appendRow(row);
	}
    }
}

void
WithdrawMaster::tenderCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _tenders->rows() - 1 && col == 0) {
	Variant id = _tenders->cellValue(row, col);
	if (!id.isNull() && id.toId() != INVALID_ID)
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

	    fixed remain = _amount->getFixed();
	    for (int i = 0; i < _tenders->rows(); ++i) {
		if (i == row) continue;
		if (_tenders->cellValue(i, 0).toId() == INVALID_ID) continue;
		remain -= _tenders->cellValue(i, 3).toFixed();
	    }

	    fixed convertRate = tender.convertRate();
	    fixed convertAmt = remain / convertRate;
	    convertAmt.moneyRound();

	    _tenders->setCellValue(row, 1, convertAmt);
	    _tenders->setCellValue(row, 2, convertRate);
	    _tenders->setCellValue(row, 3, remain);
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
}

void
WithdrawMaster::tenderFocusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _tenders->currentRow();
    int col = _tenders->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Variant val = _tenders->cellValue(row, col);
	Id id = val.toId();
	if ((val.isNull() || id == INVALID_ID) && row == _tenders->rows()-1) {
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
WithdrawMaster::referenceFocusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _refs->currentRow();
    int col = _refs->currentColumn();

    if (type == Table::MoveNext && col == 1) {
	if (row == _tenders->rows()-1) {
	    leave = true;
	} else {
	    newRow = row + 1;
	    newCol = 1;
	}
    } else if (type == Table::MovePrev && col == 1) {
	if (row > 0) {
	    newCol = 1;
	    newRow = row - 1;
	}
    }
}
