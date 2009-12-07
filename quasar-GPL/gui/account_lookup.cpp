// $Id: account_lookup.cpp,v 1.26 2004/02/11 00:19:09 bpepers Exp $
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

#include "account_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "account_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "account.h"
#include "account_master.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <qlayout.h>

AccountLookup::AccountLookup(MainWindow* main, QWidget* parent, int matchType,
			     bool header, bool posting)
    : QuasarLookup(main, parent, "AccountLookup"), activeOnly(true),
      includeHeader(header), includePosting(posting)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    _list->addTextColumn(tr("Number"), 12);
    _list->addTextColumn(tr("Type"), 24);
    setCaption(tr("Account Lookup"));

    QLabel* typeLabel = new QLabel(tr("Type:"), _search);
    type = new ComboBox(_search);
    typeLabel->setBuddy(type);

    setTabOrder(_text, type);
    _searchGrid->addWidget(typeLabel, 1, 0);
    _searchGrid->addWidget(type, 1, 1, AlignLeft | AlignVCenter);

    type->insertItem(tr("All Types"));
    type->insertItem(Account::typeName(Account::Bank));
    type->insertItem(Account::typeName(Account::AR));
    type->insertItem(Account::typeName(Account::Inventory));
    type->insertItem(Account::typeName(Account::OtherCurAsset));
    type->insertItem(Account::typeName(Account::FixedAsset));
    type->insertItem(Account::typeName(Account::OtherAsset));
    type->insertItem(Account::typeName(Account::AP));
    type->insertItem(Account::typeName(Account::CreditCard));
    type->insertItem(Account::typeName(Account::OtherCurLiability));
    type->insertItem(Account::typeName(Account::LongTermLiability));
    type->insertItem(Account::typeName(Account::Equity));
    type->insertItem(Account::typeName(Account::Income));
    type->insertItem(Account::typeName(Account::COGS));
    type->insertItem(Account::typeName(Account::Expense));
    type->insertItem(Account::typeName(Account::OtherIncome));
    type->insertItem(Account::typeName(Account::OtherExpense));

    if (matchType != -1)
	type->setCurrentItem(Account::typeName(Account::Type(matchType)));
}

AccountLookup::AccountLookup(QuasarClient* quasar, QWidget* parent,
			     int matchType, bool header, bool posting)
    : QuasarLookup(quasar, parent, "AccountLookup"), activeOnly(true),
      includeHeader(header), includePosting(posting)
{
    _label->setText(tr("Name:"));
    _text->setLength(20);
    _list->addTextColumn(tr("Name"), 20);
    _list->addTextColumn(tr("Number"), 12);
    _list->addTextColumn(tr("Type"), 24);
    setCaption(tr("Account Lookup"));

    QLabel* typeLabel = new QLabel(tr("Type:"), _search);
    type = new ComboBox(_search);
    typeLabel->setBuddy(type);

    setTabOrder(_text, type);
    _searchGrid->addWidget(typeLabel, 1, 0);
    _searchGrid->addWidget(type, 1, 1, AlignLeft | AlignVCenter);

    type->insertItem(tr("All Types"));
    type->insertItem(Account::typeName(Account::Bank));
    type->insertItem(Account::typeName(Account::AR));
    type->insertItem(Account::typeName(Account::Inventory));
    type->insertItem(Account::typeName(Account::OtherCurAsset));
    type->insertItem(Account::typeName(Account::FixedAsset));
    type->insertItem(Account::typeName(Account::OtherAsset));
    type->insertItem(Account::typeName(Account::AP));
    type->insertItem(Account::typeName(Account::CreditCard));
    type->insertItem(Account::typeName(Account::OtherCurLiability));
    type->insertItem(Account::typeName(Account::LongTermLiability));
    type->insertItem(Account::typeName(Account::Equity));
    type->insertItem(Account::typeName(Account::Income));
    type->insertItem(Account::typeName(Account::COGS));
    type->insertItem(Account::typeName(Account::Expense));
    type->insertItem(Account::typeName(Account::OtherIncome));
    type->insertItem(Account::typeName(Account::OtherExpense));

    if (matchType != -1)
	type->setCurrentItem(Account::typeName(Account::Type(matchType)));
}

AccountLookup::~AccountLookup()
{
}

QString
AccountLookup::lookupById(Id account_id)
{
    Account account;
    if (account_id != INVALID_ID && _quasar->db()->lookup(account_id, account))
	return account.name();
    return "";
}

vector<DataPair>
AccountLookup::lookupByText(const QString& text)
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    AccountSelect conditions;
    vector<Account> accounts;

    if (!text.isEmpty()) {
	conditions.number = text;
	conditions.type = matchType();
	conditions.headerOnly = (includeHeader && !includePosting);
	conditions.postingOnly = (includePosting && !includeHeader);
	conditions.activeOnly = activeOnly;
	_quasar->db()->select(accounts, conditions);
    }

    if (accounts.size() == 0 && !text.isEmpty()) {
	conditions.number = "";
	conditions.name = text + "%";
	_quasar->db()->select(accounts, conditions);
    }

    vector<DataPair> data;
    for (unsigned int i = 0; i < accounts.size(); ++i) {
	data.push_back(DataPair(accounts[i].id(), accounts[i].name()));
    }

    QApplication::restoreOverrideCursor();
    return data;
}

void
AccountLookup::refresh()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    _list->clear();
    AccountSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.type = matchType();
    conditions.headerOnly = (includeHeader && !includePosting);
    conditions.postingOnly = (includePosting && !includeHeader);
    conditions.activeOnly = activeOnly;

    int count;
    _quasar->db()->count(count, conditions);
    if (count > 100) {
	QApplication::restoreOverrideCursor();

	QWidget* parent = this;
	if (isHidden() && parentWidget() != NULL)
	    parent = parentWidget();

	QString message = tr("This will select %1 items\n"
			     "which may be slow.  Continue?").arg(count);
	int ch = QMessageBox::warning(parent, tr("Are You Sure?"), message,
				      QMessageBox::Yes, QMessageBox::No);
	if (ch != QMessageBox::Yes) return;

	QApplication::setOverrideCursor(waitCursor);
	qApp->processEvents();
    }

    vector<Account> accounts;
    _quasar->db()->select(accounts, conditions);

    for (unsigned int i = 0; i < accounts.size(); ++i) {
	Account& account = accounts[i];
	if (account.id() == skipId) continue;

	ListViewItem* item = new ListViewItem(_list, account.id());
	item->setValue(0, account.name());
	item->setValue(1, account.number());
	item->setValue(2, account.typeName());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
    _list->triggerUpdate();

    QApplication::restoreOverrideCursor();
}

QWidget*
AccountLookup::slotNew()
{
    AccountMaster* window = new AccountMaster(_main);
    if (matchType() != -1) window->setType(matchType());
    window->show();
    reject();
    return window;
}

QWidget*
AccountLookup::slotEdit(Id id)
{
    AccountMaster* window = new AccountMaster(_main, id);
    window->show();
    reject();
    return window;
}

int
AccountLookup::matchType()
{
    if (type->currentText() == tr("All Types"))
	return -1;
    return Account::type(type->currentText());
}
