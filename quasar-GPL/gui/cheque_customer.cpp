// $Id: cheque_customer.cpp,v 1.15 2004/03/05 08:50:58 bpepers Exp $
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

#include "cheque_customer.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "cheque_print.h"
#include "recurring_master.h"
#include "account.h"
#include "customer.h"
#include "term.h"
#include "company.h"
#include "cheque_select.h"
#include "receive_select.h"
#include "gltx_frame.h"
#include "table.h"
#include "multi_line_edit.h"
#include "money_edit.h"
#include "check_box.h"
#include "lookup_edit.h"
#include "account_lookup.h"
#include "customer_lookup.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <algorithm>

ChequeCustomer::ChequeCustomer(MainWindow* main, Id cheque_id)
    : DataWindow(main, "CustomerCheque", cheque_id)
{
    _helpSource = "customer_cheque.html";

    _file->insertItem(tr("Recurring"), this, SLOT(slotRecurring()));

    // Printed checkbox
    _printed = new QCheckBox(tr("Printed?"), _buttons);
    _printed->setFocusPolicy(NoFocus);
    QPushButton* print = new QPushButton(tr("Print"), _buttons);
    connect(print, SIGNAL(clicked()), SLOT(slotPrint()));

    // Create widgets
    QFrame* top = new QFrame(_frame);
    top->setFrameStyle(QFrame::Raised | QFrame::Panel);

    QLabel* accountLabel = new QLabel(tr("Bank Account:"), top);
    _account = new LookupEdit(new AccountLookup(main, this, Account::Bank),
			      top);
    _account->setLength(30);
    accountLabel->setBuddy(_account);
    connect(_account, SIGNAL(validData()), SLOT(slotAccountChanged()));

    QLabel* currentLabel = new QLabel(tr("Current Balance:"), top);
    _current = new MoneyEdit(top);
    _current->setFocusPolicy(NoFocus);

    QLabel* endingLabel = new QLabel(tr("Ending Balance:"), top);
    _ending = new MoneyEdit(top);
    _ending->setFocusPolicy(NoFocus);

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(3);
    topGrid->setMargin(3);
    topGrid->setColStretch(2, 1);
    topGrid->addColSpacing(2, 10);
    topGrid->addWidget(accountLabel, 0, 0);
    topGrid->addWidget(_account, 0, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(currentLabel, 0, 3);
    topGrid->addWidget(_current, 0, 4, AlignLeft | AlignVCenter);
    topGrid->addWidget(endingLabel, 1, 3);
    topGrid->addWidget(_ending, 1, 4, AlignLeft | AlignVCenter);

    _gltxFrame = new GltxFrame(main, tr("Cheque No."), _frame);
    _gltxFrame->hideMemo();

    QFrame* mid = new QFrame(_frame);
    mid->setFrameStyle(QFrame::Raised | QFrame::Panel);

    QLabel* customerLabel = new QLabel(tr("Customer:"), mid);
    _customer = new LookupEdit(new CustomerLookup(main, this), mid);
    _customer->setLength(30);
    customerLabel->setBuddy(_customer);
    connect(_customer, SIGNAL(validData()), SLOT(slotCustomerChanged()));

    QLabel* balanceLabel = new QLabel(tr("Balance:"), mid);
    _balance = new MoneyEdit(mid);
    _balance->setFocusPolicy(NoFocus);

    QLabel* payeeLabel = new QLabel(tr("Payee:"), mid);
    _payee = new MultiLineEdit(mid);
    _payee->setFixedVisibleLines(4);
    payeeLabel->setBuddy(_payee);

    QLabel* amountLabel = new QLabel(tr("Amount:"), mid);
    _amount = new MoneyEdit(mid);
    amountLabel->setBuddy(_amount);
    connect(_amount, SIGNAL(validData()), SLOT(recalculate()));

    QLabel* memoLabel = new QLabel(tr("&Memo:"), mid);
    _memo = new LineEdit(mid);
    _memo->setMaxLength(40);
    _memo->setMinimumWidth(_memo->fontMetrics().width('x') * 40);
    memoLabel->setBuddy(_memo);

    QGridLayout* midGrid = new QGridLayout(mid);
    midGrid->setSpacing(3);
    midGrid->setMargin(3);
    midGrid->setColStretch(2, 1);
    midGrid->addWidget(customerLabel, 0, 0);
    midGrid->addWidget(_customer, 0, 1, AlignLeft | AlignVCenter);
    midGrid->addWidget(balanceLabel, 0, 2, AlignRight | AlignVCenter);
    midGrid->addWidget(_balance, 0, 3, AlignLeft | AlignVCenter);
    midGrid->addWidget(payeeLabel, 1, 0);
    midGrid->addMultiCellWidget(_payee, 1, 3, 1, 1);
    midGrid->addWidget(amountLabel, 1, 2, AlignRight | AlignVCenter);
    midGrid->addWidget(_amount, 1, 3, AlignLeft | AlignVCenter);
    midGrid->addWidget(memoLabel, 4, 0);
    midGrid->addMultiCellWidget(_memo, 4, 4, 1, 3);

    QFrame* bot = new QFrame(_frame);
    bot->setFrameStyle(QFrame::Raised | QFrame::Panel);

    QVBoxLayout* vbox2 = new QVBoxLayout(bot);
    vbox2->setSpacing(3);
    vbox2->setMargin(3);

    _table = new Table(bot);
    _table->setVScrollBarMode(QScrollView::AlwaysOn);
    _table->setDisplayRows(5);
    _table->setLeftMargin(0);
    _table->stopUserInsertDelete();
    connect(_table, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(cellChanged(int,int,Variant)));
    connect(_table, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(focusNext(bool&,int&,int&,int)));

    new TextColumn(_table, tr("Return"), 10);
    new DateColumn(_table, tr("Date"));
    new MoneyColumn(_table, tr("Total"));
    new MoneyColumn(_table, tr("Remaining"));
    new MoneyColumn(_table, tr("Amount"));
    new CheckColumn(_table, tr("Alloc?"));

    NumberEdit* amountEdit = new MoneyEdit(_table);
    amountEdit->setMaxLength(14);
    new NumberEditor(_table, 4, amountEdit);

    CheckBox* check = new CheckBox(_table);
    new CheckEditor(_table, 5, check);
    connect(check, SIGNAL(toggled(bool)), SLOT(slotAllocClicked()));

    QFrame* totals = new QFrame(bot);

    QPushButton* payAll = new QPushButton(tr("Select All"), totals);
    payAll->setFocusPolicy(NoFocus);
    connect(payAll, SIGNAL(clicked()), SLOT(slotSelectAll()));

    QPushButton* clearAll = new QPushButton(tr("Clear Alls"), totals);
    clearAll->setFocusPolicy(NoFocus);
    connect(clearAll, SIGNAL(clicked()), SLOT(slotClearAll()));

    QLabel* allocLabel = new QLabel(tr("Allocated:"), totals);
    _alloc = new MoneyEdit(totals);
    _alloc->setFocusPolicy(NoFocus);

    QGridLayout* totalGrid = new QGridLayout(totals);
    totalGrid->setSpacing(3);
    totalGrid->setColStretch(2, 1);
    totalGrid->addWidget(payAll, 0, 0);
    totalGrid->addWidget(clearAll, 0, 1);
    totalGrid->addWidget(allocLabel, 1, 3, AlignRight | AlignVCenter);
    totalGrid->addWidget(_alloc, 1, 4);

    vbox2->addWidget(_table);
    vbox2->addWidget(totals);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(0, 1);
    grid->setRowStretch(3, 1);
    grid->addWidget(top, 0, 0);
    grid->addWidget(_gltxFrame, 1, 0);
    grid->addWidget(mid, 2, 0);
    grid->addWidget(bot, 3, 0);

    _inactive->setText(tr("Voided?"));

    setCaption(tr("Customer Cheque"));
    finalize();
}

ChequeCustomer::~ChequeCustomer()
{
}

void
ChequeCustomer::setStoreId(Id store_id)
{
    if (store_id != INVALID_ID)
	_gltxFrame->store->setId(store_id);
}

void
ChequeCustomer::setCardId(Id card_id)
{
    if (card_id != INVALID_ID) {
	_customer->setId(card_id);
	slotCustomerChanged();
    }
}

void
ChequeCustomer::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _customer;
}

void
ChequeCustomer::newItem()
{
    Id bankId = _quasar->db()->defaultBankId();

    Cheque blank;
    _orig = blank;
    _gltxFrame->defaultData(_orig);
    _orig.setType(Cheque::CUSTOMER);
    _orig.setNumber(getChequeNumber(bankId).toString());
    _orig.setAccountId(bankId);

    _curr = _orig;
    _firstField = _customer;
}

void
ChequeCustomer::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _gltxFrame->cloneData(_curr);
    _curr.setNumber(getChequeNumber(_curr.accountId()).toString());
    dataToWidget();
}

bool
ChequeCustomer::fileItem()
{
    if (_curr.number() != "#") {
	vector<Cheque> cheques;
	ChequeSelect conditions;
	conditions.number = _curr.number();
	_quasar->db()->select(cheques, conditions);

	bool used = false;
	for (unsigned int i = 0; i < cheques.size(); ++i) {
	    if (cheques[i].id() == _curr.id()) continue;
	    if (cheques[i].accountId() != _curr.accountId()) continue;
	    used = true;
	}

	if (used) {
	    QString message = tr("This cheque number is already used for\n"
		"another cheque on this account. Are you\n"
		"sure you want to file this cheque?");
	    int choice = QMessageBox::warning(this, tr("Warning"), message,
					QMessageBox::No, QMessageBox::Yes);
	    if (choice != QMessageBox::Yes)
		return false;
	}
    }

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
ChequeCustomer::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
ChequeCustomer::restoreItem()
{
    _curr = _orig;
}

void
ChequeCustomer::cloneItem()
{
    ChequeCustomer* clone = new ChequeCustomer(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
ChequeCustomer::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void
ChequeCustomer::dataToWidget()
{
    _gltxFrame->setData(_curr);
    _account->setId(_curr.accountId());
    _customer->setId(_curr.cardId());
    _payee->setText(_curr.address());
    _amount->setFixed(_curr.total());
    _memo->setText(_curr.memo());
    _printed->setChecked(_curr.printed());
    _inactive->setChecked(!_curr.isActive());

    if (_payee->text().isEmpty())
	slotCustomerChanged();

    Id account_id = _account->getId();
    fixed balance = _quasar->db()->accountBalance(account_id);
    if (_orig.accountId() == account_id)
	balance += _orig.total();

    _current->setFixed(balance);
    _ending->setFixed(balance - _amount->getFixed());
}

// Set the data object from the widgets.
void
ChequeCustomer::widgetToData()
{
    _gltxFrame->getData(_curr);
    _curr.setAccountId(_account->getId());
    _curr.setCardId(_customer->getId());
    _curr.setAddress(_payee->text());
    _curr.setMemo(_memo->text());
    _curr.setPrinted(_printed->isChecked());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    Id store_id = _curr.storeId();

    // Add bank line
    vector<AccountLine>& lines = _curr.accounts();
    lines.clear();

    // Total up discounts and amounts
    fixed total = _amount->getFixed();
    lines.push_back(AccountLine(_account->getId(), -total));

    // Add card line
    _curr.cards().clear();
    if (_curr.cardId() != INVALID_ID) {
	Customer customer;
	_quasar->db()->lookup(_curr.cardId(), customer);
	_curr.accounts().push_back(AccountLine(customer.accountId(), total));
	_curr.cards().push_back(CardLine(_curr.cardId(), total));
    }

    // Load the lines from the table
    vector<PaymentLine>& payments = _curr.payments();
    payments.clear();
    for (int row = 0; row < _table->rows(); ++row) {
	Id gltx_id = _gltxs[row].id();
	fixed amount = _table->cellValue(row, 4).toFixed();

	if (amount != 0.0)
	    payments.push_back(PaymentLine(gltx_id, amount, 0.0));
    }
}

void
ChequeCustomer::printItem(bool ask)
{
    if (ask) return;
    ChequePrint* window = new ChequePrint(_main);
    window->setCheque(_curr.id());
    window->show();
}

void
ChequeCustomer::slotAccountChanged()
{
    Id account_id = _account->getId();
    fixed balance = _quasar->db()->accountBalance(account_id);
    if (account_id == _orig.accountId())
	balance += _orig.total();

    _gltxFrame->number->setText(getChequeNumber(account_id).toString());
    _current->setFixed(balance);
    _ending->setFixed(balance - _amount->getFixed());
}

void
ChequeCustomer::slotCustomerChanged()
{
    Customer customer;
    _quasar->db()->lookup(_customer->getId(), customer);
    _payee->setText(customer.address());

    _table->clear();
    _balance->setFixed(0.0);
    if (_customer->getId() != INVALID_ID)
	slotRefresh();
    recalculate();
}

void
ChequeCustomer::slotAllocClicked()
{
    int row = _table->currentRow();
    if (row < 0 || row >= _table->rows())
	return;

    // Just get value which should trigger cell changed
    _table->cellValue(row, 5).toBool();
}

void
ChequeCustomer::slotSelectAll()
{
    slotClearAll();
    fixed total = 0.0;
    for (int row = 0; row < _table->rows(); ++row) {
	fixed remain = _table->cellValue(row, 3).toFixed();
	total += remain;

	_table->setCellValue(row, 4, remain);
	_table->setCellValue(row, 5, true);
    }
    _amount->setFixed(total);
    recalculate();
}

void
ChequeCustomer::slotClearAll()
{
    for (int row = 0; row < _table->rows(); ++row) {
	_table->setCellValue(row, 4, "");
	_table->setCellValue(row, 5, false);
    }
    recalculate();
}

void
ChequeCustomer::slotRefresh()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Customer customer;
    _quasar->db()->lookup(_customer->getId(), customer);
    fixed balance = _quasar->db()->cardBalance(customer.id());
    _balance->setFixed(balance);

    // Load transactions
    GltxSelect conditions;
    conditions.card_id = customer.id();
    conditions.unpaid = true;
    conditions.activeOnly = true;
    vector<Gltx> gltxs;
    _quasar->db()->select(gltxs, conditions);

    // Load ones that have been paid
    if (_curr.id() != INVALID_ID && _curr.cardId() == customer.id()) {
	for (unsigned int i = 0; i < _curr.payments().size(); ++i) {
	    const PaymentLine& line = _curr.payments()[i];

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
    }

    // Sort by date
    std::sort(gltxs.begin(), gltxs.end());

    // Load into table
    _gltxs.clear();
    _table->clear();
    for (unsigned int i = 0; i < gltxs.size(); ++i) {
	Gltx& gltx = gltxs[i];

	// Skip non-credits
	if (gltx.cardTotal() >= 0.0) continue;

	// Remove payments that are this transaction
	for (unsigned int j = 0; j < gltx.payments().size(); ++j) {
	    const PaymentLine& line = gltx.payments()[j];
	    if (line.gltx_id == _curr.id())
		gltx.payments().erase(gltx.payments().begin() + j);
	}

	// If remain is zero, ignore it
	fixed total = gltx.cardTotal() + gltx.tenderTotal();
	fixed remain = gltx.cardTotal() - gltx.paymentTotal();
	if (remain == 0.0) continue;

	VectorRow* row = new VectorRow(_table->columns());
	row->setValue(0, gltx.number());
	row->setValue(1, gltx.postDate());
	row->setValue(2, -total);
	row->setValue(3, -remain);
	_table->appendRow(row);

	_gltxs.push_back(gltx);
    }

    // Adjust amounts for existing payments
    if (_curr.id() != INVALID_ID) {
	for (unsigned int i = 0; i < _curr.payments().size(); ++i) {
	    const PaymentLine& line = _curr.payments()[i];

	    for (unsigned int j = 0; j < _gltxs.size(); ++j) {
		const Gltx& gltx = _gltxs[j];
		if (gltx.id() != line.gltx_id) continue;
		_table->setCellValue(j, 4, line.amount);
		_table->setCellValue(j, 5, true);
		break;
	    }
	}
    }

    if (_table->rows() > 0) _table->setCurrentCell(0, 4);
    QApplication::restoreOverrideCursor();
}

void
ChequeCustomer::recalculate()
{
    fixed amount = _amount->getFixed();
    fixed alloc = 0.0;
    for (int row = 0; row < _table->rows(); ++row) {
	alloc += _table->cellValue(row, 4).toFixed();
    }

    _ending->setFixed(_current->getFixed() - amount);
    _alloc->setFixed(alloc);
}

fixed
ChequeCustomer::getChequeNumber(Id bankId)
{
    Account account;
    if (_quasar->db()->lookup(bankId, account))
	return account.nextNumber();
    return 1;
}

void
ChequeCustomer::cellChanged(int row, int col, Variant)
{
    fixed alloc_amt = _table->cellValue(row, 4).toFixed();
    bool alloc = _table->cellValue(row, 5).toBool();

    switch (col) {
    case 4: // alloc_amt
	break;
    case 5: // alloc?
	if (alloc) {
	    alloc_amt = _table->cellValue(row, 3).toFixed();
	    _table->setCellValue(row, 4, alloc_amt);
	} else {
	    alloc_amt = 0.0;
	    _table->setCellValue(row, 4, "");
	}
	break;
    }

    bool new_alloc = (alloc_amt != 0.0);
    if (alloc != new_alloc) {
	_table->setCellValue(row, 5, new_alloc);
    }

    recalculate();
}

void
ChequeCustomer::focusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _table->currentRow();
    int col = _table->currentColumn();

    if (type == Table::MoveNext && col == 5) {
	if (row == _table->rows() - 1) {
	    leave = true;
	} else {
	    newRow = row + 1;
	    newCol = 4;
	}
    } else if (type == Table::MovePrev && col == 4) {
	if (row > 0) {
	    newRow  = row - 1;
	    newCol = 5;
	} else {
	    leave = true;
	}
    }
}

void
ChequeCustomer::payInvoice(Id invoice_id)
{
    Gltx gltx;
    if (!_quasar->db()->lookup(invoice_id, gltx)) return;
    if (gltx.cardTotal() >= 0.0) return;

    _customer->setId(gltx.cardId());
    slotCustomerChanged();

    for (int row = 0; row < _table->rows(); ++row) {
	if (_gltxs[row].id() != invoice_id) continue;

	fixed remain = _table->cellValue(row, 3).toFixed();
	_amount->setFixed(remain);

	slotClearAll();
	_table->setCellValue(row, 4, remain);
	_table->setCellValue(row, 5, true);
	break;
    }

    recalculate();
}

void
ChequeCustomer::slotRecurring()
{
    if (!saveItem(true)) return;

    RecurringMaster* screen = new RecurringMaster(_main);
    screen->setGltx(_curr.id());
    screen->show();
}
