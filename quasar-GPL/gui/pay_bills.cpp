// $Id: pay_bills.cpp,v 1.30 2005/03/13 23:13:45 bpepers Exp $
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

#include "pay_bills.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "invoice_info.h"
#include "date_popup.h"
#include "time_edit.h"
#include "lookup_edit.h"
#include "account_lookup.h"
#include "store_lookup.h"
#include "station_lookup.h"
#include "employee_lookup.h"
#include "vendor.h"
#include "account.h"
#include "company.h"
#include "receive_select.h"
#include "cheque.h"
#include "term.h"
#include "id_edit.h"
#include "money_edit.h"
#include "percent_edit.h"
#include "check_box.h"
#include "table.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <algorithm>

PayBills::PayBills(MainWindow* main)
    : QuasarWindow(main, "PayBills")
{
    _helpSource = "pay_bills.html";

    QFrame* frame = new QFrame(this);

    QVBox* buttons = new QVBox(frame);
    buttons->setMargin(4);
    buttons->setSpacing(4);

    QPushButton* ok = new QPushButton(tr("&OK"), buttons);
    QPushButton* next = new QPushButton(tr("&Next"), buttons);
    QPushButton* cancel = new QPushButton(tr("&Cancel"), buttons);
    connect(ok, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(next, SIGNAL(clicked()), this, SLOT(slotNext()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(slotCancel()));

    QWidget* top = new QWidget(frame);

    QLabel* dateLabel = new QLabel(tr("Payment Date:"), top);
    _date = new DatePopup(top);
    dateLabel->setBuddy(_date);

    QLabel* timeLabel = new QLabel(tr("Payment Time:"), top);
    _time = new TimeEdit(top);
    timeLabel->setBuddy(_time);

    QLabel* storeLabel = new QLabel(tr("Store:"), top);
    _store = new LookupEdit(new StoreLookup(_main, this), top);
    _store->setFocusPolicy(ClickFocus);
    _store->setLength(20);
    storeLabel->setBuddy(_store);

    QLabel* stationLabel = new QLabel(tr("Station:"), top);
    _station = new LookupEdit(new StationLookup(_main, this), top);
    _station->setFocusPolicy(ClickFocus);
    _station->setLength(20);
    stationLabel->setBuddy(_station);

    QLabel* employeeLabel = new QLabel(tr("Employee:"), top);
    _employee = new LookupEdit(new EmployeeLookup(_main, this), top);
    _employee->setFocusPolicy(ClickFocus);
    _employee->setLength(20);
    employeeLabel->setBuddy(_employee);

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(6);
    topGrid->setMargin(6);
    topGrid->setColStretch(2, 1);
    topGrid->setColStretch(5, 1);
    topGrid->addWidget(dateLabel, 0, 0);
    topGrid->addWidget(_date, 0, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(storeLabel, 0, 3, AlignLeft | AlignVCenter);
    topGrid->addWidget(_store, 0, 4, AlignLeft | AlignVCenter);
    topGrid->addWidget(employeeLabel, 0, 6, AlignLeft | AlignVCenter);
    topGrid->addWidget(_employee, 0, 7, AlignLeft | AlignVCenter);
    topGrid->addWidget(timeLabel, 1, 0);
    topGrid->addWidget(_time, 1, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(stationLabel, 1, 3, AlignLeft | AlignVCenter);
    topGrid->addWidget(_station, 1, 4, AlignLeft | AlignVCenter);

    QWidget* mid = new QWidget(frame);

    QGroupBox* payBy = new QGroupBox(tr("Pay By"), mid);

    QRadioButton* payCheque = new QRadioButton(tr("Cheque"), payBy);
    QRadioButton* payCard = new QRadioButton(tr("Credit Card"), payBy);

    QLabel* nextChequeLabel = new QLabel(tr("Next Cheque:"), payBy);
    _nextCheque = new IdEdit(payBy);
    nextChequeLabel->setBuddy(_nextCheque);

    QLabel* accountLabel = new QLabel(tr("Account:"), payBy);
    _lookup = new AccountLookup(main, this, Account::Bank);
    _account = new LookupEdit(_lookup, payBy);
    _account->setLength(30);
    accountLabel->setBuddy(_account);
    connect(_account, SIGNAL(validData()), SLOT(slotAccountChanged()));

    QLabel* currentLabel = new QLabel(tr("Current Balance:"), payBy);
    _current = new MoneyEdit(payBy);
    _current->setFocusPolicy(NoFocus);

    QButtonGroup* payButtons = new QButtonGroup(this);
    payButtons->hide();
    payButtons->insert(payCheque);
    payButtons->insert(payCard);
    connect(payButtons, SIGNAL(clicked(int)), SLOT(slotPayChanged(int)));

    QGridLayout* payGrid = new QGridLayout(payBy, 4, 2, payBy->frameWidth()*2);
    payGrid->addRowSpacing(0, payBy->fontMetrics().height());
    payGrid->setColStretch(1, 1);
    payGrid->setColStretch(1, 1);
    payGrid->addWidget(payCheque, 1, 0, AlignLeft | AlignVCenter);
    payGrid->addWidget(nextChequeLabel, 1, 1, AlignRight | AlignVCenter);
    payGrid->addWidget(_nextCheque, 1, 2, AlignLeft | AlignVCenter);
    payGrid->addWidget(payCard, 2, 0, AlignLeft | AlignVCenter);
    payGrid->addWidget(accountLabel, 3, 0, AlignLeft | AlignVCenter);
    payGrid->addMultiCellWidget(_account, 3, 3, 1, 2, AlignLeft|AlignVCenter);
    payGrid->addWidget(currentLabel, 4, 0, AlignLeft | AlignVCenter);
    payGrid->addMultiCellWidget(_current, 4, 4, 1, 2, AlignLeft|AlignVCenter);

    QGroupBox* show = new QGroupBox(tr("Show"), mid);

    _showAll = new QRadioButton(tr("Show all bills"), show);
    _showDue = new QRadioButton(tr("Show due on/before:"), show);
    _dueDate = new DatePopup(show);
    connect(_dueDate, SIGNAL(validData()), SLOT(slotRefresh()));

    QButtonGroup* showButtons = new QButtonGroup(this);
    showButtons->hide();
    showButtons->insert(_showAll);
    showButtons->insert(_showDue);
    connect(showButtons, SIGNAL(clicked(int)), SLOT(slotShowChanged(int)));

    QGridLayout* showGrid = new QGridLayout(show, 3, 2, show->frameWidth()*2);
    showGrid->addRowSpacing(0, show->fontMetrics().height());
    showGrid->setRowStretch(3, 1);
    showGrid->setColStretch(1, 1);
    showGrid->addWidget(_showAll, 1, 0, AlignLeft | AlignVCenter);
    showGrid->addWidget(_showDue, 2, 0, AlignLeft | AlignVCenter);
    showGrid->addWidget(_dueDate, 2, 1, AlignLeft | AlignVCenter);

    QLabel* discLabel = new QLabel(tr("Discount Account:"), mid);
    _disc = new LookupEdit(new AccountLookup(main, this, Account::OtherIncome),mid);
    _disc->setLength(30);
    _disc->setFocusPolicy(ClickFocus);
    discLabel->setBuddy(_disc);

    QGridLayout* midGrid = new QGridLayout(mid);
    midGrid->setSpacing(6);
    midGrid->setMargin(6);
    midGrid->setRowStretch(1, 1);
    midGrid->setColStretch(2, 1);
    midGrid->addMultiCellWidget(payBy, 0, 2, 0, 0);
    midGrid->addMultiCellWidget(show, 0, 1, 1, 2);
    midGrid->addWidget(discLabel, 2, 1);
    midGrid->addWidget(_disc, 2, 2, AlignLeft | AlignVCenter);

    _table = new Table(frame);
    _table->setVScrollBarMode(QScrollView::AlwaysOn);
    _table->setDisplayRows(5);
    _table->setDataColumns(10);
    _table->setLeftMargin(0);
    connect(_table, SIGNAL(cellMoved(int,int)), SLOT(cellMoved(int,int)));
    connect(_table, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(cellChanged(int,int,Variant)));
    connect(_table, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(focusNext(bool&,int&,int&,int)));

    new DateColumn(_table, tr("Due Date"));
    new TextColumn(_table, tr("Vendor"), 20);
    new TextColumn(_table, tr("Invoice"), 10);
    new MoneyColumn(_table, tr("Total"));
    new DateColumn(_table, tr("Disc Date"));
    new MoneyColumn(_table, tr("Due"));
    new MoneyColumn(_table, tr("Payment"));
    new MoneyColumn(_table, tr("Discount"));
    new CheckColumn(_table, tr("Pay?"));

    NumberEdit* paidEdit = new MoneyEdit(_table);
    paidEdit->setMaxLength(14);
    new NumberEditor(_table, 6, paidEdit);

    NumberEdit* discEdit = new MoneyEdit(_table);
    discEdit->setMaxLength(14);
    new NumberEditor(_table, 7, discEdit);

    CheckBox* check = new CheckBox(_table);
    new CheckEditor(_table, 8, check);
    connect(check, SIGNAL(toggled(bool)), SLOT(slotPayClicked()));

    QFrame* totals = new QFrame(frame);

    QLabel* totalLabel = new QLabel(tr("Total Paid:"), totals);
    _total = new MoneyEdit(totals);
    _total->setFocusPolicy(NoFocus);

    QLabel* endingLabel = new QLabel(tr("Ending Balance:"), totals);
    _ending = new MoneyEdit(totals);
    _ending->setFocusPolicy(NoFocus);

    QPushButton* payAll = new QPushButton(tr("Pay All"), totals);
    payAll->setFocusPolicy(NoFocus);
    connect(payAll, SIGNAL(clicked()), SLOT(slotPayAll()));

    QPushButton* clearAll = new QPushButton(tr("Clear Payments"), totals);
    clearAll->setFocusPolicy(NoFocus);
    connect(clearAll, SIGNAL(clicked()), SLOT(slotClearAll()));

    QPushButton* invoiceInfo = new QPushButton(tr("Invoice Info"), totals);
    invoiceInfo->setFocusPolicy(NoFocus);
    connect(invoiceInfo, SIGNAL(clicked()), SLOT(slotInvoiceInfo()));

    QGridLayout* totalGrid = new QGridLayout(totals);
    totalGrid->setSpacing(3);
    totalGrid->setColStretch(2, 1);
    totalGrid->setColStretch(4, 1);
    totalGrid->addWidget(totalLabel, 0, 0, AlignLeft | AlignVCenter);
    totalGrid->addWidget(_total, 0, 1, AlignLeft | AlignVCenter);
    totalGrid->addWidget(endingLabel, 1, 0, AlignLeft | AlignVCenter);
    totalGrid->addWidget(_ending, 1, 1, AlignLeft | AlignVCenter);
    totalGrid->addWidget(payAll, 0, 3);
    totalGrid->addWidget(clearAll, 1, 3);
    totalGrid->addWidget(invoiceInfo, 0, 5);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(2, 1);
    grid->setColStretch(0, 1);
    grid->addWidget(top, 0, 0);
    grid->addWidget(mid, 1, 0);
    grid->addWidget(_table, 2, 0);
    grid->addWidget(totals, 3, 0);
    grid->addMultiCellWidget(buttons, 0, 3, 1, 1, AlignRight | AlignTop);

    Company company;
    _quasar->db()->lookup(company);
    Id discId = company.vendorTermsAcct();

    _date->setDate(QDate::currentDate());
    _time->setTime(QTime::currentTime());
    _dueDate->setDate(QDate::currentDate());
    _store->setId(_quasar->defaultStore());
    _station->setId(_quasar->defaultStation());
    _employee->setId(_quasar->defaultEmployee());
    payCheque->setChecked(true);
    _disc->setId(discId);
    _showAll->setChecked(true);
    _dueDate->setEnabled(false);

    payCheque->setFocus();
    slotPayChanged(0);

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    setCentralWidget(frame);
    setCaption(tr("Pay Bills"));
    finalize();
}

PayBills::~PayBills()
{
}

void
PayBills::slotOk()
{
    Id bankId = _account->getId();
    Id discId = _disc->getId();
    QDate postDate = _date->getDate();
    QTime postTime = _time->getTime();
    Id store_id = _store->getId();
    Id station_id = _station->getId();
    Id employee_id = _employee->getId();
    fixed number = _nextCheque->getFixed();

    // Check manditory fields
    if (bankId == INVALID_ID) {
	error(tr("Account is manditory"), _account);
	return;
    }
    if (postDate.isNull()) {
	error(tr("Payment date is manditory"), _date);
	return;
    }

    // Test if discount account needed
    bool discNeeded = false;
    int row;
    for (row = 0; row < _table->rows(); ++row) {
	if (!_table->cellValue(row, 8).toBool())
	    continue;
	if (_table->cellValue(row, 7).toFixed() != 0.0) {
	    discNeeded = true;
	    break;
	}
    }
    if (discNeeded && discId == INVALID_ID) {
	error(tr("Discount account is manditory"), _disc);
	return;
    }

    // Post payments
    for (row = 0; row < _table->rows(); ++row) {
	if (!_table->cellValue(row, 8).toBool()) continue;

	Vendor vendor;
	_quasar->db()->lookup(_gltxs[row].cardId(), vendor);

	Cheque cheque;
	cheque.setType(Cheque::VENDOR);
	cheque.setNumber(number.toString());
	cheque.setPostDate(postDate);
	cheque.setPostTime(postTime);
	cheque.setStoreId(store_id);
	cheque.setStationId(station_id);
	cheque.setEmployeeId(employee_id);
	cheque.setMemo(tr("Cheque; ") + vendor.nameFL());
	cheque.setCardId(vendor.id());
	cheque.setAccountId(bankId);
	cheque.setDiscountId(discId);

	// Set payments
	fixed paidTotal = 0.0;
	fixed discTotal = 0.0;
	for (int i = row; i < _table->rows(); ++i) {
	    if (!_table->cellValue(i, 8).toBool()) continue;
	    const Gltx& tx = _gltxs[i];
	    if (tx.cardId() != vendor.id()) continue;

	    fixed paid = _table->cellValue(i, 6).toFixed();
	    fixed disc = _table->cellValue(i, 7).toFixed();
	    cheque.payments().push_back(PaymentLine(tx.id(), -paid, -disc));

	    paidTotal += paid;
	    discTotal += disc;
	    _table->setCellValue(i, 8, false);
	}
	fixed total = paidTotal + discTotal;

	cheque.accounts().push_back(AccountLine(bankId, -paidTotal));
	cheque.accounts().push_back(AccountLine(vendor.accountId(), total));
	if (discTotal != 0.0)
	    cheque.accounts().push_back(AccountLine(discId, -discTotal));
	cheque.cards().push_back(CardLine(vendor.id(), -total));

	_quasar->db()->create(cheque);
	number += 1;
    }

    Company company;
    _quasar->db()->lookup(company);

    if (company.vendorTermsAcct() == INVALID_ID) {
	Company orig = company;
	company.setVendorTermsAcct(discId);
	_quasar->db()->update(orig, company);
    }

    close();
}

void
PayBills::slotNext()
{
}

void
PayBills::slotCancel()
{
    close();
}

void
PayBills::slotAccountChanged()
{
    Id account_id = _account->getId();
    Account account;
    _quasar->db()->lookup(account_id, account);

    _nextCheque->setFixed(account.nextNumber());
    if (_lookup->type->currentText() == Account::typeName(Account::Bank)) {
	_nextCheque->setEnabled(true);
    } else {
	_nextCheque->setEnabled(false);
    }

    _current->setFixed(_quasar->db()->accountBalance(account_id));
    recalculate();
}

void
PayBills::slotPayChanged(int button)
{
    switch (button) {
    case 0:
	_account->setId(_quasar->db()->defaultBankId());
	_lookup->type->setCurrentText(Account::typeName(Account::Bank));
	break;
    case 1:
	_account->setId(INVALID_ID);
	_lookup->type->setCurrentText(Account::typeName(Account::CreditCard));
	break;
    }
    slotAccountChanged();
}

void
PayBills::slotShowChanged(int button)
{
    if (button == 0)
	_dueDate->setEnabled(false);
    if (button == 1)
	_dueDate->setEnabled(true);
    slotRefresh();
}

void
PayBills::slotPayAll()
{
    slotClearAll();
    for (int row = 0; row < _table->rows(); ++row) {
	fixed disc_amt = _suggest[row];
	fixed amt_due = _table->cellValue(row, 5).toFixed();
	fixed pay_amt = amt_due - disc_amt;

	_table->setCellValue(row, 6, pay_amt);
	if (disc_amt == 0.0)
	    _table->setCellValue(row, 7, "");
	else
	    _table->setCellValue(row, 7, disc_amt);
	_table->setCellValue(row, 8, true);
    }
    recalculate();
}

void
PayBills::slotPayDue()
{
}

void
PayBills::slotClearAll()
{
    for (int row = 0; row < _table->rows(); ++row) {
	_table->setCellValue(row, 6, "");
	_table->setCellValue(row, 7, "");
	_table->setCellValue(row, 8, false);
    }
    recalculate();
}

void
PayBills::slotInvoiceInfo()
{
    if (!_info.isNull()) return;

    _info = new InvoiceInfo(_main, this);
    int row = _table->currentRow();
    if (row >= 0 && row < _table->rows()) {
	fixed disc_amt = _table->cellValue(row, 7).toFixed();
	_info->setInvoice(_gltxs[row], _terms[row], disc_amt);
    }
    connect(_info, SIGNAL(newDiscount(fixed)), SLOT(setDiscount(fixed)));
    _info->show();
}

void
PayBills::setDiscount(fixed new_disc)
{
    int row = _table->currentRow();
    if (row < 0 || row >= _table->rows())
	return;

    fixed amt_due = _table->cellValue(row, 5).toFixed();
    fixed old_amt = _table->cellValue(row, 6).toFixed();
    fixed old_disc = _table->cellValue(row, 7).toFixed();

    fixed new_amt = old_amt;
    if (old_disc + old_amt == amt_due) {
	new_amt = amt_due - new_disc;
	if (new_amt + new_disc > amt_due)
	    new_amt = amt_due - new_disc;
    }

    if (new_amt == 0.0)
	_table->setCellValue(row, 6, "");
    else
	_table->setCellValue(row, 6, new_amt);
    if (new_disc == 0.0)
	_table->setCellValue(row, 7, "");
    else
	_table->setCellValue(row, 7, new_disc);
    _table->setCellValue(row, 8, (new_amt != 0.0 || new_disc != 0.0));

    recalculate();
}

void
PayBills::slotRefresh()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Load transactions
    GltxSelect conditions;
    conditions.card_type = DataObject::VENDOR;
    conditions.unpaid = true;
    conditions.activeOnly = true;
    vector<Gltx> gltxs;
    _quasar->db()->select(gltxs, conditions);

    // Sort by date
    std::sort(gltxs.begin(), gltxs.end());

    // Load into table
    _gltxs.clear();
    _terms.clear();
    _suggest.clear();
    _table->clear();
    for (unsigned int i = 0; i < gltxs.size(); ++i) {
	const Gltx& gltx = gltxs[i];

	// Get terms if its a vendor invoice
	Term term;
	if (gltx.dataType() == DataObject::RECEIVE) {
	    Receive receive;
	    _quasar->db()->lookup(gltx.id(), receive);
	    _quasar->db()->lookup(receive.termsId(), term);
	}

	// Skip credits (should they be added up and used for payments?)
	if (gltx.cardTotal() < 0.0) continue;

	// If remain is zero, ignore it
	fixed remain = gltx.cardTotal() - gltx.paymentTotal();
	if (remain == 0.0) continue;

	QDate invoiceDate = gltx.postDate();
	if (gltx.dataType() == DataObject::RECEIVE) {
	    Receive receive;
	    _quasar->db()->lookup(gltx.id(), receive);
	    invoiceDate = receive.invoiceDate();
	}
	QDate dueDate = invoiceDate + term.dueDays();
	QDate discDate = invoiceDate + term.discountDays();

	if (_showDue->isChecked() && dueDate > _dueDate->getDate())
	    continue;

	fixed suggest = 0.0;
	if (term.discount() != 0.0 && QDate::currentDate() <= discDate)
	    suggest = gltx.cardTotal() * term.discount() / 100.0;
	suggest.moneyRound();

	Card card;
	_quasar->db()->lookup(gltx.cardId(), card);
	if (card.dataType() != DataObject::VENDOR)
	    continue;

	VectorRow* row = new VectorRow(_table->columns() + 2);
	row->setValue(0, dueDate);
	row->setValue(1, card.nameFL());
	row->setValue(2, gltx.number());
	row->setValue(3, gltx.cardTotal());
	if (term.discountDays() != 0)
	    row->setValue(4, discDate);
	row->setValue(5, gltx.cardTotal() - gltx.paymentTotal());
	_table->appendRow(row);

	_gltxs.push_back(gltx);
	_terms.push_back(term);
	_suggest.push_back(suggest);
    }

    if (_table->rows() > 0) _table->setCurrentCell(0, 6);
    QApplication::restoreOverrideCursor();
}

void
PayBills::recalculate()
{
    fixed payment = 0.0;
    fixed discount = 0.0;
    for (int row = 0; row < _table->rows(); ++row) {
	payment += _table->cellValue(row, 6).toFixed();
	discount += _table->cellValue(row, 6).toFixed();
    }

    _total->setFixed(payment);
    _ending->setFixed(_current->getFixed() - payment);
    //_discAmt->setFixed(discount);
}

void
PayBills::slotPayClicked()
{
    int row = _table->currentRow();
    if (row < 0 || row >= _table->rows())
	return;

    // Just get value which should trigger cell changed
    _table->cellValue(row, 8).toBool();
}

void
PayBills::cellMoved(int row, int)
{
    if (row == -1 || _info.isNull()) return;
    fixed disc_amt = _table->cellValue(row, 7).toFixed();
    _info->setInvoice(_gltxs[row], _terms[row], disc_amt);
}

void
PayBills::cellChanged(int row, int col, Variant)
{
    fixed amt_due = _table->cellValue(row, 5).toFixed();
    fixed amt_paid = _table->cellValue(row, 6).toFixed();
    fixed disc_amt = _table->cellValue(row, 7).toFixed();
    bool pay = _table->cellValue(row, 8).toBool();

    switch (col) {
    case 6:
	if (amt_paid + disc_amt > amt_due)
	    disc_amt = amt_due - amt_paid;
	if (amt_paid + disc_amt < amt_due)
	    disc_amt = 0.0;
	if (amt_paid + _suggest[row] == amt_due)
	    disc_amt = _suggest[row];

	if (disc_amt == 0.0)
	    _table->setCellValue(row, 7, "");
	else
	    _table->setCellValue(row, 7, disc_amt);
	break;
    case 7:
	if (amt_paid + disc_amt > amt_due)
	    amt_paid = amt_due - disc_amt;

	if (amt_paid == 0.0)
	    _table->setCellValue(row, 6, "");
	else
	    _table->setCellValue(row, 6, amt_paid);
	break;
    case 8:
	if (pay && amt_paid == 0.0 && disc_amt == 0.0) {
	    amt_paid = amt_due - _suggest[row];
	    disc_amt = _suggest[row];
	    _table->setCellValue(row, 6, amt_paid);
	    _table->setCellValue(row, 7, disc_amt);
	} else if (!pay) {
	    amt_paid = 0.0;
	    disc_amt = 0.0;
	    _table->setCellValue(row, 6, "");
	    _table->setCellValue(row, 7, "");
	}
	break;
    }

    if (!_info.isNull())
	_info->setInvoice(_gltxs[row], _terms[row], disc_amt);

    bool new_pay = (amt_paid != 0.0 || disc_amt != 0.0);
    if (pay != new_pay) {
	_table->setCellValue(row, 8, new_pay);
    }

    recalculate();
}

void
PayBills::focusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _table->currentRow();
    int col = _table->currentColumn();

    if (type == Table::MovePrev && row == 0)
	leave = true;
    if (type == Table::MoveNext && col == 7) {
	if (row == _table->rows() - 1) {
	    leave = true;
	} else {
	    newRow = row + 1;
	    newCol = 7;
	}
    }
    if (type == Table::MoveNext && col == 8) {
	newRow = row + 1;
	newCol = 8;
    }
    if (type == Table::MovePrev && col == 7) {
	if (row > 0) {
	    newRow = row - 1;
	    newCol = 7;
	}
    }
    if (type == Table::MovePrev && col == 8) {
	if (row > 0) {
	    newRow = row - 1;
	    newCol = 8;
	}
    }
}
