// $Id: cheque_master.cpp,v 1.55 2004/03/05 08:50:58 bpepers Exp $
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

#include "cheque_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "cheque_print.h"
#include "recurring_master.h"
#include "account.h"
#include "cheque_select.h"
#include "gltx_frame.h"
#include "table.h"
#include "date_popup.h"
#include "multi_line_edit.h"
#include "id_edit.h"
#include "money_edit.h"
#include "lookup_edit.h"
#include "account_lookup.h"
#include "card.h"
#include "card_lookup.h"
#include "store_lookup.h"

#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qlineedit.h>
#include <qpopupmenu.h>
#include <qlayout.h>
#include <qmessagebox.h>

ChequeMaster::ChequeMaster(MainWindow* main, Id cheque_id)
    : DataWindow(main, "ChequeMaster", cheque_id)
{
    _helpSource = "cheque_master.html";

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
    topGrid->setMargin(3);
    topGrid->setSpacing(3);
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

    QLabel* cardLabel = new QLabel(tr("Card:"), mid);
    _card = new LookupEdit(new CardLookup(main, this), mid);
    _card->setLength(30);
    cardLabel->setBuddy(_card);
    connect(_card, SIGNAL(validData()), SLOT(slotCardChanged()));

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
    midGrid->setRowStretch(3, 1);
    midGrid->addWidget(cardLabel, 0, 0);
    midGrid->addWidget(_card, 0, 1, AlignLeft | AlignVCenter);
    midGrid->addWidget(balanceLabel, 0, 2, AlignRight | AlignVCenter);
    midGrid->addWidget(_balance, 0, 3, AlignLeft | AlignVCenter);
    midGrid->addWidget(payeeLabel, 1, 0);
    midGrid->addMultiCellWidget(_payee, 1, 3, 1, 1);
    midGrid->addWidget(amountLabel, 1, 2, AlignRight | AlignVCenter);
    midGrid->addWidget(_amount, 1, 3, AlignLeft | AlignVCenter);
    midGrid->addWidget(memoLabel, 4, 0);
    midGrid->addMultiCellWidget(_memo, 4, 4, 1, 3);

    _table = new Table(_frame);
    _table->setVScrollBarMode(QScrollView::AlwaysOn);
    _table->setLeftMargin(fontMetrics().width("99999"));
    _table->setDisplayRows(6);
    connect(_table, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(cellChanged(int,int,Variant)));
    connect(_table, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(focusNext(bool&,int&,int&,int)));

    // Lookups
    LookupWindow* accountLookup = new AccountLookup(_main, this);

    // Add columns
    new LookupColumn(_table, tr("Account"), 30, accountLookup);
    new MoneyColumn(_table, tr("Amount"));

    // Add editors
    new LookupEditor(_table, 0, new LookupEdit(accountLookup, _table));
    new NumberEditor(_table, 1, new MoneyEdit(_table));

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(0, 1);
    grid->setRowStretch(3, 1);
    grid->addWidget(top, 0, 0);
    grid->addWidget(_gltxFrame, 1, 0);
    grid->addWidget(mid, 2, 0);
    grid->addWidget(_table, 3, 0);

    _inactive->setText(tr("Voided?"));

    setCaption(tr("Write Cheque"));
    finalize();
}

ChequeMaster::~ChequeMaster()
{
}

void
ChequeMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);
    _curr = _orig;
    _firstField = _card;
}

void
ChequeMaster::newItem()
{
    Id bankId = _quasar->db()->defaultBankId();

    Cheque blank;
    _orig = blank;
    _gltxFrame->defaultData(_orig);
    _orig.setNumber(getChequeNumber(bankId).toString());
    _orig.setAccountId(bankId);

    _curr = _orig;
    _firstField = _card;
}

void
ChequeMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _gltxFrame->cloneData(_curr);
    _curr.setNumber(getChequeNumber(_curr.accountId()).toString());
    dataToWidget();
}

bool
ChequeMaster::fileItem()
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
ChequeMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
ChequeMaster::restoreItem()
{
    _curr = _orig;
}

void
ChequeMaster::cloneItem()
{
    ChequeMaster* clone = new ChequeMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
ChequeMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
ChequeMaster::dataToWidget()
{
    _gltxFrame->setData(_curr);
    _account->setId(_curr.accountId());
    _card->setId(_curr.cardId());
    _payee->setText(_curr.address());
    _amount->setFixed(_curr.total());
    _memo->setText(_curr.memo());
    _printed->setChecked(_curr.printed());
    _inactive->setChecked(!_curr.isActive());

    if (_payee->text().isEmpty())
	slotCardChanged();

    Id account_id = _account->getId();
    fixed balance = _quasar->db()->accountBalance(account_id);
    if (_orig.accountId() == account_id)
	balance += _orig.total();
    _current->setFixed(balance);
    _ending->setFixed(balance - _amount->getFixed());

    // Set current allocations
    const vector<AccountLine>& lines = _curr.accounts();
    _table->clear();
    for (unsigned int line = 1; line < lines.size(); ++line) {
	Id account_id = lines[line].account_id;
	fixed amount = lines[line].amount;

	VectorRow* row = new VectorRow(_table->columns());
	row->setValue(0, account_id);
	row->setValue(1, amount);
	_table->appendRow(row);
    }

    _table->appendRow(new VectorRow(_table->columns()));
}

// Set the data object from the widgets.
void
ChequeMaster::widgetToData()
{
    _gltxFrame->getData(_curr);
    _curr.setAccountId(_account->getId());
    _curr.setCardId(_card->getId());
    _curr.setAddress(_payee->text());
    _curr.setMemo(_memo->text());
    _curr.setPrinted(_printed->isChecked());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    Id store_id = _curr.storeId();

    // Add bank line
    vector<AccountLine>& lines = _curr.accounts();
    lines.clear();
    lines.push_back(AccountLine(_account->getId(), -_amount->getFixed()));

    // Load the lines from the table
    for (int row = 0; row < _table->rows(); ++row) {
	Id account_id = _table->cellValue(row, 0).toId();
	if (account_id == INVALID_ID) continue;

	fixed amount = _table->cellValue(row, 1).toFixed();
	lines.push_back(AccountLine(account_id, amount));
    }
}

void
ChequeMaster::printItem(bool ask)
{
    if (ask) return;
    ChequePrint* window = new ChequePrint(_main);
    window->setCheque(_curr.id());
    window->show();
}

void
ChequeMaster::slotCardChanged()
{
    Card card;
    _quasar->db()->lookup(_card->getId(), card);
    _payee->setText(card.address());

    fixed balance = _quasar->db()->cardBalance(card.id());
    _balance->setFixed(balance);

    recalculate();
}

void
ChequeMaster::slotAccountChanged()
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
ChequeMaster::recalculate()
{
    fixed amount = _amount->getFixed();
    _ending->setFixed(_current->getFixed() - amount);
}

fixed
ChequeMaster::getChequeNumber(Id bankId)
{
    Account account;
    if (_quasar->db()->lookup(bankId, account))
	return account.nextNumber();
    return 1;
}

void
ChequeMaster::cellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _table->rows() - 1 && col == 0) {
	Id id = _table->cellValue(row, col).toId();
	if (id != INVALID_ID)
	    _table->appendRow(new VectorRow(_table->columns()));
    }

    recalculate();
}

void
ChequeMaster::focusNext(bool& leave, int&, int&, int type)
{
    int row = _table->currentRow();
    int col = _table->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id id = _table->cellValue(row, col).toId();
	if (id == INVALID_ID && row == _table->rows() - 1) {
	    leave = true;
	} else {
	    fixed amount = _table->cellValue(row, 1).toFixed();
	    if (amount == 0.0) {
		fixed remain = _amount->getFixed();
		for (int r = 0; r < _table->rows(); ++r)
		    remain -= _table->cellValue(r, 1, false).toFixed();
		_table->setCellValue(row, 1, remain);
	    }
	}
    }
}

void
ChequeMaster::slotRecurring()
{
    if (!saveItem(true)) return;

    RecurringMaster* screen = new RecurringMaster(_main);
    screen->setGltx(_curr.id());
    screen->show();
}
