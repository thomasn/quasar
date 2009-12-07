// $Id: item_margin.cpp,v 1.9 2005/05/13 22:38:18 bpepers Exp $
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

#include "item_margin.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "object_cache.h"
#include "item_edit.h"
#include "item_lookup.h"
#include "store_lookup.h"
#include "price_edit.h"
#include "money_edit.h"
#include "percent_edit.h"
#include "combo_box.h"

#include <qapplication.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>

ItemMargin::ItemMargin(MainWindow* main)
    : QuasarWindow(main, "ItemMargin")
{
    _helpSource = "item_margin.html";

    QFrame* frame = new QFrame(this);

    QFrame* top = new QFrame(frame);

    _lookup = new ItemLookup(_main, this);
    _lookup->soldOnly = true;
    _lookup->store_id = _quasar->defaultStore();

    QLabel* itemLabel = new QLabel(tr("Item Number:"), top);
    _item = new ItemEdit(_lookup, top);
    _item->setLength(18, '9');
    itemLabel->setBuddy(_item);

    QLabel* descLabel = new QLabel(tr("Description:"), top);
    _desc = new LineEdit(top);
    _desc->setLength(30);
    _desc->setFocusPolicy(NoFocus);

    QLabel* storeLabel = new QLabel(tr("Store:"), top);
    _store = new LookupEdit(new StoreLookup(_main, this), top);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setMargin(3);
    topGrid->setSpacing(3);
    topGrid->setColStretch(2, 1);
    topGrid->addWidget(itemLabel, 0, 0);
    topGrid->addWidget(_item, 0, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(descLabel, 1, 0);
    topGrid->addWidget(_desc, 1, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(storeLabel, 2, 0);
    topGrid->addWidget(_store, 2, 1, AlignLeft | AlignVCenter);

    QGroupBox* price = new QGroupBox(tr("Price"), frame);
    QGridLayout* priceGrid = new QGridLayout(price,2,1,price->frameWidth()*2);
    priceGrid->addRowSpacing(0, price->fontMetrics().height());

    QLabel* priceSizeLabel = new QLabel(tr("Size:"), price);
    _priceSize = new ComboBox(price);
    priceSizeLabel->setBuddy(_priceSize);

    QLabel* priceLabel = new QLabel(tr("Price:"), price);
    _price = new PriceEdit(price);
    priceLabel->setBuddy(_price);

    QLabel* priceTaxLabel = new QLabel(tr("Tax:"), price);
    _priceTax = new MoneyEdit(price);
    _priceTax->setFocusPolicy(NoFocus);
    priceTaxLabel->setBuddy(_priceTax);

    QLabel* priceDepositLabel = new QLabel(tr("Deposit:"), price);
    _priceDeposit = new MoneyEdit(price);
    _priceDeposit->setFocusPolicy(NoFocus);
    priceDepositLabel->setBuddy(_priceDeposit);

    QLabel* priceBaseLabel = new QLabel(tr("Base:"), price);
    _priceBase = new MoneyEdit(price);
    priceBaseLabel->setBuddy(_priceBase);

    priceGrid->setColStretch(2, 1);
    priceGrid->addWidget(priceSizeLabel, 1, 0);
    priceGrid->addWidget(_priceSize, 1, 1, AlignLeft | AlignVCenter);
    priceGrid->addWidget(priceLabel, 2, 0);
    priceGrid->addWidget(_price, 2, 1, AlignLeft | AlignVCenter);
    priceGrid->addWidget(priceTaxLabel, 3, 0);
    priceGrid->addWidget(_priceTax, 3, 1, AlignLeft | AlignVCenter);
    priceGrid->addWidget(priceDepositLabel, 4, 0);
    priceGrid->addWidget(_priceDeposit, 4, 1, AlignLeft | AlignVCenter);
    priceGrid->addWidget(priceBaseLabel, 5, 0);
    priceGrid->addWidget(_priceBase, 5, 1, AlignLeft | AlignVCenter);

    QGroupBox* cost = new QGroupBox(tr("Rep Cost"), frame);
    QGridLayout* costGrid = new QGridLayout(cost,2,1,cost->frameWidth()*2);
    costGrid->addRowSpacing(0, cost->fontMetrics().height());

    QLabel* costSizeLabel = new QLabel(tr("Size:"), cost);
    _costSize = new ComboBox(cost);
    costSizeLabel->setBuddy(_costSize);

    QLabel* costLabel = new QLabel(tr("Cost:"), cost);
    _cost = new PriceEdit(cost);
    costLabel->setBuddy(_cost);

    QLabel* costTaxLabel = new QLabel(tr("Tax:"), cost);
    _costTax = new MoneyEdit(cost);
    _costTax->setFocusPolicy(NoFocus);
    costTaxLabel->setBuddy(_costTax);

    QLabel* costDepositLabel = new QLabel(tr("Deposit:"), cost);
    _costDeposit = new MoneyEdit(cost);
    _costDeposit->setFocusPolicy(NoFocus);
    costDepositLabel->setBuddy(_costDeposit);

    QLabel* costBaseLabel = new QLabel(tr("Base:"), cost);
    _costBase = new MoneyEdit(cost);
    costBaseLabel->setBuddy(_costBase);

    costGrid->setColStretch(2, 1);
    costGrid->addWidget(costSizeLabel, 1, 0);
    costGrid->addWidget(_costSize, 1, 1, AlignLeft | AlignVCenter);
    costGrid->addWidget(costLabel, 2, 0);
    costGrid->addWidget(_cost, 2, 1, AlignLeft | AlignVCenter);
    costGrid->addWidget(costTaxLabel, 3, 0);
    costGrid->addWidget(_costTax, 3, 1, AlignLeft | AlignVCenter);
    costGrid->addWidget(costDepositLabel, 4, 0);
    costGrid->addWidget(_costDeposit, 4, 1, AlignLeft | AlignVCenter);
    costGrid->addWidget(costBaseLabel, 5, 0);
    costGrid->addWidget(_costBase, 5, 1, AlignLeft | AlignVCenter);

    QGroupBox* margin = new QGroupBox(tr("Margin"), frame);
    QGridLayout* marginGrid = new QGridLayout(margin, 2, 1,
					      margin->frameWidth()*2);
    marginGrid->addRowSpacing(0, margin->fontMetrics().height());

    QLabel* marginSizeLabel = new QLabel(tr("Size:"), margin);
    _marginSize = new ComboBox(margin);
    marginSizeLabel->setBuddy(_marginSize);

    QLabel* marginPriceLabel = new QLabel(tr("Price:"), margin);
    _marginPrice = new MoneyEdit(margin);
    marginPriceLabel->setBuddy(_marginPrice);

    QLabel* targetLabel = new QLabel(tr("Target GM:"), margin);
    _targetGM = new PercentEdit(margin);
    _targetGM->setFocusPolicy(NoFocus);
    targetLabel->setBuddy(_targetGM);

    QLabel* repCostLabel = new QLabel(tr("Rep Cost:"), margin);
    _repCost = new MoneyEdit(margin);
    repCostLabel->setBuddy(_repCost);

    QLabel* repProfitLabel = new QLabel(tr("Rep Profit:"), margin);
    _repProfit = new MoneyEdit(margin);
    repProfitLabel->setBuddy(_repProfit);

    QLabel* repMarginLabel = new QLabel(tr("Rep Margin:"), margin);
    _repMargin = new PercentEdit(margin);
    repMarginLabel->setBuddy(_repMargin);

    QLabel* lastCostLabel = new QLabel(tr("Last Cost:"), margin);
    _lastCost = new MoneyEdit(margin);
    _lastCost->setFocusPolicy(NoFocus);
    lastCostLabel->setBuddy(_lastCost);

    QLabel* lastProfitLabel = new QLabel(tr("Last Profit:"), margin);
    _lastProfit = new MoneyEdit(margin);
    lastProfitLabel->setBuddy(_lastProfit);

    QLabel* lastMarginLabel = new QLabel(tr("Last Margin:"), margin);
    _lastMargin = new PercentEdit(margin);
    lastMarginLabel->setBuddy(_lastMargin);

    QLabel* avgCostLabel = new QLabel(tr("Avg Cost:"), margin);
    _avgCost = new MoneyEdit(margin);
    _avgCost->setFocusPolicy(NoFocus);
    avgCostLabel->setBuddy(_avgCost);

    QLabel* avgProfitLabel = new QLabel(tr("Avg Profit:"), margin);
    _avgProfit = new MoneyEdit(margin);
    avgProfitLabel->setBuddy(_avgProfit);

    QLabel* avgMarginLabel = new QLabel(tr("Avg Margin:"), margin);
    _avgMargin = new PercentEdit(margin);
    avgMarginLabel->setBuddy(_avgMargin);

    marginGrid->setColStretch(2, 1);
    marginGrid->setColStretch(5, 1);
    marginGrid->addColSpacing(2, 10);
    marginGrid->addColSpacing(5, 10);
    marginGrid->addWidget(marginSizeLabel, 1, 0);
    marginGrid->addWidget(_marginSize, 1, 1, AlignLeft | AlignVCenter);
    marginGrid->addWidget(marginPriceLabel, 1, 3);
    marginGrid->addWidget(_marginPrice, 1, 4, AlignLeft | AlignVCenter);
    marginGrid->addWidget(targetLabel, 1, 6);
    marginGrid->addWidget(_targetGM, 1, 7, AlignLeft | AlignVCenter);
    marginGrid->addWidget(repCostLabel, 2, 0);
    marginGrid->addWidget(_repCost, 2, 1, AlignLeft | AlignVCenter);
    marginGrid->addWidget(repProfitLabel, 2, 3);
    marginGrid->addWidget(_repProfit, 2, 4, AlignLeft | AlignVCenter);
    marginGrid->addWidget(repMarginLabel, 2, 6);
    marginGrid->addWidget(_repMargin, 2, 7, AlignLeft | AlignVCenter);
    marginGrid->addWidget(lastCostLabel, 3, 0);
    marginGrid->addWidget(_lastCost, 3, 1, AlignLeft | AlignVCenter);
    marginGrid->addWidget(lastProfitLabel, 3, 3);
    marginGrid->addWidget(_lastProfit, 3, 4, AlignLeft | AlignVCenter);
    marginGrid->addWidget(lastMarginLabel, 3, 6);
    marginGrid->addWidget(_lastMargin, 3, 7, AlignLeft | AlignVCenter);
    marginGrid->addWidget(avgCostLabel, 4, 0);
    marginGrid->addWidget(_avgCost, 4, 1, AlignLeft | AlignVCenter);
    marginGrid->addWidget(avgProfitLabel, 4, 3);
    marginGrid->addWidget(_avgProfit, 4, 4, AlignLeft | AlignVCenter);
    marginGrid->addWidget(avgMarginLabel, 4, 6);
    marginGrid->addWidget(_avgMargin, 4, 7, AlignLeft | AlignVCenter);

    QFrame* box = new QFrame(frame);

    QPushButton* refresh = new QPushButton(tr("&Refresh"), box);
    refresh->setMinimumSize(refresh->sizeHint());
    connect(refresh, SIGNAL(clicked()), SLOT(slotRefresh()));

    QPushButton* save = new QPushButton(tr("&Save"), box);
    save->setMinimumSize(refresh->sizeHint());
    connect(save, SIGNAL(clicked()), SLOT(slotSave()));

    QPushButton* close = new QPushButton(tr("Cl&ose"), box);
    close->setMinimumSize(refresh->sizeHint());
    connect(close, SIGNAL(clicked()), SLOT(slotClose()));

    QGridLayout* boxGrid = new QGridLayout(box);
    boxGrid->setSpacing(6);
    boxGrid->setMargin(6);
    boxGrid->setColStretch(1, 1);
    boxGrid->addWidget(refresh, 0, 0, AlignLeft | AlignVCenter);
    boxGrid->addWidget(save, 0, 1, AlignRight | AlignVCenter);
    boxGrid->addWidget(close, 0, 2, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->addMultiCellWidget(top, 0, 0, 0, 1);
    grid->addWidget(price, 1, 0);
    grid->addWidget(cost, 1, 1);
    grid->addMultiCellWidget(margin, 2, 2, 0, 1);
    grid->addMultiCellWidget(box, 3, 3, 0, 1);

    connect(_item, SIGNAL(validData()), SLOT(slotItemChanged()));
    connect(_store, SIGNAL(validData()), SLOT(slotStoreChanged()));
    connect(_priceSize, SIGNAL(activated(int)), SLOT(slotPriceSizeChanged()));
    connect(_price, SIGNAL(validData()), SLOT(slotPriceChanged()));
    connect(_priceBase, SIGNAL(validData()), SLOT(slotPriceBaseChanged()));
    connect(_costSize, SIGNAL(activated(int)), SLOT(slotCostSizeChanged()));
    connect(_cost, SIGNAL(validData()), SLOT(slotCostChanged()));
    connect(_costBase, SIGNAL(validData()), SLOT(slotCostBaseChanged()));
    connect(_marginSize, SIGNAL(activated(int)),SLOT(slotMarginSizeChanged()));
    connect(_marginPrice, SIGNAL(validData()), SLOT(slotMarginPriceChanged()));
    connect(_repCost, SIGNAL(validData()), SLOT(slotRepCostChanged()));
    connect(_repProfit, SIGNAL(validData()), SLOT(slotRepProfitChanged()));
    connect(_repMargin, SIGNAL(validData()), SLOT(slotRepMarginChanged()));
    connect(_lastProfit, SIGNAL(validData()), SLOT(slotLastProfitChanged()));
    connect(_lastMargin, SIGNAL(validData()), SLOT(slotLastMarginChanged()));
    connect(_avgProfit, SIGNAL(validData()), SLOT(slotAvgProfitChanged()));
    connect(_avgMargin, SIGNAL(validData()), SLOT(slotAvgMarginChanged()));

    setStoreId(_quasar->defaultStore());
    _item->setFocus();

    setCentralWidget(frame);
    setCaption(tr("Item Margin"));
    finalize();
}

ItemMargin::~ItemMargin()
{
}

void
ItemMargin::setStoreId(Id store_id)
{
    _store->setId(store_id);
    _currStoreId = store_id;
}

void
ItemMargin::setItem(Id item_id, const QString& number)
{
    if (_item->getId() == item_id) return;
    _item->setValue(Plu(item_id, number));
    newItem();
}

void
ItemMargin::slotItemChanged()
{
    if (!checkForChanges()) {
	_item->setId(_curr.id());
	return;
    }

    newItem();
}

void
ItemMargin::slotStoreChanged()
{
    if (!checkForChanges()) {
	_store->setId(_currStoreId);
	return;
    }

    _lookup->store_id = _store->getId();
    _currStoreId = _store->getId();

    newItem();
}

void
ItemMargin::slotPriceSizeChanged()
{
    if (_curr.id() == INVALID_ID) return;
    if (!checkForChanges()) {
	_priceSize->setCurrentItem(_currPriceSize);
	return;
    }

    _currPriceSize = _priceSize->currentText();
    _price->setPrice(_curr.price(_currStoreId, _currPriceSize));
    slotPriceChanged();
}

void
ItemMargin::slotPriceChanged()
{
    if (_curr.id() == INVALID_ID) return;

    Price price = _price->getPrice();
    fixed priceSizeQty = _curr.sizeQty(_currPriceSize);
    fixed onePrice = price.calculate(1.0);
    fixed priceDeposit = _curr.deposit() * priceSizeQty;
    fixed priceBase = onePrice;

    if (_curr.priceIncludesDeposit() && priceBase != 0.0)
	priceBase -= priceDeposit;

    ObjectCache cache(_db);

    fixed priceTax = 0.0;
    if (_curr.priceIncludesTax() && priceBase != 0.0) {
	priceTax = _quasar->db()->calculateTaxOff(cache, _sellTax, priceBase);
	priceBase -= priceTax;
    } else {
	priceTax = _quasar->db()->calculateTaxOn(cache, _sellTax, priceBase);
	priceTax.moneyRound();
    }

    _priceTax->setFixed(priceTax);
    _priceDeposit->setFixed(priceDeposit);
    _priceBase->setFixed(priceBase);

    recalculateMargins();
}

void
ItemMargin::slotPriceBaseChanged()
{
    if (_curr.id() == INVALID_ID) return;

    fixed price = _priceBase->getFixed();

    ObjectCache cache(_db);

    if (_curr.priceIncludesTax()) {
	fixed priceTax = _quasar->db()->calculateTaxOn(cache, _sellTax, price);
	priceTax.moneyRound();
	price += priceTax;
    }

    if (_curr.priceIncludesDeposit())
	price += _curr.deposit() * _curr.sizeQty(_priceSize->currentText());

    _price->setPrice(price);
    slotPriceChanged();
}

void
ItemMargin::slotCostSizeChanged()
{
    if (_curr.id() == INVALID_ID) return;
    if (!checkForChanges()) {
	_costSize->setCurrentItem(_currCostSize);
	return;
    }

    _currCostSize = _costSize->currentText();
    _cost->setPrice(_curr.cost(_currStoreId, _currCostSize));
    slotCostChanged();
}

void
ItemMargin::slotCostChanged()
{
    if (_curr.id() == INVALID_ID) return;

    Price cost = _cost->getPrice();
    fixed costSizeQty = _curr.sizeQty(_currCostSize);
    fixed oneCost = cost.calculate(1.0);
    fixed costDeposit = _curr.deposit() * costSizeQty;
    fixed costBase = oneCost;

    if (_curr.costIncludesDeposit() && costBase != 0.0)
	costBase -= costDeposit;

    ObjectCache cache(_db);

    fixed costTax = 0.0;
    if (_curr.costIncludesTax() && costBase != 0.0) {
	costTax = _quasar->db()->calculateTaxOff(cache, _purchaseTax, costBase);
	costBase -= costTax;
    } else {
	costTax = _quasar->db()->calculateTaxOn(cache, _purchaseTax, costBase);
	costTax.moneyRound();
    }

    if (_curr.costDiscount() != 0.0) {
	fixed discount = costBase * _curr.costDiscount() / 100.0;
	discount.moneyRound();
	costBase -= discount;
    }

    _cost->setPrice(cost);
    _costTax->setFixed(costTax);
    _costDeposit->setFixed(costDeposit);
    _costBase->setFixed(costBase);

    recalculateMargins();
}

void
ItemMargin::slotCostBaseChanged()
{
    if (_curr.id() == INVALID_ID) return;

    fixed cost = _costBase->getFixed();

    if (_curr.costDiscount() != 0.0) {
	fixed discount = cost * _curr.costDiscount() / 100.0;
	discount.moneyRound();
	cost += discount;
    }

    ObjectCache cache(_db);

    if (_curr.costIncludesTax()) {
	fixed costTax = _quasar->db()->calculateTaxOn(cache, _purchaseTax,
						      cost);
	costTax.moneyRound();
	cost += costTax;
    }

    if (_curr.costIncludesDeposit())
	cost += _curr.deposit() * _curr.sizeQty(_costSize->currentText());

    _cost->setPrice(cost);
    slotCostChanged();
}

void
ItemMargin::slotMarginSizeChanged()
{
    if (_curr.id() == INVALID_ID) return;

    _currMarginSize = _marginSize->currentText();
    _targetGM->setFixed(_curr.targetGM(_currStoreId, _currMarginSize));

    recalculateMargins();
}

void
ItemMargin::slotMarginPriceChanged()
{
    if (_curr.id() == INVALID_ID) return;

    fixed marginPrice = _marginPrice->getFixed();
    fixed priceSizeQty = _curr.sizeQty(_currPriceSize);
    fixed marginSizeQty = _curr.sizeQty(_currMarginSize);
    fixed priceBase = marginPrice / marginSizeQty * priceSizeQty;
    priceBase.moneyRound();

    _priceBase->setFixed(priceBase);
    slotPriceBaseChanged();
}

void
ItemMargin::slotRepCostChanged()
{
    if (_curr.id() == INVALID_ID) return;

    fixed repCost = _repCost->getFixed();
    fixed costSizeQty = _curr.sizeQty(_currCostSize);
    fixed marginSizeQty = _curr.sizeQty(_currMarginSize);
    fixed costBase = repCost / marginSizeQty * costSizeQty;
    costBase.moneyRound();

    _costBase->setFixed(costBase);
    slotCostBaseChanged();
}

void
ItemMargin::slotRepProfitChanged()
{
    if (_curr.id() == INVALID_ID) return;

    fixed repCost = _repCost->getFixed();
    fixed repProfit = _repProfit->getFixed();
    fixed marginPrice = repCost + repProfit;

    _marginPrice->setFixed(marginPrice);
    slotMarginPriceChanged();
}

void
ItemMargin::slotRepMarginChanged()
{
    if (_curr.id() == INVALID_ID) return;

    fixed repCost = _repCost->getFixed();
    fixed repMargin = _repMargin->getFixed();
    fixed marginPrice = repCost / (1.0 - repMargin / 100.0);
    marginPrice.moneyRound();

    _marginPrice->setFixed(marginPrice);
    slotMarginPriceChanged();
}

void
ItemMargin::slotLastProfitChanged()
{
    if (_curr.id() == INVALID_ID) return;

    fixed lastCost = _lastCost->getFixed();
    fixed lastProfit = _lastProfit->getFixed();
    fixed marginPrice = lastCost + lastProfit;

    _marginPrice->setFixed(marginPrice);
    slotMarginPriceChanged();
}

void
ItemMargin::slotLastMarginChanged()
{
    if (_curr.id() == INVALID_ID) return;

    fixed lastCost = _lastCost->getFixed();
    fixed lastMargin = _lastMargin->getFixed();
    fixed marginPrice = lastCost / (1.0 - lastMargin / 100.0);
    marginPrice.moneyRound();

    _marginPrice->setFixed(marginPrice);
    slotMarginPriceChanged();
}

void
ItemMargin::slotAvgProfitChanged()
{
    if (_curr.id() == INVALID_ID) return;

    fixed avgCost = _avgCost->getFixed();
    fixed avgProfit = _avgProfit->getFixed();
    fixed marginPrice = avgCost + avgProfit;

    _marginPrice->setFixed(marginPrice);
    slotMarginPriceChanged();
}

void
ItemMargin::slotAvgMarginChanged()
{
    if (_curr.id() == INVALID_ID) return;

    fixed avgCost = _avgCost->getFixed();
    fixed avgMargin = _avgMargin->getFixed();
    fixed marginPrice = avgCost / (1.0 - avgMargin / 100.0);
    marginPrice.moneyRound();

    _marginPrice->setFixed(marginPrice);
    slotMarginPriceChanged();
}

void
ItemMargin::slotRefresh()
{
    slotItemChanged();
}

void
ItemMargin::slotSave()
{
    if (_curr.id() == INVALID_ID) return;

    Price price = _price->getPrice();
    Price cost = _cost->getPrice();
    Price oldPrice = _curr.price(_currStoreId, _currPriceSize);
    Price oldCost = _curr.cost(_currStoreId, _currCostSize);

    if (oldPrice == price && oldCost == cost) {
	QString message = tr("There are no changes to be saved");
	QMessageBox::information(this, tr("No Changes"), message);
	return;
    }

    Item orig = _curr;
    _curr.setPrice(_currStoreId, _currPriceSize, price);
    _curr.setCost(_currStoreId, _currCostSize, cost);

    if (!_quasar->db()->update(orig, _curr)) {
	QString message = tr("Saving changes failed");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QString message = tr("Your changes have been saved");
    QMessageBox::information(this, tr("Saved"), message);
}

void
ItemMargin::slotClose()
{
    if (!checkForChanges()) return;
    close();
}

bool
ItemMargin::checkForChanges()
{
    if (_curr.id() == INVALID_ID) return true;

    Price price = _price->getPrice();
    Price cost = _cost->getPrice();
    Price oldPrice = _curr.price(_currStoreId, _currPriceSize);
    Price oldCost = _curr.cost(_currStoreId, _currCostSize);

    if (oldPrice == price && oldCost == cost)
	return true;

    QString message = tr("Do you want to save your changes?");
    int ch = QMessageBox::warning(this, tr("Save Changes?"), message,
				  QMessageBox::Yes, QMessageBox::No,
				  QMessageBox::Cancel);
    if (ch == QMessageBox::Cancel) return false;
    if (ch == QMessageBox::No) return true;

    slotSave();
    return true;
}

void
ItemMargin::newItem()
{
    _desc->setText("");
    _priceSize->clear();
    _costSize->clear();
    _marginSize->clear();

    Id item_id = _item->getId();
    if (item_id == INVALID_ID) {
	_curr.setId(INVALID_ID);
	_priceSize->insertItem("No Size");
	_price->setText("");
	_priceTax->setText("");
	_priceDeposit->setText("");
	_priceBase->setText("");
	_costSize->insertItem("No Size");
	_cost->setText("");
	_costTax->setText("");
	_costDeposit->setText("");
	_costBase->setText("");
	_marginSize->insertItem("No Size");
	_marginPrice->setText("");
	_repCost->setText("");
	_repProfit->setText("");
	_repMargin->setText("");
	_lastCost->setText("");
	_lastProfit->setText("");
	_lastMargin->setText("");
	_avgCost->setText("");
	_avgProfit->setText("");
	_avgMargin->setText("");
	return;
    }

    _quasar->db()->lookup(item_id, _curr);
    _quasar->db()->lookup(_curr.sellTax(), _sellTax);
    _quasar->db()->lookup(_curr.purchaseTax(), _purchaseTax);

    _desc->setText(_curr.description());
    for (unsigned i = 0; i < _curr.sizes().size(); ++i) {
	QString size = _curr.sizes()[i].name;
	_marginSize->insertItem(size);

	Price price = _curr.price(_currStoreId, size);
	if (!price.isNull()) {
	    _priceSize->insertItem(size);
	    if (size == _curr.sellSize()) {
		_priceSize->setCurrentItem(size);
		_marginSize->setCurrentItem(size);
		_price->setPrice(price);
	    }
	}

	Price cost = _curr.cost(_currStoreId, size);
	if (!cost.isNull()) {
	    _costSize->insertItem(size);
	    if (size == _curr.purchaseSize()) {
		_costSize->setCurrentItem(size);
		_cost->setPrice(cost);
	    }
	}
    }

    // Get the average cost
    QDate date = QDate::currentDate();
    fixed onHand, totalCost, onOrder;
    _quasar->db()->itemGeneral(_curr.id(), "", _currStoreId, date,
			       onHand, totalCost, onOrder);
    _currAvgCost = 0.0;
    if (onHand != 0.0)
	_currAvgCost = totalCost / onHand;

    // Get the last landed cost
    fixed recvQty, recvCost;
    _quasar->db()->itemLastRecv(_curr.id(), "", _currStoreId, date,
				recvQty, recvCost);
    _currLastCost = 0.0;
    if (recvQty != 0.0)
	_currLastCost = recvCost / recvQty;

    _currPriceSize = _priceSize->currentText();
    _currCostSize = _costSize->currentText();
    _currMarginSize = _marginSize->currentText();
    _targetGM->setFixed(_curr.targetGM(_currStoreId, _currMarginSize));

    slotPriceChanged();
    slotCostChanged();
}

void
ItemMargin::recalculateMargins()
{
    fixed priceBase = _priceBase->getFixed();
    fixed priceSizeQty = _curr.sizeQty(_currPriceSize);
    fixed costBase = _costBase->getFixed();
    fixed costSizeQty = _curr.sizeQty(_currCostSize);
    fixed marginSizeQty = _curr.sizeQty(_currMarginSize);

    fixed marginPrice = priceBase / priceSizeQty * marginSizeQty;
    fixed repCost = costBase / costSizeQty * marginSizeQty;
    marginPrice.moneyRound();
    repCost.moneyRound();

    fixed lastCost = _currLastCost * marginSizeQty;
    lastCost.moneyRound();
    if (lastCost == 0.0) lastCost = repCost;

    fixed avgCost = _currAvgCost * marginSizeQty;
    avgCost.moneyRound();
    if (avgCost == 0.0) avgCost = repCost;

    fixed repProfit = marginPrice - repCost;
    fixed lastProfit = marginPrice - lastCost;
    fixed avgProfit = marginPrice - avgCost;
    fixed repMargin = repProfit / marginPrice;
    fixed lastMargin = lastProfit / marginPrice;
    fixed avgMargin = avgProfit / marginPrice;

    _marginPrice->setFixed(marginPrice);
    _repCost->setFixed(repCost);
    _repProfit->setFixed(repProfit);
    _repMargin->setFixed(repMargin * 100.0);
    _lastCost->setFixed(lastCost);
    _lastProfit->setFixed(lastProfit);
    _lastMargin->setFixed(lastMargin * 100.0);
    _avgCost->setFixed(avgCost);
    _avgProfit->setFixed(avgProfit);
    _avgMargin->setFixed(avgMargin * 100.0);
}
