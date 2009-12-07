// $Id: recurring_master.cpp,v 1.18 2005/02/11 18:44:27 bpepers Exp $
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

#include "recurring_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "general.h"
#include "item_adjust.h"
#include "invoice.h"
#include "receive.h"
#include "cheque.h"
#include "store.h"
#include "card.h"
#include "group.h"
#include "item.h"
#include "recurring_select.h"
#include "card_select.h"
#include "lookup_edit.h"
#include "date_popup.h"
#include "gltx_lookup.h"
#include "group_lookup.h"
#include "combo_box.h"
#include "integer_edit.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qmessagebox.h>

RecurringMaster::RecurringMaster(MainWindow* main, Id recurring_id)
    : DataWindow(main, "RecurringMaster", recurring_id)
{
    _helpSource = "recurring_master.html";

    // Post button
    QPushButton* posting = new QPushButton(tr("Post"), _buttons);
    connect(posting, SIGNAL(clicked()), SLOT(slotPost()));

    // Create widgets
    QGroupBox* tx = new QGroupBox(tr("Transaction"), _frame);

    QLabel* typeLabel = new QLabel(tr("Type:"), tx);
    _type = new LineEdit(20, tx);
    _type->setFocusPolicy(NoFocus);

    QLabel* numberLabel = new QLabel(tr("Id #:"), tx);
    _number = new LineEdit(10, tx);
    _number->setFocusPolicy(NoFocus);

    QLabel* storeLabel = new QLabel(tr("Store:"), tx);
    _store = new LineEdit(tx);
    _store->setFocusPolicy(NoFocus);

    QLabel* dateLabel = new QLabel(tr("Date:"), tx);
    _date = new LineEdit(tx);
    _date->setFocusPolicy(NoFocus);

    QLabel* descLabel = new QLabel(tr("Description:"), tx);
    _desc = new LineEdit(30, tx);
    descLabel->setBuddy(_desc);

    _cardLabel = new QRadioButton(tr("Card:"), tx);
    _card = new LineEdit(tx);
    _card->setFocusPolicy(NoFocus);
    connect(_cardLabel, SIGNAL(toggled(bool)), SLOT(slotTypeChanged()));

    _groupLabel = new QRadioButton(tr("Group:"), tx);
    _groupLookup = new GroupLookup(main, this, Group::CUSTOMER);
    _group = new LookupEdit(_groupLookup, tx);
    connect(_groupLabel, SIGNAL(toggled(bool)), SLOT(slotTypeChanged()));

    QButtonGroup* types = new QButtonGroup(this);
    types->hide();
    types->insert(_cardLabel);
    types->insert(_groupLabel);

    QGridLayout* txGrid = new QGridLayout(tx);
    txGrid->setMargin(6);
    txGrid->setSpacing(6);
    txGrid->setColStretch(2, 1);
    txGrid->addRowSpacing(0, tx->fontMetrics().height());
    txGrid->addWidget(typeLabel, 1, 0);
    txGrid->addWidget(_type, 1, 1);
    txGrid->addWidget(storeLabel, 1, 3);
    txGrid->addWidget(_store, 1, 4);
    txGrid->addWidget(numberLabel, 2, 0);
    txGrid->addWidget(_number, 2, 1);
    txGrid->addWidget(dateLabel, 2, 3);
    txGrid->addWidget(_date, 2, 4);
    txGrid->addWidget(descLabel, 3, 0);
    txGrid->addMultiCellWidget(_desc, 3, 3, 1, 4);
    txGrid->addWidget(_cardLabel, 4, 0);
    txGrid->addMultiCellWidget(_card, 4, 4, 1, 4);
    txGrid->addWidget(_groupLabel, 5, 0);
    txGrid->addMultiCellWidget(_group, 5, 5, 1, 4);

    QGroupBox* freq = new QGroupBox(tr("Frequency"), _frame);

    QLabel* freqLabel = new QLabel(tr("Type:"), freq);
    _freq = new ComboBox(false, freq);
    freqLabel->setBuddy(_freq);

    _freq->insertItem(tr("Daily"));
    _freq->insertItem(tr("Weekly"));
    _freq->insertItem(tr("Bi-Weekly"));
    _freq->insertItem(tr("Semi-Monthly"));
    _freq->insertItem(tr("Monthly"));
    _freq->insertItem(tr("Quarterly"));
    _freq->insertItem(tr("Semi-Anually"));
    _freq->insertItem(tr("Anually"));
    _freq->insertItem(tr("Other"));

    QLabel* maxPostLabel = new QLabel(tr("Max Postings:"), freq);
    _maxPost = new IntegerEdit(freq);
    maxPostLabel->setBuddy(_maxPost);

    QLabel* day1Label = new QLabel(tr("Day 1:"), freq);
    _day1 = new IntegerEdit(freq);
    day1Label->setBuddy(_day1);

    QLabel* day2Label = new QLabel(tr("Day 2:"), freq);
    _day2 = new IntegerEdit(freq);
    day2Label->setBuddy(_day2);

    QGridLayout* freqGrid = new QGridLayout(freq);
    freqGrid->setMargin(6);
    freqGrid->setSpacing(6);
    freqGrid->setColStretch(2, 1);
    freqGrid->addRowSpacing(0, freq->fontMetrics().height());
    freqGrid->addWidget(freqLabel, 1, 0);
    freqGrid->addWidget(_freq, 1, 1);
    freqGrid->addWidget(maxPostLabel, 2, 0);
    freqGrid->addWidget(_maxPost, 2, 1);
    freqGrid->addWidget(day1Label, 1, 3);
    freqGrid->addWidget(_day1, 1, 4);
    freqGrid->addWidget(day2Label, 2, 3);
    freqGrid->addWidget(_day2, 2, 4);

    QGroupBox* post = new QGroupBox(tr("Postings"), _frame);

    QLabel* lastPostLabel = new QLabel(tr("Last Posted:"), post);
    _lastPost = new DatePopup(post);
    lastPostLabel->setBuddy(_lastPost);

    QLabel* nextDueLabel = new QLabel(tr("Next Due:"), post);
    _nextDue = new LineEdit(post);
    _nextDue->setFocusPolicy(NoFocus);
    nextDueLabel->setBuddy(_nextDue);

    QLabel* postCntLabel = new QLabel(tr("Posting Count:"), post);
    _postCnt = new IntegerEdit(post);
    postCntLabel->setBuddy(_postCnt);

    QLabel* overdueLabel = new QLabel(tr("Overdue Days:"), post);
    _overdue = new IntegerEdit(post);
    _overdue->setFocusPolicy(NoFocus);
    overdueLabel->setBuddy(_overdue);

    QGridLayout* postGrid = new QGridLayout(post);
    postGrid->setMargin(6);
    postGrid->setSpacing(6);
    postGrid->setColStretch(2, 1);
    postGrid->addRowSpacing(0, post->fontMetrics().height());
    postGrid->addWidget(lastPostLabel, 1, 0);
    postGrid->addWidget(_lastPost, 1, 1);
    postGrid->addWidget(nextDueLabel, 2, 0);
    postGrid->addWidget(_nextDue, 2, 1);
    postGrid->addWidget(postCntLabel, 1, 3);
    postGrid->addWidget(_postCnt, 1, 4);
    postGrid->addWidget(overdueLabel, 2, 3);
    postGrid->addWidget(_overdue, 2, 4);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->addWidget(tx, 0, 0);
    grid->addWidget(freq, 1, 0);
    grid->addWidget(post, 2, 0);

    connect(_freq, SIGNAL(activated(int)), SLOT(slotRefresh()));

    setCaption(tr("Recurring Transaction"));
    finalize();
}

RecurringMaster::~RecurringMaster()
{
}

void
RecurringMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _freq;
}

void
RecurringMaster::newItem()
{
    Recurring blank;
    _orig = blank;

    _curr = _orig;
    _firstField = _freq;
}

void
RecurringMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
}

bool
RecurringMaster::fileItem()
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
RecurringMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
RecurringMaster::restoreItem()
{
    _curr = _orig;
}

void
RecurringMaster::cloneItem()
{
    RecurringMaster* clone = new RecurringMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
RecurringMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
RecurringMaster::dataToWidget()
{
    _desc->setText(_curr.description());
    _freq->setCurrentItem(_curr.frequency());
    _day1->setInt(_curr.day1());
    _day2->setInt(_curr.day2());
    _maxPost->setInt(_curr.maxPostings());
    _lastPost->setDate(_curr.lastPosted());
    _postCnt->setInt(_curr.postingCount());
    _group->setId(_curr.cardGroup());
    _inactive->setChecked(!_curr.isActive());

    if (_maxPost->getInt() == 0) _maxPost->setText("");
    if (_postCnt->getInt() == 0) _postCnt->setText("");

    setGltx(_curr.gltxId());
    slotRefresh();
}

// Set the data object from the widgets.
void
RecurringMaster::widgetToData()
{
    _curr.setDescription(_desc->text());
    _curr.setFrequency(_freq->currentItem());
    _curr.setDay1(_day1->getInt());
    _curr.setDay2(_day2->getInt());
    _curr.setMaxPostings(_maxPost->getInt());
    _curr.setLastPosted(_lastPost->getDate());
    if (_groupLabel->isChecked())
        _curr.setCardGroup(_group->getId());
    else
        _curr.setCardGroup(INVALID_ID);
    _quasar->db()->setActive(_curr, !_inactive->isChecked());
}

void
RecurringMaster::setGltx(Id gltx_id)
{
    Gltx gltx;
    _quasar->db()->lookup(gltx_id, gltx);

    Store store;
    _quasar->db()->lookup(gltx.storeId(), store);

    _type->setText(gltx.dataTypeName());
    _number->setText(gltx.number());
    _store->setText(store.name());
    _date->setText(DateValcon().format(gltx.postDate()));

    Card card;
    _quasar->db()->lookup(gltx.cardId(), card);
    _card->setText(card.nameFL());

    if (_curr.cardGroup() == INVALID_ID) {
	_cardLabel->setChecked(true);
    } else {
        _groupLabel->setChecked(true);
    }

    if (_lastPost->getDate().isNull()) {
	_lastPost->setDate(gltx.postDate());
	slotRefresh();
    }

    switch (gltx.dataType()) {
    case DataObject::INVOICE:
	_cardLabel->setText(tr("Customer:"));
	_groupLabel->setEnabled(true);
	_group->setEnabled(true);
	_groupLookup->type->setCurrentItem(Group::typeName(Group::CUSTOMER));
	break;
    case DataObject::RECEIVE:
	_cardLabel->setText(tr("Vendor:"));
	_groupLabel->setEnabled(true);
	_group->setEnabled(true);
	_groupLookup->type->setCurrentItem(Group::typeName(Group::VENDOR));
	break;
    default:
	_cardLabel->setText(tr("Card:"));
	_groupLabel->setEnabled(false);
	_group->setEnabled(false);
    }

    _curr.setGltxId(gltx_id);
}

void
RecurringMaster::slotTypeChanged()
{
    if (_cardLabel->isChecked()) {
        _card->setEnabled(true);
        _group->setEnabled(false);
    } else {
        _card->setEnabled(false);
        _group->setEnabled(true);
    }
}

void
RecurringMaster::slotRefresh()
{
    _day1->setEnabled(_freq->currentItem() == Recurring::SemiMonthly);
    _day2->setEnabled(_freq->currentItem() == Recurring::SemiMonthly);

    QDate last = _lastPost->getDate();
    if (last.isNull()) {
	_nextDue->setText("");
	_overdue->setText("");
    } else {
	Recurring recurring;
	recurring.setFrequency(_freq->currentItem());
	recurring.setDay1(_day1->getInt());
	recurring.setDay2(_day2->getInt());
	recurring.setLastPosted(last);

	_nextDue->setText(DateValcon().format(recurring.nextDue()));
	_overdue->setInt(recurring.overdueDays());
    }
}

void
RecurringMaster::slotPost()
{
    if (!saveItem(true)) return;

    Gltx gltx;
    if (!_quasar->db()->lookup(_curr.gltxId(), gltx)) {
	QMessageBox::critical(this, tr("Error"), tr("Can't read transaction"));
	return;
    }

    QDate nextDue = _curr.nextDue();
    if (nextDue > QDate::currentDate()) {
	QString message = tr("The next due date is in the future\n"
			     "so this will post a future transaction.\n"
			     "Are you sure you want to do this?");
	int choice = QMessageBox::warning(this, tr("Warning"), message,
					  tr("Yes"), tr("No"));
	if (choice != 0) return;
    }

    if (_curr.cardGroup() != INVALID_ID) {
        CardSelect conditions;
	conditions.group_id = _curr.cardGroup();
	vector<Card> cards;
	_quasar->db()->select(cards, conditions);
	for (unsigned int i = 0; i < cards.size(); ++i)
	    if (!postOne(gltx, cards[i].id()))
	        return;
    } else {
        if (!postOne(gltx, gltx.cardId()))
	    return;
    }

    int postingCnt = _curr.postingCount();
    int maxPostings = _curr.maxPostings();

    Recurring old = _curr;
    _curr.setLastPosted(nextDue);
    _curr.setPostingCount(++postingCnt);
    if (maxPostings > 0 && postingCnt >= maxPostings)
	_quasar->db()->setActive(_curr, false);
    _quasar->db()->update(old, _curr);

    _id = _curr.id();
    startEdit();
}

bool
RecurringMaster::postOne(const Gltx& gltx, Id card_id)
{
    QDate nextDue = _curr.nextDue();
    Card card;
    _quasar->db()->lookup(card_id, card);

    General general;
    ItemAdjust adjustment;
    Invoice invoice;
    Receive receive;
    Cheque cheque;
    unsigned int i;

    switch (gltx.dataType()) {
    case DataObject::GENERAL:
	_quasar->db()->lookup(_curr.gltxId(), general);
	_quasar->db()->setActive(general, true);
	general.setNumber("#");
	general.setShiftId(INVALID_ID);
	general.setPostDate(nextDue);
	general.setPrinted(false);
	general.setPaid(false);
	general.payments().clear();

	if (!_quasar->db()->create(general)) {
	    QMessageBox::critical(this, tr("Error"), tr("Create failed"));
	    return false;
	}
	break;
    case DataObject::ITEM_ADJUST:
	_quasar->db()->lookup(_curr.gltxId(), adjustment);
	_quasar->db()->setActive(adjustment, true);
	adjustment.setNumber("#");
	adjustment.setShiftId(INVALID_ID);
	adjustment.setPostDate(nextDue);
	adjustment.setPrinted(false);
	adjustment.setPaid(false);
	adjustment.payments().clear();

	if (!_quasar->db()->create(adjustment)) {
	    QMessageBox::critical(this, tr("Error"), tr("Create failed"));
	    return false;
	}
	break;
    case DataObject::INVOICE:
	_quasar->db()->lookup(_curr.gltxId(), invoice);
	_quasar->db()->setActive(invoice, true);
	invoice.setNumber("#");
	invoice.setCustomerId(card_id);
	invoice.setCustomerAddress(card.address());
	invoice.setShiftId(INVALID_ID);
	invoice.setPostDate(nextDue);
	invoice.setPrinted(false);
	invoice.setPaid(false);
	invoice.payments().clear();

	// Recalculate item costs
	for (i = 0; i < invoice.items().size(); ++i) {
	    InvoiceItem& line = invoice.items()[i];
	    if (line.voided || line.item_id == INVALID_ID) continue;

	    Id store = invoice.storeId();
	    fixed qty = fabs(line.quantity);
	    fixed sign = line.quantity.sign();

	    Item item;
	    _quasar->db()->lookup(line.item_id, item);

	    fixed sell_cost;
	    _quasar->db()->itemSellingCost(item, line.size, store, qty,
					   line.ext_price, sell_cost);
	    line.inv_cost = sell_cost * sign;
	}

	// Fix gltx_card lines
	for (i = 0; i < invoice.cards().size(); ++i)
	    invoice.cards()[i].card_id = card_id;

	if (!_quasar->db()->create(invoice)) {
	    QMessageBox::critical(this, tr("Error"), tr("Create failed"));
	    return false;
	}
	break;
    case DataObject::RECEIVE:
	_quasar->db()->lookup(_curr.gltxId(), receive);
	_quasar->db()->setActive(receive, true);
	receive.setNumber("#");
	receive.setVendorId(card_id);
	receive.setVendorAddress(card.address());
	receive.setShiftId(INVALID_ID);
	receive.setPostDate(nextDue);
	receive.setInvoiceDate(nextDue);
	receive.setPrinted(false);
	receive.setPaid(false);
	receive.setClosed(false);
	receive.payments().clear();

	// Fix gltx_card lines
	for (i = 0; i < receive.cards().size(); ++i)
	    receive.cards()[i].card_id = card_id;

	if (!_quasar->db()->create(receive)) {
	    QMessageBox::critical(this, tr("Error"), tr("Create failed"));
	    return false;
	}
	break;
    case DataObject::CHEQUE:
	_quasar->db()->lookup(_curr.gltxId(), cheque);
	_quasar->db()->setActive(cheque, true);
	cheque.setNumber("#");
	cheque.setCardId(card_id);
	cheque.setAddress(card.address());
	cheque.setShiftId(INVALID_ID);
	cheque.setPostDate(nextDue);
	cheque.setPrinted(false);
	cheque.setPaid(false);
	cheque.payments().clear();

	if (!_quasar->db()->create(cheque)) {
	    QMessageBox::critical(this, tr("Error"), tr("Create failed"));
	    return false;
	}
	break;
    default:
	QString message = tr("Can't post %1 transactions yet")
	    .arg(gltx.dataTypeName());
	QMessageBox::critical(this, tr("Error"), message);
	return false;
    }

    return true;
}
