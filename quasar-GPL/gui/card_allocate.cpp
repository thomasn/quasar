// $Id: card_allocate.cpp,v 1.3 2004/12/06 18:41:23 bpepers Exp $
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
// See http://www.linuxcanada.com or email card@linuxcanada.com for
// information about Quasar Accounting support and maintenance options.
//
// Contact card@linuxcanada.com if any conditions of this licensing are
// not clear to you.

#include "card_allocate.h"
#include "main_window.h"
#include "screen_decl.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "card_lookup.h"
#include "lookup_edit.h"
#include "list_view_item.h"
#include "money_edit.h"
#include "customer.h"
#include "vendor.h"
#include "gltx_select.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qmessagebox.h>

CardAllocate::CardAllocate(MainWindow* main)
    : QuasarWindow(main, "CardAllocate")
{
    _helpSource = "card_allocate.html";

    QFrame* frame = new QFrame(this);

    QLabel* cardLabel = new QLabel(tr("Card:"), frame);
    _card = new LookupEdit(new CardLookup(main, this), frame);
    _card->setLength(30);
    cardLabel->setBuddy(_card);
    connect(_card, SIGNAL(validData()), SLOT(slotCardChanged()));

    QLabel* balanceLabel = new QLabel(tr("Balance:"), frame);
    _balance = new MoneyEdit(frame);
    _balance->setFocusPolicy(NoFocus);

    _tabs = new QTabWidget(frame);

    _creditFrame = new QFrame(_tabs);
    _tabs->addTab(_creditFrame, tr("Credits"));

    _creditList = new ListView(_creditFrame);
    _creditList->addTextColumn(tr("Tx #"), 10, AlignRight);
    _creditList->addDateColumn(tr("Date"));
    _creditList->addMoneyColumn(tr("Amount"));
    _creditList->addMoneyColumn(tr("Remaining"));
    _creditList->addMoneyColumn(tr("Allocate"));
    _creditList->setAllColumnsShowFocus(true);
    _creditList->setVScrollBarMode(QScrollView::AlwaysOn);
    connect(_creditList, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotPickLine()));

    QLabel* creditTotalLabel = new QLabel(tr("Total:"), _creditFrame);
    _creditTotal = new MoneyEdit(_creditFrame);
    _creditTotal->setFocusPolicy(NoFocus);

    QLabel* creditAllocLabel = new QLabel(tr("Allocated:"), _creditFrame);
    _creditAlloc = new MoneyEdit(_creditFrame);
    _creditAlloc->setFocusPolicy(NoFocus);

    QGridLayout* creditGrid = new QGridLayout(_creditFrame);
    creditGrid->setSpacing(6);
    creditGrid->setMargin(6);
    creditGrid->setRowStretch(0, 1);
    creditGrid->setColStretch(2, 1);
    creditGrid->addMultiCellWidget(_creditList, 0, 0, 0, 4);
    creditGrid->addWidget(creditTotalLabel, 1, 0, AlignLeft | AlignVCenter);
    creditGrid->addWidget(_creditTotal, 1, 1, AlignLeft | AlignVCenter);
    creditGrid->addWidget(creditAllocLabel, 1, 3, AlignLeft | AlignVCenter);
    creditGrid->addWidget(_creditAlloc, 1, 4, AlignLeft | AlignVCenter);

    _debitFrame = new QFrame(_tabs);
    _tabs->addTab(_debitFrame, tr("Debits"));

    _debitList = new ListView(_debitFrame);
    _debitList->addTextColumn(tr("Tx #"), 10, AlignRight);
    _debitList->addDateColumn(tr("Date"));
    _debitList->addMoneyColumn(tr("Amount"));
    _debitList->addMoneyColumn(tr("Remaining"));
    _debitList->addMoneyColumn(tr("Allocate"));
    _debitList->setAllColumnsShowFocus(true);
    _debitList->setVScrollBarMode(QScrollView::AlwaysOn);
    connect(_debitList, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotPickLine()));

    QLabel* debitTotalLabel = new QLabel(tr("Total:"), _debitFrame);
    _debitTotal = new MoneyEdit(_debitFrame);
    _debitTotal->setFocusPolicy(NoFocus);

    QLabel* debitAllocLabel = new QLabel(tr("Allocated:"), _debitFrame);
    _debitAlloc = new MoneyEdit(_debitFrame);
    _debitAlloc->setFocusPolicy(NoFocus);

    QGridLayout* debitGrid = new QGridLayout(_debitFrame);
    debitGrid->setSpacing(6);
    debitGrid->setMargin(6);
    debitGrid->setRowStretch(0, 1);
    debitGrid->setColStretch(2, 1);
    debitGrid->addMultiCellWidget(_debitList, 0, 0, 0, 4);
    debitGrid->addWidget(debitTotalLabel, 1, 0, AlignLeft | AlignVCenter);
    debitGrid->addWidget(_debitTotal, 1, 1, AlignLeft | AlignVCenter);
    debitGrid->addWidget(debitAllocLabel, 1, 3, AlignLeft | AlignVCenter);
    debitGrid->addWidget(_debitAlloc, 1, 4, AlignLeft | AlignVCenter);

    QFrame* buttons = new QFrame(frame);

    QPushButton* autoAlloc = new QPushButton(tr("&Auto Allocate"), buttons);
    connect(autoAlloc, SIGNAL(clicked()), SLOT(slotAutoAllocate()));

    QPushButton* post = new QPushButton(tr("&Post"), buttons);
    connect(post, SIGNAL(clicked()), SLOT(slotPost()));

    QPushButton* cancel = new QPushButton(tr("&Cancel"), buttons);
    connect(cancel, SIGNAL(clicked()), SLOT(slotCancel()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(1, 1);
    buttonGrid->addWidget(autoAlloc, 0, 0);
    buttonGrid->addWidget(post, 0, 2);
    buttonGrid->addWidget(cancel, 0, 3);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setRowStretch(2, 1);
    grid->setColStretch(2, 1);
    grid->addWidget(cardLabel, 0, 0, AlignLeft | AlignVCenter);
    grid->addWidget(_card, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(balanceLabel, 1, 0, AlignLeft | AlignVCenter);
    grid->addWidget(_balance, 1, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_tabs, 2, 2, 0, 3);
    grid->addMultiCellWidget(buttons, 3, 3, 0, 3);

    setCentralWidget(frame);
    setCaption(tr("Card Allocation"));
    finalize();
}

CardAllocate::~CardAllocate()
{
}

void
CardAllocate::setCardId(Id card_id)
{
    _card->setId(card_id);
    slotCardChanged();
}

void
CardAllocate::slotCardChanged()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    fixed balance = _db->cardBalance(_card->getId());
    _balance->setFixed(balance);

    GltxSelect conditions;
    conditions.card_id = _card->getId();
    conditions.activeOnly = true;
    conditions.unpaid = true;
    vector<Gltx> gltxs;
    _db->select(gltxs, conditions);

    // TODO: sort gltxs by date

    _creditList->clear();
    _debitList->clear();
    _credits.clear();
    _debits.clear();

    fixed creditTotal, debitTotal;
    for (unsigned int i = 0; i < gltxs.size(); ++i) {
	const Gltx& gltx = gltxs[i];
	fixed remain = gltx.cardTotal() - gltx.paymentTotal();

	ListViewItem* item;
	fixed sign = 1.0;
	if (remain < 0) {
	    item = new ListViewItem(_debitList, gltx.id());
	    _debits.push_back(gltx);
	    sign = -1.0;
	    debitTotal += remain * sign;
	} else {
	    item = new ListViewItem(_creditList, gltx.id());
	    _credits.push_back(gltx);
	    creditTotal += remain;
	}

	item->setValue(0, gltx.number());
	item->setValue(1, gltx.postDate());
	item->setValue(2, gltx.cardTotal() * sign);
	item->setValue(3, remain * sign);
    }

    _creditTotal->setFixed(creditTotal);
    _creditAlloc->setFixed(0);
    _debitTotal->setFixed(debitTotal);
    _debitAlloc->setFixed(0);

    QApplication::restoreOverrideCursor();
}

void
CardAllocate::slotPickLine()
{
    ListViewItem* item;
    if (_tabs->currentPage() == _creditFrame)
	item = (ListViewItem*)_creditList->currentItem();
    else
	item = (ListViewItem*)_debitList->currentItem();
    if (item == NULL) return;

    Gltx gltx;
    Id gltx_id = item->id;
    if (!_quasar->db()->lookup(gltx_id, gltx)) return;

    QWidget* edit = editGltx(gltx, _main);
    if (edit != NULL) edit->show();
}

void
CardAllocate::slotAutoAllocate()
{
    fixed creditTotal = _creditTotal->getFixed();
    if (creditTotal == 0.0) {
	QString message = tr("No credits to allocate to");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    fixed debitTotal = _debitTotal->getFixed();
    if (debitTotal == 0.0) {
	QString message = tr("No debits to allocate from");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QString message = tr("Auto allocate to oldest?");
    int choice = QMessageBox::information(this, tr("Auto Allocate?"), message,
					  tr("Yes"), tr("No"));
    if (choice != 0) return;

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Clear out existing allocations
    ListViewItem* item = _creditList->firstChild();
    while (item != NULL) {
	item->setValue(4, "");
	item = item->nextSibling();
    }
    item = _debitList->firstChild();
    while (item != NULL) {
	item->setValue(4, "");
	item = item->nextSibling();
    }

    // Try to allocate each credit
    item = _creditList->firstChild();
    while (item != NULL) {
	fixed remain = item->value(3).toFixed();
	fixed alloc = item->value(4).toFixed();

	ListViewItem* debitItem = _debitList->firstChild();
	while (debitItem != NULL) {
	    fixed debitRemain = debitItem->value(3).toFixed();
	    fixed debitAlloc = debitItem->value(4).toFixed();

	    if (debitRemain != debitAlloc) {
		fixed amount = remain - alloc;
		if (amount > debitRemain - debitAlloc)
		    amount = debitRemain - debitAlloc;

		debitAlloc += amount;
		alloc += amount;

		debitItem->setValue(4, debitAlloc);
		item->setValue(4, alloc);
	    }
	    debitItem = debitItem->nextSibling();
	}
	item = item->nextSibling();
    }

    fixed creditAlloc = 0;
    item = _creditList->firstChild();
    while (item != NULL) {
	creditAlloc += item->value(4).toFixed();
	item = item->nextSibling();
    }

    fixed debitAlloc = 0;
    item = _debitList->firstChild();
    while (item != NULL) {
	debitAlloc += item->value(4).toFixed();
	item = item->nextSibling();
    }

    _creditAlloc->setFixed(creditAlloc);
    _debitAlloc->setFixed(debitAlloc);

    QApplication::restoreOverrideCursor();
}

void
CardAllocate::slotPost()
{
    fixed creditAlloc = _creditAlloc->getFixed();
    fixed debitAlloc = _debitAlloc->getFixed();

    if (creditAlloc != debitAlloc) {
	QString message = tr("Allocated credits doesn't match debits");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    if (creditAlloc == 0) {
	QString message = tr("Nothing allocated");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    // TODO: validate alloc <= remain in all cases

    QString message = tr("Are you sure you want to post the allocations?");
    int choice = QMessageBox::warning(this, tr("Warning"), message,
				      tr("Yes"), tr("No"));
    if (choice != 0) return;

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    ListViewItem* item = _creditList->firstChild();
    ListViewItem* firstDebitItem = _debitList->firstChild();
    while (item != NULL) {
	fixed remain = item->value(3).toFixed();
	fixed alloc = item->value(4).toFixed();
	if (remain == 0) continue;

	ListViewItem* debitItem = firstDebitItem;
	while (debitItem != NULL) {
	    fixed debitRemain = debitItem->value(3).toFixed();
	    fixed debitAlloc = debitItem->value(4).toFixed();

	    if (debitAlloc != 0) {
		fixed amount = debitAlloc;
		if (amount > alloc)
		    amount = alloc;

		Id debitId = debitItem->id;
		Id creditId = item->id;
		_db->allocPayment(debitId, creditId, -amount);

		debitRemain -= amount;
		debitAlloc -= amount;
		remain -= amount;
		alloc -= amount;

		debitItem->setValue(3, debitRemain);
		debitItem->setValue(4, debitAlloc);
		item->setValue(3, remain);
		item->setValue(4, alloc);
	    }

	    if (debitRemain == 0 && debitItem == firstDebitItem)
		firstDebitItem = debitItem->nextSibling();
	    if (remain == 0) break;

	    debitItem = debitItem->nextSibling();
	}
	item = item->nextSibling();
    }
    _db->commit();

    QApplication::restoreOverrideCursor();
    close();
}

void
CardAllocate::slotCancel()
{
    // TODO: ask if sure if allocations selected
    close();
}
