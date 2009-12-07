// $Id: price_list.cpp,v 1.20 2004/10/15 05:18:52 bpepers Exp $
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

#include "price_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "item_edit.h"
#include "item_lookup.h"
#include "card_lookup.h"
#include "promo_batch_lookup.h"
#include "store_lookup.h"
#include "item_price_select.h"
#include "price_master.h"
#include "card.h"
#include "item.h"
#include "subdept.h"
#include "group.h"
#include "list_view.h"
#include "percent_valcon.h"
#include "money_valcon.h"
#include "grid.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qcheckbox.h>

PriceList::PriceList(MainWindow* main, bool prices)
    : ActiveList(main, "PriceList", true)
{
    _helpSource = "price_cost_list.html";

    // List for prices
    _tabs->changeTab(_list, tr("Prices"));
    _list->addTextColumn(tr("Item Type"), 12);
    _list->addTextColumn(tr("Name"), 12);
    _list->addTextColumn(tr("Size"), 8);
    _list->addTextColumn(tr("Card Type"), 12);
    _list->addTextColumn(tr("Name"), 12);
    _list->addTextColumn(tr("Price"), 24);
    _list->setSorting(0);

    // List for costs
    _cost = new ListView(_tabs);
    _tabs->addTab(_cost, tr("Costs"));
    _cost->setAllColumnsShowFocus(true);
    _cost->setRootIsDecorated(false);
    _cost->setShowSortIndicator(true);
    _cost->addTextColumn(tr("Item Type"), 12);
    _cost->addTextColumn(tr("Name"), 12);
    _cost->addTextColumn(tr("Size"), 8);
    _cost->addTextColumn(tr("Card Type"), 12);
    _cost->addTextColumn(tr("Name"), 12);
    _cost->addTextColumn(tr("Cost"), 24);
    _cost->setSorting(0);
    connectList(_cost);

    QLabel* itemLabel = new QLabel(tr("Item:"), _search);
    _item = new ItemEdit(new ItemLookup(_main, this), _search);
    _item->setLength(18, '9');
    itemLabel->setBuddy(_item);

    QLabel* cardLabel = new QLabel(tr("Card:"), _search);
    _card = new LookupEdit(new CardLookup(_main, this), _search);
    _card->setLength(30);
    cardLabel->setBuddy(_card);

    QLabel* batchLabel = new QLabel(tr("Promo Batch:"), _search);
    _batch = new LookupEdit(new PromoBatchLookup(main, this), _search);
    _batch->setLength(10);

    QLabel* storeLabel = new QLabel(tr("Store:"), _search);
    _store = new LookupEdit(new StoreLookup(main, this), _search);
    _store->setLength(30);

    QGridLayout* grid = new QGridLayout(_search);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(2, 1);
    grid->addWidget(itemLabel, 0, 0);
    grid->addWidget(_item, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(cardLabel, 1, 0);
    grid->addWidget(_card, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(batchLabel, 0, 3);
    grid->addWidget(_batch, 0, 4, AlignLeft | AlignVCenter);
    grid->addWidget(storeLabel, 1, 3);
    grid->addWidget(_store, 1, 4, AlignLeft | AlignVCenter);

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    if (!prices)
	_tabs->setCurrentPage(1);

    setCaption(tr("Prices and Costs"));
    finalize(false);
    _item->setFocus();
}

PriceList::~PriceList()
{
}

void
PriceList::setStoreId(Id store_id)
{
    _store->setId(store_id);
}

void
PriceList::setItemId(Id item_id)
{
    _item->setId(item_id);
    slotRefresh();
}

void
PriceList::setCardId(Id card_id)
{
    _card->setId(card_id);
}

bool
PriceList::isActive(Id price_id)
{
    ItemPrice price;
    _quasar->db()->lookup(price_id, price);
    return price.isActive();
}

void
PriceList::setActive(Id price_id, bool active)
{
    if (price_id == INVALID_ID) return;

    ItemPrice price;
    _quasar->db()->lookup(price_id, price);

    ItemPrice orig = price;
    _quasar->db()->setActive(price, active);
    _quasar->db()->update(orig, price);
}

void
PriceList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::ITEM_PRICE)
	slotRefresh();
}

void
PriceList::clearLists()
{
    _list->clear();
    _cost->clear();
}

void
PriceList::slotInactiveChanged()
{
    clearLists();

    int columns = _list->columns();
    if (_inactive->isChecked()) {
	_list->addCheckColumn(tr("Inactive?"));
	_cost->addCheckColumn(tr("Inactive?"));
	resize(width() + _list->columnWidth(columns), height());
    } else {
	int column = _list->findColumn(tr("Inactive?"));
	if (column != -1) {
	    resize(width() - _list->columnWidth(column), height());
	    _list->removeColumn(column);
	    _cost->removeColumn(_cost->findColumn(tr("Inactive?")));
	}
    }

    slotRefresh();
}

void
PriceList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    int columns = _list->columns();
    if (showInactive && columns != 7) {
	_list->addCheckColumn(tr("Inactive?"));
	_cost->addCheckColumn(tr("Inactive?"));
	resize(width() + _list->columnWidth(6), height());
    }
    if (!showInactive && columns != 6) {
	resize(width() - _list->columnWidth(6), height());
	_list->removeColumn(6);
	_cost->removeColumn(6);
    }

    Id price_id = currentId();
    Id cost_id = currentId(_cost);
    QListViewItem* priceItem = NULL;
    QListViewItem* costItem = NULL;
    clearLists();

    Item item;
    _quasar->db()->lookup(_item->getId(), item);

    Card card;
    _quasar->db()->lookup(_card->getId(), card);

    ItemPriceSelect conditions;
    conditions.activeOnly = !showInactive;
    conditions.item = item;
    // TODO: set cardMatch
    conditions.card = card;
    conditions.batch_id = _batch->getId();
    conditions.store_id = _store->getId();
    vector<ItemPrice> prices;
    _quasar->db()->select(prices, conditions);

    for (unsigned int i = 0; i < prices.size(); i++) {
	ItemPrice& price = prices[i];

	QString itemType, itemName;
	if (price.itemId() != INVALID_ID) {
	    Item item;
	    _quasar->db()->lookup(price.itemId(), item);
	    itemType = tr("Item");
	    itemName = item.description();
	} else if (price.itemGroup() != INVALID_ID) {
	    Group group;
	    _quasar->db()->lookup(price.itemGroup(), group);
	    itemType = tr("Group");
	    itemName = group.name();
	} else if (price.subdeptId() != INVALID_ID) {
	    Subdept subdept;
	    _quasar->db()->lookup(price.subdeptId(), subdept);
	    itemType = tr("Subdepartment");
	    itemName = subdept.name();
	} else {
	    itemType = tr("All");
	}

	QString cardType, cardName;
	if (price.cardId() != INVALID_ID) {
	    Card card;
	    _quasar->db()->lookup(price.cardId(), card);
	    cardType = card.dataTypeName();
	    cardName = card.name();
	} else if (price.cardGroup() != INVALID_ID) {
	    Group group;
	    _quasar->db()->lookup(price.cardGroup(), group);
	    cardType = tr("Group");
	    cardName = group.name();
	} else {
	    cardType = tr("All");
	}

	QString priceText;
	PercentValcon percent;
	MoneyValcon money;
	switch (price.method()) {
	case ItemPrice::PRICE:
	    priceText = price.price().toString();
	    break;
	case ItemPrice::COST_PLUS:
	    priceText = tr("Cost+ ") + percent.format(price.percentDiscount());
	    break;
	case ItemPrice::PERCENT_OFF:
	    priceText = percent.format(price.percentDiscount()) + tr(" off");
	    break;
	case ItemPrice::DOLLAR_OFF:
	    priceText = money.format(price.dollarDiscount()) + tr(" off");
	    break;
	}

	ListViewItem* lvi;
	if (price.isCost())
	    lvi = new ListViewItem(_cost, price.id());
	else
	    lvi = new ListViewItem(_list, price.id());

	lvi->setValue(0, itemType);
	lvi->setValue(1, itemName);
	lvi->setValue(2, price.size());
	lvi->setValue(3, cardType);
	lvi->setValue(4, cardName);
	lvi->setValue(5, priceText);
	if (showInactive) lvi->setValue(6, !price.isActive());
	if (price.id() == price_id) priceItem = lvi;
	if (price.id() == cost_id) costItem = lvi;
    }

    if (priceItem == NULL) priceItem = _list->firstChild();
    if (costItem == NULL) costItem = _cost->firstChild();

    _list->setCurrentItem(priceItem);
    _list->setSelected(priceItem, true);
    _cost->setCurrentItem(costItem);
    _cost->setSelected(costItem, true);
}

void
PriceList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    ListView* list = currentList();
    QString title = tr("Price List");
    if (list == _cost) title = tr("Cost List");

    Grid* grid = Grid::buildGrid(list, title);

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
PriceList::performNew()
{
    ListView* list = currentList();

    bool isCost = (list == _cost);
    PriceMaster* master = new PriceMaster(_main, INVALID_ID, isCost);
    master->show();
}

void
PriceList::performEdit()
{
    ListView* list = currentList();
    Id price_id = currentId();

    bool isCost = (list == _cost);
    PriceMaster* master = new PriceMaster(_main, price_id, isCost);
    master->show();
}
