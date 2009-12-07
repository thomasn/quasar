// $Id: aged_receivables.cpp,v 1.29 2005/01/30 04:25:31 bpepers Exp $
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

#include "aged_receivables.h"
#include "main_window.h"
#include "screen_decl.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "lookup_edit.h"
#include "account_lookup.h"
#include "customer_type_lookup.h"
#include "customer_lookup.h"
#include "group_lookup.h"
#include "pat_group_lookup.h"
#include "store_lookup.h"
#include "list_view_item.h"
#include "account.h"
#include "customer.h"
#include "pat_group.h"
#include "group.h"
#include "gltx.h"
#include "gltx_select.h"
#include "money_edit.h"
#include "percent_edit.h"
#include "date_popup.h"
#include "grid.h"
#include "text_frame.h"
#include "line_frame.h"

#include <qapplication.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qtimer.h>

AgedReceivables::AgedReceivables(MainWindow* main)
    : QuasarWindow(main, "AgedReceivables")
{
    _helpSource = "aged_receivables.html";

    QFrame* frame = new QFrame(this);

    _all = new QRadioButton(tr("All Customers"), frame);
    _one = new QRadioButton(tr("One Customer"), frame);

    QButtonGroup* buttonGroup = new QButtonGroup(frame);
    buttonGroup->hide();
    buttonGroup->insert(_all);
    buttonGroup->insert(_one);
    connect(buttonGroup, SIGNAL(clicked(int)), SLOT(slotTypeChanged(int)));

    QLabel* typeLabel = new QLabel(tr("Type:"), frame);
    _type = new LookupEdit(new CustomerTypeLookup(main, this), frame);
    _type->setLength(20);
    typeLabel->setBuddy(_type);

    QLabel* groupLabel = new QLabel(tr("Group:"), frame);
    _group = new LookupEdit(new GroupLookup(main,this,Group::CUSTOMER),frame);
    _group->setLength(20);

    QLabel* patGroupLabel = new QLabel(tr("Pat Group:"), frame);
    _patgroup = new LookupEdit(new PatGroupLookup(main, this), frame);
    _patgroup->setLength(20);

    QLabel* customerLabel = new QLabel(tr("Customer:"), frame);
    _customer = new LookupEdit(new CustomerLookup(main, this), frame);
    _customer->setLength(30);
    customerLabel->setBuddy(_customer);

    QLabel* dateLabel = new QLabel(tr("Date:"), frame);
    _date = new DatePopup(frame);
    dateLabel->setBuddy(_date);

    QLabel* storeLabel = new QLabel(tr("Store:"), frame);
    _store = new LookupEdit(new StoreLookup(main, this), frame);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    _tabs = new QTabWidget(frame);
    buildTab(_debits);
    buildTab(_credits);
    _tabs->addTab(_debits.frame, tr("Debits"));
    _tabs->addTab(_credits.frame, tr("Credits"));

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
    grid->setColStretch(3, 1);
    grid->setRowStretch(5, 1);
    grid->addColSpacing(0, 10);
    grid->addMultiCellWidget(_all, 0, 0, 0, 2);
    grid->addWidget(typeLabel, 1, 1);
    grid->addWidget(_type, 1, 2, AlignLeft | AlignVCenter);
    grid->addWidget(groupLabel, 2, 1);
    grid->addWidget(_group, 2, 2, AlignLeft | AlignVCenter);
    grid->addWidget(patGroupLabel, 3, 1);
    grid->addWidget(_patgroup, 3, 2, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_one, 4, 4, 0, 2);
    grid->addWidget(customerLabel, 5, 1);
    grid->addWidget(_customer, 5, 2, AlignLeft | AlignVCenter);
    grid->addWidget(dateLabel, 0, 4);
    grid->addWidget(_date, 0, 5, AlignLeft | AlignVCenter);
    grid->addWidget(storeLabel, 1, 4);
    grid->addWidget(_store, 1, 5, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_tabs, 6, 6, 0, 5);
    grid->addMultiCellWidget(buttons, 7, 7, 0, 5);

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    _all->setChecked(true);
    slotTypeChanged(0);
    _type->setFocus();

    setCentralWidget(frame);
    setCaption(tr("Aged Receivables"));
    finalize();
}

AgedReceivables::~AgedReceivables()
{
}

void
AgedReceivables::slotTypeChanged(int index)
{
    _type->setEnabled(index == 0);
    _group->setEnabled(index == 0);
    _patgroup->setEnabled(index == 0);
    _customer->setEnabled(index == 1);
}

void
AgedReceivables::buildTab(AgeTab& tab)
{
    tab.frame = new QFrame(_tabs);

    tab.lines = new ListView(tab.frame);
    tab.lines->addTextColumn(tr("Customer"), 24);
    tab.lines->addTextColumn(tr("Number"), 8);
    tab.lines->addMoneyColumn(tr("Balance"));
    tab.lines->addMoneyColumn(tr("Current"));
    tab.lines->addMoneyColumn(tr("30 days"));
    tab.lines->addMoneyColumn(tr("60 days"));
    tab.lines->addMoneyColumn(tr("90 days"));
    tab.lines->addMoneyColumn(tr("120+ days"));
    tab.lines->setAllColumnsShowFocus(true);
    tab.lines->setShowSortIndicator(true);
    connect(tab.lines, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotPickLine()));

    QFrame* total = new QFrame(tab.frame);
    QLabel* balanceLabel = new QLabel(tr("Balance"), total);
    QLabel* currentLabel = new QLabel(tr("Current"), total);
    QLabel* pd30Label = new QLabel(tr("30 days"), total);
    QLabel* pd60Label = new QLabel(tr("60 days"), total);
    QLabel* pd90Label = new QLabel(tr("90 days"), total);
    QLabel* pd120Label = new QLabel(tr("120+ days"), total);
    QLabel* totalLabel = new QLabel(tr("Total"), total);
    QLabel* percentLabel = new QLabel(tr("Percent"), total);

    tab.balanceTotal = new MoneyEdit(total);
    tab.balanceTotal->setFocusPolicy(NoFocus);
    tab.balanceTotal->setMinCharWidth(14, '9');
    tab.currentTotal = new MoneyEdit(total);
    tab.currentTotal->setFocusPolicy(NoFocus);
    tab.currentTotal->setMinCharWidth(14, '9');
    tab.pd30Total = new MoneyEdit(total);
    tab.pd30Total->setFocusPolicy(NoFocus);
    tab.pd30Total->setMinCharWidth(14, '9');
    tab.pd60Total = new MoneyEdit(total);
    tab.pd60Total->setFocusPolicy(NoFocus);
    tab.pd60Total->setMinCharWidth(14, '9');
    tab.pd90Total = new MoneyEdit(total);
    tab.pd90Total->setFocusPolicy(NoFocus);
    tab.pd90Total->setMinCharWidth(14, '9');
    tab.pd120Total = new MoneyEdit(total);
    tab.pd120Total->setFocusPolicy(NoFocus);
    tab.pd120Total->setMinCharWidth(14, '9');

    tab.balancePercent = new PercentEdit(total);
    tab.balancePercent->setFocusPolicy(NoFocus);
    tab.balancePercent->setMinCharWidth(14, '9');
    tab.currentPercent = new PercentEdit(total);
    tab.currentPercent->setFocusPolicy(NoFocus);
    tab.currentPercent->setMinCharWidth(14, '9');
    tab.pd30Percent = new PercentEdit(total);
    tab.pd30Percent->setFocusPolicy(NoFocus);
    tab.pd30Percent->setMinCharWidth(14, '9');
    tab.pd60Percent = new PercentEdit(total);
    tab.pd60Percent->setFocusPolicy(NoFocus);
    tab.pd60Percent->setMinCharWidth(14, '9');
    tab.pd90Percent = new PercentEdit(total);
    tab.pd90Percent->setFocusPolicy(NoFocus);
    tab.pd90Percent->setMinCharWidth(14, '9');
    tab.pd120Percent = new PercentEdit(total);
    tab.pd120Percent->setFocusPolicy(NoFocus);
    tab.pd120Percent->setMinCharWidth(14, '9');

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
    totalGrid->addWidget(tab.balanceTotal, 1, 1);
    totalGrid->addWidget(tab.currentTotal, 1, 2);
    totalGrid->addWidget(tab.pd30Total, 1, 3);
    totalGrid->addWidget(tab.pd60Total, 1, 4);
    totalGrid->addWidget(tab.pd90Total, 1, 5);
    totalGrid->addWidget(tab.pd120Total, 1, 6);
    totalGrid->addWidget(percentLabel, 2, 0);
    totalGrid->addWidget(tab.balancePercent, 2, 1);
    totalGrid->addWidget(tab.currentPercent, 2, 2);
    totalGrid->addWidget(tab.pd30Percent, 2, 3);
    totalGrid->addWidget(tab.pd60Percent, 2, 4);
    totalGrid->addWidget(tab.pd90Percent, 2, 5);
    totalGrid->addWidget(tab.pd120Percent, 2, 6);

    QGridLayout* grid = new QGridLayout(tab.frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->addWidget(tab.lines, 0, 0);
    grid->addWidget(total, 1, 0);
}

void
AgedReceivables::setStoreId(Id store_id)
{
    _store->setId(store_id);
}

void
AgedReceivables::setCustomerId(Id customer_id)
{
    _customer->setId(customer_id);
    slotRefresh();
}

void
AgedReceivables::slotPickLine()
{
    AgeTab* tab;
    if (_tabs->currentPage() == _debits.frame)
	tab = &_debits;
    else
	tab = &_credits;

    ListViewItem* item = (ListViewItem*)tab->lines->currentItem();
    if (item == NULL) return;

    if (_all->isChecked()) {
	_one->setChecked(true);
	_customer->setId(item->id);
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
AgedReceivables::slotPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    AgeTab* tab;
    if (_tabs->currentPage() == _debits.frame)
	tab = &_debits;
    else
	tab = &_credits;

    Font::defaultFont = Font("helvetica", 7);
    Grid* grid = Grid::buildGrid(tab->lines, tr("Aged Receivables"));
    Grid* header = new Grid(1, grid);
    grid->set(0, 0, 1, grid->columns(), header, Grid::AllSides);

    header->setColumnWeight(0, 1);
    TextFrame* text = new TextFrame(tr("Aged Receivables"), header);
    text->setFont(Font("Times", 24));
    header->set(USE_NEXT, 0, text);

    if (_all->isChecked()) {
	if (_type->getId() != INVALID_ID) {
	    CustomerType type;
	    _quasar->db()->lookup(_type->getId(), type);
	    text = new TextFrame(tr("Type: ") + type.name(), header);
	    text->setFont(Font("Times", 14));
	    header->set(USE_NEXT, 0, " ");
	    header->set(USE_NEXT, 0, text);
	}
	if (_group->getId() != INVALID_ID) {
	    Group group;
	    _quasar->db()->lookup(_group->getId(), group);
	    text = new TextFrame(tr("Group: ") + group.name(), header);
	    text->setFont(Font("Times", 14));
	    header->set(USE_NEXT, 0, " ");
	    header->set(USE_NEXT, 0, text);
	}
	if (_patgroup->getId() != INVALID_ID) {
	    PatGroup group;
	    _quasar->db()->lookup(_patgroup->getId(), group);
	    text = new TextFrame(tr("PatGroup: ") + group.name(), header);
	    text->setFont(Font("Times", 14));
	    header->set(USE_NEXT, 0, " ");
	    header->set(USE_NEXT, 0, text);
	}
    } else {
	if (_customer->getId() != INVALID_ID) {
	    Card card;
	    _quasar->db()->lookup(_customer->getId(), card);
	    text = new TextFrame(("Customer: ") + card.nameFL(), header);
	    text->setFont(Font("Times", 14));
	    header->set(USE_NEXT, 0, " ");
	    header->set(USE_NEXT, 0, text);
	}
    }

    if (_store->getId() != INVALID_ID) {
	Store store;
	_quasar->db()->lookup(_store->getId(), store);
	text = new TextFrame(("Store: ") + store.name(), header);
	text->setFont(Font("Times", 14));
	header->set(USE_NEXT, 0, " ");
	header->set(USE_NEXT, 0, text);
    }

    QDate date = _date->getDate();
    if (!date.isNull()) {
	text = new TextFrame(tr("As/at: ") + date.toString(), header);
	text->setFont(Font("Times", 14));
	header->set(USE_NEXT, 0, " ");
	header->set(USE_NEXT, 0, text);
    }

    // Add totals line
    grid->set(grid->rows(), 0, " ");
    grid->set(USE_NEXT, 0, tr("Total"));
    grid->set(USE_CURR, 4, tab->balanceTotal->text());
    grid->set(USE_CURR, 6, tab->currentTotal->text());
    grid->set(USE_CURR, 8, tab->pd30Total->text());
    grid->set(USE_CURR, 10, tab->pd60Total->text());
    grid->set(USE_CURR, 12, tab->pd90Total->text());
    grid->set(USE_CURR, 14, tab->pd120Total->text());

    // Add percents line
    grid->set(USE_NEXT, 0, tr("Percent"));
    grid->set(USE_CURR, 4, tab->balancePercent->text());
    grid->set(USE_CURR, 6, tab->currentPercent->text());
    grid->set(USE_CURR, 8, tab->pd30Percent->text());
    grid->set(USE_CURR, 10, tab->pd60Percent->text());
    grid->set(USE_CURR, 12, tab->pd90Percent->text());
    grid->set(USE_CURR, 14, tab->pd120Percent->text());

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
AgedReceivables::slotRefresh()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    fixed dbBalanceTotal = 0.0;
    fixed dbCurrentTotal = 0.0;
    fixed dbPd30Total = 0.0;
    fixed dbPd60Total = 0.0;
    fixed dbPd90Total = 0.0;
    fixed dbPd120Total = 0.0;

    fixed crBalanceTotal = 0.0;
    fixed crCurrentTotal = 0.0;
    fixed crPd30Total = 0.0;
    fixed crPd60Total = 0.0;
    fixed crPd90Total = 0.0;
    fixed crPd120Total = 0.0;

    _debits.lines->clear();
    if (_all->isChecked()) {
	_debits.lines->setColumnText(0, tr("Customer"));
	_debits.lines->setColumnAlignment(0, AlignLeft);
    } else {
	_debits.lines->setColumnText(0, tr("Id Number"));
	_debits.lines->setColumnAlignment(0, AlignRight);
    }

    _credits.lines->clear();
    if (_all->isChecked()) {
	_credits.lines->setColumnText(0, tr("Customer"));
	_credits.lines->setColumnAlignment(0, AlignLeft);
    } else {
	_credits.lines->setColumnText(0, tr("Id Number"));
	_credits.lines->setColumnAlignment(0, AlignRight);
    }

    // Load transactions
    GltxSelect conditions;
    conditions.store_id = _store->getId();
    if (_all->isChecked()) {
	conditions.cust_type_id = _type->getId();
	conditions.card_group_id = _group->getId();
	conditions.card_patgroup_id = _patgroup->getId();
    } else {
	conditions.card_id = _customer->getId();
    }
    if (_date->getDate().isNull()) {
	conditions.unpaid = true;
    } else {
	conditions.end_date = _date->getDate();
    }
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

	AgeTab* tab;
	if (info.balance >= 0.0)
	    tab = &_debits;
	else
	    tab = &_credits;

	ListViewItem* item = new ListViewItem(tab->lines, info.id);
	if (_one->isChecked())
	    item->setValue(0, info.number);
	else
	    item->setValue(0, info.name);
	item->setValue(1, info.cardNum);
	item->setValue(2, info.balance);
	if (info.current != 0.0) item->setValue(3, info.current);
	if (info.pd30 != 0.0) item->setValue(4, info.pd30);
	if (info.pd60 != 0.0) item->setValue(5, info.pd60);
	if (info.pd90 != 0.0) item->setValue(6, info.pd90);
	if (info.pd120 != 0.0) item->setValue(7, info.pd120);

	if (info.balance >= 0.0) {
	    dbBalanceTotal += info.balance;
	    dbCurrentTotal += info.current;
	    dbPd30Total += info.pd30;
	    dbPd60Total += info.pd60;
	    dbPd90Total += info.pd90;
	    dbPd120Total += info.pd120;
	} else {
	    crBalanceTotal += info.balance;
	    crCurrentTotal += info.current;
	    crPd30Total += info.pd30;
	    crPd60Total += info.pd60;
	    crPd90Total += info.pd90;
	    crPd120Total += info.pd120;
	}
    }

    // Set totals
    _debits.balanceTotal->setFixed(dbBalanceTotal);
    _debits.currentTotal->setFixed(dbCurrentTotal);
    _debits.pd30Total->setFixed(dbPd30Total);
    _debits.pd60Total->setFixed(dbPd60Total);
    _debits.pd90Total->setFixed(dbPd90Total);
    _debits.pd120Total->setFixed(dbPd120Total);
    _credits.balanceTotal->setFixed(crBalanceTotal);
    _credits.currentTotal->setFixed(crCurrentTotal);
    _credits.pd30Total->setFixed(crPd30Total);
    _credits.pd60Total->setFixed(crPd60Total);
    _credits.pd90Total->setFixed(crPd90Total);
    _credits.pd120Total->setFixed(crPd120Total);

    // Set percents
    _debits.balancePercent->setFixed(dbBalanceTotal / dbBalanceTotal * 100.0);
    _debits.currentPercent->setFixed(dbCurrentTotal / dbBalanceTotal * 100.0);
    _debits.pd30Percent->setFixed(dbPd30Total / dbBalanceTotal * 100.0);
    _debits.pd60Percent->setFixed(dbPd60Total / dbBalanceTotal * 100.0);
    _debits.pd90Percent->setFixed(dbPd90Total / dbBalanceTotal * 100.0);
    _debits.pd120Percent->setFixed(dbPd120Total / dbBalanceTotal * 100.0);
    _credits.balancePercent->setFixed(crBalanceTotal / crBalanceTotal * 100.0);
    _credits.currentPercent->setFixed(crCurrentTotal / crBalanceTotal * 100.0);
    _credits.pd30Percent->setFixed(crPd30Total / crBalanceTotal * 100.0);
    _credits.pd60Percent->setFixed(crPd60Total / crBalanceTotal * 100.0);
    _credits.pd90Percent->setFixed(crPd90Total / crBalanceTotal * 100.0);
    _credits.pd120Percent->setFixed(crPd120Total / crBalanceTotal * 100.0);

    QApplication::restoreOverrideCursor();
}

void
AgedReceivables::addGltx(Gltx& gltx)
{
    if (gltx.cardId() == INVALID_ID) return;
    if (gltx.cardTotal() == 0.0) return;

    // Remove payments after as/at date
    if (!_date->getDate().isNull()) {
	for (int i = gltx.payments().size() - 1; i >= 0; --i) {
	    Gltx payment;
	    _db->lookup(gltx.payments()[i].gltx_id, payment);
	    if (payment.postDate() > _date->getDate())
		gltx.payments().erase(gltx.payments().begin() + i);
	}
    }

    fixed remain = gltx.cardTotal() - gltx.paymentTotal();
    if (remain == 0.0) return;

    Customer customer;
    if (!findCustomer(gltx.cardId(), customer)) return;

    Store store;
    if (!findStore(gltx.storeId(), store)) return;

    QDate date = _date->getDate();
    if (date.isNull()) date = QDate::currentDate();

    // If not by transaction, try to add to existing info
    if (_all->isChecked()) {
	for (unsigned int i = 0; i < _info.size(); ++i) {
	    AgeInfo& info = _info[i];
	    if (info.id != customer.id()) continue;

	    info.balance += remain;
	    if (remain < 0.0) {
		info.pd120 += remain;
	    } else {
		int days = date - gltx.postDate();
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
    if (_all->isChecked()) {
	info.id = customer.id();
	info.name = customer.name();
	info.cardNum = customer.number();
	info.balance = remain;
    } else {
	info.id = gltx.id();
	info.number = gltx.number();
	info.balance = remain;
    }

    if (remain < 0.0) {
	info.pd120 = remain;
    } else {
	int days = date - gltx.postDate();
	if (days < 30) info.current = remain;
	else if (days < 60) info.pd30 = remain;
	else if (days < 90) info.pd60 = remain;
	else if (days < 120) info.pd90 = remain;
	else info.pd120 = remain;
    }

    _info.push_back(info);
}

bool
AgedReceivables::findCustomer(Id customer_id, Customer& customer)
{
    for (unsigned int i = 0; i < _customers.size(); ++i) {
	if (_customers[i].id() == customer_id) {
	    customer = _customers[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(customer_id, customer))
	return false;

    _customers.push_back(customer);
    return true;
}

bool
AgedReceivables::findStore(Id store_id, Store& store)
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
