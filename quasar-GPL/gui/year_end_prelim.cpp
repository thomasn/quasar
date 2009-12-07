// $Id: year_end_prelim.cpp,v 1.7 2005/03/26 01:41:04 bpepers Exp $
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

#include "year_end_prelim.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "date_popup.h"
#include "company.h"
#include "store.h"
#include "account.h"
#include "general.h"
#include "account_select.h"
#include "store_select.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qmessagebox.h>

YearEndPrelim::YearEndPrelim(MainWindow* main)
    : QuasarWindow(main, "YearEndPrelim")
{
    _helpSource = "year_end_prelim.html";

    QFrame* frame = new QFrame(this);

    QString desc = tr("The preliminary year end is the first part of\n"
		      "the year end process.  It transfers your\n"
		      "income statement balances to retained earnings\n"
		      "and allows you time to finalize any adjustments\n"
		      "you wish to do in the previous year before a\n"
		      "final year end is done.");
    QLabel* descLabel = new QLabel(desc, frame);

    QLabel* dateLabel = new QLabel(tr("Start of new year:"), frame);
    _date = new DatePopup(frame);
    _date->setDate(QDate::currentDate());
    dateLabel->setBuddy(_date);

    QFrame* buttons = new QFrame(frame);
    QPushButton* ok = new QPushButton(tr("OK"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);

    connect(ok, SIGNAL(clicked()), SLOT(slotOk()));
    connect(cancel, SIGNAL(clicked()), SLOT(close()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(ok, 0, 1);
    buttonGrid->addWidget(cancel, 0, 2);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setColStretch(1, 1);
    grid->addMultiCellWidget(descLabel, 0, 0, 0, 1);
    grid->addWidget(dateLabel, 1, 0);
    grid->addWidget(_date, 1, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(buttons, 2, 2, 0, 1);

    setCentralWidget(frame);
    setCaption(tr("Preliminary Year End"));
    finalize();
}

YearEndPrelim::~YearEndPrelim()
{
}

void
YearEndPrelim::slotOk()
{
    Company company;
    _db->lookup(company);

    if (!company.lastYearClosed()) {
	QString message = tr("Last year is not closed");
	QApplication::beep();
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QDate date = _date->getDate();
    if (date.isNull()) {
	QString message = tr("Start of year date is manditory");
	QApplication::beep();
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    if (date <= company.startOfYear()) {
	QString message = tr("Start of year date is invalid");
	QApplication::beep();
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Get list of account and stores for transfer processing
    vector<Account> accounts;
    _db->selectChart(accounts, AccountSelect());
    vector<Store> stores;
    _db->select(stores, StoreSelect());

    // Check each store for the need to create a transfer of income
    // statement balance to retained earnings
    for (unsigned int i = 0; i < stores.size(); ++i) {
	Store& store = stores[i];
	vector<Id> transfer_ids;
	vector<fixed> transfer_amts;

	for (unsigned int j = 0; j < accounts.size(); ++j) {
	    const Account& account = accounts[j];
	    if (account.type() < Account::Income) continue;
	    if (account.isHeader()) continue;

	    fixed balance = _db->accountBalance(account.id(), date - 1,
						store.id());
	    if (balance == 0.0) continue;

	    transfer_ids.push_back(account.id());
	    transfer_amts.push_back(balance);
	}

	General transfer;
	if (transfer_ids.size() > 0) {
	    transfer.setNumber("#");
	    transfer.setPostDate(date);
	    transfer.setPostTime(QTime(0, 0, 0));
	    transfer.setMemo("Year End Transfer");
	    transfer.setStoreId(store.id());
	    transfer.setStationId(_quasar->defaultStation());
	    transfer.setEmployeeId(_quasar->defaultEmployee());

	    fixed total = 0.0;
	    for (unsigned int j = 0; j < transfer_ids.size(); ++j) {
		AccountLine line;
		line.account_id = transfer_ids[j];
		line.amount = -transfer_amts[j];
		transfer.accounts().push_back(line);
		total += line.amount;
	    }

	    AccountLine line;
	    line.account_id = company.retainedEarnings();
	    line.amount = -total;
	    transfer.accounts().push_back(line);

	    if (!_db->create(transfer)) {
		QApplication::restoreOverrideCursor();
		QString message = tr("Failed creating transfer");
		QApplication::beep();
		QMessageBox::critical(this, tr("Error"), message);
		return;
	    }
	}

	Store orig = store;
	store.setYearEndTransferId(transfer.id());
	if (!_db->update(orig, store)) {
	    QApplication::restoreOverrideCursor();
	    QString message = tr("Failed setting transfer id");
	    QApplication::beep();
	    QMessageBox::critical(this, tr("Error"), message);
	    return;
	}
    }

    Company orig = company;
    company.setLastYearClosed(false);
    company.setStartOfYear(_date->getDate());
    if (!_db->update(orig, company)) {
	QApplication::restoreOverrideCursor();
	QString message = tr("Failed updating company");
	QApplication::beep();
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QApplication::restoreOverrideCursor();
    QString message = tr("Preliminary year end finished");
    QMessageBox::information(this, tr("Finished"), message);

    close();
}
