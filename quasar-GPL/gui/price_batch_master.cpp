// $Id: price_batch_master.cpp,v 1.16 2005/05/13 20:47:49 bpepers Exp $
//
// Copyright (C) 1998-2004 Linux Canada Inc.  All rights reserved.
//
// This file is part of Quasar Acpricebatching
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
// information about Quasar Acpricebatching support and maintenance options.
//
// Contact sales@linuxcanada.com if any conditions of this licensing are
// not clear to you.

#include "price_batch_master.h"
#include "price_batch_items.h"
#include "price_batch_calc.h"
#include "label_batch.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "object_cache.h"
#include "price_batch_select.h"
#include "table.h"
#include "item_edit.h"
#include "date_edit.h"
#include "price_edit.h"
#include "item_lookup.h"
#include "store_lookup.h"
#include "company.h"
#include "store.h"
#include "tax.h"
#include "grid.h"
#include "text_frame.h"
#include "line_frame.h"
#include "object_cache.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>

PriceBatchMaster::PriceBatchMaster(MainWindow* main, Id batch_id)
    : DataWindow(main, "PriceBatchMaster", batch_id)
{
    _cache = new ObjectCache(_db);

    _helpSource = "price_batch_master.html";

    QPushButton* addItems = new QPushButton(tr("Add Items"), _buttons);
    connect(addItems, SIGNAL(clicked()), SLOT(slotAddItems()));

    QPushButton* clearItems = new QPushButton(tr("Clear Items"), _buttons);
    connect(clearItems, SIGNAL(clicked()), SLOT(slotClearItems()));

    QPushButton* calculate = new QPushButton(tr("Calculate"), _buttons);
    connect(calculate, SIGNAL(clicked()), SLOT(slotCalculate()));

    QPushButton* round = new QPushButton(tr("Round"), _buttons);
    connect(round, SIGNAL(clicked()), SLOT(slotRound()));

    QPushButton* print = new QPushButton(tr("Print"), _buttons);
    connect(print, SIGNAL(clicked()), SLOT(slotPrint()));

    QPushButton* labels = new QPushButton(tr("Labels"), _buttons);
    connect(labels, SIGNAL(clicked()), SLOT(slotLabels()));

    QPushButton* execute = new QPushButton(tr("Execute"), _buttons);
    connect(execute, SIGNAL(clicked()), SLOT(slotExecute()));

    QFrame* top = new QFrame(_frame);
    top->setFrameStyle(QFrame::Raised | QFrame::Panel);

    QLabel* numberLabel = new QLabel(tr("Batch No.:"), top);
    _number = new LineEdit(top);
    _number->setLength(10, '9');
    numberLabel->setBuddy(_number);

    QLabel* descLabel = new QLabel(tr("Description:"), top);
    _description = new LineEdit(top);
    _description->setLength(40);
    descLabel->setBuddy(_description);

    QLabel* storeLabel = new QLabel(tr("Store:"), top);
    _store = new LookupEdit(new StoreLookup(_main, this), top);
    _store->setLength(30);
    storeLabel->setBuddy(_store);
    connect(_store, SIGNAL(validData()), SLOT(slotStoreChanged()));

    QLabel* execDateLabel = new QLabel(tr("Executed On:"), top);
    _execDate = new DateEdit(top);
    _execDate->setFocusPolicy(NoFocus);

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(3);
    topGrid->setMargin(3);
    topGrid->setColStretch(2, 1);
    topGrid->addColSpacing(2, 20);
    topGrid->addWidget(numberLabel, 0, 0);
    topGrid->addWidget(_number, 0, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(descLabel, 1, 0);
    topGrid->addWidget(_description, 1, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(storeLabel, 2, 0);
    topGrid->addWidget(_store, 2, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(execDateLabel, 0, 3);
    topGrid->addWidget(_execDate, 0, 4, AlignLeft | AlignVCenter);

    QFrame* mid = new QFrame(_frame);
    mid->setFrameStyle(QFrame::Raised | QFrame::Panel);

    _items = new Table(mid);
    _items->setVScrollBarMode(QScrollView::AlwaysOn);
    _items->setLeftMargin(fontMetrics().width("99999"));
    _items->setDisplayRows(10);
    connect(_items, SIGNAL(cellMoved(int,int)), SLOT(cellMoved(int,int)));
    connect(_items, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(cellChanged(int,int,Variant)));
    connect(_items, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(focusNext(bool&,int&,int&,int)));

    // Lookups
    _itemLookup = new ItemLookup(_main, this);
    _itemLookup->soldOnly = true;

    // Add columns
    new LookupColumn(_items, tr("Item Number"), 18, _itemLookup);
    new TextColumn(_items, tr("Description"), 20);
    new TextColumn(_items, tr("Size"), 8);
    new PriceColumn(_items, tr("Cost"));
    new PriceColumn(_items, tr("Price"));
    new PriceColumn(_items, tr("New Price"));

    // Add editors
    _size = new QComboBox(false, _items);
    new LookupEditor(_items, 0, new ItemEdit(_itemLookup, _items));
    new ComboEditor(_items, 2, _size);
    new PriceEditor(_items, 5, new PriceEdit(_items));

    QGridLayout* midGrid = new QGridLayout(mid);
    midGrid->setSpacing(3);
    midGrid->setMargin(3);
    midGrid->setColStretch(0, 1);
    midGrid->addWidget(_items, 0, 0);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(1, 1);
    grid->addWidget(top, 0, 0);
    grid->addWidget(mid, 1, 0);

    setCaption(tr("Price Batch"));
    finalize();
}

PriceBatchMaster::~PriceBatchMaster()
{
    delete _cache;
}

void
PriceBatchMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _number;
}

void
PriceBatchMaster::newItem()
{
    PriceBatch blank;
    _orig = blank;
    _orig.setNumber("#");
    _orig.setStoreId(_quasar->defaultStore());

    _curr = _orig;
    _firstField = _number;
}

void
PriceBatchMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _curr.setNumber("#");
    _curr.setDescription("");
    _curr.setExecutedOn(QDate());
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
}

bool
PriceBatchMaster::fileItem()
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
PriceBatchMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
PriceBatchMaster::restoreItem()
{
    _curr = _orig;
}

void
PriceBatchMaster::cloneItem()
{
    PriceBatchMaster* clone = new PriceBatchMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
PriceBatchMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
PriceBatchMaster::dataToWidget()
{
    _number->setText(_curr.number());
    _description->setText(_curr.description());
    _store->setId(_curr.storeId());
    _execDate->setDate(_curr.executedOn());
    _inactive->setChecked(!_curr.isActive());

    // Clear the caches
    _item_cache.clear();
    Id store_id = _curr.storeId();

    // Load the items
    _items->setUpdatesEnabled(false);
    _items->clear();
    const vector<PriceBatchItem>& items = _curr.items();
    for (unsigned int line = 0; line < items.size(); ++line) {
	Id item_id = items[line].item_id;
	QString number = items[line].number;
	QString size = items[line].size;
	Price new_price = items[line].new_price;
	Price old_cost = items[line].old_cost;
	Price old_price = items[line].old_price;

	Item item;
	findItem(item_id, item);

	QString description = item.description();
	if (item_id == INVALID_ID)
	    description = tr("** Unknown Item **");

	VectorRow* row = new VectorRow(_items->columns());
	row->setValue(0, Plu(item_id, number));
	row->setValue(1, description);
	row->setValue(2, size);
	row->setValue(3, old_cost);
	row->setValue(4, old_price);
	row->setValue(5, new_price);
	_items->appendRow(row);
    }
    _items->setUpdatesEnabled(true);
    _items->appendRow(new VectorRow(_items->columns()));
}

// Set the data object from the widgets.
void
PriceBatchMaster::widgetToData()
{
    _curr.setNumber(_number->text());
    _curr.setDescription(_description->text());
    _curr.setStoreId(_store->getId());
    _curr.setExecutedOn(_execDate->getDate());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    // Process all the items
    _curr.items().clear();
    for (int row = 0; row < _items->rows(); ++row) {
        Plu plu = _items->cellValue(row, 0).toPlu();
	QString size = _items->cellValue(row, 2).toString();
	Price old_cost = _items->cellValue(row, 3).toPrice();
	Price old_price = _items->cellValue(row, 4).toPrice();
	Price new_price = _items->cellValue(row, 5).toPrice();
	if (plu.itemId() == INVALID_ID && plu.number().isEmpty()) continue;

	PriceBatchItem line;
	line.item_id = plu.itemId();
	line.number = plu.number();
	line.size = size;
	line.new_price = new_price;
	line.old_cost = old_cost;
	line.old_price = old_price;
	_curr.items().push_back(line);
    }
}

void
PriceBatchMaster::slotStoreChanged()
{
    _itemLookup->store_id = _store->getId();

    _items->clear();
    _items->appendRow(new VectorRow(_items->columns()));
}

void
PriceBatchMaster::cellMoved(int row, int)
{
    if (row == _items->currentRow()) return;
    _size->clear();
    if (row == -1) return;

    Id item_id = _items->cellValue(row, 0).toId();
    Item item;
    if (!findItem(item_id, item))
	return;

    QString size = _items->cellValue(row, 2).toString();
    for (unsigned int i = 0; i < item.sizes().size(); ++i) {
	_size->insertItem(item.sizes()[i].name);
	if (item.sizes()[i].name == size)
	    _size->setCurrentItem(_size->count() - 1);
    }
}

void
PriceBatchMaster::cellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _items->rows() - 1 && col == 0) {
	Id id = _items->cellValue(row, col).toId();
	if (id != INVALID_ID)
	    _items->appendRow(new VectorRow(_items->columns()));
    }

    // Check for other changes
    Plu plu;
    Item item;
    switch (col) {
    case 0: // item_id
	plu = _items->cellValue(row, 0).toPlu();
	if (findItem(plu.itemId(), item)) {
	    QString number = plu.number();
	    QString size = item.numberSize(number);
	    if (size.isEmpty()) size = item.sellSize();

	    _size->clear();
	    for (unsigned int i = 0; i < item.sizes().size(); ++i) {
		_size->insertItem(item.sizes()[i].name);
		if (item.sizes()[i].name == size)
		    _size->setCurrentItem(_size->count() - 1);
	    }

	    Id store_id = _store->getId();
	    fixed cost = item.cost(store_id, size).calculate(1.0);
	    Price price = item.price(store_id, size);

	    if (cost == 0) {
	        Price purch_cost = item.cost(store_id, item.purchaseSize());
		fixed purch_qty = item.sizeQty(item.purchaseSize());
		fixed size_qty = item.sizeQty(size);
		cost = purch_cost.calculate(1.0) * size_qty / purch_qty;
	    }

	    ObjectCache cache(_db);

	    // Remove deposit/tax included in cost
	    if (item.costIncludesDeposit() && cost != 0.0)
		cost -= item.deposit() * item.sizeQty(size);
	    if (item.costIncludesTax() && cost != 0.0) {
		Tax purchaseTax;
		_cache->findTax(item.purchaseTax(), purchaseTax);
		cost -= _db->calculateTaxOff(cache, purchaseTax, cost);
	    }
	    if (item.costDiscount() != 0.0) {
		fixed discount = cost * item.costDiscount() / 100.0;
		discount.moneyRound();
		cost -= discount;
	    }

	    Price newPrice = price;
	    if (_items->cellValue(row, 1) == tr("** Unknown Item **")) {
		newPrice = _items->cellValue(row, 5).toPrice();
		if (newPrice.isNull())
		    newPrice = price;
	    }

	    _items->setCellValue(row, 1, item.description());
	    _items->setCellValue(row, 2, size);
	    _items->setCellValue(row, 3, Price(cost));
	    _items->setCellValue(row, 4, price);
	    _items->setCellValue(row, 5, newPrice);
	} else {
	    if (row != _items->rows() - 1) {
		_items->setCellValue(row, 1, tr("** Unknown Item **"));
	    }
	}
	break;
    case 2: // size
	plu = _items->cellValue(row, 0).toPlu();
	if (findItem(plu.itemId(), item)) {
	    QString size = _items->cellValue(row, 2).toString();

	    Id store_id = _store->getId();
	    fixed cost = item.cost(store_id, size).calculate(1.0);
	    Price price = item.price(store_id, size);

	    if (cost == 0) {
		Price purch_cost = item.cost(store_id, item.purchaseSize());
		fixed purch_qty = item.sizeQty(item.purchaseSize());
		fixed size_qty = item.sizeQty(size);
		cost = purch_cost.calculate(1.0) * size_qty / purch_qty;
	    }

	    ObjectCache cache(_db);

	    // Remove deposit/tax included in cost
	    if (item.costIncludesDeposit() && cost != 0.0)
		cost -= item.deposit() * item.sizeQty(size);
	    if (item.costIncludesTax() && cost != 0.0) {
		Tax purchaseTax;
		_cache->findTax(item.purchaseTax(), purchaseTax);
		cost -= _db->calculateTaxOff(cache, purchaseTax, cost);
	    }
	    if (item.costDiscount() != 0.0)
		cost -= cost * item.costDiscount();

	    _items->setCellValue(row, 3, Price(cost));
	    _items->setCellValue(row, 4, price);
	    _items->setCellValue(row, 5, price);
	}
	break;
    }
}

void
PriceBatchMaster::focusNext(bool& leave, int&, int& newCol, int type)
{
    int row = _items->currentRow();
    int col = _items->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id id = _items->cellValue(row, col).toId();
	if (id == INVALID_ID && row == _items->rows() - 1) {
	    leave = true;
	} else {
	    newCol = 5;
	}
    } else if (type == Table::MovePrev && col == 5) {
	newCol = 2;
    } else if (type == Table::MoveNext && col == 2) {
	newCol = 5;
    } else if (type == Table::MovePrev && col == 2) {
        newCol = 0;
    }
}

bool
PriceBatchMaster::findItem(Id item_id, Item& item)
{
    for (unsigned int i = 0; i < _item_cache.size(); ++i) {
	if (_item_cache[i].id() == item_id) {
	    item = _item_cache[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(item_id, item))
	return false;

    _item_cache.push_back(item);
    return true;
}

void
PriceBatchMaster::setItem(const Item& item)
{
    for (unsigned int i = 0; i < _item_cache.size(); ++i) {
	if (_item_cache[i].id() == item.id()) {
	    _item_cache[i] = item;
	    return;
	}
    }
    _item_cache.push_back(item);
}

void
PriceBatchMaster::slotAddItems()
{
    Id store_id = _store->getId();
    if (store_id == INVALID_ID) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), tr("A store is required"));
	_store->setFocus();
	return;
    }

    PriceBatchItems* dialog = new PriceBatchItems(this, _main, store_id);
    if (!dialog->exec()) {
	delete dialog;
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    vector<Item> items;
    vector<QString> numbers;
    vector<QString> descs;
    vector<QString> sizes;
    vector<fixed> costs;
    vector<Price> prices;
    dialog->getItems(items, numbers, descs, sizes, costs, prices);
    delete dialog;

    for (unsigned int i = 0; i < items.size(); ++i) {
	Item item = items[i];
	QString number = numbers[i];
	QString description = descs[i];
	QString size = sizes[i];
	fixed cost = costs[i];
	Price price = prices[i];

	setItem(item);

	VectorRow* row = new VectorRow(_items->columns());
	row->setValue(0, Plu(item.id(), number));
	row->setValue(1, description);
	row->setValue(2, size);
	row->setValue(3, Price(cost));
	row->setValue(4, price);
	row->setValue(5, price);
	_items->insertRow(_items->rows() - 1, row);
    }

    QApplication::restoreOverrideCursor();
    qApp->beep();

    QString message = tr("%1 items added to batch").arg(items.size());
    QMessageBox::information(this, tr("Complete"), message);
}

void
PriceBatchMaster::slotClearItems()
{
    QString message = tr("Are you sure you want to clear all items?");
    int ch = QMessageBox::warning(this, tr("Continue?"), message,
				  QMessageBox::No, QMessageBox::Yes);
    if (ch != QMessageBox::Yes) return;

    _items->clear();
    _items->appendRow(new VectorRow(_items->columns()));
}

void
PriceBatchMaster::slotCalculate()
{
    PriceBatchCalc* dialog = new PriceBatchCalc(this);
    bool ok = dialog->exec();
    int type = dialog->getType();
    fixed percent = dialog->getPercent();
    delete dialog;
    if (!ok) return;

    setEnabled(false);
    _items->setUpdatesEnabled(false);

    // Setup progress dialog
    QProgressDialog* progress =new QProgressDialog(tr("Calculating prices..."),
						   tr("Cancel"),_items->rows(),
						   this, "Progress", true);
    progress->setMinimumDuration(1000);
    progress->setCaption(tr("Progress"));

    for (int row = 0; row < _items->rows(); ++row) {
	progress->setProgress(row);
	qApp->processEvents();
	if (progress->wasCancelled()) break;

	Id item_id = _items->cellValue(row, 0).toId();
	if (item_id == INVALID_ID) continue;

	QString number = _items->cellValue(row, 0).toPlu().number();
	QString size = _items->cellValue(row, 2).toString();
	fixed cost = _items->cellValue(row, 3).toPrice().calculate(1.0);
	fixed oldPrice = _items->cellValue(row, 4).toPrice().calculate(1.0);

	Item item;
	findItem(item_id, item);

	if (type == PriceBatchCalc::Target) {
	    percent = item.targetGM(_store->getId(), size);
	    if (percent == 0.0) continue;
	}

	fixed newPrice;
	switch (type) {
	case PriceBatchCalc::Markup:
	    if (!item.isPurchased()) continue;
	    newPrice = cost + cost * percent / 100.0;
	    break;
	case PriceBatchCalc::Change:
	    newPrice = oldPrice + oldPrice * percent / 100.0;
	    break;
	case PriceBatchCalc::Margin:
	case PriceBatchCalc::Target:
	    if (!item.isPurchased()) continue;
	    newPrice = cost * 100.0 / (100.0 - percent);
	    break;
	}
	newPrice.moneyRound();

	ObjectCache cache(_db);

	// Adjust price for included deposit/tax
	if (item.priceIncludesTax()) {
	    Tax sellTax;
	    _cache->findTax(item.sellTax(), sellTax);
	    newPrice += _db->calculateTaxOn(cache, sellTax, newPrice);
	}
	if (item.priceIncludesDeposit())
	    newPrice += item.deposit() * item.sizeQty(size);
	newPrice.moneyRound();

	_items->setCellValue(row, 5, Price(newPrice));
    }
    progress->setProgress(_items->rows());
    delete progress;

    _items->setUpdatesEnabled(true);
    setEnabled(true);
}

void
PriceBatchMaster::slotRound()
{
    Company company;
    _db->lookup(company);
    if (company.rounding().size() == 0) {
	QString message = tr("No price rounding has been setup for\n"
			     "this company");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QString message = tr("Are you sure you want to round the\n"
			 "item prices?");
    int choice = QMessageBox::warning(this, tr("Price Rounding"), message,
				      QMessageBox::No, QMessageBox::Yes);
    if (choice != QMessageBox::Yes) return;

    for (int row = 0; row < _items->rows(); ++row) {
	Price new_price = _items->cellValue(row, 5).toPrice();
	if (new_price.isNull()) continue;

	Price round_price = company.priceRound(new_price);
	if (round_price != new_price)
	    _items->setCellValue(row, 5, round_price);
    }
}

void
PriceBatchMaster::slotPrint()
{
    if (!saveItem(true)) return;
    dataToWidget();

    // Setup grid for printing
    Grid* grid = new Grid(5);
    Grid* header = new Grid(1, grid);
    TextFrame* text;
    text = new TextFrame(tr("Price Change Batch"), header);
    text->setJustify(ALIGN_CENTER);
    text->setFont(Font("Times", 24, 75));
    header->set(USE_NEXT, 0, text);
    if (_curr.executedOn().isNull()) {
	text = new TextFrame(tr("Preliminary"), header);
    } else {
	QString dateText = DateValcon().format(_curr.executedOn());
	text = new TextFrame(tr("Executed On: %1").arg(dateText), header);
    }
    text->setJustify(ALIGN_CENTER);
    text->setFont(Font("Times", 18));
    header->set(USE_NEXT, 0, text);
    header->setColumnWeight(0, 1);

    grid->set(0, 0, 1, grid->columns(), header, Grid::AllSides);
    grid->set(USE_NEXT, 0, "");
    for (int column = 0; column < 5; ++column) {
	grid->setColumnSticky(column, column < 3 ? Grid::Left : Grid::Right);
	grid->setColumnPad(column, 5);
    }
    grid->set(2, 0, tr("Item Number"));
    grid->set(3, 0, new LineFrame(grid), Grid::LeftRight);
    grid->set(2, 1, tr("Description"));
    grid->set(3, 1, new LineFrame(grid), Grid::LeftRight);
    grid->set(2, 2, tr("Size"));
    grid->set(3, 2, new LineFrame(grid), Grid::LeftRight);
    grid->set(2, 3, tr("Old Price"));
    grid->set(3, 3, new LineFrame(grid), Grid::LeftRight);
    grid->set(2, 4, tr("New Price"));
    grid->set(3, 4, new LineFrame(grid), Grid::LeftRight);
    grid->setHeaderRows(grid->rows());

    // Setup progress dialog
    QProgressDialog* progress = new QProgressDialog(tr("Printing prices..."),
						    tr("Cancel"),
						    _curr.items().size(),
						    this, "Progress", true);
    progress->setMinimumDuration(1000);
    progress->setCaption(tr("Progress"));

    // Print prices
    for (unsigned int i = 0; i < _curr.items().size(); ++i) {
	progress->setProgress(i);
	qApp->processEvents();
	if (progress->wasCancelled()) break;

	Id item_id = _curr.items()[i].item_id;
	if (item_id == INVALID_ID) continue;

	QString number = _curr.items()[i].number;
	QString size = _curr.items()[i].size;
	Price oldPrice = _curr.items()[i].old_price;
	Price newPrice = _curr.items()[i].new_price;

	Item item;
	findItem(item_id, item);

	grid->set(USE_NEXT, 0, number);
	grid->set(USE_CURR, 1, item.description());
	grid->set(USE_CURR, 2, size);
	grid->set(USE_CURR, 3, oldPrice.toString());
	grid->set(USE_CURR, 4, newPrice.toString());
    }
    bool cancelled = progress->wasCancelled();
    progress->setProgress(_curr.items().size());
    delete progress;

    if (!cancelled) grid->print(this);
    delete grid;
}

void
PriceBatchMaster::slotLabels()
{
    if (!saveItem(true)) return;
    dataToWidget();

    LabelBatch batch;
    batch.setNumber("#");
    batch.setDescription(tr("From price batch #%1").arg(_curr.number()));
    batch.setStoreId(_curr.storeId());

    for (unsigned int i = 0; i < _curr.items().size(); ++i) {
	const PriceBatchItem& line1 = _curr.items()[i];
	if (line1.item_id == INVALID_ID) continue;

	LabelBatchItem line2;
	line2.item_id = line1.item_id;
	line2.number = line1.number;
	line2.size = line1.size;
	line2.price = line1.new_price;
	line2.count = 1;
	batch.items().push_back(line2);
    }

    if (_db->create(batch)) {
	QString message = tr("Created label batch #%1").arg(batch.number());
	QMessageBox::information(this, tr("Information"), message);
    } else {
	QString message = tr("Error creating label batch");
	QMessageBox::critical(this, tr("Error"), message);
    }
}

void
PriceBatchMaster::slotExecute()
{
    Id store_id = _store->getId();
    if (store_id == INVALID_ID) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), tr("A store is required"));
	_store->setFocus();
	return;
    }

    QString message = tr("Are you sure you want to execute this batch?");
    int ch = QMessageBox::warning(this, tr("Continue?"), message,
				  QMessageBox::No, QMessageBox::Yes);
    if (ch != QMessageBox::Yes) return;

    QDate oldDate = _execDate->getDate();
    _execDate->setDate(QDate::currentDate());
    if (!saveItem(false)) {
	_execDate->setDate(oldDate);
	return;
    }
    dataToWidget();

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Change prices
    int change_cnt = 0;
    for (int row = 0; row < _items->rows(); ++row) {
	Id item_id = _items->cellValue(row, 0).toId();
	if (item_id == INVALID_ID) continue;

	QString number = _items->cellValue(row, 0).toPlu().number();
	QString size = _items->cellValue(row, 2).toString();
	Price price = _items->cellValue(row, 5).toPrice();

	Item orig, item;
	findItem(item_id, orig);

	item = orig;
	bool found = false;
	for (unsigned int i = 0; i < item.costs().size(); ++i) {
	    if (item.costs()[i].store_id != store_id) continue;
	    if (item.costs()[i].size != size) continue;

	    item.costs()[i].price = price;
	    found = true;
	    break;
	}
	if (!found) {
	    ItemCost line;
	    line.store_id = store_id;
	    line.size = size;
	    line.price = price;
	    item.costs().push_back(line);
	}

	if (item != orig) {
	    _quasar->db()->update(orig, item);
	    setItem(item);
	    ++change_cnt;
	}
    }

    QApplication::restoreOverrideCursor();
    qApp->beep();

    if (change_cnt > 0) {
	QString message = tr("Do you wish to print out the price\n"
	    "changes that were done?");
	int ch = QMessageBox::information(this, tr("Print"), message,
					  QMessageBox::No, QMessageBox::Yes);
	if (ch == QMessageBox::Yes)
	    slotPrint();

	message = tr("Do you wish to create labels for the\n"
	    "changes that were done?");
	ch = QMessageBox::information(this, tr("Labels"), message,
				      QMessageBox::No, QMessageBox::Yes);
	if (ch == QMessageBox::Yes)
	    slotLabels();
    }

    // Set inactive
    _inactive->setChecked(true);
    saveItem(false);

    QMessageBox::information(this, tr("Complete"), tr("Price batch executed"));
    close();
}
