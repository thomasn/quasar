// $Id: stock_status.cpp,v 1.22 2005/03/13 22:09:36 bpepers Exp $
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

#include "stock_status.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "dept.h"
#include "subdept.h"
#include "group.h"
#include "location.h"
#include "item.h"
#include "store.h"
#include "list_view_item.h"
#include "item_edit.h"
#include "dept_lookup.h"
#include "subdept_lookup.h"
#include "group_lookup.h"
#include "location_lookup.h"
#include "store_lookup.h"
#include "item_lookup.h"
#include "item_select.h"
#include "store_select.h"
#include "grid.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlayout.h>

StockStatus::StockStatus(MainWindow* main)
    : QuasarWindow(main, "StockStatus")
{
    _helpSource = "stock_status.html";

    QFrame* frame = new QFrame(this);

    QFrame* single = new QFrame(frame);
    _single = new QRadioButton(tr("Single Item"), single);

    ItemLookup* lookup = new ItemLookup(_main, this);
    lookup->stockedOnly = true;
    lookup->inventoriedOnly = true;

    QLabel* itemLabel = new QLabel(tr("Item Number:"), single);
    _item = new LookupEdit(lookup, single);
    _item->setLength(18, '9');
    itemLabel->setBuddy(_item);

    QLabel* descLabel = new QLabel(tr("Description:"), single);
    _desc = new LineEdit(single);
    _desc->setLength(30);
    _desc->setFocusPolicy(NoFocus);

    QLabel* sizeLabel = new QLabel(tr("Size:"), single);
    _size = new LineEdit(single);
    _size->setLength(8);
    _size->setFocusPolicy(NoFocus);

    QGridLayout* singGrid = new QGridLayout(single);
    singGrid->setSpacing(3);
    singGrid->setMargin(3);
    singGrid->setColStretch(3, 1);
    singGrid->addColSpacing(0, 20);
    singGrid->addColSpacing(3, 10);
    singGrid->addMultiCellWidget(_single, 0, 0, 0, 5);
    singGrid->addWidget(itemLabel, 1, 1);
    singGrid->addWidget(_item, 1, 2);
    singGrid->addWidget(sizeLabel, 1, 4);
    singGrid->addWidget(_size, 1, 5);
    singGrid->addWidget(descLabel, 2, 1);
    singGrid->addWidget(_desc, 2, 2);

    QFrame* multiple = new QFrame(frame);
    _multiple = new QRadioButton(tr("Multiple Items"), multiple);

    QLabel* deptLabel = new QLabel(tr("Department:"), multiple);
    _deptLookup = new DeptLookup(_main, this);
    _dept = new LookupEdit(_deptLookup, multiple);
    _dept->setLength(30);
    deptLabel->setBuddy(_dept);

    QLabel* subdeptLabel = new QLabel(tr("Subdepartment:"), multiple);
    _subdeptLookup = new SubdeptLookup(_main, this);
    _subdept = new LookupEdit(_subdeptLookup, multiple);
    _subdept->setLength(30);
    subdeptLabel->setBuddy(_subdept);

    QLabel* groupLabel = new QLabel(tr("Group:"), multiple);
    GroupLookup* groupLookup = new GroupLookup(_main, this, Group::ITEM);
    _group = new LookupEdit(groupLookup, multiple);
    _group->setLength(30);
    groupLabel->setBuddy(_group);

    QLabel* locationLabel = new QLabel(tr("Location:"), multiple);
    _locLookup = new LocationLookup(_main, this);
    _location = new LookupEdit(_locLookup, multiple);
    _location->setLength(30);
    locationLabel->setBuddy(_location);

    QLabel* storeLabel = new QLabel(tr("Store:"), multiple);
    _store = new LookupEdit(new StoreLookup(_main, this), multiple);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    QGridLayout* multGrid = new QGridLayout(multiple);
    multGrid->setSpacing(3);
    multGrid->setMargin(3);
    multGrid->setColStretch(3, 1);
    multGrid->addColSpacing(0, 20);
    multGrid->addColSpacing(3, 10);
    multGrid->addMultiCellWidget(_multiple, 0, 0, 0, 5);
    multGrid->addWidget(deptLabel, 1, 1);
    multGrid->addWidget(_dept, 1, 2);
    multGrid->addWidget(groupLabel, 1, 4);
    multGrid->addWidget(_group, 1, 5);
    multGrid->addWidget(subdeptLabel, 2, 1);
    multGrid->addWidget(_subdept, 2, 2);
    multGrid->addWidget(locationLabel, 2, 4);
    multGrid->addWidget(_location, 2, 5);
    multGrid->addWidget(storeLabel, 3, 4);
    multGrid->addWidget(_store, 3, 5);

    QButtonGroup* types = new QButtonGroup(this);
    types->hide();
    types->insert(_single);
    types->insert(_multiple);
    connect(types, SIGNAL(clicked(int)), SLOT(slotTypeChanged()));

    _list = new ListView(frame);
    _list->setAllColumnsShowFocus(true);
    _list->setShowSortIndicator(true);
    connect(_list, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotItemSelected()));

    QFrame* buttons = new QFrame(frame);

    QPushButton* refresh = new QPushButton(tr("&Refresh"), buttons);
    refresh->setMinimumSize(refresh->sizeHint());
    connect(refresh, SIGNAL(clicked()), SLOT(slotRefresh()));

    QPushButton* print = new QPushButton(tr("&Print"), buttons);
    print->setMinimumSize(refresh->sizeHint());
    connect(print, SIGNAL(clicked()), SLOT(slotPrint()));

    QPushButton* close = new QPushButton(tr("&Close"), buttons);
    close->setMinimumSize(refresh->sizeHint());
    connect(close, SIGNAL(clicked()), SLOT(close()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(2, 1);
    buttonGrid->addWidget(refresh, 0, 0, AlignLeft | AlignVCenter);
    buttonGrid->addWidget(print, 0, 1, AlignLeft | AlignVCenter);
    buttonGrid->addWidget(close, 0, 2, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setRowStretch(2, 1);
    grid->addWidget(single, 0, 0);
    grid->addWidget(multiple, 1, 0);
    grid->addWidget(_list, 2, 0);
    grid->addWidget(buttons, 3, 0);

    connect(_dept, SIGNAL(validData()), SLOT(slotDeptChanged()));
    connect(_subdept, SIGNAL(validData()), SLOT(slotSubdeptChanged()));
    connect(_store, SIGNAL(validData()), SLOT(slotStoreChanged()));
    connect(_item, SIGNAL(validData()), SLOT(slotRefresh()));

    _single->setChecked(true);
    _item->setFocus();
    _store->setId(_quasar->defaultStore());
    slotTypeChanged();

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    setCentralWidget(frame);
    setCaption(tr("Stock Status"));
    finalize();
}

StockStatus::~StockStatus()
{
}

void
StockStatus::setItem(Id item_id, const QString& number)
{
    if (_item->getId() == item_id) return;
    _single->setChecked(true);
    _item->setValue(Plu(item_id, number));
    slotRefresh();
}

void
StockStatus::slotTypeChanged()
{
    _list->clear();
    while (_list->columns() > 0) _list->removeColumn(0);

    if (_single->isChecked()) {
	_item->setEnabled(true);
	_desc->setEnabled(true);
	_size->setEnabled(true);
	_dept->setEnabled(false);
	_subdept->setEnabled(false);
	_group->setEnabled(false);
	_location->setEnabled(false);
	_store->setEnabled(false);

	_list->addTextColumn(tr("Store"), 30);
	_list->addTextColumn(tr("Location"), 30);
	_list->addNumberColumn(tr("On Hand"));
	_list->addNumberColumn(tr("On Order"));
    } else {
	_item->setEnabled(false);
	_desc->setEnabled(false);
	_size->setEnabled(false);
	_dept->setEnabled(true);
	_subdept->setEnabled(true);
	_group->setEnabled(true);
	_location->setEnabled(true);
	_store->setEnabled(true);

	_list->addTextColumn(tr("Item"), 18, AlignRight);
	_list->addTextColumn(tr("Description"), 30);
	_list->addTextColumn(tr("Size"), 10);
	_list->addNumberColumn(tr("On Hand"));
	_list->addNumberColumn(tr("On Order"));
    }
}

void
StockStatus::slotDeptChanged()
{
    if (_subdept->getId() != INVALID_ID) {
	Subdept subdept;
	_quasar->db()->lookup(_subdept->getId(), subdept);
	if (subdept.deptId() != _dept->getId())
	    _subdept->setId(INVALID_ID);
    }
    _subdeptLookup->dept->setId(_dept->getId());
}

void
StockStatus::slotSubdeptChanged()
{
    if (_subdept->getId() != INVALID_ID) {
	Subdept subdept;
	_quasar->db()->lookup(_subdept->getId(), subdept);
	_dept->setId(subdept.deptId());
	_subdeptLookup->dept->setId(subdept.deptId());
    }
}

void
StockStatus::slotStoreChanged()
{
    _location->setId(INVALID_ID);
    _locLookup->store_id = _store->getId();
}

void
StockStatus::slotItemSelected()
{
    if (_single->isChecked()) return;

    ListViewItem* current = _list->currentItem();
    if (current == NULL || current->id == INVALID_ID) return;

    Item item;
    _quasar->db()->lookup(current->id, item);

    _single->setChecked(true);
    _item->setValue(Plu(item.id(), item.number()));
    slotTypeChanged();

    slotRefresh();
}

void
StockStatus::slotRefresh()
{
    static bool inRefresh = false;
    if (inRefresh) return;
    inRefresh = true;

    _list->clear();
    if (_single->isChecked())
	refreshSingle();
    else
	refreshMultiple();

    inRefresh = false;
}

void
StockStatus::refreshSingle()
{
    _desc->setText("");
    _size->setText("");

    Id item_id = _item->getId();
    if (item_id == INVALID_ID) return;

    Item item;
    _quasar->db()->lookup(item_id, item);

    _desc->setText(item.description());
    _size->setText(item.sellSize());
    fixed sizeQty = item.sizeQty(item.sellSize());

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    StoreSelect conditions;
    conditions.activeOnly = true;
    vector<Store> stores;
    _quasar->db()->select(stores, conditions);

    fixed total_on_hand = 0.0;
    fixed total_on_order = 0.0;
    ListViewItem* last = NULL;
    for (unsigned int i = 0; i < stores.size(); ++i) {
	const Store& store = stores[i];

	Location location;
	Id location_id = item.locationId(store.id());
	_quasar->db()->lookup(location_id, location);

	QDate date = QDate::currentDate();
	fixed on_hand, total_cost, on_order;
	_quasar->db()->itemGeneral(item.id(), "", store.id(), date,
				   on_hand, total_cost, on_order);

	ListViewItem* lvi = new ListViewItem(_list, last);
	last = lvi;
	lvi->setValue(0, store.name());
	lvi->setValue(1, location.name());
	lvi->setValue(2, on_hand / sizeQty);
	lvi->setValue(3, on_order / sizeQty);

	total_on_hand += on_hand;
	total_on_order += on_order;
    }

    if (stores.size() > 1) {
	ListViewItem* lvi = new ListViewItem(_list, last);
	lvi->setValue(0, tr("Total"));
	lvi->setValue(2, total_on_hand / sizeQty);
	lvi->setValue(3, total_on_order / sizeQty);
	lvi->isLast = true;
    }

    QApplication::restoreOverrideCursor();
}

void
StockStatus::refreshMultiple()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    ItemSelect conditions;
    conditions.dept_id = _dept->getId();
    conditions.subdept_id = _subdept->getId();
    conditions.group_id = _group->getId();
    conditions.location_id = _location->getId();
    conditions.store_id = _store->getId();
    conditions.activeOnly = true;
    conditions.stockedOnly = true;
    conditions.inventoriedOnly = true;

    vector<Item> items;
    _quasar->db()->select(items, conditions);

    ListViewItem* last = NULL;
    for (unsigned int i = 0; i < items.size(); ++i) {
	const Item& item = items[i];
	QString size = item.sellSize();
	fixed sizeQty = item.sizeQty(size);

	QDate date = QDate::currentDate();
	fixed on_hand, total_cost, on_order;
	_quasar->db()->itemGeneral(item.id(), "", _store->getId(), date,
				   on_hand, total_cost, on_order);

	ListViewItem* lvi = new ListViewItem(_list, last, item.id());
	last = lvi;
	lvi->setValue(0, item.number());
	lvi->setValue(1, item.description());
	lvi->setValue(2, size);
	lvi->setValue(3, on_hand / sizeQty);
	lvi->setValue(4, on_order / sizeQty);
    }

    QApplication::restoreOverrideCursor();
}

void
StockStatus::slotPrint()
{
    QString title = tr("Stock Status");
    if (_single->isChecked()) {
	Item item;
	_quasar->db()->lookup(_item->getId(), item);

	title += "\n\n" + item.number() + "\n" + item.description();
    } else {
	QStringList lines;
	if (_dept->getId() != INVALID_ID) {
	    Dept dept;
	    _quasar->db()->lookup(_dept->getId(), dept);
	    lines << tr("Department: ") + dept.name();
	}
	if (_subdept->getId() != INVALID_ID) {
	    Subdept subdept;
	    _quasar->db()->lookup(_subdept->getId(), subdept);
	    lines << tr("Subdepartment: ") + subdept.name();
	}
	if (_group->getId() != INVALID_ID) {
	    Group group;
	    _quasar->db()->lookup(_group->getId(), group);
	    lines << tr("Group: ") + group.name();
	}
	if (_location->getId() != INVALID_ID) {
	    Location location;
	    _quasar->db()->lookup(_location->getId(), location);
	    lines << tr("Location: ") + location.name();
	}

	if (lines.count() > 0)
	    title += "\n\n" + lines.join("\n");
    }

    Grid* grid = Grid::buildGrid(_list, title);
    grid->print(this);
    delete grid;
}
