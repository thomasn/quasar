// $Id: ledger_transfer.cpp,v 1.19 2005/01/30 04:25:31 bpepers Exp $
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

#include "ledger_transfer.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "general_select.h"
#include "gltx_frame.h"
#include "table.h"
#include "money_edit.h"
#include "lookup_edit.h"
#include "account_lookup.h"
#include "store_lookup.h"
#include "account.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qlayout.h>
#include <qmessagebox.h>

LedgerTransfer::LedgerTransfer(MainWindow* main, Id transfer_id)
    : DataWindow(main, "LedgerTransfer", transfer_id)
{
    _helpSource = "ledger_transfer.html";

    // Get the company for transfer account
    _quasar->db()->lookup(_company);

    // Create widgets
    _gltxFrame = new GltxFrame(main, tr("Transfer No."), _frame);
    _gltxFrame->setTitle(tr("From"));

    QLabel* accountLabel = new QLabel(tr("Account:"), _frame);
    AccountLookup* lookup = new AccountLookup(main, this, Account::Expense);
    _account = new LookupEdit(lookup, _frame);
    _account->setLength(30);
    accountLabel->setBuddy(_account);
    connect(_account, SIGNAL(validData()), SLOT(slotAccountChanged()));

    QLabel* balanceLabel = new QLabel(tr("Balance:"), _frame);
    _balance = new MoneyEdit(_frame);
    _balance->setFocusPolicy(NoFocus);

    QLabel* amountLabel = new QLabel(tr("Amount:"), _frame);
    _amount = new MoneyEdit(_frame);
    amountLabel->setBuddy(_amount);
    connect(_amount, SIGNAL(validData()), SLOT(recalculate()));

    _table = new Table(_frame);
    _table->setVScrollBarMode(QScrollView::AlwaysOn);
    connect(_table, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(cellChanged(int,int,Variant)));
    connect(_table, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(focusNext(bool&,int&,int&,int)));

    // Lookups
    LookupWindow* storeLookup = new StoreLookup(_main, this);

    // Add columns
    new LookupColumn(_table, tr("Store"), 30, storeLookup);
    new TextColumn(_table, tr("Transfer No."), 10);
    new MoneyColumn(_table, tr("Amount"));

    // Add editors
    new LookupEditor(_table, 0, new LookupEdit(storeLookup, _table));
    new NumberEditor(_table, 2, new MoneyEdit(_table));

    QLabel* transferLabel = new QLabel(tr("Transfer Account:"), _frame);
    lookup = new AccountLookup(main, this, Account::OtherCurLiability);
    _transfer_acct = new LookupEdit(lookup, _frame);
    _transfer_acct->setLength(30);
    transferLabel->setBuddy(_transfer_acct);

    QLabel* allocLabel = new QLabel(tr("Allocated:"), _frame);
    _alloc = new MoneyEdit(_frame);
    _alloc->setFocusPolicy(NoFocus);

    QLabel* differenceLabel = new QLabel(tr("Difference:"), _frame);
    _difference = new MoneyEdit(_frame);
    _difference->setFocusPolicy(NoFocus);

    QPalette palette = _difference->palette();
    palette.setColor(QPalette::Active, QColorGroup::Text, QColor(255, 10, 10));
    _difference->setPalette(palette);

    _inactive->setText(tr("Voided?"));

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(2, 1);
    grid->setRowStretch(4, 1);
    grid->addColSpacing(2, 20);
    grid->addMultiCellWidget(_gltxFrame, 0, 0, 0, 4);
    grid->addWidget(accountLabel, 1, 0);
    grid->addWidget(_account, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(balanceLabel, 1, 3);
    grid->addWidget(_balance, 1, 4, AlignLeft | AlignVCenter);
    grid->addWidget(amountLabel, 2, 0);
    grid->addWidget(_amount, 2, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_table, 4, 4, 0, 4);
    grid->addWidget(transferLabel, 5, 0);
    grid->addWidget(_transfer_acct, 5, 1, AlignLeft | AlignVCenter);
    grid->addWidget(allocLabel, 5, 3, AlignRight | AlignVCenter);
    grid->addWidget(_alloc, 5, 4, AlignRight | AlignVCenter);
    grid->addWidget(differenceLabel, 6, 3, AlignRight | AlignVCenter);
    grid->addWidget(_difference, 6, 4, AlignRight | AlignVCenter);

    setCaption(tr("Ledger Transfer"));
    finalize();
}

LedgerTransfer::~LedgerTransfer()
{
}

void
LedgerTransfer::oldItem()
{
    General origLink;
    _quasar->db()->lookup(_id, _orig);
    _quasar->db()->lookup(_orig.linkId(), origLink);

    // Switch to show consistantly
    if (_orig.number() > origLink.number()) {
	_orig = origLink;
    }

    GeneralSelect conditions;
    conditions.link_id = _orig.id();
    _quasar->db()->select(_origLinks, conditions);

    _curr = _orig;
    _links = _origLinks;
    _firstField = _gltxFrame->firstField();
}

void
LedgerTransfer::newItem()
{
    General blank;
    _orig = blank;
    _gltxFrame->defaultData(_orig);
    _origLinks.clear();

    _curr = _orig;
    _links = _origLinks;
    _firstField = _gltxFrame->firstField();
}

void
LedgerTransfer::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    // TODO: links?
    _gltxFrame->cloneData(_curr);
    dataToWidget();
}

bool
LedgerTransfer::fileItem()
{
    if (_curr.number() != "#")
	if (checkGltxUsed(_curr.number(), DataObject::GENERAL, _curr.id()))
	    return false;

    if (_orig.id() == INVALID_ID) {
	if (!_quasar->db()->create(_curr)) return false;
	for (int i = _links.size() - 1; i >= 0; --i) {
	    if (!_quasar->db()->create(_links[i]))
		return false;
	    if (!_quasar->db()->linkTx(_curr.id(), _links[i].id()))
		return false;
	}
    } else {
	if (_links.size() != _origLinks.size()) return false;

	if (!_quasar->db()->update(_orig, _curr)) return false;
	for (unsigned int i = 0; i < _links.size(); ++i)
	    if (!_quasar->db()->update(_origLinks[i], _links[i]))
		return false;
    }

    if (_company.transferAccount() == INVALID_ID) {
	Company orig = _company;
	_company.setTransferAccount(_curr.accounts()[1].account_id);
	_quasar->db()->update(orig, _company);
    }

    _orig = _curr;
    _origLinks = _links;
    _id = _curr.id();

    return true;
}

bool
LedgerTransfer::deleteItem()
{
    if (!_quasar->db()->remove(_orig))
	return false;

    for (unsigned int i = 0; i < _origLinks.size(); ++i) {
	if (!_quasar->db()->remove(_origLinks[i])) {
	    _quasar->db()->create(_orig);
	    for (unsigned int j = 0; j < i; ++j) {
		_quasar->db()->create(_origLinks[j]);
		_quasar->db()->linkTx(_orig.id(), _origLinks[j].id());
		return false;
	    }
	    return false;
	}
    }

    return true;
}

void
LedgerTransfer::restoreItem()
{
    _curr = _orig;
    _links = _origLinks;
}

void
LedgerTransfer::cloneItem()
{
    LedgerTransfer* clone = new LedgerTransfer(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
LedgerTransfer::isChanged()
{
    if (_curr != _orig) return true;
    if (_links.size() != _origLinks.size()) return true;
    for (unsigned int i = 0; i < _links.size(); ++i)
	if (_links[i] != _origLinks[i]) return true;
    return false;
}

// Set the widgets from the data object.
void 
LedgerTransfer::dataToWidget()
{
    _gltxFrame->setData(_curr);
    _inactive->setChecked(!_curr.isActive());
    _table->clear();

    if (_curr.accounts().size() < 2) {
	_account->setId(INVALID_ID);
	_amount->clear();
	_transfer_acct->setId(_company.transferAccount());
    } else {
	_account->setId(_curr.accounts()[0].account_id);
	_amount->setFixed(-_curr.accounts()[0].amount);
	_transfer_acct->setId(_curr.accounts()[1].account_id);

	// Load the transfers
	_table->clear();
	for (unsigned int i = 0; i < _links.size(); ++i) {
	    Id store_id = _links[i].storeId();
	    QString number = _links[i].number();
	    fixed amount = _links[i].accounts()[0].amount;

	    VectorRow* row = new VectorRow(_table->columns());
	    row->setValue(0, store_id);
	    row->setValue(1, number);
	    row->setValue(2, amount);
	    _table->appendRow(row);
	}
    }

    _table->appendRow(new VectorRow(_table->columns()));
    slotAccountChanged();
    recalculate();
}

// Set the data object from the widgets.
void
LedgerTransfer::widgetToData()
{
    _gltxFrame->getData(_curr);
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    Id account_id = _account->getId();
    fixed amount = _amount->getFixed();
    Id transfer_id = _transfer_acct->getId();

    // Clear accounts
    vector<AccountLine>& accounts = _curr.accounts();
    accounts.clear();

    // Posting in main store
    if (_account->getId() != INVALID_ID) {
	accounts.push_back(AccountLine(account_id, -amount));
	accounts.push_back(AccountLine(transfer_id, amount));
    }

    // Post allocations
    _links.clear();
    for (int row = 0; row < _table->rows(); ++row) {
	Id store_id = _table->cellValue(row, 0).toId();
	fixed amount = _table->cellValue(row, 2).toFixed();
	if (store_id == INVALID_ID) continue;

	General link;
	if (row < int(_origLinks.size())) link = _origLinks[row];
	link.setReference(_curr.reference());
	link.setPostDate(_curr.postDate());
	link.setPostTime(_curr.postTime());
	link.setMemo(_curr.memo());
	link.setStationId(_curr.stationId());
	link.setEmployeeId(_curr.employeeId());
	link.setShiftId(_curr.shiftId());
	link.setStoreId(store_id);
	link.setLinkId(_curr.id());
	_quasar->db()->setActive(link, !_inactive->isChecked());

	link.accounts().clear();
	link.accounts().push_back(AccountLine(account_id, amount));
	link.accounts().push_back(AccountLine(transfer_id, -amount));
	_links.push_back(link);
    }
}

void
LedgerTransfer::slotAccountChanged()
{
    Id account_id = _account->getId();
    if (account_id == INVALID_ID) {
	_balance->clear();
	return;
    }

    QDate date = QDate::currentDate();
    Id store_id = _gltxFrame->store->getId();
    fixed balance = _quasar->db()->accountBalance(account_id, date, store_id);

    _balance->setFixed(balance);
}

void
LedgerTransfer::cellChanged(int row, int col, Variant)
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
LedgerTransfer::focusNext(bool& leave, int&, int& newCol,int type)
{
    int row = _table->currentRow();
    int col = _table->currentColumn();

    fixed remain = _difference->getFixed();

    if (type == Table::MoveNext && col == 0) {
	Id id = _table->cellValue(row, col).toId();
	fixed amount = _table->cellValue(row, 1).toFixed();
	if (id == INVALID_ID && row == _table->rows() - 1) {
	    leave = true;
	} else if (amount == 0.0 && remain != 0.0) {
	    _table->setCellValue(row, 2, remain);
	    recalculate();
	}
	newCol = 2;
    } else if (type == Table::MovePrev && col == 0 && row > 0) {
	fixed amount = _table->cellValue(row - 1, 1).toFixed();
	if (amount == 0.0 && remain != 0.0) {
	    _table->setCellValue(row - 1, 1, remain);
	    recalculate();
	}
    } else if (type == Table::MovePrev && col == 2) {
	newCol = 0;
    }
}

void
LedgerTransfer::recalculate()
{
    fixed alloc = 0.0;
    for (int i = 0; i < _table->rows(); ++i)
	alloc += _table->cellValue(i, 2).toFixed();
    _alloc->setFixed(alloc);

    if (alloc == _amount->getFixed())
	_difference->clear();
    else
	_difference->setFixed(_amount->getFixed() - alloc);
}
