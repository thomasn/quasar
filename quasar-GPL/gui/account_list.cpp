// $Id: account_list.cpp,v 1.75 2005/01/30 04:25:31 bpepers Exp $
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

#include "account_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "account_select.h"
#include "account_master.h"
#include "general_master.h"
#include "reconcile_master.h"
#include "grid.h"

#include <assert.h>
#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>
#include <qpopupmenu.h>
#include <qcheckbox.h>
#include <qmessagebox.h>

AccountList::AccountList(MainWindow* main)
    : ActiveList(main, "AccountList")
{
    _helpSource = "account_list.html";

    // Column definitions
    _list->addTextColumn(tr("Account"), 30);
    _list->addTextColumn(tr("Number"), 10);
    _list->addTextColumn(tr("Type"), 24);
    _list->addMoneyColumn(tr("Balance"));
    _list->setSorting(-1);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    _quasar->db()->lookup(_company);

    setCaption(tr("Chart of Accounts"));
    finalize();
}

AccountList::~AccountList()
{
}

void
AccountList::slotActivities()
{
    ActiveList::slotActivities();
    _activities->insertItem(tr("Journal Entry"), this, SLOT(slotJournal()));
    _activities->insertItem(tr("Reconci&le"), this, SLOT(slotReconcile()));
}

bool
AccountList::isActive(Id account_id)
{
    Account account;
    _quasar->db()->lookup(account_id, account);
    return account.isActive();
}

void
AccountList::setActive(Id account_id, bool active)
{
    if (account_id == INVALID_ID) return;

    if (!active) {
	fixed balance = _quasar->db()->accountBalance(account_id);
	if (balance != 0.0) {
	    QString message = tr("This account has a balance and may not be\n"
		"set inactive until it has been adjusted to zero.");
	    QMessageBox::warning(this, tr("Warning"), message,
				 QMessageBox::Ok, 0);
	    return;
	}

	AccountSelect conditions;
	conditions.parent_id = account_id;
	vector<Account> accounts;
	_quasar->db()->select(accounts, conditions);
	for (unsigned int i = 0; i < accounts.size(); ++i) {
	    if (accounts[i].isActive()) {
		QString message = tr("This account is a header with active\n"
		    "subaccounts.  It cannot be set to inactive\n"
		    "until the subaccounts are all inactive.");
		QMessageBox::warning(this, tr("Warning"), message,
				     QMessageBox::Ok, 0);
		return;
	    }
	}
    }

    Account account;
    _quasar->db()->lookup(account_id, account);

    Account orig = account;
    _quasar->db()->setActive(account, active);
    _quasar->db()->update(orig, account);
}

void
AccountList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::ACCOUNT)
	slotRefresh();
}

fixed
AccountList::getBalance(const Account& account)
{
    fixed balance = 0.0;

    // Header balance is sum of child balances
    if (account.isHeader()) {
	for (unsigned int i = 0; i < _accounts.size(); ++i) {
	    if (_accounts[i].parentId() == account.id())
		balance += getBalance(_accounts[i]);
	}
	return balance;
    }

    // Start with this years balance
    for (unsigned int i = 0; i < _account_ids.size(); ++i) {
	if (_account_ids[i] == account.id()) {
	    balance = _balances[i];
	    break;
	}
    }

    return balance;
}

class AccountListItem: public ListViewItem {
public:
    AccountListItem(ListView* parent, ListViewItem* after, Id id);
    AccountListItem(ListViewItem* parent, ListViewItem* after, Id id);
    virtual ~AccountListItem();

    ListViewItem* after;
};

AccountListItem::AccountListItem(ListView* p, ListViewItem* a, Id id)
    : ListViewItem(p, a, id)
{
    after = NULL;
}

AccountListItem::AccountListItem(ListViewItem* p, ListViewItem* a, Id id)
    : ListViewItem(p, a, id)
{
    after = NULL;
}

AccountListItem::~AccountListItem()
{
}

void
AccountList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id account_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    AccountSelect conditions;
    conditions.activeOnly = !showInactive;
    _quasar->db()->selectChart(_accounts, conditions);

    Company company;
    _quasar->db()->lookup(company);

    QDate date = QDate::currentDate();
    _quasar->db()->accountBalances(date, _account_ids, _balances);

    ListViewItem* after = NULL;
    for (unsigned int i = 0; i < _accounts.size(); ++i) {
	const Account& account = _accounts[i];
	fixed balance = getBalance(account);

	AccountListItem* lvi = NULL;
	if (account.parentId() == INVALID_ID) {
	    lvi = new AccountListItem(_list, after, account.id());
	    after = lvi;
	} else {
	    AccountListItem* li = (AccountListItem*)findId(account.parentId());
	    assert(li != NULL);
	    lvi = new AccountListItem(li, li->after, account.id());
	    li->after = lvi;
	}

	lvi->setValue(0, account.name());
	lvi->setValue(1, account.number());
	lvi->setValue(2, account.typeName());
	lvi->setValue(3, balance);
	if (showInactive) lvi->setValue(4, !account.isActive());
	lvi->setOpen(true);
	if (account.id() == account_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
AccountList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Chart of Accounts"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
AccountList::performNew()
{
    AccountMaster* master = new AccountMaster(_main, INVALID_ID);
    master->show();
}

void
AccountList::performEdit()
{
    Id account_id = currentId();
    AccountMaster* master = new AccountMaster(_main, account_id);
    master->show();
}

void
AccountList::slotJournal()
{
    GeneralMaster* master = new GeneralMaster(_main, INVALID_ID);
    master->show();
}

void
AccountList::slotReconcile()
{
    ReconcileMaster* window = new ReconcileMaster(_main);
    // TODO: set bank id?
    window->show();
}
