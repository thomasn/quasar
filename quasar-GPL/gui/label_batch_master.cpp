// $Id: label_batch_master.cpp,v 1.9 2005/01/30 04:25:31 bpepers Exp $
//
// Copyright (C) 1998-2004 Linux Canada Inc.  All rights reserved.
//
// This file is part of Quasar Aclabelbatching
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
// information about Quasar Aclabelbatching support and maintenance options.
//
// Contact sales@linuxcanada.com if any conditions of this licensing are
// not clear to you.

#include "label_batch_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "label_defn.h"
#include "label_batch_select.h"
#include "combo_box.h"
#include "table.h"
#include "integer_edit.h"
#include "item_edit.h"
#include "price_edit.h"
#include "item_lookup.h"
#include "store_lookup.h"
#include "location.h"
#include "extra.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qpaintdevicemetrics.h>

LabelBatchMaster::LabelBatchMaster(MainWindow* main, Id batch_id)
    : DataWindow(main, "LabelBatchMaster", batch_id)
{
    _helpSource = "label_batch_master.html";

    QPushButton* print = new QPushButton(tr("Print"), _buttons);
    connect(print, SIGNAL(clicked()), SLOT(slotPrint()));

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

    QLabel* typeLabel = new QLabel(tr("Type:"), top);
    _type = new ComboBox(top);
    _type->setMinimumWidth(fontMetrics().width("_") * 30);
    _type->insertItem("");
    typeLabel->setBuddy(_type);

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
    topGrid->addWidget(typeLabel, 3, 0);
    topGrid->addWidget(_type, 3, 1, AlignLeft | AlignVCenter);

    QFrame* mid = new QFrame(_frame);
    mid->setFrameStyle(QFrame::Raised | QFrame::Panel);

    _table = new Table(mid);
    _table->setVScrollBarMode(QScrollView::AlwaysOn);
    _table->setLeftMargin(fontMetrics().width("99999"));
    _table->setDisplayRows(5);
    connect(_table, SIGNAL(cellMoved(int,int)), SLOT(cellMoved(int,int)));
    connect(_table, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(cellChanged(int,int,Variant)));
    connect(_table, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(focusNext(bool&,int&,int&,int)));

    // Lookups
    _itemLookup = new ItemLookup(_main, this);
    _itemLookup->soldOnly = true;

    // Add columns
    new LookupColumn(_table, tr("Item Number"), 18, _itemLookup);
    new TextColumn(_table, tr("Description"), 20);
    new TextColumn(_table, tr("Size"), 8);
    new PriceColumn(_table, tr("Price"));
    new NumberColumn(_table, tr("# Labels"), 10);

    // Add editors
    _size = new ComboBox(false, _table);
    new LookupEditor(_table, 0, new ItemEdit(_itemLookup, _table));
    new ComboEditor(_table, 2, _size);
    new PriceEditor(_table, 3, new PriceEdit(_table));
    new NumberEditor(_table, 4, new IntegerEdit(_table));

    QGridLayout* midGrid = new QGridLayout(mid);
    midGrid->setSpacing(3);
    midGrid->setMargin(3);
    midGrid->setColStretch(0, 1);
    midGrid->addWidget(_table, 0, 0);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(1, 1);
    grid->addWidget(top, 0, 0);
    grid->addWidget(mid, 1, 0);

    setCaption(tr("Label Batch"));
    finalize();

    _quasar->resourceList("shelf_labels", "name", _labelFiles, _labelNames);

    QStringList names = _labelNames;
    names.sort();

    _type->clear();
    _type->insertItem("");
    for (unsigned int i = 0; i < names.size(); ++i)
	_type->insertItem(names[i]);

    _type->setCurrentItem(_curr.type());
}

LabelBatchMaster::~LabelBatchMaster()
{
}

void
LabelBatchMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _number;
}

void
LabelBatchMaster::newItem()
{
    LabelBatch blank;
    _orig = blank;
    _orig.setNumber("#");
    _orig.setStoreId(_quasar->defaultStore());

    _curr = _orig;
    _firstField = _number;
}

void
LabelBatchMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _curr.setNumber("#");
    _curr.setDescription("");
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
}

bool
LabelBatchMaster::fileItem()
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
LabelBatchMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
LabelBatchMaster::restoreItem()
{
    _curr = _orig;
}

void
LabelBatchMaster::cloneItem()
{
    LabelBatchMaster* clone = new LabelBatchMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
LabelBatchMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
LabelBatchMaster::dataToWidget()
{
    _number->setText(_curr.number());
    _description->setText(_curr.description());
    _type->setCurrentItem(_curr.type());
    _store->setId(_curr.storeId());
    _inactive->setChecked(!_curr.isActive());

    // Clear the caches
    _item_cache.clear();

    // Load the items
    _table->setUpdatesEnabled(false);
    _table->clear();
    const vector<LabelBatchItem>& items = _curr.items();
    for (unsigned int line = 0; line < items.size(); ++line) {
	Id item_id = items[line].item_id;
	QString number = items[line].number;
	QString size = items[line].size;
	Price price = items[line].price;
	int count = items[line].count;

	Item item;
	_quasar->db()->lookup(item_id, item);

	QString description = item.description();
	if (item_id == INVALID_ID)
	    description = tr("** Unknown Item **");

	VectorRow* row = new VectorRow(_table->columns());
	row->setValue(0, Plu(item_id, number));
	row->setValue(1, description);
	row->setValue(2, size);
	row->setValue(3, price);
	row->setValue(4, count);
	_table->appendRow(row);
    }
    _table->setUpdatesEnabled(true);
    _table->appendRow(new VectorRow(_table->columns()));
}

// Set the data object from the widgets.
void
LabelBatchMaster::widgetToData()
{
    _curr.setNumber(_number->text());
    _curr.setDescription(_description->text());
    _curr.setType(_type->currentText());
    _curr.setStoreId(_store->getId());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    // Process all the items
    vector<LabelBatchItem>& items = _curr.items();
    items.clear();
    for (int row = 0; row < _table->rows(); ++row) {
        Plu plu = _table->cellValue(row, 0).toPlu();
	QString size = _table->cellValue(row, 2).toString();
	Price price = _table->cellValue(row, 3).toPrice();
	int count = _table->cellValue(row, 4).toInt();
	if (plu.itemId() == INVALID_ID) continue;

	LabelBatchItem item;
	item.item_id = plu.itemId();
	item.number = plu.number();
	item.size = size;
	item.price = price;
	item.count = count;
	items.push_back(item);
    }
}

void
LabelBatchMaster::slotStoreChanged()
{
    _itemLookup->store_id = _store->getId();

    _table->clear();
    _table->appendRow(new VectorRow(_table->columns()));
}

void
LabelBatchMaster::cellMoved(int row, int)
{
    if (row == _table->currentRow()) return;
    _size->clear();
    if (row == -1) return;

    Id item_id = _table->cellValue(row, 0).toId();
    Item item;
    if (!findItem(item_id, item))
	return;

    QString size = _table->cellValue(row, 2).toString();
    for (unsigned int i = 0; i < item.sizes().size(); ++i) {
	_size->insertItem(item.sizes()[i].name);
	if (item.sizes()[i].name == size)
	    _size->setCurrentItem(_size->count() - 1);
    }
}

void
LabelBatchMaster::cellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _table->rows() - 1 && col == 0) {
	Id id = _table->cellValue(row, col).toId();
	if (id != INVALID_ID)
	    _table->appendRow(new VectorRow(_table->columns()));
    }

    // Check for other changes
    Plu plu;
    Item item;
    switch (col) {
    case 0: // item_id
        plu = _table->cellValue(row, 0).toPlu();
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

	    Price price = item.price(_store->getId(), size);

	    int count = 1;
	    if (_table->cellValue(row, 1) == tr("** Unknown Item **"))
		count = _table->cellValue(row, 4).toInt();

	    _table->setCellValue(row, 1, item.description());
	    _table->setCellValue(row, 2, size);
	    _table->setCellValue(row, 3, price);
	    _table->setCellValue(row, 4, count);
	} else {
	    if (row != _table->rows() - 1) {
		_table->setCellValue(row, 1, tr("** Unknown Item **"));
	    }
	}
	break;
    case 2: // size
	plu = _table->cellValue(row, 0).toPlu();
	if (findItem(plu.itemId(), item)) {
	    QString size = _table->cellValue(row, 2).toString();

	    Price price = item.price(_store->getId(), size);
	    _table->setCellValue(row, 3, price);
	}
	break;
    }
}

void
LabelBatchMaster::focusNext(bool& leave, int&, int& newCol, int type)
{
    int row = _table->currentRow();
    int col = _table->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id id = _table->cellValue(row, col).toId();
	if (id == INVALID_ID && row == _table->rows() - 1) {
	    leave = true;
	} else if (id == INVALID_ID) {
	    newCol = 0;
	} else {
	    newCol = 4;
	}
    } else if (type == Table::MovePrev && col == 2) {
	newCol = 0;
    }
}

bool
LabelBatchMaster::findItem(Id item_id, Item& item)
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
LabelBatchMaster::slotAlignment()
{
    QPrinter* printer = new QPrinter(QPrinter::HighResolution);
    printer->setColorMode(QPrinter::Color);
    printer->setPageSize(QPrinter::Letter);
    printer->setFullPage(true);
    printer->setOrientation(QPrinter::Portrait);
    if (!printer->setup(NULL)) {
        delete printer;
        return;
    }

    QPaintDeviceMetrics metrics(printer);
    int width = metrics.logicalDpiX();
    int height = metrics.logicalDpiY();
    int pageWidth = metrics.width();
    int pageHeight = metrics.height();

    QPainter p;
    p.begin(printer);

    p.drawRect(0, 0, width, height);
    p.drawRect(pageWidth - width, 0, width, height);
    p.drawRect(0, pageHeight - height, width, height);
    p.drawRect(pageWidth - width, pageHeight - height, width, height);
    p.drawRect(width * 3, height * 3, width, height);

    p.end();
    delete printer;
}

void
LabelBatchMaster::slotPrint()
{
    if (_type->count() == 0) {
	QMessageBox::critical(this, tr("Error"), tr("Invalid label type"));
	return;
    }

    QString name = _type->currentText();
    if (name.isEmpty()) {
	QMessageBox::critical(this, tr("Error"), tr("Pick a label type"));
	return;
    }

    QString fileName;
    for (unsigned int i = 0; i < _labelNames.size(); ++i) {
	if (_labelNames[i] == name) {
	    fileName = _labelFiles[i];
	    break;
	}
    }
    if (fileName.isEmpty()) {
	QMessageBox::critical(this, tr("Error"), tr("Can't find label type"));
	return;
    }

    if (!saveItem(true)) return;
    dataToWidget();

    QString filePath;
    if (!_quasar->resourceFetch("shelf_labels", fileName, filePath)) {
	QString msg = tr("Failed fetching label type '%1'").arg(fileName);
	QMessageBox::critical(this, tr("Error"), msg);
	return;
    }

    LabelDefn defn;
    if (!defn.load(filePath)) {
	QString msg = tr("Label type '%1' is invalid").arg(fileName);
	QMessageBox::critical(this, tr("Error"), msg);
	return;
    }

    QDialog* dialog = new QDialog(this);
    dialog->setCaption("Starting position");
    QLabel* rowLabel = new QLabel(tr("Starting Row:"), dialog);
    IntegerEdit* row = new IntegerEdit(dialog);
    QLabel* colLabel = new QLabel(tr("Starting Column:"), dialog);
    IntegerEdit* col = new IntegerEdit(dialog);

    QFrame* buttons = new QFrame(dialog);
    QPushButton* ok = new QPushButton(tr("OK"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);
    ok->setDefault(true);
    connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setMargin(3);
    buttonGrid->setSpacing(3);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(ok, 0, 1);
    buttonGrid->addWidget(cancel, 0, 2);

    QGridLayout* grid = new QGridLayout(dialog);
    grid->setMargin(3);
    grid->setSpacing(3);
    grid->addWidget(rowLabel, 0, 0);
    grid->addWidget(row, 0, 1);
    grid->addWidget(colLabel, 1, 0);
    grid->addWidget(col, 1, 1);
    grid->addMultiCellWidget(buttons, 2, 2, 0, 1);

    row->setInt(1);
    col->setInt(1);
    row->selectAll();
    bool good = dialog->exec();
    int startRow = row->getInt() - 1;
    int startColumn = col->getInt() - 1;
    int count = 1;
    delete dialog;
    if (!good) return;

    if (startRow < 0 || startRow >= defn.rows) {
	QMessageBox::critical(this, tr("Error"), tr("Invalid start row"));
	return;
    }
    if (startColumn < 0 || startColumn >= defn.columns) {
	QMessageBox::critical(this, tr("Error"), tr("Invalid start column"));
	return;
    }
    if (count < 1) {
	QMessageBox::critical(this, tr("Error"),tr("Invalid labels per item"));
	return;
    }

    vector<Id> item_ids;
    vector<QString> numbers;
    vector<QString> sizes;
    vector<Price> prices;
    vector<int> counts;
    for (int i = 0; i < _table->rows(); ++i) {
	Id item_id = _table->cellValue(i, 0).toId();
	int count = _table->cellValue(i, 4).toInt();
	if (item_id == INVALID_ID) continue;
	if (count <= 0) continue;

	item_ids.push_back(item_id);
	numbers.push_back(_table->cellValue(i, 0).toPlu().number());
	sizes.push_back(_table->cellValue(i, 2).toString());
	prices.push_back(_table->cellValue(i, 3).toPrice());
	counts.push_back(count);
    }

    // Check thats something was selected
    if (item_ids.size() == 0) {
	QMessageBox::critical(this, tr("Error"),
			      tr("No items selected to print"));
	return;
    }

    QPrinter* printer = new QPrinter(QPrinter::HighResolution);
    printer->setColorMode(QPrinter::Color);
    printer->setPageSize(QPrinter::Letter);
    printer->setFullPage(true);
    if (defn.landscape)
	printer->setOrientation(QPrinter::Landscape);
    else
	printer->setOrientation(QPrinter::Portrait);
    if (!printer->setup(NULL)) {
        delete printer;
        return;
    }

    // TODO: if page size picked is not the same as the label definition
    // page size, adjust label definition using percents

    // Adjust for DPI
    QPaintDeviceMetrics metrics(printer);
    double scaleX = metrics.logicalDpiX() / 72.0;
    double scaleY = metrics.logicalDpiY() / 72.0;
    defn.dpiAdjust(scaleX, scaleY);

    // Check that paper size matches
    if (fabs(metrics.width() - defn.pageWidth) > (2.0 * scaleX) ||
	fabs(metrics.height() - defn.pageHeight) > (2.0 * scaleY)) {
	QString message = tr("The paper size chosen doesn't match the size\n"
	    "used for the labels selected.");
	QMessageBox::critical(NULL, tr("Paper Size Error"), message);
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    QPainter p;
    p.begin(printer);
    if (defn.scale != 1.0) p.scale(defn.scale, defn.scale);

    int skip = startRow * defn.columns + startColumn;
    unsigned int pos = 0;
    unsigned int cnt = count;
    unsigned int pages = 0;
    fixed xOffset = defn.labelWidth + defn.horizontalGap;
    fixed yOffset = defn.labelHeight + defn.verticalGap;
    while (pos < item_ids.size()) {
	// Draw one page
	for (int row = 0; row < defn.rows; ++row) {
	    for (int column = 0; column < defn.columns; ++column) {
		if (skip > 0) { --skip; continue; }
		if (pos >= item_ids.size()) continue;

		Id item_id = item_ids[pos];
		QString number = numbers[pos];
		QString size = sizes[pos];
		Price price = prices[pos];

		Item item;
		if (!findItem(item_id, item))
		    continue;

		QString orderNumber;
		if (item.vendors().size() > 0)
		    orderNumber = item.vendors()[0].number;
		fixed orderQty = item.sizeQty(item.purchaseSize());

		Location location;
		_db->lookup(item.locationId(_store->getId()), location);

		TextMap mapping;
		mapping["number"] = number;
		mapping["description"] = item.description();
		mapping["price"] = price.toString();
		mapping["size"] = size;
		mapping["order_number"] = orderNumber;
		mapping["order_qty"] = orderQty.toString();
		mapping["date"] = QDate::currentDate().toString();
		mapping["location"] = location.name();
		for (unsigned int e = 0; e < item.extra().size(); ++e) {
		    Extra extra;
		    _quasar->db()->lookup(item.extra()[e].first, extra);
		    QString key = "extra:" + extra.name();
		    mapping[key] = item.extra()[e].second;
		}
		// TODO: unit_price mapping
		// TODO: complete mapping from item

		fixed x = defn.leftMargin + column * xOffset;
		fixed y = defn.topMargin + row * yOffset;
		defn.draw(&p, x, y, mapping);

		if (--cnt == 0) {
		    cnt = count;
		    if (--counts[pos] == 0) {
			++pos;
		    }
		}
	    }
	}

	// End the page
	if (pos < item_ids.size()) {
	    ++pages;
	    printer->newPage();
	}
    }
    p.end();
    delete printer;

    QApplication::restoreOverrideCursor();

    QString message = tr("Were the labels printed properly?");
    int choice = QMessageBox::information(this, tr("Finished?"), message,
					  tr("Yes"), tr("No"));
    if (choice == 0) {
	_inactive->setChecked(true);
	saveItem(false);
	close();
    }
}
