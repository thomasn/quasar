// $Id: promo_batch_master.cpp,v 1.6 2005/01/30 04:25:31 bpepers Exp $
//
// Copyright (C) 1998-2004 Linux Canada Inc.  All rights reserved.
//
// This file is part of Quasar Acpromobatching
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
// information about Quasar Acpromobatching support and maintenance options.
//
// Contact sales@linuxcanada.com if any conditions of this licensing are
// not clear to you.

#include "promo_batch_master.h"
#include "label_batch.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "promo_batch_select.h"
#include "table.h"
#include "item_edit.h"
#include "date_popup.h"
#include "price_edit.h"
#include "double_edit.h"
#include "item_lookup.h"
#include "store_lookup.h"
#include "store.h"
#include "item_price.h"
#include "grid.h"
#include "text_frame.h"
#include "line_frame.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>

PromoBatchMaster::PromoBatchMaster(MainWindow* main, Id batch_id)
    : DataWindow(main, "PromoBatchMaster", batch_id)
{
    _helpSource = "promo_batch_master.html";

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

    QLabel* fromDateLabel = new QLabel(tr("From:"), top);
    _fromDate = new DatePopup(top);
    fromDateLabel->setBuddy(_fromDate);

    QLabel* toDateLabel = new QLabel(tr("To:"), top);
    _toDate = new DatePopup(top);
    toDateLabel->setBuddy(_toDate);

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
    topGrid->addWidget(fromDateLabel, 0, 3);
    topGrid->addWidget(_fromDate, 0, 4, AlignLeft | AlignVCenter);
    topGrid->addWidget(toDateLabel, 1, 3);
    topGrid->addWidget(_toDate, 1, 4, AlignLeft | AlignVCenter);
    topGrid->addWidget(execDateLabel, 2, 3);
    topGrid->addWidget(_execDate, 2, 4, AlignLeft | AlignVCenter);

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
    new PriceColumn(_items, tr("Price"));
    new PriceColumn(_items, tr("Promo"));
    new NumberColumn(_items, tr("Ordered"));

    // Add editors
    _size = new QComboBox(false, _items);
    new LookupEditor(_items, 0, new ItemEdit(_itemLookup, _items));
    new ComboEditor(_items, 2, _size);
    new PriceEditor(_items, 4, new PriceEdit(_items));
    new NumberEditor(_items, 5, new DoubleEdit(_items));

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

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    setCaption(tr("Promotion Batch"));
    finalize();
}

PromoBatchMaster::~PromoBatchMaster()
{
}

void
PromoBatchMaster::oldItem()
{
    _db->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _number;
}

void
PromoBatchMaster::newItem()
{
    PromoBatch blank;
    _orig = blank;
    _orig.setNumber("#");
    _orig.setStoreId(_quasar->defaultStore());

    _curr = _orig;
    _firstField = _number;
}

void
PromoBatchMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _db->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _curr.setNumber("#");
    _curr.setDescription("");
    _curr.setExecutedOn(QDate());
    for (unsigned int i = 0; i < _curr.items().size(); ++i)
	_curr.items()[i].price_id = INVALID_ID;
    _db->setActive(_curr, true);
    dataToWidget();
}

bool
PromoBatchMaster::fileItem()
{
    // If executed, can only change valid/invalid state
    if (!_orig.executedOn().isNull()) {
	bool changed = false;
	if (_curr.items() != _orig.items()) changed = true;
	else if (_curr.fromDate() != _orig.fromDate()) changed = true;
	else if (_curr.toDate() != _orig.toDate()) changed = true;

	if (changed) return error("Can't change executed batch");
    }

    if (_orig.id() == INVALID_ID) {
	if (!_db->create(_curr)) return false;
    } else {
	if (!_db->update(_orig, _curr)) return false;
    }

    if (_orig.isActive() != _curr.isActive()) {
	for (unsigned int i = 0; i < _curr.items().size(); ++i) {
	    Id price_id = _curr.items()[i].price_id;
	    if (price_id == INVALID_ID) continue;

	    ItemPrice price, orig;
	    _db->lookup(price_id, orig);

	    price = orig;
	    _db->setActive(price, _curr.isActive());
	    _db->update(orig, price);
	}
    }

    _orig = _curr;
    _id = _curr.id();

    return true;
}

bool
PromoBatchMaster::deleteItem()
{
    _db->remove(_curr);

    for (unsigned int i = 0; i < _curr.items().size(); ++i) {
	Id price_id = _curr.items()[i].price_id;
	if (price_id == INVALID_ID) continue;

	ItemPrice price;
	_db->lookup(price_id, price);
	_db->remove(price);
    }

    return true;
}

void
PromoBatchMaster::restoreItem()
{
    _curr = _orig;
}

void
PromoBatchMaster::cloneItem()
{
    PromoBatchMaster* clone = new PromoBatchMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
PromoBatchMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
PromoBatchMaster::dataToWidget()
{
    _number->setText(_curr.number());
    _description->setText(_curr.description());
    _store->setId(_curr.storeId());
    _fromDate->setDate(_curr.fromDate());
    _toDate->setDate(_curr.toDate());
    _execDate->setDate(_curr.executedOn());
    _inactive->setChecked(!_curr.isActive());

    // Clear the caches
    _item_cache.clear();
    Id store_id = _curr.storeId();

    // Load the items
    _items->setUpdatesEnabled(false);
    _items->clear();
    const vector<PromoBatchItem>& items = _curr.items();
    for (unsigned int line = 0; line < items.size(); ++line) {
	Id item_id = items[line].item_id;
	QString number = items[line].number;
	QString size = items[line].size;
	Price promoPrice = items[line].price;
	fixed ordered = items[line].ordered;
	Id price_id = items[line].price_id;

	Item item;
	findItem(item_id, item);

	QString description = item.description();
	if (item_id == INVALID_ID)
	    description = tr("** Unknown Item **");

	Price currentPrice = item.price(store_id, size);

	VectorRow* row = new VectorRow(_items->columns() + 1);
	row->setValue(0, Plu(item_id, number));
	row->setValue(1, description);
	row->setValue(2, size);
	row->setValue(3, currentPrice);
	row->setValue(4, promoPrice);
	row->setValue(5, ordered);
	row->setValue(6, price_id);
	_items->appendRow(row);
    }
    _items->setUpdatesEnabled(true);
    _items->appendRow(new VectorRow(_items->columns() + 1));
}

// Set the data object from the widgets.
void
PromoBatchMaster::widgetToData()
{
    _curr.setNumber(_number->text());
    _curr.setDescription(_description->text());
    _curr.setStoreId(_store->getId());
    _curr.setFromDate(_fromDate->getDate());
    _curr.setToDate(_toDate->getDate());
    _curr.setExecutedOn(_execDate->getDate());
    _db->setActive(_curr, !_inactive->isChecked());

    // Process all the items
    _curr.items().clear();
    for (int row = 0; row < _items->rows(); ++row) {
        Plu plu = _items->cellValue(row, 0).toPlu();
	QString size = _items->cellValue(row, 2).toString();
	Price promoPrice = _items->cellValue(row, 4).toPrice();
	fixed ordered = _items->cellValue(row, 5).toFixed();
	Id price_id = _items->row(row)->value(6).toId();
	if (plu.itemId() == INVALID_ID && plu.number().isEmpty()) continue;

	PromoBatchItem line;
	line.item_id = plu.itemId();
	line.number = plu.number();
	line.size = size;
	line.price = promoPrice;
	line.ordered = ordered;
	line.price_id = price_id;
	_curr.items().push_back(line);
    }
}

void
PromoBatchMaster::slotStoreChanged()
{
    _itemLookup->store_id = _store->getId();

    _items->clear();
    _items->appendRow(new VectorRow(_items->columns() + 1));
}

void
PromoBatchMaster::cellMoved(int row, int)
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
PromoBatchMaster::cellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _items->rows() - 1 && col == 0) {
	Id id = _items->cellValue(row, col).toId();
	if (id != INVALID_ID)
	    _items->appendRow(new VectorRow(_items->columns() + 1));
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
	    Price currentPrice = item.price(store_id, size);

	    _items->setCellValue(row, 1, item.description());
	    _items->setCellValue(row, 2, size);
	    _items->setCellValue(row, 3, currentPrice);
	    _items->setCellValue(row, 4, "");
	    _items->setCellValue(row, 5, "");
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
	    Price currentPrice = item.price(store_id, size);

	    _items->setCellValue(row, 3, currentPrice);
	}
	break;
    }
}

void
PromoBatchMaster::focusNext(bool& leave, int&, int& newCol, int type)
{
    int row = _items->currentRow();
    int col = _items->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id id = _items->cellValue(row, col).toId();
	if (id == INVALID_ID && row == _items->rows() - 1) {
	    leave = true;
	} else {
	    newCol = 4;
	}
    } else if (type == Table::MovePrev && col == 4) {
	newCol = 2;
    } else if (type == Table::MoveNext && col == 2) {
	newCol = 4;
    } else if (type == Table::MovePrev && col == 2) {
        newCol = 0;
    }
}

bool
PromoBatchMaster::findItem(Id item_id, Item& item)
{
    for (unsigned int i = 0; i < _item_cache.size(); ++i) {
	if (_item_cache[i].id() == item_id) {
	    item = _item_cache[i];
	    return true;
	}
    }

    if (!_db->lookup(item_id, item))
	return false;

    _item_cache.push_back(item);
    return true;
}

void
PromoBatchMaster::setItem(const Item& item)
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
PromoBatchMaster::slotPrint()
{
    if (!saveItem(true)) return;
    dataToWidget();

    // Setup grid for printing
    Grid* grid = new Grid(6);
    Grid* header = new Grid(1, grid);
    TextFrame* text;
    text = new TextFrame(tr("Promotion Batch"), header);
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
    for (int column = 0; column < 6; ++column) {
	grid->setColumnSticky(column, column < 3 ? Grid::Left : Grid::Right);
	grid->setColumnPad(column, 5);
    }
    grid->set(2, 0, tr("Item Number"));
    grid->set(3, 0, new LineFrame(grid), Grid::LeftRight);
    grid->set(2, 1, tr("Description"));
    grid->set(3, 1, new LineFrame(grid), Grid::LeftRight);
    grid->set(2, 2, tr("Size"));
    grid->set(3, 2, new LineFrame(grid), Grid::LeftRight);
    grid->set(2, 3, tr("Current Price"));
    grid->set(3, 3, new LineFrame(grid), Grid::LeftRight);
    grid->set(2, 4, tr("Promo Price"));
    grid->set(3, 4, new LineFrame(grid), Grid::LeftRight);
    grid->set(2, 5, tr("Ordered"));
    grid->set(3, 5, new LineFrame(grid), Grid::LeftRight);
    grid->setHeaderRows(grid->rows());

    // Setup progress dialog
    QProgressDialog* progress = new QProgressDialog(tr("Printing promos..."),
						    tr("Cancel"),
						    _curr.items().size(),
						    this, "Progress", true);
    progress->setMinimumDuration(1000);
    progress->setCaption(tr("Progress"));

    // Print promos
    for (unsigned int i = 0; i < _curr.items().size(); ++i) {
	progress->setProgress(i);
	qApp->processEvents();
	if (progress->wasCancelled()) break;

	Id item_id = _curr.items()[i].item_id;
	if (item_id == INVALID_ID) continue;

	QString number = _curr.items()[i].number;
	QString size = _curr.items()[i].size;
	Price promoPrice = _curr.items()[i].price;
	fixed ordered = _curr.items()[i].ordered;

	Item item;
	findItem(item_id, item);

	Price currentPrice = item.price(_store->getId(), size);

	grid->set(USE_NEXT, 0, number);
	grid->set(USE_CURR, 1, item.description());
	grid->set(USE_CURR, 2, size);
	grid->set(USE_CURR, 3, currentPrice.toString());
	grid->set(USE_CURR, 4, promoPrice.toString());
	grid->set(USE_CURR, 5, ordered.toString());
    }
    bool cancelled = progress->wasCancelled();
    progress->setProgress(_curr.items().size());
    delete progress;

    if (!cancelled) grid->print(this);
    delete grid;
}

void
PromoBatchMaster::slotLabels()
{
    if (!saveItem(true)) return;
    dataToWidget();

    LabelBatch batch;
    batch.setNumber("#");
    batch.setDescription(tr("From promo batch #%1").arg(_curr.number()));
    batch.setStoreId(_curr.storeId());

    for (unsigned int i = 0; i < _curr.items().size(); ++i) {
	const PromoBatchItem& line1 = _curr.items()[i];
	if (line1.item_id == INVALID_ID) continue;

	LabelBatchItem line2;
	line2.item_id = line1.item_id;
	line2.number = line1.number;
	line2.size = line1.size;
	line2.price = line1.price;
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
PromoBatchMaster::slotExecute()
{
    Id store_id = _store->getId();
    if (store_id == INVALID_ID) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), tr("A store is required"));
	_store->setFocus();
	return;
    }

    QDate oldDate = _execDate->getDate();
    if (!oldDate.isNull()) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), tr("Already executed"));
	return;
    }

    QString message = tr("Are you sure you want to execute this batch?");
    int ch = QMessageBox::warning(this, tr("Continue?"), message,
				  QMessageBox::No, QMessageBox::Yes);
    if (ch != QMessageBox::Yes) return;

    _execDate->setDate(QDate::currentDate());
    QDate fromDate = _fromDate->getDate();
    QDate toDate = _toDate->getDate();

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Create ItemPrice
    for (int row = 0; row < _items->rows(); ++row) {
	Id item_id = _items->cellValue(row, 0).toId();
	if (item_id == INVALID_ID) continue;

	QString number = _items->cellValue(row, 0).toPlu().number();
	QString size = _items->cellValue(row, 2).toString();
	Price price = _items->cellValue(row, 4).toPrice();

	Item item;
	findItem(item_id, item);

	ItemPrice info;
	info.setIsCost(false);
	info.setItemId(item_id);
	info.setNumber(number);
	info.setSize(size);
	info.setStoreId(store_id);
	info.setPromotion(true);
	info.setDiscountable(item.isDiscountable());
	info.setStartDate(fromDate);
	info.setStopDate(toDate);
	info.setMethod(ItemPrice::PRICE);
	info.setPrice(price);
	_db->create(info);

	_items->row(row)->setValue(6, info.id());
    }

    saveItem(false);
    dataToWidget();

    QApplication::restoreOverrideCursor();
    qApp->beep();

    message = tr("Do you wish to print out the price\n"
		 "changes that were done?");
    ch = QMessageBox::information(this, tr("Print"), message,
				  QMessageBox::No, QMessageBox::Yes);
    if (ch == QMessageBox::Yes)
	slotPrint();

    message = tr("Do you wish to create labels for the\n"
		 "changes that were done?");
    ch = QMessageBox::information(this, tr("Labels"), message,
				  QMessageBox::No, QMessageBox::Yes);
    if (ch == QMessageBox::Yes)
	slotLabels();

    QMessageBox::information(this, tr("Complete"),
			     tr("Promotion batch executed"));
    close();
}
