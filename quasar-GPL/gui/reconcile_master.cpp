// $Id: reconcile_master.cpp,v 1.18 2005/01/30 04:25:31 bpepers Exp $
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

#include "reconcile_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "reconcile_select.h"
#include "lookup_edit.h"
#include "date_popup.h"
#include "money_edit.h"
#include "gltx.h"
#include "card.h"
#include "account.h"
#include "account_lookup.h"
#include "list_view.h"
#include "list_view_item.h"
#include "grid.h"
#include "text_frame.h"

#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <algorithm>

bool operator<(const ReconcileLine& lhs, const ReconcileLine& rhs) {
    if (lhs.gltx_id == rhs.gltx_id)
	return lhs.seq_num < rhs.seq_num;
    return lhs.gltx_id < rhs.gltx_id;
}

ReconcileMaster::ReconcileMaster(MainWindow* main, Id reconcile_id)
    : DataWindow(main, "ReconcileMaster", reconcile_id)
{
    _helpSource = "reconcile_master.html";

    // Print button
    _print = new QPushButton(tr("Print"), _buttons);
    connect(_print, SIGNAL(clicked()), SLOT(slotPrint()));

    QFrame* top = new QFrame(_frame);
    QFrame* deps = new QFrame(_frame);
    QFrame* chqs = new QFrame(_frame);
    QFrame* bot = new QFrame(_frame);

    QLabel* accountLabel = new QLabel(tr("Account:"), top);
    AccountLookup* acctLook = new AccountLookup(main, this, Account::Bank);
    _account = new LookupEdit(acctLook, top);
    _account->setLength(30);
    accountLabel->setBuddy(_account);
    connect(_account, SIGNAL(validData()), SLOT(slotAccountChanged()));

    QLabel* stmtDateLabel = new QLabel(tr("Statement Date:"), top);
    _stmt_date = new DatePopup(top);
    stmtDateLabel->setBuddy(_stmt_date);
    connect(_stmt_date, SIGNAL(validData()), SLOT(slotStmtDateChanged()));

    QLabel* endDateLabel = new QLabel(tr("Ending Date:"), top);
    _end_date = new DatePopup(top);
    _end_date->setFocusPolicy(ClickFocus);
    endDateLabel->setBuddy(_end_date);
    connect(_end_date, SIGNAL(validData()), SLOT(slotEndDateChanged()));

    QLabel* openBalanceLabel = new QLabel(tr("Opening Balance:"), top);
    _open_balance = new MoneyEdit(top);
    _open_balance->setFocusPolicy(NoFocus);
    openBalanceLabel->setBuddy(_open_balance);

    QLabel* stmtBalanceLabel = new QLabel(tr("Statement Balance:"), top);
    _stmt_balance = new MoneyEdit(top);
    stmtBalanceLabel->setBuddy(_stmt_balance);
    connect(_stmt_balance, SIGNAL(validData()), SLOT(recalculate()));

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setMargin(3);
    topGrid->setSpacing(3);
    topGrid->setColStretch(2, 1);
    topGrid->addWidget(accountLabel, 0, 0);
    topGrid->addMultiCellWidget(_account, 0, 0, 1, 4, AlignLeft|AlignVCenter);
    topGrid->addWidget(stmtDateLabel, 1, 0);
    topGrid->addWidget(_stmt_date, 1, 1);
    topGrid->addWidget(endDateLabel, 2, 0);
    topGrid->addWidget(_end_date, 2, 1);
    topGrid->addWidget(openBalanceLabel, 1, 3);
    topGrid->addWidget(_open_balance, 1, 4);
    topGrid->addWidget(stmtBalanceLabel, 2, 3);
    topGrid->addWidget(_stmt_balance, 2, 4);

    QLabel* depositLabel = new QLabel(tr("Deposit and Other Credits:"), deps);
    _deposits = new ListView(deps);
    _deposits->setSelectionMode(QListView::Multi);
    _deposits->setAllColumnsShowFocus(true);
    _deposits->setShowSortIndicator(true);
    _deposits->addDateColumn(tr("Date"));
    _deposits->addTextColumn(tr("Number"), 10, AlignRight);
    _deposits->addTextColumn(tr("Payee"), 20);
    _deposits->addTextColumn(tr("Memo"), 20);
    _deposits->addMoneyColumn(tr("Amount"));
    _deposits->setMaximumHeight(100);
    connect(_deposits, SIGNAL(selectionChanged()), SLOT(recalculate()));

    QGridLayout* depsGrid = new QGridLayout(deps);
    depsGrid->setMargin(3);
    depsGrid->setSpacing(3);
    depsGrid->addWidget(depositLabel, 0, 0, AlignLeft | AlignVCenter);
    depsGrid->addWidget(_deposits, 1, 0);

    QLabel* chequeLabel = new QLabel(tr("Cheques and Payments:"), chqs);
    _cheques = new ListView(chqs);
    _cheques->setSelectionMode(QListView::Multi);
    _cheques->setAllColumnsShowFocus(true);
    _cheques->setShowSortIndicator(true);
    _cheques->addDateColumn(tr("Date"));
    _cheques->addTextColumn(tr("Number"), 10, AlignRight);
    _cheques->addTextColumn(tr("Payee"), 20);
    _cheques->addTextColumn(tr("Memo"), 20);
    _cheques->addMoneyColumn(tr("Amount"));
    _cheques->setMaximumHeight(100);
    connect(_cheques, SIGNAL(selectionChanged()), SLOT(recalculate()));

    QGridLayout* chqsGrid = new QGridLayout(chqs);
    chqsGrid->setMargin(3);
    chqsGrid->setSpacing(3);
    chqsGrid->addWidget(chequeLabel, 0, 0, AlignLeft | AlignVCenter);
    chqsGrid->addWidget(_cheques, 1, 0);

    int countWidth = fontMetrics().width("9999");
    int moneyWidth = fontMetrics().width("$999,999,999.99");

    QLabel* totalLabel = new QLabel(tr("Items marked cleared:"), bot);
    QLabel* depositTotalLabel = new QLabel(tr(" Deposits and Other Credits"),
					   bot);
    QLabel* chequeTotalLabel = new QLabel(tr(" Cheques and Payments"), bot);
    _deposit_cnt = new QLabel("0", bot);
    _deposit_cnt->setMinimumWidth(countWidth);
    _deposit_cnt->setAlignment(AlignRight);
    _deposit_total = new QLabel("$0.00", bot);
    _deposit_total->setMinimumWidth(moneyWidth);
    _deposit_total->setAlignment(AlignRight);
    _cheque_cnt = new QLabel("0", bot);
    _cheque_cnt->setMinimumWidth(countWidth);
    _cheque_cnt->setAlignment(AlignRight);
    _cheque_total = new QLabel("$0.00", bot);
    _cheque_total->setMinimumWidth(moneyWidth);
    _cheque_total->setAlignment(AlignRight);

    QLabel* stmtTotalLabel = new QLabel(tr("Statement Balance"), bot);
    QLabel* clearedTotalLabel = new QLabel(tr("Cleared Balance"), bot);
    QLabel* differenceLabel = new QLabel(tr("Difference"), bot);
    _stmt_total = new QLabel("$0.00", bot);
    _stmt_total->setMinimumWidth(moneyWidth);
    _stmt_total->setAlignment(AlignRight);
    _cleared_total = new QLabel("$0.00", bot);
    _cleared_total->setMinimumWidth(moneyWidth);
    _cleared_total->setAlignment(AlignRight);
    _difference = new QLabel("$0.00", bot);
    _difference->setMinimumWidth(moneyWidth);
    _difference->setAlignment(AlignRight);

    QGridLayout* botGrid = new QGridLayout(bot);
    botGrid->setMargin(3);
    botGrid->setSpacing(3);
    botGrid->setColStretch(3, 1);
    botGrid->addMultiCellWidget(totalLabel, 0, 0, 0, 2,AlignLeft|AlignVCenter);
    botGrid->addWidget(_deposit_cnt, 1, 0, AlignRight|AlignVCenter);
    botGrid->addWidget(depositTotalLabel, 1, 1);
    botGrid->addWidget(_deposit_total, 1, 2, AlignRight|AlignVCenter);
    botGrid->addWidget(_cheque_cnt, 2, 0, AlignRight|AlignVCenter);
    botGrid->addWidget(chequeTotalLabel, 2, 1);
    botGrid->addWidget(_cheque_total, 2, 2, AlignRight|AlignVCenter);
    botGrid->addWidget(stmtTotalLabel, 0, 4);
    botGrid->addWidget(_stmt_total, 0, 5);
    botGrid->addWidget(clearedTotalLabel, 1, 4);
    botGrid->addWidget(_cleared_total, 1, 5);
    botGrid->addWidget(differenceLabel, 2, 4);
    botGrid->addWidget(_difference, 2, 5);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(8);
    grid->setMargin(3);
    grid->addWidget(top, 0, 0);
    grid->addWidget(deps, 1, 0);
    grid->addWidget(chqs, 2, 0);
    grid->addWidget(bot, 3, 0);

    _inactive->setText(tr("Reconciled?"));
    connect(_inactive, SIGNAL(clicked()), SLOT(slotReconciledChanged()));

    setCaption(tr("Account Reconcile"));
    finalize();
}

ReconcileMaster::~ReconcileMaster()
{
}

void
ReconcileMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);
    std::sort(_orig.lines().begin(), _orig.lines().end());

    _curr = _orig;
    _firstField = _stmt_date;
}

void
ReconcileMaster::newItem()
{
    Reconcile blank;
    _orig = blank;
    _orig.setEndDate(QDate::currentDate());

    _curr = _orig;
    _firstField = _account;
}

void
ReconcileMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _curr.setReconciled(false);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
}

bool
ReconcileMaster::fileItem()
{
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
ReconcileMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
ReconcileMaster::restoreItem()
{
    _curr = _orig;
}

void
ReconcileMaster::cloneItem()
{
    ReconcileMaster* clone = new ReconcileMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
ReconcileMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
ReconcileMaster::dataToWidget()
{
    _account->setId(_curr.accountId());
    _stmt_date->setDate(_curr.statementDate());
    _end_date->setDate(_curr.endDate());
    _stmt_balance->setFixed(_curr.statementBalance());
    _inactive->setChecked(!_curr.isActive());

    slotAccountChanged();

    for (unsigned int i = 0; i < _curr.lines().size(); ++i) {
	Id gltx_id = _curr.lines()[i].gltx_id;
	int seq_num = _curr.lines()[i].seq_num;

	Gltx gltx;
	_quasar->db()->lookup(gltx_id, gltx);

	ListView* list = _deposits;
	if (gltx.accounts()[seq_num].amount < 0.0) list = _cheques;

	QListViewItemIterator it(list);
	for (; it.current(); ++it) {
	    ListViewItem* item = (ListViewItem*)it.current();

	    if (gltx_id != item->id) continue;
	    if (seq_num != item->extra[0].toInt()) continue;

	    list->setSelected(item, true);
	}
    }
}

// Set the data object from the widgets.
void
ReconcileMaster::widgetToData()
{
    _curr.setAccountId(_account->getId());
    _curr.setStatementDate(_stmt_date->getDate());
    _curr.setEndDate(_end_date->getDate());
    _curr.setStatementBalance(_stmt_balance->getFixed());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    _curr.lines().clear();
    QListViewItemIterator it1(_deposits);
    for (; it1.current(); ++it1) {
	ListViewItem* item = (ListViewItem*)it1.current();
	if (item->isSelected()) {
	    int seq_num = item->extra[0].toInt();
	    _curr.lines().push_back(ReconcileLine(item->id, seq_num));
	}
    }
    QListViewItemIterator it2(_cheques);
    for (; it2.current(); ++it2) {
	ListViewItem* item = (ListViewItem*)it2.current();
	if (item->isSelected()) {
	    int seq_num = item->extra[0].toInt();
	    _curr.lines().push_back(ReconcileLine(item->id, seq_num));
	}
    }

    std::sort(_curr.lines().begin(), _curr.lines().end());

    if (difference == 0.0 && _curr.isActive() && _curr.lines().size() > 0) {
	QString message = tr("This account appears to be fully reconciled.\n"
	    "Do you wish to mark it as reconciled and\n"
	    "optionally print out a report?");
	int ch = QMessageBox::information(this, tr("Reconciled?"), message,
				tr("Reconcile Now"), tr("Reconcile Later"));
	if (ch == 0) {
	    _quasar->db()->setActive(_curr, false);
	}
    }
}

bool
ReconcileMaster::dataValidate()
{
    if (difference != 0.0 && _inactive->isChecked())
	return error(tr("Can't be reconciled if difference isn't zero"));
    return true;
}

void
ReconcileMaster::printItem(bool)
{
    if (_curr.isActive()) return;

    QString message = tr("Choose the level of detail you wish\n"
	"to see in the printout.");
    int ch = QMessageBox::information(this, tr("Select Report Type"), message,
				      tr("None"), tr("Summary"));
    if (ch != 1) return;

    Grid* grid = new Grid(3);
    grid->setColumnWeight(0, 10);
    grid->setColumnWeight(1, 1);
    grid->setColumnWeight(2, 1);
    grid->setColumnSticky(0, Grid::Left);
    grid->setColumnSticky(1, Grid::Right);
    grid->setColumnSticky(2, Grid::Right);

    Id account_id = _account->getId();
    QDate stmt_date = _stmt_date->getDate();
    QDate end_date = _end_date->getDate();

    Account account;
    _quasar->db()->lookup(account_id, account);
    fixed balance = _quasar->db()->accountBalance(account_id, stmt_date);

    // Add header
    Grid* header = new Grid(1, grid);
    header->setColumnWeight(0, 1);
    TextFrame* text = new TextFrame(tr("Reconciliation Report"), header);
    text->setFont(Font("Times", 24));
    header->set(USE_NEXT, 0, text);
    text = new TextFrame(account.name(), header);
    text->setFont(Font("Times", 18));
    header->set(USE_NEXT, 0, text);
    text = new TextFrame(tr("Statement Ending ") + _stmt_date->text(), header);
    text->setFont(Font("Times", 18));
    header->set(USE_NEXT, 0, text);
    grid->set(0, 0, 1, 3, header, Grid::AllSides);

    grid->set(USE_NEXT, 0, "");
    grid->set(USE_NEXT, 0, tr("Cleared Transactions"));
    grid->set(USE_NEXT, 0, tr("    Opening Balance"));
    grid->set(USE_CURR, 2, _open_balance->text());
    grid->set(USE_NEXT, 0, tr("        Cleared Cheques and Payments"));
    grid->set(USE_CURR, 1, _cheque_cnt->text() + tr(" Items"));
    grid->set(USE_CURR, 2, _cheque_total->text());
    grid->set(USE_NEXT, 0, tr("        Cleared Deposits and Other Credits"));
    grid->set(USE_CURR, 1, _deposit_cnt->text() + tr(" Items"));
    grid->set(USE_CURR, 2, _deposit_total->text());
    grid->set(USE_NEXT, 0, tr("    Cleared Balance"));
    grid->set(USE_CURR, 2, _cleared_total->text());

    int unclearChequeCnt = 0;
    int newChequeCnt = 0;
    fixed unclearChequeTotal = 0.0;
    fixed newChequeTotal = 0.0;

    QListViewItemIterator it1(_cheques);
    for (; it1.current(); ++it1) {
	ListViewItem* item = (ListViewItem*)it1.current();
	if (!item->isSelected()) {
	    QDate date = item->value(0).toDate();
	    if (date > stmt_date) {
		newChequeCnt += 1;
		newChequeTotal += item->value(4).toFixed();
	    } else {
		unclearChequeCnt += 1;
		unclearChequeTotal += item->value(4).toFixed();
	    }
	}
    }

    int unclearDepositCnt = 0;
    int newDepositCnt = 0;
    fixed unclearDepositTotal = 0.0;
    fixed newDepositTotal = 0.0;

    QListViewItemIterator it2(_deposits);
    for (; it2.current(); ++it2) {
	ListViewItem* item = (ListViewItem*)it2.current();
	if (!item->isSelected()) {
	    QDate date = item->value(0).toDate();
	    if (date > stmt_date) {
		newDepositCnt += 1;
		newDepositTotal += item->value(4).toFixed();
	    } else {
		unclearDepositCnt += 1;
		unclearDepositTotal += item->value(4).toFixed();
	    }
	}
    }

    grid->set(USE_NEXT, 0, "");
    grid->set(USE_NEXT, 0, tr("Uncleared Transactions"));
    grid->set(USE_NEXT, 0, tr("        Uncleared Cheques and Payments"));
    grid->set(USE_CURR, 1, QString::number(unclearChequeCnt) + tr(" Items"));
    grid->set(USE_CURR, 2, MoneyValcon().format(unclearChequeTotal));
    grid->set(USE_NEXT, 0, tr("        Uncleared Deposits and Other Credits"));
    grid->set(USE_CURR, 1, QString::number(unclearDepositCnt) + tr(" Items"));
    grid->set(USE_CURR, 2, MoneyValcon().format(unclearDepositTotal));

    grid->set(USE_NEXT, 0, "");
    grid->set(USE_NEXT, 0, tr("New Transactions"));
    grid->set(USE_NEXT, 0, tr("    Balance as of ") + _stmt_date->text());
    grid->set(USE_CURR, 2, MoneyValcon().format(balance));
    grid->set(USE_NEXT, 0, tr("        New Cheques and Payments"));
    grid->set(USE_CURR, 1, QString::number(newChequeCnt) + tr(" Items"));
    grid->set(USE_CURR, 2, MoneyValcon().format(newChequeTotal));
    grid->set(USE_NEXT, 0, tr("        New Deposits and Other Credits"));
    grid->set(USE_CURR, 1, QString::number(newDepositCnt) + tr(" Items"));
    grid->set(USE_CURR, 2, MoneyValcon().format(newDepositTotal));
    grid->set(USE_NEXT, 0, tr("    Ending account balance"));
    grid->set(USE_CURR, 2, MoneyValcon().format(balance + newDepositTotal -
						newChequeTotal));

    if (ch == 2) {
	// TODO: add details
    }

    grid->print(this);
    delete grid;
}

void
ReconcileMaster::slotAccountChanged()
{
    Id account_id = _account->getId();
    if (account_id == INVALID_ID) {
	_stmt_balance->setFixed(0.0);
	_stmt_date->setText("");
	_open_balance->setFixed(0.0);
	_deposits->clear();
	_cheques->clear();
	recalculate();
	return;
    }

    loadData();
    recalculate();
}

void
ReconcileMaster::slotStmtDateChanged()
{
    loadData();
    recalculate();
}

void
ReconcileMaster::slotEndDateChanged()
{
    loadData();
    recalculate();
}

void
ReconcileMaster::recalculate()
{
    fixed openBalance = _open_balance->getFixed();
    fixed stmtBalance = _stmt_balance->getFixed();

    depositCnt = 0;
    depositTotal = 0.0;
    QListViewItemIterator it1(_deposits);
    for (; it1.current(); ++it1) {
	ListViewItem* item = (ListViewItem*)it1.current();
	if (item->isSelected()) {
	    depositCnt += 1;
	    depositTotal += item->value(4).toFixed();
	}
    }

    chequeCnt = 0;
    chequeTotal = 0.0;
    QListViewItemIterator it2(_cheques);
    for (; it2.current(); ++it2) {
	ListViewItem* item = (ListViewItem*)it2.current();
	if (item->isSelected()) {
	    chequeCnt += 1;
	    chequeTotal += item->value(4).toFixed();
	}
    }

    clearedTotal = openBalance + depositTotal - chequeTotal;
    difference = stmtBalance - clearedTotal;

    _deposit_cnt->setText(QString::number(depositCnt));
    _deposit_total->setText(MoneyValcon().format(depositTotal));
    _cheque_cnt->setText(QString::number(chequeCnt));
    _cheque_total->setText(MoneyValcon().format(chequeTotal));
    _stmt_total->setText(MoneyValcon().format(stmtBalance));
    _cleared_total->setText(MoneyValcon().format(clearedTotal));
    _difference->setText(MoneyValcon().format(difference));

    _print->setEnabled(difference == 0.0);
    _inactive->setEnabled(difference == 0.0 || _inactive->isChecked());
}

void
ReconcileMaster::slotReconciledChanged()
{
    _inactive->setEnabled(difference == 0.0 || _inactive->isChecked());
}

void
ReconcileMaster::loadData()
{
    // Save current selections
    vector<Id> old_ids;
    vector<int> old_nums;
    QListViewItemIterator it1(_deposits);
    for (; it1.current(); ++it1) {
	ListViewItem* item = (ListViewItem*)it1.current();
	if (item->isSelected()) {
	    old_ids.push_back(item->id);
	    old_nums.push_back(item->extra[0].toInt());
	}
    }
    QListViewItemIterator it2(_cheques);
    for (; it2.current(); ++it2) {
	ListViewItem* item = (ListViewItem*)it2.current();
	if (item->isSelected()) {
	    old_ids.push_back(item->id);
	    old_nums.push_back(item->extra[0].toInt());
	}
    }

    _deposits->clear();
    _cheques->clear();

    Id account_id = _account->getId();
    QDate stmt_date = _stmt_date->getDate();
    QDate end_date = _end_date->getDate();

    if (account_id == INVALID_ID) return;
    if (stmt_date.isNull()) return;
    if (end_date.isNull()) return;

    fixed open_balance;
    vector<Id> gltx_ids;
    vector<int> seq_nums;
    _quasar->db()->selectReconcile(account_id, stmt_date, end_date,
				   open_balance, gltx_ids, seq_nums);

    _open_balance->setFixed(open_balance);
    ListViewItem* after1 = NULL;
    ListViewItem* after2 = NULL;
    for (unsigned int i = 0; i < gltx_ids.size(); ++i) {
	Id gltx_id = gltx_ids[i];
	int seq_num = seq_nums[i];

	Gltx gltx;
	_quasar->db()->lookup(gltx_id, gltx);
	if (!gltx.isActive()) continue;

	Card card;
	_quasar->db()->lookup(gltx.cardId(), card);

	fixed amount = gltx.accounts()[seq_num].amount;
	ListViewItem* item;
	if (amount < 0.0) {
	    item = new ListViewItem(_cheques, after1, gltx_id);
	    item->setValue(4, -amount);
	    after1 = item;
	} else {
	    item = new ListViewItem(_deposits, after2, gltx_id);
	    item->setValue(4, amount);
	    after2 = item;
	}
	item->extra.push_back(seq_num);
	item->setValue(0, gltx.postDate());
	item->setValue(1, gltx.number());
	item->setValue(2, card.nameFL());
	item->setValue(3, gltx.memo());

	for (unsigned int j = 0; j < old_ids.size(); ++j) {
	    if (gltx_id != old_ids[j]) continue;
	    if (seq_num != old_nums[j]) continue;
	    item->listView()->setSelected(item, true);
	}
    }
}
