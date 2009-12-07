// $Id: price_master.cpp,v 1.20 2004/01/31 01:50:30 arandell Exp $
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

#include "price_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "item_edit.h"
#include "item_lookup.h"
#include "subdept_lookup.h"
#include "customer_lookup.h"
#include "vendor_lookup.h"
#include "store_lookup.h"
#include "group_lookup.h"
#include "date_popup.h"
#include "integer_edit.h"
#include "money_edit.h"
#include "percent_edit.h"
#include "price_edit.h"
#include "item.h"
#include "group.h"

#include <qapplication.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qwidgetstack.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qlayout.h>
#include <qwhatsthis.h>

PriceMaster::PriceMaster(MainWindow* main, Id price_id, bool isCost)
    : DataWindow(main, "PriceMaster", price_id), _cost(isCost)
{
    if (_cost)
	_helpSource = "cost_master.html";
    else
	_helpSource = "price_master.html";

    // Create widgets
    QGroupBox* item = new QGroupBox(tr("Item Selection"), _frame);
    QGridLayout* itemGrid = new QGridLayout(item, 5, 2, item->frameWidth()*2);
    itemGrid->addRowSpacing(0, item->fontMetrics().height());

    _itemIdLabel = new QRadioButton(tr("Item:"), item);
    _itemId = new ItemEdit(new ItemLookup(_main, this), item);
    _itemId->setLength(18, '9');
    connect(_itemId, SIGNAL(validData()), SLOT(slotItemIdChanged()));

    _itemGroupLabel = new QRadioButton(tr("Item Group:"), item);
    _itemGroup = new LookupEdit(new GroupLookup(_main, this, Group::ITEM),
				 item);
    _itemGroup->setLength(20);

    _subdeptIdLabel = new QRadioButton(tr("Subdepartment:"), item);
    _subdeptId = new LookupEdit(new SubdeptLookup(_main, this), item);
    _subdeptId->setLength(14);

    QLabel* sizeLabel = new QLabel(tr("Size:"), item);
    _sizes = new QWidgetStack(item);
    sizeLabel->setBuddy(_sizes);

    _comboSize = new QComboBox(_sizes);
    _editSize = new LineEdit(_sizes);
    _editSize->setMaxLength(8);
    _sizes->addWidget(_comboSize, 1);
    _sizes->addWidget(_editSize, 2);

    QButtonGroup* itemButtons = new QButtonGroup(this);
    itemButtons->hide();
    itemButtons->insert(_itemIdLabel);
    itemButtons->insert(_itemGroupLabel);
    itemButtons->insert(_subdeptIdLabel);
    connect(itemButtons, SIGNAL(clicked(int)), SLOT(slotItemChanged()));

    itemGrid->setRowStretch(5, 1);
    itemGrid->setColStretch(2, 1);
    itemGrid->addWidget(_itemIdLabel, 1, 0);
    itemGrid->addWidget(_itemId, 1, 1, AlignLeft | AlignVCenter);
    itemGrid->addWidget(_itemGroupLabel, 2, 0);
    itemGrid->addWidget(_itemGroup, 2, 1, AlignLeft | AlignVCenter);
    itemGrid->addWidget(_subdeptIdLabel, 3, 0);
    itemGrid->addWidget(_subdeptId, 3, 1, AlignLeft | AlignVCenter);
    itemGrid->addWidget(sizeLabel, 4, 0);
    itemGrid->addWidget(_sizes, 4, 1, AlignLeft | AlignVCenter);

    QGroupBox* card = new QGroupBox(tr("Card Selection"), _frame);
    QGridLayout* cardGrid = new QGridLayout(card, 4, 2, card->frameWidth()*2);
    cardGrid->addRowSpacing(0, card->fontMetrics().height());

    CardLookup* cardLookup;
    if (_cost) {
	_cardIdLabel = new QRadioButton(tr("Vendor:"), card);
	cardLookup = new VendorLookup(_main, this);
    } else {
	_cardIdLabel = new QRadioButton(tr("Customer:"), card);
	cardLookup = new CustomerLookup(_main, this);
    }
    _cardId = new LookupEdit(cardLookup, card);
    _cardId->setLength(30);

    int type;
    if (_cost) {
	_cardGroupLabel = new QRadioButton(tr("Vendor Group:"), card);
	type = Group::VENDOR;
    } else {
	_cardGroupLabel = new QRadioButton(tr("Customer Group:"), card);
	type = Group::CUSTOMER;
    }
    _cardGroup = new LookupEdit(new GroupLookup(_main, this, type), card);
    _cardGroup->setLength(20);

    QButtonGroup* cardButtons = new QButtonGroup(this);
    cardButtons->hide();
    cardButtons->insert(_cardIdLabel);
    cardButtons->insert(_cardGroupLabel);
    connect(cardButtons, SIGNAL(clicked(int)), SLOT(slotCardChanged()));

    cardGrid->setRowStretch(3, 1);
    cardGrid->setColStretch(2, 1);
    cardGrid->addWidget(_cardIdLabel, 1, 0);
    cardGrid->addWidget(_cardId, 1, 1, AlignLeft | AlignVCenter);
    cardGrid->addWidget(_cardGroupLabel, 2, 0);
    cardGrid->addWidget(_cardGroup, 2, 1, AlignLeft | AlignVCenter);

    QLabel* storeLabel = new QLabel(tr("Store:"), _frame);
    _store = new LookupEdit(new StoreLookup(_main, this), _frame);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    QFrame* mid = new QFrame(_frame);
    _promotion = new QCheckBox(tr("Promotion?"), mid);
    _discountable = new QCheckBox(tr("Discountable?"), mid);

    QLabel* startLabel = new QLabel(tr("Start Date:"), mid);
    _startDate = new DatePopup(mid);
    startLabel->setBuddy(_startDate);

    QLabel* stopLabel = new QLabel(tr("Stop Date:"), mid);
    _stopDate = new DatePopup(mid);
    stopLabel->setBuddy(_stopDate);

    QLabel* daysLabel = new QLabel(tr("Day of week:"), mid);
    _days = new LineEdit(mid);
    _days->setLength(8, 'Y');
    daysLabel->setBuddy(_days);

    QLabel* minQtyLabel = new QLabel(tr("Minimum Qty:"), mid);
    _minQty = new IntegerEdit(mid);
    _minQty->setLength(8);
    minQtyLabel->setBuddy(_minQty);

    QLabel* qtyLimitLabel = new QLabel(tr("Qty Limit:"), mid);
    _qtyLimit = new IntegerEdit(mid);
    _qtyLimit->setLength(8);
    qtyLimitLabel->setBuddy(_qtyLimit);

    QGridLayout* midGrid = new QGridLayout(mid);
    midGrid->setSpacing(3);
    midGrid->setColStretch(1, 1);
    midGrid->setColStretch(4, 1);
    midGrid->addWidget(_promotion, 0, 0);
    midGrid->addWidget(_discountable, 1, 0);
    midGrid->addWidget(startLabel, 0, 2);
    midGrid->addWidget(_startDate, 0, 3);
    midGrid->addWidget(stopLabel, 1, 2);
    midGrid->addWidget(_stopDate, 1, 3);
    midGrid->addWidget(daysLabel, 0, 5);
    midGrid->addWidget(_days, 0, 6, AlignLeft | AlignVCenter);
    midGrid->addWidget(minQtyLabel, 1, 5);
    midGrid->addWidget(_minQty, 1, 6, AlignLeft | AlignVCenter);
    midGrid->addWidget(qtyLimitLabel, 2, 5);
    midGrid->addWidget(_qtyLimit, 2, 6, AlignLeft | AlignVCenter);

    QFrame* bot = new QFrame(_frame);
    QGroupBox* price = new QGroupBox(tr("Price Method"), bot);
    QGridLayout* priceGrid = new QGridLayout(price,5,2,price->frameWidth()*2);
    priceGrid->addRowSpacing(0, price->fontMetrics().height());
    priceGrid->setColStretch(1, 1);

    _priceLabel = new QRadioButton(tr("Price:"), price);
    _price = new PriceEdit(price);
    _price->setLength(30);

    _costLabel = new QRadioButton(tr("Cost Plus:"), price);
    _costPlus = new PercentEdit(price);
    _costPlus->setLength(10);

    _percentLabel = new QRadioButton(tr("Percent Off:"), price);
    _percentOff = new PercentEdit(price);
    _percentOff->setLength(10);

    _dollarLabel = new QRadioButton(tr("Dollar Off:"), price);
    _dollarOff = new MoneyEdit(price);
    _dollarOff->setLength(10);

    QButtonGroup* priceButtons = new QButtonGroup(this);
    priceButtons->hide();
    priceButtons->insert(_priceLabel);
    priceButtons->insert(_costLabel);
    priceButtons->insert(_percentLabel);
    priceButtons->insert(_dollarLabel);
    connect(priceButtons, SIGNAL(clicked(int)), SLOT(slotPriceChanged()));

    priceGrid->setColStretch(2, 1);
    priceGrid->addWidget(_priceLabel, 1, 0);
    priceGrid->addWidget(_price, 1, 1, AlignLeft | AlignVCenter);
    priceGrid->addWidget(_costLabel, 2, 0);
    priceGrid->addWidget(_costPlus, 2, 1, AlignLeft | AlignVCenter);
    priceGrid->addWidget(_percentLabel, 3, 0);
    priceGrid->addWidget(_percentOff, 3, 1, AlignLeft | AlignVCenter);
    priceGrid->addWidget(_dollarLabel, 4, 0);
    priceGrid->addWidget(_dollarOff, 4, 1, AlignLeft | AlignVCenter);

    QGridLayout* botGrid = new QGridLayout(bot);
    botGrid->setSpacing(3);
    botGrid->setMargin(3);
    botGrid->addWidget(price, 0, 0);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->addMultiCellWidget(item, 0, 1, 0, 0);
    grid->addMultiCellWidget(card, 0, 0, 1, 2);
    grid->addWidget(storeLabel, 1, 1);
    grid->addWidget(_store, 1, 2);
    grid->addMultiCellWidget(mid, 2, 2, 0, 2);
    grid->addMultiCellWidget(bot, 3, 3, 0, 2);

    _itemIdLabel->setChecked(true);
    _cardIdLabel->setChecked(true);

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    if (_cost) {
	_costLabel->hide();
	_costPlus->hide();
    }
    _orig.setIsCost(_cost);
    _curr.setIsCost(_cost);

    if (_cost)
	setCaption(tr("Cost Master"));
    else
	setCaption(tr("Price Master"));
    finalize();
}

PriceMaster::~PriceMaster()
{
}

void
PriceMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _price;
}

void
PriceMaster::newItem()
{
    ItemPrice blank;
    _orig = blank;
    _orig.setIsCost(_cost);

    _curr = _orig;
    _firstField = _itemIdLabel;
}

void
PriceMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
}

bool
PriceMaster::fileItem()
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
PriceMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
PriceMaster::restoreItem()
{
    _curr = _orig;
}

void
PriceMaster::cloneItem()
{
    PriceMaster* clone = new PriceMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
PriceMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
PriceMaster::dataToWidget()
{
    _itemId->setValue(Plu(_curr.itemId(), _curr.number()));
    _itemGroup->setId(_curr.itemGroup());
    _subdeptId->setId(_curr.subdeptId());
    _cardId->setId(_curr.cardId());
    _cardGroup->setId(_curr.cardGroup());
    _store->setId(_curr.storeId());
    _promotion->setChecked(_curr.isPromotion());
    _discountable->setChecked(_curr.isDiscountable());
    _startDate->setDate(_curr.startDate());
    _stopDate->setDate(_curr.stopDate());
    _days->setText(_curr.dayOfWeekString());
    _minQty->setFixed(_curr.minimumQty());
    _qtyLimit->setFixed(_curr.qtyLimit());
    _inactive->setChecked(!_curr.isActive());

    if (_curr.subdeptId() != INVALID_ID)
	_subdeptIdLabel->setChecked(true);
    else if (_curr.itemGroup() != INVALID_ID)
	_itemGroupLabel->setChecked(true);
    else
	_itemIdLabel->setChecked(true);

    if (_curr.cardGroup() != INVALID_ID)
	_cardGroupLabel->setChecked(true);
    else
	_cardIdLabel->setChecked(true);

    switch (_curr.method()) {
    case ItemPrice::PRICE:
	_price->setPrice(_curr.price());
	_priceLabel->setChecked(true);
	break;
    case ItemPrice::COST_PLUS:
	_costPlus->setFixed(_curr.percentDiscount());
	_costLabel->setChecked(true);
	break;
    case ItemPrice::PERCENT_OFF:
	_percentOff->setFixed(_curr.percentDiscount());
	_percentLabel->setChecked(true);
	break;
    case ItemPrice::DOLLAR_OFF:
	_dollarOff->setFixed(_curr.dollarDiscount());
	_dollarLabel->setChecked(true);
	break;
    }

    _comboSize->clear();
    slotItemIdChanged();
    _sizes->raiseWidget(_editSize);
    _editSize->setText(_curr.size());

    slotItemChanged();
    slotCardChanged();
    slotPriceChanged();
}

// Set the data object from the widgets.
void
PriceMaster::widgetToData()
{
    if (_itemIdLabel->isChecked()) {
	_curr.setItemId(_itemId->value().toPlu().itemId());
	_curr.setNumber(_itemId->value().toPlu().number());
    } else {
	_curr.setItemId(INVALID_ID);
	_curr.setNumber("");
    }
    if (_itemGroupLabel->isChecked())
	_curr.setItemGroup(_itemGroup->getId());
    else
	_curr.setItemGroup(INVALID_ID);
    if (_subdeptIdLabel->isChecked())
	_curr.setSubdeptId(_subdeptId->getId());
    else
	_curr.setSubdeptId(INVALID_ID);
    _curr.setSize(currentSize());
    if (_cardIdLabel->isChecked())
	_curr.setCardId(_cardId->getId());
    else
	_curr.setCardId(INVALID_ID);
    if (_cardGroupLabel->isChecked())
	_curr.setCardGroup(_cardGroup->getId());
    else
	_curr.setCardGroup(INVALID_ID);
    _curr.setStoreId(_store->getId());
    _curr.setPromotion(_promotion->isChecked());
    _curr.setDiscountable(_discountable->isChecked());
    _curr.setStartDate(_startDate->getDate());
    _curr.setStopDate(_stopDate->getDate());
    _curr.setDayOfWeek(_days->text());
    _curr.setMinimumQty(_minQty->getFixed());
    _curr.setQtyLimit(_qtyLimit->getFixed());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    if (_priceLabel->isChecked()) {
	_curr.setMethod(ItemPrice::PRICE);
	_curr.setPrice(_price->getPrice());
	_curr.setPercentDiscount(0.0);
	_curr.setDollarDiscount(0.0);
    } else if (_costLabel->isChecked()) {
	_curr.setMethod(ItemPrice::COST_PLUS);
	_curr.setPrice(Price());
	_curr.setPercentDiscount(_costPlus->getFixed());
	_curr.setDollarDiscount(0.0);
    } else if (_percentLabel->isChecked()) {
	_curr.setMethod(ItemPrice::PERCENT_OFF);
	_curr.setPrice(Price());
	_curr.setPercentDiscount(_percentOff->getFixed());
	_curr.setDollarDiscount(0.0);
    } else {
	_curr.setMethod(ItemPrice::DOLLAR_OFF);
	_curr.setPrice(Price());
	_curr.setPercentDiscount(0.0);
	_curr.setDollarDiscount(_dollarOff->getFixed());
    }
}

QString
PriceMaster::currentSize()
{
    if (_sizes->id(_sizes->visibleWidget()) == 1)
	return _comboSize->currentText();
    return _editSize->text();
}

void
PriceMaster::slotItemChanged()
{
    _itemId->setEnabled(_itemIdLabel->isChecked());
    _itemGroup->setEnabled(_itemGroupLabel->isChecked());
    _subdeptId->setEnabled(_subdeptIdLabel->isChecked());

    QString size = currentSize();
    if (_itemIdLabel->isChecked()) {
	_sizes->raiseWidget(_comboSize);
	bool found = false;
	for (int i = 0; i < _comboSize->count(); ++i) {
	    if (_comboSize->text(i) == size) {
		_comboSize->setCurrentItem(i);
		found = true;
	    }
	}
	if (!found && _comboSize->count() > 0)
	    _comboSize->setCurrentItem(0);
    } else {
	_sizes->raiseWidget(_editSize);
	_editSize->setText(size);
    }
}

void
PriceMaster::slotItemIdChanged()
{
    Item item;
    _quasar->db()->lookup(_itemId->getId(), item);
    QString size;
    if (_cost)
	size = item.purchaseSize();
    else
	size = item.sellSize();
    _editSize->setText(size);

    _comboSize->clear();
    if (_costLabel->isChecked() || _percentLabel->isChecked()) {
	_comboSize->insertItem("");
	if (size.isEmpty())
	    _comboSize->setCurrentItem(_comboSize->count() - 1);
    }

    for (unsigned int i = 0; i < item.sizes().size(); ++i) {
	_comboSize->insertItem(item.sizes()[i].name);
	if (item.sizes()[i].name == size)
	    _comboSize->setCurrentItem(_comboSize->count() - 1);
    }
}

void
PriceMaster::slotCardChanged()
{
    _cardId->setEnabled(_cardIdLabel->isChecked());
    _cardGroup->setEnabled(_cardGroupLabel->isChecked());
}

void
PriceMaster::slotPriceChanged()
{
    _price->setEnabled(_priceLabel->isChecked());
    _costPlus->setEnabled(_costLabel->isChecked());
    _percentOff->setEnabled(_percentLabel->isChecked());
    _dollarOff->setEnabled(_dollarLabel->isChecked());

    Item item;
    _quasar->db()->lookup(_itemId->getId(), item);
    QString size = currentSize();

    _comboSize->clear();
    if (_costLabel->isChecked() || _percentLabel->isChecked()) {
	_comboSize->insertItem("");
	if (size.isEmpty())
	    _comboSize->setCurrentItem(_comboSize->count() - 1);
    }

    for (unsigned int i = 0; i < item.sizes().size(); ++i) {
	_comboSize->insertItem(item.sizes()[i].name);
	if (item.sizes()[i].name == size)
	    _comboSize->setCurrentItem(_comboSize->count() - 1);
    }
}
