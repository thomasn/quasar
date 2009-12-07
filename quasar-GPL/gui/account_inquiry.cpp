// $Id: account_inquiry.cpp,v 1.30 2005/01/30 04:25:31 bpepers Exp $
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

#include "account_inquiry.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "account.h"
#include "gltx.h"
#include "store.h"
#include "company.h"
#include "gltx_select.h"
#include "lookup_edit.h"
#include "account_lookup.h"
#include "list_view.h"
#include "list_view_item.h"
#include "date_valcon.h"
#include "money_valcon.h"
#include "grid.h"
#include "text_frame.h"
#include "line_frame.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qprogressdialog.h>

AccountInquiry::AccountInquiry(MainWindow* main, QWidget* parent)
    : InquiryWindow(main, parent, "AccountInquiry", new AccountLookup(main, parent))
{
    search->setLength(30);
}

AccountInquiry::~AccountInquiry()
{
}

void
AccountInquiry::slotRefresh(QDate from, QDate to, Id store_id)
{
    if (loading) return;
    loading = true;
    needsRefresh = false;
    delete _grid;
    _grid = NULL;

    // Clear data
    list->clear();
    for (int label = 0; label < 10; ++label)
	labels[label]->setText(" ");

    // Get account to show and return if none
    Id account_id = search->getId();
    if (account_id == INVALID_ID) {
	loading = false;
	return;
    }

    setEnabled(false);
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Get account information
    Account account;
    quasar->db()->lookup(account_id, account);

    fixed beginBalance = 0.0;
    fixed totalDebits = 0.0;
    fixed totalCredits = 0.0;

    labels[0]->setText(tr("Beginning Balance"));
    labels[1]->setText(tr("Total Debits"));
    labels[2]->setText(tr("Total Credits"));
    labels[3]->setText(tr("Net Change"));
    labels[4]->setText(tr("Ending Balance"));

    // Get beginning balance
    if (!from.isNull()) {
	beginBalance = quasar->db()->accountBalance(account_id, from - 1,
						    store_id);
    }

    // Select transactions
    vector<Gltx> gltxs;
    vector<fixed> amounts;
    quasar->db()->selectAccount(account_id, store_id, from, to, gltxs,
				amounts);

    // Setup grid
    _grid = new Grid(8);
    Grid* header = new Grid(1, _grid);
    TextFrame* text;
    text = new TextFrame(tr("Account Inquiry"), header);
    text->setFont(Font("Times", 24));
    header->set(USE_NEXT, 0, text);
    text = new TextFrame(account.name(), header);
    text->setFont(Font("Times", 18));
    header->set(USE_NEXT, 0, text);
    text = new TextFrame(DateValcon().format(from) + tr(" to ") +
			 DateValcon().format(to), header);
    text->setFont(Font("Times", 18));
    header->set(USE_NEXT, 0, text);
    header->setColumnWeight(0, 1);

    _grid->set(0, 0, 1, _grid->columns(), header, Grid::AllSides);
    _grid->set(USE_NEXT, 0, "");
    for (int column = 0; column < 8; ++column) {
	_grid->setColumnSticky(column, column < 4 ? Grid::Left : Grid::Right);
	_grid->setColumnPad(column, 5);
    }
    _grid->set(2, 0, tr("Type"));
    _grid->set(3, 0, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 1, tr("Id #"));
    _grid->set(3, 1, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 2, tr("Store"));
    _grid->set(3, 2, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 3, tr("Date"));
    _grid->set(3, 3, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 4, tr("Memo"));
    _grid->set(3, 4, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 5, tr("Debit"));
    _grid->set(3, 5, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 6, tr("Credit"));
    _grid->set(3, 6, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 7, tr("Voided?"));
    _grid->set(3, 7, new LineFrame(_grid), Grid::LeftRight);
    _grid->setHeaderRows(_grid->rows());

    // Setup progress dialog
    QProgressDialog* progress = new QProgressDialog(tr("Loading transactions..."),
						    tr("Cancel"), gltxs.size(),
						    this, "Progress", true);
    progress->setMinimumDuration(1000);
    progress->setCaption(tr("Progress"));

    // Process each transaction
    for (unsigned int i = 0; i < gltxs.size(); ++i) {
	const Gltx& gltx = gltxs[i];
	fixed amount = amounts[i];

	if (i % 50 == 0) {
	    progress->setProgress(i);
	    qApp->processEvents();
	    if (progress->wasCancelled()) {
		// TODO: add cancelled line
		break;
	    }
	}

	Store store;
	quasar->db()->lookup(gltx.storeId(), store);

	ListViewItem* item = new ListViewItem(list, gltx.id());
	item->setValue(0, gltx.dataTypeName());
	item->setValue(1, gltx.number());
	item->setValue(2, store.number());
	item->setValue(3, gltx.postDate());
	item->setValue(4, gltx.memo());
	item->setValue(7, !gltx.isActive());

	_grid->set(USE_NEXT, 0, gltx.dataTypeName());
	_grid->set(USE_CURR, 1, gltx.number());
	_grid->set(USE_CURR, 2, store.number());
	_grid->set(USE_CURR, 3, DateValcon().format(gltx.postDate()));
	_grid->set(USE_CURR, 4, gltx.memo());
	_grid->set(USE_CURR, 7, gltx.isActive() ? " " : "x");

	if (amount >= 0.0) {
	    item->setValue(5, amount);
	    _grid->set(USE_CURR, 5, MoneyValcon().format(amount));
	    if (gltx.isActive()) totalDebits += amount;
	} else {
	    item->setValue(6, -amount);
	    _grid->set(USE_CURR, 6, MoneyValcon().format(-amount));
	    if (gltx.isActive()) totalCredits += -amount;
	}
    }
    progress->setProgress(gltxs.size());
    delete progress;

    // Set bottom totals
    fixed netChange = totalDebits - totalCredits;
    fixed endBalance = beginBalance + netChange;

    MoneyValcon moneyValcon;
    labels[5]->setText(moneyValcon.format(beginBalance));
    labels[6]->setText(moneyValcon.format(totalDebits));
    labels[7]->setText(moneyValcon.format(totalCredits));
    labels[8]->setText(moneyValcon.format(netChange));
    labels[9]->setText(moneyValcon.format(endBalance));

    _grid->set(USE_NEXT, 0, "");
    _grid->set(USE_NEXT, 0, 1, 5, tr("Beginning Balance"), Grid::Right);
    _grid->set(USE_CURR, 5, 1, 2, MoneyValcon().format(beginBalance));
    _grid->set(USE_NEXT, 0, 1, 5, tr("Total Debits"), Grid::Right);
    _grid->set(USE_CURR, 5, 1, 2, MoneyValcon().format(totalDebits));
    _grid->set(USE_NEXT, 0, 1, 5, tr("Total Credits"), Grid::Right);
    _grid->set(USE_CURR, 5, 1, 2, MoneyValcon().format(totalCredits));
    _grid->set(USE_NEXT, 0, 1, 5, tr("Net Change"), Grid::Right);
    _grid->set(USE_CURR, 5, 1, 2, MoneyValcon().format(netChange));
    _grid->set(USE_NEXT, 0, 1, 5, tr("Ending Balance"), Grid::Right);
    _grid->set(USE_CURR, 5, 1, 2, MoneyValcon().format(endBalance));

    QApplication::restoreOverrideCursor();
    setEnabled(true);
    loading = false;
}
