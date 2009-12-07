// $Id: count_master.cpp,v 1.28 2005/01/30 04:25:31 bpepers Exp $
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

#include "count_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "count_select.h"
#include "table.h"
#include "date_popup.h"
#include "double_edit.h"
#include "item_edit.h"
#include "item_lookup.h"
#include "location_lookup.h"
#include "employee_lookup.h"
#include "store_lookup.h"
#include "store.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qmessagebox.h>

CountMaster::CountMaster(MainWindow* main, Id count_id)
    : DataWindow(main, "CountMaster", count_id)
{
    _helpSource = "count_master.html";

    QFrame* top = new QFrame(_frame);
    top->setFrameStyle(QFrame::Raised | QFrame::Panel);

    QLabel* numberLabel = new QLabel(tr("Count No.:"), top);
    _number = new LineEdit(top);
    _number->setLength(14, '9');
    numberLabel->setBuddy(_number);

    QLabel* descLabel = new QLabel(tr("Description:"), top);
    _description = new LineEdit(top);
    _description->setLength(40);
    descLabel->setBuddy(_description);

    QLabel* dateLabel = new QLabel(tr("Date:"), top);
    _date = new DatePopup(top);
    dateLabel->setBuddy(_date);

    QLabel* locationLabel = new QLabel(tr("Location:"), top);
    _locLookup = new LocationLookup(_main, this);
    _location = new LookupEdit(_locLookup, top);
    _location->setLength(30);
    locationLabel->setBuddy(_location);

    QLabel* employeeLabel = new QLabel(tr("Employee:"), top);
    _employee = new LookupEdit(new EmployeeLookup(_main, this), top);
    _employee->setLength(30);
    employeeLabel->setBuddy(_employee);

    QLabel* storeLabel = new QLabel(tr("Store:"), top);
    _store = new LookupEdit(new StoreLookup(_main, this), top);
    _store->setFocusPolicy(ClickFocus);
    _store->setLength(30);
    storeLabel->setBuddy(_store);
    connect(_store, SIGNAL(validData()), SLOT(slotStoreChanged()));

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(3);
    topGrid->setMargin(3);
    topGrid->setColStretch(2, 1);
    topGrid->addColSpacing(2, 20);
    topGrid->addWidget(numberLabel, 0, 0);
    topGrid->addWidget(_number, 0, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(descLabel, 1, 0);
    topGrid->addWidget(_description, 1, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(dateLabel, 2, 0);
    topGrid->addWidget(_date, 2, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(locationLabel, 3, 0);
    topGrid->addWidget(_location, 3, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(employeeLabel, 4, 0);
    topGrid->addWidget(_employee, 4, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(storeLabel, 5, 0);
    topGrid->addWidget(_store, 5, 1, AlignLeft | AlignVCenter);

    QFrame* mid = new QFrame(_frame);
    mid->setFrameStyle(QFrame::Raised | QFrame::Panel);

    _items = new Table(mid);
    _items->setVScrollBarMode(QScrollView::AlwaysOn);
    _items->setLeftMargin(fontMetrics().width("99999"));
    _items->setDisplayRows(5);
    connect(_items, SIGNAL(cellMoved(int,int)), SLOT(cellMoved(int,int)));
    connect(_items, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(cellChanged(int,int,Variant)));
    connect(_items, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(focusNext(bool&,int&,int&,int)));

    // Lookups
    _itemLookup = new ItemLookup(_main, this);
    _itemLookup->inventoriedOnly = true;

    // Add columns
    new LookupColumn(_items, tr("Item Number"), 18, _itemLookup);
    new TextColumn(_items, tr("Description"), 30);
    new TextColumn(_items, tr("Size"), 10);
    new NumberColumn(_items, tr("On Hand"));
    new NumberColumn(_items, tr("Counted"));

    LineEdit* descEdit = new LineEdit(_items);
    NumberEdit* qtyEdit = new DoubleEdit(_items);
    descEdit->setMaxLength(40);
    qtyEdit->setMaxLength(8);

    // Add editors
    _size = new QComboBox(false, _items);
    new LookupEditor(_items, 0, new ItemEdit(_itemLookup, _items));
    new LineEditor(_items, 1, descEdit);
    new ComboEditor(_items, 2, _size);
    new NumberEditor(_items, 4, qtyEdit);

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

    setCaption(tr("Count Items"));
    finalize();
}

CountMaster::~CountMaster()
{
}

void
CountMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _description;
}

void
CountMaster::newItem()
{
    Count blank;
    _orig = blank;
    _orig.setNumber("#");
    _orig.setDate(QDate::currentDate());
    _orig.setStoreId(_quasar->defaultStore());

    _curr = _orig;
    _firstField = _description;
}

void
CountMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _curr.setNumber("#");
    _curr.setDescription("");
    for (unsigned int i = 0; i < _curr.items().size(); ++i) {
	_curr.items()[i].on_hand = 0.0;
	_curr.items()[i].oh_set = false;
    }
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
}

bool
CountMaster::fileItem()
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
CountMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
CountMaster::restoreItem()
{
    _curr = _orig;
}

void
CountMaster::cloneItem()
{
    CountMaster* clone = new CountMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
CountMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
CountMaster::dataToWidget()
{
    _number->setText(_curr.number());
    _description->setText(_curr.description());
    _date->setDate(_curr.date());
    _location->setId(_curr.locationId());
    _employee->setId(_curr.employeeId());
    _store->setId(_curr.storeId());
    _inactive->setChecked(!_curr.isActive());

    _locLookup->store_id = _curr.storeId();
    _itemLookup->store_id = _curr.storeId();

    // Clear the caches
    _item_cache.clear();

    // Load the items
    _items->setUpdatesEnabled(false);
    _items->clear();
    const vector<CountItem>& items = _curr.items();
    for (unsigned int line = 0; line < items.size(); ++line) {
	Id item_id = items[line].item_id;
	QString number = items[line].number;
	QString size = items[line].size;
	fixed size_qty = items[line].size_qty;
	fixed quantity = items[line].quantity;
	fixed on_hand = items[line].on_hand;
	bool oh_set = items[line].oh_set;

	Item item;
	findItem(item_id, item);

	QString description = item.description();
	if (item_id == INVALID_ID)
	    description = tr("** Unknown Item **");

	if (!oh_set && item.id() != INVALID_ID) {
	    fixed total_cost, on_order;
	    QDate date = QDate::currentDate();
	    _quasar->db()->itemGeneral(item.id(), "", _curr.storeId(),
				       date, on_hand, total_cost, on_order);
	    on_hand = on_hand / size_qty;
	}

	VectorRow* row = new VectorRow(_items->columns());
	row->setValue(0, Plu(item_id, number));
	row->setValue(1, description);
	row->setValue(2, size);
	row->setValue(3, on_hand);
	row->setValue(4, quantity);
	_items->appendRow(row);
    }
    _items->setUpdatesEnabled(true);
    _items->appendRow(new VectorRow(_items->columns()));
}

// Set the data object from the widgets.
void
CountMaster::widgetToData()
{
    _curr.setNumber(_number->text());
    _curr.setDescription(_description->text());
    _curr.setDate(_date->getDate());
    _curr.setLocationId(_location->getId());
    _curr.setEmployeeId(_employee->getId());
    _curr.setStoreId(_store->getId());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    // Process all the items
    _curr.items().clear();
    for (int row = 0; row < _items->rows(); ++row) {
        Plu plu = _items->cellValue(row, 0).toPlu();
	QString size = _items->cellValue(row, 2).toString();
	fixed quantity = _items->cellValue(row, 4).toFixed();
	if (plu.itemId() == INVALID_ID && plu.number().isEmpty()) continue;

	Item item;
	findItem(plu.itemId(), item);

	CountItem line;
	line.item_id = plu.itemId();
	line.number = plu.number();
	line.size = size;
	line.size_qty = item.sizeQty(size);
	line.quantity = quantity;
	line.on_hand = 0.0;
	line.oh_set = false;
	_curr.items().push_back(line);
    }
}

void
CountMaster::slotStoreChanged()
{
    _locLookup->store_id = _store->getId();
    _itemLookup->store_id = _store->getId();

    _items->clear();
    _items->appendRow(new VectorRow(_items->columns()));
}

void
CountMaster::cellMoved(int row, int)
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
CountMaster::cellChanged(int row, int col, Variant)
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

	    fixed on_hand, total_cost, on_order;
	    Id store_id = _store->getId();
	    QDate date = QDate::currentDate();
	    _quasar->db()->itemGeneral(item.id(), "", store_id, date,
				       on_hand, total_cost, on_order);

	    fixed quantity = 0.0;
	    if (_items->cellValue(row, 1) == tr("** Unknown Item **"))
		quantity = _items->cellValue(row, 4).toFixed();

	    _items->setCellValue(row, 1, item.description());
	    _items->setCellValue(row, 2, size);
	    _items->setCellValue(row, 3, on_hand / item.sizeQty(size));
	    _items->setCellValue(row, 4, quantity);
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

	    fixed on_hand, total_cost, on_order;
	    Id store_id = _store->getId();
	    QDate date = QDate::currentDate();
	    _quasar->db()->itemGeneral(item.id(), "", store_id, date,
				       on_hand, total_cost, on_order);

	    _items->setCellValue(row, 3, on_hand / item.sizeQty(size));
	}
	break;
    }
}

void
CountMaster::focusNext(bool& leave, int&, int& newCol, int type)
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
CountMaster::findItem(Id item_id, Item& item)
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
CountMaster::appendItem(const Item& item, const QString& number,
			const QString& in_size, fixed qty)
{
    QString size = in_size;
    if (size.isEmpty()) size = item.sellSize();
    fixed size_qty = item.sizeQty(size);

    fixed on_hand, total_cost, on_order;
    QDate date = QDate::currentDate();
    _quasar->db()->itemGeneral(item.id(), "", _curr.storeId(),
			       date, on_hand, total_cost, on_order);
    on_hand = on_hand / size_qty;

    VectorRow* row = new VectorRow(_items->columns());
    row->setValue(0, Plu(item.id(), number));
    row->setValue(1, item.description());
    row->setValue(2, size);
    row->setValue(3, on_hand);
    row->setValue(4, qty);
    _items->insertRow(_items->rows() - 1, row);
}
