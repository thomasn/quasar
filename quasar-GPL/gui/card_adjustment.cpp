// $Id: card_adjustment.cpp,v 1.12 2004/06/16 20:50:11 bpepers Exp $
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

#include "card_adjustment.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "recurring_master.h"
#include "card_select.h"
#include "customer.h"
#include "vendor.h"
#include "account.h"
#include "gltx_frame.h"
#include "table.h"
#include "money_edit.h"
#include "lookup_edit.h"
#include "store_lookup.h"
#include "gltx_lookup.h"
#include "card_lookup.h"
#include "account_lookup.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpopupmenu.h>
#include <qvbox.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qlayout.h>
#include <qmessagebox.h>

CardAdjustment::CardAdjustment(MainWindow* main, Id adjustment_id)
    : DataWindow(main, "CardAdjustment", adjustment_id)
{
    _helpSource = "card_adjustment.html";

    _file->insertItem(tr("Recurring"), this, SLOT(slotRecurring()));

    // Create widgets
    _gltxFrame = new GltxFrame(main, tr("Adjustment No."), _frame);
    _gltxFrame->hideMemo();

    QFrame* mid = new QFrame(_frame);
    mid->setFrameStyle(QFrame::Raised | QFrame::Panel);

    QLabel* cardLabel = new QLabel(tr("Card:"), mid);
    CardLookup* cardLookup = new CardLookup(main, this);
    cardLookup->type->setCurrentItem(tr("Customer and Vendor"));
    cardLookup->type->setEnabled(false);
    _card = new LookupEdit(cardLookup, mid);
    _card->setLength(30);
    cardLabel->setBuddy(_card);
    connect(_card, SIGNAL(validData()), SLOT(slotCardChanged()));

    QLabel* balanceLabel = new QLabel(tr("Balance:"), mid);
    _balance = new MoneyEdit(mid);
    _balance->setFocusPolicy(NoFocus);

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
    midGrid->addWidget(cardLabel, 0, 0);
    midGrid->addWidget(_card, 0, 1, AlignLeft | AlignVCenter);
    midGrid->addWidget(balanceLabel, 0, 2, AlignRight | AlignVCenter);
    midGrid->addWidget(_balance, 0, 3, AlignLeft | AlignVCenter);
    midGrid->addWidget(amountLabel, 1, 0);
    midGrid->addWidget(_amount, 1, 1, AlignLeft | AlignVCenter);
    midGrid->addWidget(memoLabel, 2, 0);
    midGrid->addMultiCellWidget(_memo, 2, 2, 1, 3);

    _accounts = new Table(_frame);
    _accounts->setVScrollBarMode(QScrollView::AlwaysOn);
    _accounts->setLeftMargin(fontMetrics().width("99999"));
    _accounts->setDisplayRows(6);
    connect(_accounts, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(cellChanged(int,int,Variant)));
    connect(_accounts, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(focusNext(bool&,int&,int&,int)));

    // Lookups
    LookupWindow* accountLookup = new AccountLookup(_main, this);

    // Add columns
    new LookupColumn(_accounts, tr("Account"), 30, accountLookup);
    new MoneyColumn(_accounts, tr("Amount"));

    // Add editors
    new LookupEditor(_accounts, 0, new LookupEdit(accountLookup, _accounts));
    new NumberEditor(_accounts, 1, new MoneyEdit(_accounts));

    QLabel* remainLabel = new QLabel(tr("Difference:"), _frame);
    _remain = new MoneyEdit(_frame);
    _remain->setFocusPolicy(NoFocus);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(0, 1);
    grid->setRowStretch(2, 1);
    grid->addMultiCellWidget(_gltxFrame, 0, 0, 0, 2);
    grid->addMultiCellWidget(mid, 1, 1, 0, 2);
    grid->addMultiCellWidget(_accounts, 2, 2, 0, 2);
    grid->addWidget(remainLabel, 3, 1, AlignRight | AlignVCenter);
    grid->addWidget(_remain, 3, 2, AlignLeft | AlignVCenter);

    _inactive->setText(tr("Voided?"));

    setCaption(tr("Card Adjustment"));
    finalize();
}

CardAdjustment::~CardAdjustment()
{
}

void
CardAdjustment::oldItem()
{
    _quasar->db()->lookup(_id, _orig);
    _curr = _orig;
    _firstField = _card;
}

void
CardAdjustment::newItem()
{
    CardAdjust blank;
    _orig = blank;
    _gltxFrame->defaultData(_orig);

    _curr = _orig;
    _firstField = _card;
}

void
CardAdjustment::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _gltxFrame->cloneData(_curr);
    dataToWidget();
}

bool
CardAdjustment::fileItem()
{
    if (_curr.number() != "#")
	if (checkGltxUsed(_curr.number(), DataObject::CARD_ADJUST, _curr.id()))
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
CardAdjustment::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
CardAdjustment::restoreItem()
{
    _curr = _orig;
}

void
CardAdjustment::cloneItem()
{
    CardAdjustment* clone = new CardAdjustment(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
CardAdjustment::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
CardAdjustment::dataToWidget()
{
    _gltxFrame->setData(_curr);
    _card->setId(_curr.cardId());
    _amount->setFixed(_curr.total());
    _memo->setText(_curr.memo());
    _inactive->setChecked(!_curr.isActive());

    Card card;
    _quasar->db()->lookup(_curr.cardId(), card);

    // Load the accounts
    _accounts->clear();
    for (unsigned int i = 1; i < _curr.accounts().size(); ++i) {
	Id account_id = _curr.accounts()[i].account_id;
	fixed amount = _curr.accounts()[i].amount;

	if (card.dataType() == DataObject::CUSTOMER)
	    amount = -amount;

	VectorRow* row = new VectorRow(_accounts->columns());
	row->setValue(0, account_id);
	row->setValue(1, amount);
	_accounts->appendRow(row);
    }
    _accounts->appendRow(new VectorRow(_accounts->columns()));

    slotCardChanged();
    recalculate();
}

// Set the data object from the widgets.
void
CardAdjustment::widgetToData()
{
    _gltxFrame->getData(_curr);
    _curr.setCardId(_card->getId());
    _curr.setMemo(_memo->text());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    Id store_id = _curr.storeId();
    Id card_id = _curr.cardId();
    fixed amount = _amount->getFixed();

    Card card;
    _quasar->db()->lookup(card_id, card);

    vector<CardLine>& cards = _curr.cards();
    vector<AccountLine>& accounts = _curr.accounts();

    // Card balance change
    cards.clear();
    cards.push_back(CardLine(card_id, amount));

    // Ledger postings
    accounts.clear();
    if (card.dataType() == DataObject::CUSTOMER) {
	Customer customer;
	_quasar->db()->lookup(card_id, customer);
	accounts.push_back(AccountLine(customer.accountId(), amount));

	// Add the accounts
	for (int row = 0; row < _accounts->rows(); ++row) {
	    Id account_id = _accounts->cellValue(row, 0).toId();
	    if (account_id == INVALID_ID) continue;
	    fixed amount = _accounts->cellValue(row, 1).toFixed();

	    accounts.push_back(AccountLine(account_id, amount * -1));
	}
    } else {
	Vendor vendor;
	_quasar->db()->lookup(card_id, vendor);
	accounts.push_back(AccountLine(vendor.accountId(), amount * -1));

	// Add the accounts
	for (int row = 0; row < _accounts->rows(); ++row) {
	    Id account_id = _accounts->cellValue(row, 0).toId();
	    if (account_id == INVALID_ID) continue;
	    fixed amount = _accounts->cellValue(row, 1).toFixed();

	    accounts.push_back(AccountLine(account_id, amount));
	}
    }
}

void
CardAdjustment::slotCardChanged()
{
    fixed balance = _quasar->db()->cardBalance(_card->getId());
    _balance->setFixed(balance);
}

void
CardAdjustment::recalculate()
{
    fixed amount = _amount->getFixed();
    fixed alloc = 0.0;

    for (int row = 0; row < _accounts->rows(); ++row) {
	Id account_id = _accounts->cellValue(row, 0).toId();
	if (account_id == INVALID_ID) continue;
	alloc += _accounts->cellValue(row, 1).toFixed();
    }

    if (alloc == amount)
	_remain->clear();
    else
	_remain->setFixed(amount - alloc);
}

void
CardAdjustment::cellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _accounts->rows() - 1 && col == 0) {
	Id id = _accounts->cellValue(row, col).toId();
	if (id != INVALID_ID)
	    _accounts->appendRow(new VectorRow(_accounts->columns()));
    }

    recalculate();
}

void
CardAdjustment::focusNext(bool& leave, int&, int&, int type)
{
    int row = _accounts->currentRow();
    int col = _accounts->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id id = _accounts->cellValue(row, col).toId();
	if (id == INVALID_ID && row == _accounts->rows() - 1) {
	    leave = true;
	} else {
	    fixed amount = _accounts->cellValue(row, 1).toFixed();
	    if (amount == 0.0) {
		fixed remain = _amount->getFixed();
		for (int r = 0; r < _accounts->rows(); ++r)
		    remain -= _accounts->cellValue(r, 1, false).toFixed();
		_accounts->setCellValue(row, 1, remain);
		recalculate();
	    }
	}
    }
}

void
CardAdjustment::slotRecurring()
{
    if (!saveItem(true)) return;

    RecurringMaster* screen = new RecurringMaster(_main);
    screen->setGltx(_curr.id());
    screen->show();
}
