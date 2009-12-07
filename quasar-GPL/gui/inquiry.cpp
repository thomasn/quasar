// $Id: inquiry.cpp,v 1.49 2005/01/30 04:25:31 bpepers Exp $
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

#include "inquiry.h"
#include "main_window.h"
#include "screen_decl.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "account_inquiry.h"
#include "card_inquiry.h"
#include "item_inquiry.h"
#include "lookup_edit.h"
#include "date_range.h"
#include "date_popup.h"
#include "store_lookup.h"
#include "item_lookup.h"
#include "list_view_item.h"
#include "money_valcon.h"

#include <assert.h>
#include <qtabwidget.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qmessagebox.h>

Inquiry::Inquiry(MainWindow* main)
    : QuasarWindow(main, "Inquiry"), _loading(false)
{
    _helpSource = "inquiry.html";

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

    QLabel* storeLabel = new QLabel(tr("&Store:"), frame);
    _store = new LookupEdit(new StoreLookup(_main, this), frame);
    _store->setLength(30);
    storeLabel->setBuddy(_store);
    connect(_store, SIGNAL(validData()), SLOT(slotStoreChange()));

    _tabs = new QTabWidget(frame);
    _account = new AccountInquiry(main, _tabs);
    _card = new CardInquiry(main, _tabs);
    _item = new ItemInquiry(main, _tabs);

    _tabs->addTab(_account, tr("&Account"));
    _tabs->addTab(_card, tr("Car&d"));
    _tabs->addTab(_item, tr("&Item"));

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

    QGridLayout* boxGrid = new QGridLayout(box);
    boxGrid->setSpacing(6);
    boxGrid->setMargin(6);
    boxGrid->setColStretch(2, 1);
    boxGrid->addWidget(refresh, 0, 0, AlignLeft | AlignVCenter);
    boxGrid->addWidget(print, 0, 1, AlignLeft | AlignVCenter);
    boxGrid->addWidget(close, 0, 2, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setRowStretch(2, 1);
    grid->setColStretch(6, 1);
    grid->addWidget(dateLabel, 0, 0);
    grid->addWidget(_dateRange, 0, 1);
    grid->addWidget(fromLabel, 0, 2);
    grid->addWidget(_from, 0, 3, AlignLeft | AlignVCenter);
    grid->addWidget(toLabel, 0, 4);
    grid->addWidget(_to, 0, 5, AlignLeft | AlignVCenter);
    grid->addWidget(storeLabel, 1, 0);
    grid->addMultiCellWidget(_store, 1, 1, 1, 5, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_tabs, 2, 2, 0, 6);
    grid->addMultiCellWidget(box, 3, 3, 0, 6);

    connect(_tabs, SIGNAL(selected(const QString&)), SLOT(slotTabChange()));

    connect(_account->search, SIGNAL(validData()), SLOT(slotAccountChange()));
    connect(_account->list, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotEdit()));
    connect(_account->list, SIGNAL(returnPressed(QListViewItem*)),
	    SLOT(slotEdit()));

    connect(_card->search, SIGNAL(validData()), SLOT(slotCardChange()));
    connect(_card->list, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotEdit()));
    connect(_card->list, SIGNAL(returnPressed(QListViewItem*)),
	    SLOT(slotEdit()));

    connect(_item->search, SIGNAL(validData()), SLOT(slotItemChange()));
    connect(_item->list, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotEdit()));
    connect(_item->list, SIGNAL(returnPressed(QListViewItem*)),
	    SLOT(slotEdit()));

    _dateRange->setFromPopup(_from);
    _dateRange->setToPopup(_to);
    _dateRange->setCurrentItem(DateRange::Today);
    _dateRange->setFocus();

    _store->setId(_quasar->defaultStore());
    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    setCaption(tr("Inquiry"));
    setCentralWidget(frame);
    finalize();
}

Inquiry::~Inquiry()
{
}

void
Inquiry::setCurrentTab(int tab)
{
    _tabs->setCurrentPage(tab);
}

void
Inquiry::setCurrentId(Id id)
{
    InquiryWindow* window = (InquiryWindow*)_tabs->currentPage();
    window->search->setId(id);
}

void
Inquiry::setCurrentDates(QDate from, QDate to)
{
    _from->setDate(from);
    _to->setDate(to);
}

void
Inquiry::setStoreId(Id store_id)
{
    _store->setId(store_id);
    slotStoreChange();
}

void
Inquiry::setAccountId(Id account_id)
{
    setCurrentTab(0);
    setCurrentId(account_id);
    slotRefresh();
}

void
Inquiry::setCardId(Id card_id)
{
    setCurrentTab(1);
    setCurrentId(card_id);
    slotRefresh();
}

void
Inquiry::setItemId(Id item_id)
{
    setCurrentTab(2);
    setCurrentId(item_id);
    slotRefresh();
}

void
Inquiry::slotRefresh()
{
    _account->needsRefresh = true;
    _card->needsRefresh = true;
    _item->needsRefresh = true;
    slotTabChange();
}

void
Inquiry::slotPrint()
{
    InquiryWindow* window = (InquiryWindow*)_tabs->currentPage();
    window->slotPrint();
}

void
Inquiry::slotEdit()
{
    InquiryWindow* window = (InquiryWindow*)_tabs->currentPage();
    ListViewItem* item = (ListViewItem*)window->list->currentItem();
    if (item == NULL) return;

    Gltx gltx;
    Id gltx_id = item->id;
    if (!_quasar->db()->lookup(gltx_id, gltx)) return;

    QWidget* edit = editGltx(gltx, _main);
    if (edit != NULL) edit->show();
}

void
Inquiry::slotTabChange()
{
    InquiryWindow* window = (InquiryWindow*)_tabs->currentPage();
    if (window == _account && _account->needsRefresh)
	slotAccountChange();
    if (window == _card && _card->needsRefresh)
	slotCardChange();
    if (window == _item && _item->needsRefresh)
	slotItemChange();
}

void
Inquiry::slotStoreChange()
{
    ItemLookup* lookup = (ItemLookup*)_item->search->lookupWindow();
    lookup->store_id = _store->getId();
}

void
Inquiry::slotAccountChange()
{
    _account->slotRefresh(_from->getDate(), _to->getDate(), _store->getId());
}

void
Inquiry::slotCardChange()
{
    _card->slotRefresh(_from->getDate(), _to->getDate(), _store->getId());
}

void
Inquiry::slotItemChange()
{
    _item->slotRefresh(_from->getDate(), _to->getDate(), _store->getId());
}

void
Inquiry::closeEvent(QCloseEvent* e)
{
    if (_loading)
	e->ignore();
    else
	QMainWindow::closeEvent(e);
}
