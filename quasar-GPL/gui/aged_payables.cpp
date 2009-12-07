// $Id: aged_payables.cpp,v 1.21 2005/01/30 04:25:31 bpepers Exp $
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

#include "aged_payables.h"
#include "main_window.h"
#include "screen_decl.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "lookup_edit.h"
#include "account_lookup.h"
#include "vendor_lookup.h"
#include "group_lookup.h"
#include "store_lookup.h"
#include "list_view_item.h"
#include "account.h"
#include "vendor.h"
#include "group.h"
#include "gltx.h"
#include "gltx_select.h"
#include "money_edit.h"
#include "percent_edit.h"
#include "grid.h"
#include "text_frame.h"
#include "line_frame.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qtimer.h>

AgedPayables::AgedPayables(MainWindow* main)
    : QuasarWindow(main, "AgedPayables")
{
    _helpSource = "aged_payables.html";

    QFrame* frame = new QFrame(this);

    QLabel* levelLabel = new QLabel(tr("Level:"), frame);
    _level = new QComboBox(false, frame);
    levelLabel->setBuddy(_level);
    connect(_level, SIGNAL(activated(int)), SLOT(slotLevelChanged()));

    QLabel* storeLabel = new QLabel(tr("Store:"), frame);
    _store = new LookupEdit(new StoreLookup(main, this), frame);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    QLabel* accountLabel = new QLabel(tr("Account:"), frame);
    _account = new LookupEdit(new AccountLookup(main, this, Account::AR),
			       frame);
    _account->setLength(30);
    accountLabel->setBuddy(_account);

    QLabel* vendorLabel = new QLabel(tr("Vendor:"), frame);
    _vendor = new LookupEdit(new VendorLookup(main, this), frame);
    _vendor->setLength(30);
    vendorLabel->setBuddy(_vendor);

    QLabel* groupLabel = new QLabel(tr("Group:"), frame);
    _group = new LookupEdit(new GroupLookup(main, this, Group::VENDOR),
			    frame);
    _group->setLength(20);

    _lines = new ListView(frame);
    _lines->addTextColumn(tr("Vendor"), 24);
    _lines->addMoneyColumn(tr("Balance"));
    _lines->addMoneyColumn(tr("Current"));
    _lines->addMoneyColumn(tr("30 days"));
    _lines->addMoneyColumn(tr("60 days"));
    _lines->addMoneyColumn(tr("90 days"));
    _lines->addMoneyColumn(tr("120+ days"));
    _lines->setAllColumnsShowFocus(true);
    _lines->setShowSortIndicator(true);
    connect(_lines, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotPickLine()));

    QFrame* total = new QFrame(frame);
    QLabel* balanceLabel = new QLabel(tr("Balance"), total);
    QLabel* currentLabel = new QLabel(tr("Current"), total);
    QLabel* pd30Label = new QLabel(tr("30 days"), total);
    QLabel* pd60Label = new QLabel(tr("60 days"), total);
    QLabel* pd90Label = new QLabel(tr("90 days"), total);
    QLabel* pd120Label = new QLabel(tr("120+ days"), total);
    QLabel* totalLabel = new QLabel(tr("Total"), total);
    QLabel* percentLabel = new QLabel(tr("Percent"), total);

    _balanceTotal = new MoneyEdit(total);
    _balanceTotal->setFocusPolicy(NoFocus);
    _balanceTotal->setMinCharWidth(14, '9');
    _currentTotal = new MoneyEdit(total);
    _currentTotal->setFocusPolicy(NoFocus);
    _currentTotal->setMinCharWidth(14, '9');
    _pd30Total = new MoneyEdit(total);
    _pd30Total->setFocusPolicy(NoFocus);
    _pd30Total->setMinCharWidth(14, '9');
    _pd60Total = new MoneyEdit(total);
    _pd60Total->setFocusPolicy(NoFocus);
    _pd60Total->setMinCharWidth(14, '9');
    _pd90Total = new MoneyEdit(total);
    _pd90Total->setFocusPolicy(NoFocus);
    _pd90Total->setMinCharWidth(14, '9');
    _pd120Total = new MoneyEdit(total);
    _pd120Total->setFocusPolicy(NoFocus);
    _pd120Total->setMinCharWidth(14, '9');

    _balancePercent = new PercentEdit(total);
    _balancePercent->setFocusPolicy(NoFocus);
    _balancePercent->setMinCharWidth(14, '9');
    _currentPercent = new PercentEdit(total);
    _currentPercent->setFocusPolicy(NoFocus);
    _currentPercent->setMinCharWidth(14, '9');
    _pd30Percent = new PercentEdit(total);
    _pd30Percent->setFocusPolicy(NoFocus);
    _pd30Percent->setMinCharWidth(14, '9');
    _pd60Percent = new PercentEdit(total);
    _pd60Percent->setFocusPolicy(NoFocus);
    _pd60Percent->setMinCharWidth(14, '9');
    _pd90Percent = new PercentEdit(total);
    _pd90Percent->setFocusPolicy(NoFocus);
    _pd90Percent->setMinCharWidth(14, '9');
    _pd120Percent = new PercentEdit(total);
    _pd120Percent->setFocusPolicy(NoFocus);
    _pd120Percent->setMinCharWidth(14, '9');

    QGridLayout* totalGrid = new QGridLayout(total);
    totalGrid->setSpacing(3);
    totalGrid->setMargin(3);
    totalGrid->addWidget(balanceLabel, 0, 1);
    totalGrid->addWidget(currentLabel, 0, 2);
    totalGrid->addWidget(pd30Label, 0, 3);
    totalGrid->addWidget(pd60Label, 0, 4);
    totalGrid->addWidget(pd90Label, 0, 5);
    totalGrid->addWidget(pd120Label, 0, 6);
    totalGrid->addWidget(totalLabel, 1, 0);
    totalGrid->addWidget(_balanceTotal, 1, 1);
    totalGrid->addWidget(_currentTotal, 1, 2);
    totalGrid->addWidget(_pd30Total, 1, 3);
    totalGrid->addWidget(_pd60Total, 1, 4);
    totalGrid->addWidget(_pd90Total, 1, 5);
    totalGrid->addWidget(_pd120Total, 1, 6);
    totalGrid->addWidget(percentLabel, 2, 0);
    totalGrid->addWidget(_balancePercent, 2, 1);
    totalGrid->addWidget(_currentPercent, 2, 2);
    totalGrid->addWidget(_pd30Percent, 2, 3);
    totalGrid->addWidget(_pd60Percent, 2, 4);
    totalGrid->addWidget(_pd90Percent, 2, 5);
    totalGrid->addWidget(_pd120Percent, 2, 6);

    QFrame* buttons = new QFrame(frame);
    QPushButton* refresh = new QPushButton(tr("&Refresh"), buttons);
    connect(refresh, SIGNAL(clicked()), SLOT(slotRefresh()));
    refresh->setMinimumSize(refresh->sizeHint());

    QPushButton* print = new QPushButton(tr("&Print"), buttons);
    connect(print, SIGNAL(clicked()), SLOT(slotPrint()));
    print->setMinimumSize(refresh->sizeHint());

    QPushButton* ok = new QPushButton(tr("&Close"), buttons);
    connect(ok, SIGNAL(clicked()), SLOT(close()));
    ok->setMinimumSize(refresh->sizeHint());

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(2, 1);
    buttonGrid->addWidget(refresh, 0, 0, AlignLeft | AlignVCenter);
    buttonGrid->addWidget(print, 0, 1, AlignLeft | AlignVCenter);
    buttonGrid->addWidget(ok, 0, 2, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setColStretch(2, 1);
    grid->setRowStretch(3, 1);
    grid->addWidget(levelLabel, 0, 0);
    grid->addWidget(_level, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(storeLabel, 1, 0);
    grid->addWidget(_store, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(accountLabel, 0, 3);
    grid->addWidget(_account, 0, 4, AlignLeft | AlignVCenter);
    grid->addWidget(vendorLabel, 1, 3);
    grid->addWidget(_vendor, 1, 4, AlignLeft | AlignVCenter);
    grid->addWidget(groupLabel, 2, 3);
    grid->addWidget(_group, 2, 4, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_lines, 3, 3, 0, 4);
    grid->addMultiCellWidget(total, 4, 4, 0, 4);
    grid->addMultiCellWidget(buttons, 5, 5, 0, 4);

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    } else {
	_level->insertItem(tr("Store"));
    }

    _level->insertItem(tr("Vendor"));
    _level->insertItem(tr("Transaction"));
    _level->setCurrentItem(_level->count() - 2);
    _level->setFocus();

    setCentralWidget(frame);
    setCaption(tr("Aged Payables"));
    finalize();
}

AgedPayables::~AgedPayables()
{
}

void
AgedPayables::setStoreId(Id store_id)
{
    _store->setId(store_id);
}

void
AgedPayables::setVendorId(Id vendor_id)
{
    _vendor->setId(vendor_id);
    slotRefresh();
}

void
AgedPayables::slotLevelChanged()
{
    _store->setEnabled(_level->currentText() != tr("Store"));
}

void
AgedPayables::slotPickLine()
{
    ListViewItem* item = (ListViewItem*)_lines->currentItem();
    if (item == NULL) return;

    if (_level->currentText() == tr("Store")) {
	_level->setCurrentItem(_level->currentItem() + 1);
	_store->setId(item->id);
	slotRefresh();
    } else if (_level->currentText() == tr("Vendor")) {
	_level->setCurrentItem(_level->currentItem() + 1);
	_vendor->setId(item->id);
	slotRefresh();
    } else {
	Gltx gltx;
	Id gltx_id = item->id;
	if (!_quasar->db()->lookup(gltx_id, gltx)) return;

	QWidget* edit = editGltx(gltx, _main);
	if (edit != NULL) edit->show();
    }
}

void
AgedPayables::slotPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_lines, tr("Aged Payables"));
    Grid* header = new Grid(1, grid);
    grid->set(0, 0, 1, grid->columns(), header, Grid::AllSides);

    header->setColumnWeight(0, 1);
    TextFrame* text = new TextFrame(tr("Aged Payables"), header);
    text->setFont(Font("Times", 24));
    header->set(USE_NEXT, 0, text);
    if (_vendor->getId() != INVALID_ID) {
	Card card;
	_quasar->db()->lookup(_vendor->getId(), card);
	text = new TextFrame(card.nameFL(), header);
	text->setFont(Font("Times", 14));
	header->set(USE_NEXT, 0, " ");
	header->set(USE_NEXT, 0, text);
    }

    // Add totals line
    grid->set(grid->rows(), 0, " ");
    grid->set(USE_NEXT, 0, tr("Total"));
    grid->set(USE_CURR, 1, _balanceTotal->text());
    grid->set(USE_CURR, 2, _currentTotal->text());
    grid->set(USE_CURR, 3, _pd30Total->text());
    grid->set(USE_CURR, 4, _pd60Total->text());
    grid->set(USE_CURR, 5, _pd90Total->text());
    grid->set(USE_CURR, 6, _pd120Total->text());

    // Add percents line
    grid->set(USE_NEXT, 0, tr("Percent"));
    grid->set(USE_CURR, 1, _balancePercent->text());
    grid->set(USE_CURR, 2, _currentPercent->text());
    grid->set(USE_CURR, 3, _pd30Percent->text());
    grid->set(USE_CURR, 4, _pd60Percent->text());
    grid->set(USE_CURR, 5, _pd90Percent->text());
    grid->set(USE_CURR, 6, _pd120Percent->text());

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
AgedPayables::slotRefresh()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    fixed balanceTotal = 0.0;
    fixed currentTotal = 0.0;
    fixed pd30Total = 0.0;
    fixed pd60Total = 0.0;
    fixed pd90Total = 0.0;
    fixed pd120Total = 0.0;

    _lines->clear();
    if (_level->currentText() == tr("Store")) {
	_lines->setColumnText(0, tr("Store"));
	_lines->setValcon(0, NULL);
    } else if (_level->currentText() == tr("Vendor")) {
	_lines->setColumnText(0, tr("Vendor"));
	_lines->setValcon(0, NULL);
    } else {
	_lines->setColumnText(0, tr("Id Number"));
	_lines->setValcon(0, new NumberValcon());
    }

    // Load transactions
    GltxSelect conditions;
    conditions.store_id = _store->getId();
    conditions.card_id = _vendor->getId();
    conditions.unpaid = true;
    conditions.activeOnly = true;
    _quasar->db()->select(_gltxs, conditions);

    // Add transactions
    _info.clear();
    unsigned int i;
    for (i = 0; i < _gltxs.size(); ++i)
	addGltx(_gltxs[i]);

    // Allocate extra credits, add to GUI, and add up totals
    for (i = 0; i < _info.size(); ++i) {
	AgeInfo& info = _info[i];
	if (info.pd120 < 0.0) {
	    info.pd90 += info.pd120;
	    info.pd120 = 0.0;
	}
	if (info.pd90 < 0.0) {
	    info.pd60 += info.pd90;
	    info.pd90 = 0.0;
	}
	if (info.pd60 < 0.0) {
	    info.pd30 += info.pd60;
	    info.pd60 = 0.0;
	}
	if (info.pd30 < 0.0) {
	    info.current += info.pd30;
	    info.pd30 = 0.0;
	}

	ListViewItem* item = new ListViewItem(_lines, info.id);
	if (_level->currentText() == tr("Transaction"))
	    item->setValue(0, info.number);
	else
	    item->setValue(0, info.name);
	item->setValue(1, info.balance);
	if (info.current != 0.0) item->setValue(2, info.current);
	if (info.pd30 != 0.0) item->setValue(3, info.pd30);
	if (info.pd60 != 0.0) item->setValue(4, info.pd60);
	if (info.pd90 != 0.0) item->setValue(5, info.pd90);
	if (info.pd120 != 0.0) item->setValue(6, info.pd120);

	balanceTotal += info.balance;
	currentTotal += info.current;
	pd30Total += info.pd30;
	pd60Total += info.pd60;
	pd90Total += info.pd90;
	pd120Total += info.pd120;
    }

    // Set totals
    _balanceTotal->setFixed(balanceTotal);
    _currentTotal->setFixed(currentTotal);
    _pd30Total->setFixed(pd30Total);
    _pd60Total->setFixed(pd60Total);
    _pd90Total->setFixed(pd90Total);
    _pd120Total->setFixed(pd120Total);

    // Set percents
    _balancePercent->setFixed(balanceTotal / balanceTotal * 100.0);
    _currentPercent->setFixed(currentTotal / balanceTotal * 100.0);
    _pd30Percent->setFixed(pd30Total / balanceTotal * 100.0);
    _pd60Percent->setFixed(pd60Total / balanceTotal * 100.0);
    _pd90Percent->setFixed(pd90Total / balanceTotal * 100.0);
    _pd120Percent->setFixed(pd120Total / balanceTotal * 100.0);

    QApplication::restoreOverrideCursor();
}

void
AgedPayables::addGltx(const Gltx& gltx)
{
    if (gltx.cardId() == INVALID_ID) return;
    if (gltx.cardTotal() == 0.0) return;

    fixed remain = gltx.cardTotal() - gltx.paymentTotal();
    if (remain == 0.0) return;

    Vendor vendor;
    if (!findVendor(gltx.cardId(), vendor)) return;

    Store store;
    if (!findStore(gltx.storeId(), store)) return;

    if (_account->getId() != INVALID_ID) {
	if (vendor.accountId() != _account->getId()) return;
    }

    if (_group->getId() != INVALID_ID) {
	bool found = false;
	for (unsigned int i = 0; i < vendor.groups().size(); ++i) {
	    if (_group->getId() == vendor.groups()[i]) {
		found = true;
		break;
	    }
	}
	if (!found) return;
    }

    // If not by transaction, try to add to existing info
    if (_level->currentText() != tr("Transaction")) {
	for (unsigned int i = 0; i < _info.size(); ++i) {
	    AgeInfo& info = _info[i];
	    if (_level->currentText() == tr("Vendor")) {
		if (info.id != vendor.id()) continue;
	    } else {
		if (info.id != store.id()) continue;
	    }
	    info.balance += remain;
	    if (remain < 0.0) {
		info.pd120 += remain;
	    } else {
		int days = QDate::currentDate() - gltx.postDate();
		if (days < 30) info.current += remain;
		else if (days < 60) info.pd30 += remain;
		else if (days < 90) info.pd60 += remain;
		else if (days < 120) info.pd90 += remain;
		else info.pd120 += remain;
	    }
	    return;
	}
    }

    // Create new info
    AgeInfo info;
    if (_level->currentText() == tr("Store")) {
	info.id = store.id();
	info.name = store.name();
	info.balance = remain;
    } else if (_level->currentText() == tr("Vendor")) {
	info.id = vendor.id();
	info.name = vendor.name();
	info.balance = remain;
    } else {
	info.id = gltx.id();
	info.number = gltx.number();
	info.balance = remain;
    }

    if (remain < 0.0) {
	info.pd120 = remain;
    } else {
	int days = QDate::currentDate() - gltx.postDate();
	if (days < 30) info.current = remain;
	else if (days < 60) info.pd30 = remain;
	else if (days < 90) info.pd60 = remain;
	else if (days < 120) info.pd90 = remain;
	else info.pd120 = remain;
    }

    _info.push_back(info);
}

bool
AgedPayables::findVendor(Id vendor_id, Vendor& vendor)
{
    for (unsigned int i = 0; i < _vendors.size(); ++i) {
	if (_vendors[i].id() == vendor_id) {
	    vendor = _vendors[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(vendor_id, vendor))
	return false;

    _vendors.push_back(vendor);
    return true;
}

bool
AgedPayables::findStore(Id store_id, Store& store)
{
    for (unsigned int i = 0; i < _stores.size(); ++i) {
	if (_stores[i].id() == store_id) {
	    store = _stores[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(store_id, store))
	return false;

    _stores.push_back(store);
    return true;
}
