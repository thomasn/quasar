// $Id: card_transfer.cpp,v 1.19 2004/06/16 20:50:11 bpepers Exp $
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

#include "card_transfer.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "company.h"
#include "card_select.h"
#include "customer.h"
#include "vendor.h"
#include "account.h"
#include "gltx_frame.h"
#include "money_edit.h"
#include "lookup_edit.h"
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

CardTransfer::CardTransfer(MainWindow* main, Id transfer_id)
    : DataWindow(main, "CardTransfer", transfer_id)
{
    _helpSource = "card_transfer.html";

    // Create widgets
    _gltxFrame = new GltxFrame(main, tr("Transfer No."), _frame);
    _gltxFrame->setTitle(tr("From"));
    _gltxFrame->hideMemo();

    QFrame* mid = new QFrame(_frame);
    mid->setFrameStyle(QFrame::Raised | QFrame::Panel);

    QLabel* fromLabel = new QLabel(tr("From:"), mid);
    CardLookup* fromLookup = new CardLookup(main, this);
    fromLookup->type->setCurrentItem(tr("Customer and Vendor"));
    fromLookup->type->setEnabled(false);
    _from = new LookupEdit(fromLookup, mid);
    _from->setLength(30);
    fromLabel->setBuddy(_from);
    connect(_from, SIGNAL(validData()), SLOT(slotFromChanged()));

    QLabel* fromBalanceLabel = new QLabel(tr("Balance:"), mid);
    _fromBalance = new MoneyEdit(mid);
    _fromBalance->setFocusPolicy(NoFocus);

    QLabel* toLabel = new QLabel(tr("To:"), mid);
    CardLookup* toLookup = new CardLookup(main, this);
    toLookup->type->setCurrentItem(tr("Customer and Vendor"));
    toLookup->type->setEnabled(false);
    _to = new LookupEdit(toLookup, mid);
    _to->setLength(30);
    toLabel->setBuddy(_to);
    connect(_to, SIGNAL(validData()), SLOT(slotToChanged()));

    QLabel* toBalanceLabel = new QLabel(tr("Balance:"), mid);
    _toBalance = new MoneyEdit(mid);
    _toBalance->setFocusPolicy(NoFocus);

    QLabel* amountLabel = new QLabel(tr("Amount:"), mid);
    _amount = new MoneyEdit(mid);
    amountLabel->setBuddy(_amount);

    QLabel* memoLabel = new QLabel(tr("&Memo:"), mid);
    _memo = new LineEdit(mid);
    _memo->setMaxLength(40);
    _memo->setMinimumWidth(_memo->fontMetrics().width('x') * 40);
    memoLabel->setBuddy(_memo);

    QGridLayout* midGrid = new QGridLayout(mid);
    midGrid->setSpacing(3);
    midGrid->setMargin(3);
    midGrid->setColStretch(2, 1);
    midGrid->addWidget(fromLabel, 0, 0);
    midGrid->addWidget(_from, 0, 1, AlignLeft | AlignVCenter);
    midGrid->addWidget(fromBalanceLabel, 0, 2, AlignRight | AlignVCenter);
    midGrid->addWidget(_fromBalance, 0, 3, AlignLeft | AlignVCenter);
    midGrid->addWidget(toLabel, 1, 0);
    midGrid->addWidget(_to, 1, 1, AlignLeft | AlignVCenter);
    midGrid->addWidget(toBalanceLabel, 1, 2, AlignRight | AlignVCenter);
    midGrid->addWidget(_toBalance, 1, 3, AlignLeft | AlignVCenter);
    midGrid->addWidget(amountLabel, 2, 0);
    midGrid->addWidget(_amount, 2, 1, AlignLeft | AlignVCenter);
    midGrid->addWidget(memoLabel, 3, 0);
    midGrid->addMultiCellWidget(_memo, 3, 3, 1, 3);

    QGroupBox* to = new QGroupBox(tr("To"), _frame);

    QLabel* toNumberLabel = new QLabel(tr("Transfer No."), to);
    _toNumber = new LineEdit(9, to);
    toNumberLabel->setBuddy(_toNumber);

    QLabel* toShiftLabel = new QLabel(tr("Shift:"), to);
    _toShift = new LookupEdit(new GltxLookup(_main, this, DataObject::SHIFT),
			       to);
    _toShift->setLength(10);
    _toShift->setFocusPolicy(ClickFocus);
    toShiftLabel->setBuddy(_toShift);

    QGridLayout* toGrid = new QGridLayout(to, 1, 1, to->frameWidth() * 2);
    toGrid->setSpacing(3);
    toGrid->setMargin(6);
    toGrid->setColStretch(2, 1);
    toGrid->addColSpacing(2, 10);
    toGrid->addRowSpacing(0, to->fontMetrics().height());
    toGrid->addWidget(toNumberLabel, 1, 0);
    toGrid->addWidget(_toNumber, 1, 1, AlignLeft | AlignVCenter);
    toGrid->addWidget(toShiftLabel, 1, 3);
    toGrid->addWidget(_toShift, 1, 4, AlignLeft | AlignVCenter);

    QLabel* accountLabel = new QLabel(tr("Transfer Account:"), _frame);
    AccountLookup* lookup = new AccountLookup(main, this, Account::Expense);
    _account = new LookupEdit(lookup, _frame);
    _account->setLength(30);
    accountLabel->setBuddy(_account);

    _inactive->setText(tr("Voided?"));

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(2, 1);
    grid->setRowStretch(1, 1);
    grid->addMultiCellWidget(_gltxFrame, 0, 0, 0, 4);
    grid->addMultiCellWidget(mid, 1, 1, 0, 4);
    grid->addMultiCellWidget(to, 2, 2, 0, 4);
    grid->addWidget(accountLabel, 3, 0);
    grid->addWidget(_account, 3, 1, AlignLeft | AlignVCenter);

    setCaption(tr("Card Transfer"));
    finalize();
}

CardTransfer::~CardTransfer()
{
}

void
CardTransfer::oldItem()
{
    _quasar->db()->lookup(_id, _orig);
    _quasar->db()->lookup(_orig.linkId(), _origLink);

    // Switch to show consistantly
    if (_orig.number() > _origLink.number()) {
	CardAdjust temp = _orig;
	_orig = _origLink;
	_origLink = temp;
    }

    _curr = _orig;
    _link = _origLink;
    _firstField = _from;
}

void
CardTransfer::newItem()
{
    CardAdjust blank;
    _orig = blank;
    _gltxFrame->defaultData(_orig);

    _origLink = _orig;
    _origLink.setCardId(INVALID_ID);

    _curr = _orig;
    _link = _origLink;
    _firstField = _from;
}

void
CardTransfer::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _quasar->db()->lookup(_curr.linkId(), _link);
    _gltxFrame->cloneData(_curr);
    _gltxFrame->cloneData(_link);
    dataToWidget();
}

bool
CardTransfer::fileItem()
{
    if (_curr.number() != "#")
	if (checkGltxUsed(_curr.number(), DataObject::CARD_ADJUST, _curr.id()))
	    return false;
    if (_link.number() != "#")
	if (checkGltxUsed(_link.number(), DataObject::CARD_ADJUST, _link.id()))
	    return false;

    if (_orig.id() == INVALID_ID) {
	if (!_quasar->db()->create(_curr)) return false;
	if (!_quasar->db()->create(_link)) return false;
	if (!_quasar->db()->linkTx(_curr.id(), _link.id())) return false;
    } else {
	if (!_quasar->db()->update(_orig, _curr)) return false;
	if (!_quasar->db()->update(_origLink, _link)) return false;
    }

    Company company;
    _quasar->db()->lookup(company);
    if (company.transferAccount() == INVALID_ID) {
	Company orig = company;
	company.setTransferAccount(_account->getId());
	_quasar->db()->update(orig, company);
    }

    _orig = _curr;
    _origLink = _link;
    _id = _curr.id();

    return true;
}

bool
CardTransfer::deleteItem()
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
CardTransfer::restoreItem()
{
    _curr = _orig;
    _link = _origLink;
}

void
CardTransfer::cloneItem()
{
    CardTransfer* clone = new CardTransfer(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
CardTransfer::isChanged()
{
    return _curr != _orig || _link != _origLink;
}

// Set the widgets from the data object.
void 
CardTransfer::dataToWidget()
{
    _gltxFrame->setData(_curr);
    _from->setId(_curr.cardId());
    _to->setId(_link.cardId());
    _memo->setText(_curr.memo());
    _toNumber->setText(_link.number());
    _toShift->setId(_link.shiftId());
    _inactive->setChecked(!_curr.isActive());

    if (_curr.accounts().size() < 2) {
	_amount->setText("");
	_account->setId(INVALID_ID);
    } else {
        Card from, to;
        _quasar->db()->lookup(_curr.cardId(), from);
        _quasar->db()->lookup(_link.cardId(), to);

        if (from.dataType() == to.dataType()) {
            if (to.dataType() == DataObject::CUSTOMER)
                _amount->setFixed(_curr.accounts()[1].amount);
            else if (to.dataType() == DataObject::VENDOR)
                _amount->setFixed(-_curr.accounts()[1].amount);
        } else {
            if (to.dataType() == DataObject::CUSTOMER)
                _amount->setFixed(-_curr.accounts()[1].amount);
            else if (to.dataType() == DataObject::VENDOR)
                _amount->setFixed(_curr.accounts()[1].amount);
        }

	_account->setId(_curr.accounts()[1].account_id);
    }

    slotFromChanged();
    slotToChanged();
}

// Set the data object from the widgets.
void
CardTransfer::widgetToData()
{
    _gltxFrame->getData(_curr);
    _curr.setMemo(_memo->text());
    _curr.setCardId(_from->getId());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    _gltxFrame->getData(_link);
    _link.setNumber(_toNumber->text());
    _link.setShiftId(_toShift->getId());
    _link.setMemo(_memo->text());
    _link.setCardId(_to->getId());
    _quasar->db()->setActive(_link, !_inactive->isChecked());

    fixed amount = _amount->getFixed();
    Id transferAcct = _account->getId();
    Card card;

    Id fromAcct;
    _quasar->db()->lookup(_curr.cardId(), card);
    DataObject::DataType fromType = card.dataType();
    if (card.dataType() == DataObject::CUSTOMER) {
	Customer customer;
	_quasar->db()->lookup(card.id(), customer);
	fromAcct = customer.accountId();
    } else if (card.dataType() == DataObject::VENDOR) {
	Vendor vendor;
	_quasar->db()->lookup(card.id(), vendor);
	fromAcct = vendor.accountId();
    }

    Id toAcct;
    _quasar->db()->lookup(_link.cardId(), card);
    DataObject::DataType toType = card.dataType();
    if (card.dataType() == DataObject::CUSTOMER) {
	Customer customer;
	_quasar->db()->lookup(card.id(), customer);
	toAcct = customer.accountId();
    } else if (card.dataType() == DataObject::VENDOR) {
	Vendor vendor;
	_quasar->db()->lookup(card.id(), vendor);
	toAcct = vendor.accountId();
    }

    // Process fromAcct and transfer acct on first card
    _curr.cards().clear();
    _curr.cards().push_back(CardLine(_curr.cardId(), -amount));
    _curr.accounts().clear();
    if (fromType == DataObject::CUSTOMER) {
        _curr.accounts().push_back(AccountLine(fromAcct, -amount));
        _curr.accounts().push_back(AccountLine(transferAcct, amount));
    } else if (fromType == DataObject::VENDOR) {
        _curr.accounts().push_back(AccountLine(fromAcct, amount));
        _curr.accounts().push_back(AccountLine(transferAcct, -amount));
    }

    // Process toAcct and transfer acct on second card
    _link.cards().clear();
    if (fromType == toType) {
        _link.cards().push_back(CardLine(_link.cardId(), amount));
        _link.accounts().clear();
        if (toType == DataObject::CUSTOMER) {
            _link.accounts().push_back(AccountLine(toAcct, amount));
            _link.accounts().push_back(AccountLine(transferAcct, -amount));
        } else if (toType == DataObject::VENDOR) {
            _link.accounts().push_back(AccountLine(toAcct, -amount));
            _link.accounts().push_back(AccountLine(transferAcct, amount));
       }
    } else {
        _link.cards().push_back(CardLine(_link.cardId(), -amount));
        _link.accounts().clear();
        if (toType == DataObject::CUSTOMER) {
            _link.accounts().push_back(AccountLine(toAcct, -amount));
            _link.accounts().push_back(AccountLine(transferAcct, amount));
        } else if (toType == DataObject::VENDOR) {
            _link.accounts().push_back(AccountLine(toAcct, amount));
            _link.accounts().push_back(AccountLine(transferAcct, -amount));
	}
    }
}

void
CardTransfer::slotFromChanged()
{
    fixed balance = _quasar->db()->cardBalance(_from->getId());
    _fromBalance->setFixed(balance);
}

void
CardTransfer::slotToChanged()
{
    fixed balance = _quasar->db()->cardBalance(_to->getId());
    _toBalance->setFixed(balance);
}
