// $Id: journal.cpp,v 1.55 2005/01/30 04:25:31 bpepers Exp $
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

#include "journal.h"
#include "main_window.h"
#include "screen_decl.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "gltx_select.h"
#include "account_select.h"
#include "journal_window.h"
#include "date_range.h"
#include "date_popup.h"
#include "lookup_edit.h"
#include "store_lookup.h"
#include "list_view_item.h"
#include "money_valcon.h"
#include "date_valcon.h"
#include "grid.h"

#include <qapplication.h>
#include <qtabwidget.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qprogressdialog.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qtimer.h>
#include <algorithm>

Journal::Journal(MainWindow* main)
    : QuasarWindow(main, "Journal")
{
    _helpSource = "journal.html";

    QFrame* frame = new QFrame(this);

    QLabel* dateLabel = new QLabel(tr("Dates:"), frame);
    _dateRange = new DateRange(frame);
    dateLabel->setBuddy(_dateRange);

    QLabel* fromLabel = new QLabel(tr("From:"), frame);
    _from = new DatePopup(frame);
    fromLabel->setBuddy(_from);

    QLabel* toLabel = new QLabel(tr("To:"), frame);
    _to = new DatePopup(frame);
    toLabel->setBuddy(_to);

    QLabel* storeLabel = new QLabel(tr("Store:"), frame);
    _store = new LookupEdit(new StoreLookup(_main, this), frame);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    _tabs = new QTabWidget(frame);
    _general = new JournalWindow(_tabs, "GeneralTab");
    _disbursements = new JournalWindow(_tabs, "DisbursementsTab");
    _receipts = new JournalWindow(_tabs, "ReceiptsTab");
    _sales = new JournalWindow(_tabs, "SalesTab");
    _purchases = new JournalWindow(_tabs, "PurchasesTab");
    _inventory = new JournalWindow(_tabs, "InventoryTab");
    _all = new JournalWindow(_tabs, "AllTab");

    _tabs->addTab(_general, tr("&General"));
    _tabs->addTab(_disbursements, tr("&Disbursements"));
    _tabs->addTab(_receipts, tr("&Receipts"));
    _tabs->addTab(_sales, tr("&Sales"));
    _tabs->addTab(_purchases, tr("&Purchases"));
    _tabs->addTab(_inventory, tr("&Inventory"));
    _tabs->addTab(_all, tr("&All"));

    QFrame* box = new QFrame(frame);

    QPushButton* refresh = new QPushButton(tr("&Refresh"), box);
    refresh->setMinimumSize(refresh->sizeHint());
    connect(refresh, SIGNAL(clicked()), SLOT(slotRefresh()));

    QPushButton* print = new QPushButton(tr("&Print"), box);
    print->setMinimumSize(refresh->sizeHint());
    connect(print, SIGNAL(clicked()), SLOT(slotPrint()));

    QPushButton* close = new QPushButton(tr("Cl&ose"), box);
    close->setMinimumSize(refresh->sizeHint());
    connect(close, SIGNAL(clicked()), SLOT(close()));

    QGridLayout* grid = new QGridLayout(box);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setColStretch(2, 1);
    grid->addWidget(refresh, 0, 0, AlignLeft | AlignVCenter);
    grid->addWidget(print, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(close, 0, 2, AlignRight | AlignVCenter);

    QGridLayout* grid2 = new QGridLayout(frame);
    grid2->setSpacing(6);
    grid2->setMargin(6);
    grid2->setRowStretch(2, 1);
    grid2->setColStretch(6, 1);
    grid2->addWidget(dateLabel, 0, 0);
    grid2->addWidget(_dateRange, 0, 1);
    grid2->addWidget(fromLabel, 0, 2);
    grid2->addWidget(_from, 0, 3, AlignLeft | AlignVCenter);
    grid2->addWidget(toLabel, 0, 4);
    grid2->addWidget(_to, 0, 5, AlignLeft | AlignVCenter);
    grid2->addWidget(storeLabel, 1, 0);
    grid2->addMultiCellWidget(_store, 1, 1, 1, 5, AlignLeft | AlignVCenter);
    grid2->addMultiCellWidget(_tabs, 2, 2, 0, 6);
    grid2->addMultiCellWidget(box, 3, 3, 0, 6);

    connect(_general->list, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotEdit()));
    connect(_general->list, SIGNAL(returnPressed(QListViewItem*)),
	    SLOT(slotEdit()));
    connect(_disbursements->list, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotEdit()));
    connect(_disbursements->list, SIGNAL(returnPressed(QListViewItem*)),
	    SLOT(slotEdit()));
    connect(_receipts->list, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotEdit()));
    connect(_receipts->list, SIGNAL(returnPressed(QListViewItem*)),
	    SLOT(slotEdit()));
    connect(_sales->list, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotEdit()));
    connect(_sales->list, SIGNAL(returnPressed(QListViewItem*)),
	    SLOT(slotEdit()));
    connect(_purchases->list, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotEdit()));
    connect(_purchases->list, SIGNAL(returnPressed(QListViewItem*)),
	    SLOT(slotEdit()));
    connect(_inventory->list, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotEdit()));
    connect(_inventory->list, SIGNAL(returnPressed(QListViewItem*)),
	    SLOT(slotEdit()));
    connect(_all->list, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotEdit()));
    connect(_all->list, SIGNAL(returnPressed(QListViewItem*)),
	    SLOT(slotEdit()));

    _dateRange->setFromPopup(_from);
    _dateRange->setToPopup(_to);
    _dateRange->setCurrentItem(DateRange::Today);
    _dateRange->setFocus();
    _loading = false;

    _store->setId(_quasar->defaultStore());
    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    setCaption(tr("Transaction Journal"));
    setCentralWidget(frame);
    finalize();
    _refreshPending = false;
}

Journal::~Journal()
{
}

void
Journal::setStoreId(Id store_id)
{
    _store->setId(store_id);
}

void
Journal::setCurrentTab(int tab)
{
    _tabs->setCurrentPage(tab);
}

void
Journal::slotRefresh()
{
    QDate startDate = _from->getDate();
    QDate endDate = _to->getDate();
    if (startDate.isNull() || endDate.isNull()) {
	QMessageBox::critical(this, tr("Error"),
			      tr("Blank dates are not allowed"));
	return;
    }

    bool pending = _refreshPending;
    if (_refreshPending) return;
    _refreshPending = true;

    if (!_from->valid() || !_to->valid()) {
	_refreshPending = pending;
	return;
    }

    QTimer::singleShot(0, this, SLOT(slotRealRefresh()));
}

void
Journal::slotRealRefresh()
{
    if (_loading) return;
    _loading = true;
    setEnabled(false);
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Disabled updates so that progress bar updating doesn't also do lists
    _general->setUpdatesEnabled(false);
    _disbursements->setUpdatesEnabled(false);
    _receipts->setUpdatesEnabled(false);
    _sales->setUpdatesEnabled(false);
    _purchases->setUpdatesEnabled(false);
    _inventory->setUpdatesEnabled(false);
    _all->setUpdatesEnabled(false);

    // Load accounts if needed
    if (_accounts.size() == 0) {
	_quasar->db()->select(_accounts, AccountSelect());
    }

    // Clear out old values
    _general->clearList();
    _disbursements->clearList();
    _receipts->clearList();
    _sales->clearList();
    _purchases->clearList();
    _inventory->clearList();
    _all->clearList();

    _lastStart = _from->getDate();
    _lastEnd = _to->getDate();
    int days = _lastEnd - _lastStart + 1;

    // Setup progress dialog
    QProgressDialog* progress = new QProgressDialog(tr("Loading transactions..."),
						    tr("Cancel"), days, this,
						    "Progress", true);
    progress->setMinimumDuration(0);
    progress->setCaption(tr("Progress"));

    for (int day = 0; day < days; ++day) {
	loadDate(_lastStart + day);

	progress->setProgress(day);
	qApp->processEvents();
	if (progress->wasCancelled()) {
	    // TODO: add cancelled line
	    break;
	}
    }

    progress->setProgress(days);
    delete progress;

    // Enable updates on lists
    _general->setUpdatesEnabled(true);
    _disbursements->setUpdatesEnabled(true);
    _receipts->setUpdatesEnabled(true);
    _sales->setUpdatesEnabled(true);
    _purchases->setUpdatesEnabled(true);
    _inventory->setUpdatesEnabled(true);
    _all->setUpdatesEnabled(true);

    QApplication::restoreOverrideCursor();
    setEnabled(true);
    _loading = false;
    _refreshPending = false;
}

void
Journal::loadDate(QDate date)
{
    // Select transactions from gltx
    GltxSelect conditions;
    conditions.start_date = date;
    conditions.end_date = date;
    conditions.store_id = _store->getId();
    vector<Gltx> gltxs;
    _quasar->db()->select(gltxs, conditions);

    // Sort by date and number
    std::sort(gltxs.begin(), gltxs.end());

    // Process each transaction
    for (unsigned int i = 0; i < gltxs.size(); ++i) {
	const Gltx& gltx = gltxs[i];
	if (gltx.dataType() == DataObject::QUOTE) continue;

	// Add to all and one other depending on type
	addToList(_all, gltx);
	switch (gltx.dataType()) {
	case DataObject::GENERAL:	addToList(_general, gltx); break;
	case DataObject::NOSALE:	addToList(_general, gltx); break;
	case DataObject::SHIFT:		addToList(_general, gltx); break;
	case DataObject::CHEQUE:	addToList(_disbursements, gltx); break;
	case DataObject::PAYOUT:	addToList(_disbursements, gltx); break;
	case DataObject::TEND_ADJUST:	addToList(_disbursements, gltx); break;
	case DataObject::WITHDRAW:	addToList(_disbursements, gltx); break;
	case DataObject::RECEIPT:	addToList(_receipts, gltx); break;
	case DataObject::INVOICE:	addToList(_sales, gltx); break;
	case DataObject::RETURN:	addToList(_sales, gltx); break;
	case DataObject::CARD_ADJUST:	addToList(_sales, gltx); break;
	case DataObject::RECEIVE:	addToList(_purchases, gltx); break;
	case DataObject::CLAIM:		addToList(_purchases, gltx); break;
	case DataObject::ITEM_ADJUST:	addToList(_inventory, gltx); break;
	default:
	    qWarning("Unknown tx type: %s", gltx.dataTypeName().latin1());
	}
    }
}

void
Journal::slotEdit()
{
    JournalWindow* window = (JournalWindow*)_tabs->currentPage();
    ListViewItem* item = (ListViewItem*)window->list->currentItem();
    if (item == NULL) return;

    Gltx gltx;
    Id gltx_id = item->id;
    if (!_quasar->db()->lookup(gltx_id, gltx)) return;

    QWidget* edit = editGltx(gltx, _main);
    if (edit != NULL) edit->show();
}

void
Journal::closeEvent(QCloseEvent* e)
{
    if (_loading)
	e->ignore();
    else
	QMainWindow::closeEvent(e);
}

void
Journal::slotPrint()
{
    JournalWindow* current = (JournalWindow*)_tabs->currentPage();
    if (current->list->childCount() == 0) {
	QMessageBox::warning(this, tr("Warning"),
			     tr("No transactions to print"));
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    QString title = tr("Transactions");
    if (current == _general) title = tr("General Transactions");
    else if (current == _disbursements) title = tr("Disbursements");
    else if (current == _receipts) title = tr("Receipts");
    else if (current == _sales) title = tr("Sales");
    else if (current == _purchases) title = tr("Purchases");
    else if (current == _inventory) title = tr("Adjustments");

    title += "\n\n" + DateValcon().format(_lastStart) + tr("  to  ") +
	DateValcon().format(_lastEnd);

    Grid* grid = Grid::buildGrid(current->list, title);

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
Journal::addToList(JournalWindow* window, const Gltx& gltx)
{
    if (window == NULL) return;
    ListViewItem* item = new ListViewItem(window->list, window->last,
					  gltx.id());
    window->last = item;

    Store store;
    findStore(gltx.storeId(), store);

    item->setValue(0, gltx.dataTypeName());
    item->setValue(1, gltx.number());
    item->setValue(2, store.number());
    item->setValue(3, gltx.postDate());
    item->setValue(4, gltx.memo());
    item->setValue(7, !gltx.isActive());

    for (unsigned int i = 0; i < gltx.accounts().size(); ++i) {
	const AccountLine& line = gltx.accounts()[i];

	Account account;
	for (unsigned int a = 0; a < _accounts.size(); ++a) {
	    if (_accounts[a].id() == line.account_id) {
		account = _accounts[a];
		break;
	    }
	}

	ListViewItem* item = new ListViewItem(window->list, window->last,
					      gltx.id());
	window->last = item;

	item->setValue(4, account.name());
	if (line.amount >= 0.0)
	    item->setValue(5, line.amount);
	else
	    item->setValue(6, -line.amount);
    }
}

bool
Journal::findStore(Id store_id, Store& store)
{
    for (unsigned int i = 0; i < _store_cache.size(); ++i) {
	if (_store_cache[i].id() == store_id) {
	    store = _store_cache[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(store_id, store))
	return false;

    _store_cache.push_back(store);
    return true;
}
