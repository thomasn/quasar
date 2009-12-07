// $Id: item_list.cpp,v 1.41 2005/01/30 04:25:31 bpepers Exp $
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

#include "item_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "item.h"
#include "item_select.h"
#include "item.h"
#include "item_select.h"
#include "dept.h"
#include "subdept.h"
#include "group.h"
#include "item_price.h"
#include "item_master.h"
#include "line_edit.h"
#include "lookup_edit.h"
#include "dept_lookup.h"
#include "subdept_lookup.h"
#include "location_lookup.h"
#include "store_lookup.h"
#include "group_lookup.h"
#include "vendor_lookup.h"
#include "inquiry.h"
#include "item_history.h"
#include "price_list.h"
#include "sales_history.h"
#include "stock_status.h"
#include "on_order.h"
#include "item_margin.h"
#include "money_valcon.h"
#include "grid.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpopupmenu.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>
#include <qtabwidget.h>
#include <qmessagebox.h>

ItemList::ItemList(MainWindow* main)
    : ActiveList(main, "ItemList")
{
    _helpSource = "item_list.html";

    _list->addTextColumn(tr("Item Number"), 18, AlignRight);
    _list->addTextColumn(tr("Description"), 30);
    _list->addTextColumn(tr("Department"), 20);
    _list->addTextColumn(tr("Subdepartment"), 20);
    _list->addTextColumn(tr("Size"), 10);
    _list->setSorting(0);

    QLabel* numberLabel = new QLabel(tr("&Item Number:"), _search);
    _number = new LineEdit(_search);
    _number->setLength(18, '9');
    numberLabel->setBuddy(_number);
    connect(_number, SIGNAL(returnPressed()), SLOT(slotRefresh()));

    QLabel* descLabel = new QLabel(tr("&Description:"), _search);
    _description = new LineEdit(_search);
    _description->setLength(40);
    descLabel->setBuddy(_description);
    connect(_description, SIGNAL(returnPressed()), SLOT(slotRefresh()));

    QLabel* deptLabel = new QLabel(tr("Department:"), _search);
    _dept = new LookupEdit(new DeptLookup(_main, this), _search);
    _dept->setLength(30);
    deptLabel->setBuddy(_dept);
    connect(_dept, SIGNAL(validData()), SLOT(slotDeptChanged()));
    connect(_dept, SIGNAL(returnPressed()), SLOT(slotRefresh()));

    QLabel* groupLabel = new QLabel(tr("&Group:"), _search);
    _group = new LookupEdit(new GroupLookup(_main, this, Group::ITEM),
			     _search);
    _group->setLength(15);
    groupLabel->setBuddy(_group);
    connect(_group, SIGNAL(returnPressed()), SLOT(slotRefresh()));

    QLabel* subdeptLabel = new QLabel(tr("S&ubdepartment:"), _search);
    _subLookup = new SubdeptLookup(_main, this);
    _subdept = new LookupEdit(_subLookup, _search);
    _subdept->setLength(30);
    subdeptLabel->setBuddy(_subdept);
    connect(_subdept, SIGNAL(validData()), SLOT(slotSubdeptChanged()));
    connect(_subdept, SIGNAL(returnPressed()), SLOT(slotRefresh()));

    QLabel* locationLabel = new QLabel(tr("&Location:"), _search);
    _locLookup = new LocationLookup(_main, this);
    _location = new LookupEdit(_locLookup, _search);
    _location->setLength(30);
    locationLabel->setBuddy(_location);
    connect(_location, SIGNAL(returnPressed()), SLOT(slotRefresh()));

    QLabel* vendorLabel = new QLabel(tr("&Vendor:"), _search);
    _vendor = new LookupEdit(new VendorLookup(_main, this), _search);
    _vendor->setLength(30);
    vendorLabel->setBuddy(_vendor);
    connect(_vendor, SIGNAL(returnPressed()), SLOT(slotRefresh()));

    QLabel* storeLabel = new QLabel(tr("&Store:"), _search);
    _store = new LookupEdit(new StoreLookup(_main, this), _search);
    _store->setLength(30);
    storeLabel->setBuddy(_store);
    connect(_store, SIGNAL(validData()), SLOT(slotStoreChanged()));
    connect(_store, SIGNAL(returnPressed()), SLOT(slotRefresh()));

    QGridLayout* grid = new QGridLayout(_search);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(2, 1);
    grid->addWidget(numberLabel, 0, 0);
    grid->addWidget(_number, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(descLabel, 0, 3);
    grid->addWidget(_description, 0, 4, AlignLeft | AlignVCenter);
    grid->addWidget(deptLabel, 1, 0);
    grid->addWidget(_dept, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(groupLabel, 1, 3);
    grid->addWidget(_group, 1, 4, AlignLeft | AlignVCenter);
    grid->addWidget(subdeptLabel, 2, 0);
    grid->addWidget(_subdept, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(locationLabel, 2, 3);
    grid->addWidget(_location, 2, 4, AlignLeft | AlignVCenter);
    grid->addWidget(vendorLabel, 3, 0);
    grid->addWidget(_vendor, 3, 1, AlignLeft | AlignVCenter);
    grid->addWidget(storeLabel, 3, 3);
    grid->addWidget(_store, 3, 4, AlignLeft | AlignVCenter);

    _locLookup->store_id = _quasar->defaultStore();
    _store->setId(_quasar->defaultStore());
    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    _show_vendor = new QCheckBox(tr("Show Vendor Number?"), _extra);
    _show_unstocked = new QCheckBox(tr("Show Not Stocked?"), _extra);
    connect(_show_unstocked, SIGNAL(toggled(bool)),SLOT(slotStockedChanged()));
    _grid->setColStretch(1, 1);
    _grid->setColStretch(3, 1);
    _grid->addWidget(_show_vendor, 0, 2, AlignLeft | AlignVCenter);
    _grid->addWidget(_show_unstocked, 0, 4, AlignLeft | AlignVCenter);

    setCaption(tr("Item List"));
    finalize(false);
    _number->setFocus();
}

ItemList::~ItemList()
{
}

void
ItemList::slotDeptChanged()
{
    if (_subdept->getId() != INVALID_ID) {
	Subdept subdept;
	_quasar->db()->lookup(_subdept->getId(), subdept);
	if (subdept.deptId() != _dept->getId())
	    _subdept->setId(INVALID_ID);
    }
    _subLookup->dept->setId(_dept->getId());
}

void
ItemList::slotSubdeptChanged()
{
    if (_subdept->getId() != INVALID_ID) {
	Subdept subdept;
	_quasar->db()->lookup(_subdept->getId(), subdept);
	_dept->setId(subdept.deptId());
	_subLookup->dept->setId(subdept.deptId());
    }
}

void
ItemList::slotStoreChanged()
{
    _location->setId(INVALID_ID);
    _locLookup->store_id = _store->getId();
}

void
ItemList::slotActivities()
{
    ActiveList::slotActivities();
    _activities->insertItem(tr("Inquiry"), this, SLOT(slotInquiry()));
    _activities->insertItem(tr("Item History"), this, SLOT(slotItemHistory()));
    _activities->insertItem(tr("Prices and Costs"),this,SLOT(slotPriceCost()));
    _activities->insertItem(tr("Sales History"),this,SLOT(slotSalesHistory()));
    _activities->insertItem(tr("Stock Status"), this, SLOT(slotStockStatus()));
    _activities->insertItem(tr("On Order"), this, SLOT(slotOnOrder()));
    _activities->insertItem(tr("Item Margin"), this, SLOT(slotItemMargin()));
}

void
ItemList::addToPopup(QPopupMenu* menu)
{
    ActiveList::addToPopup(menu);
    menu->insertItem(tr("Inquiry"), this, SLOT(slotInquiry()));
    menu->insertItem(tr("Item History"), this, SLOT(slotItemHistory()));
    menu->insertItem(tr("Prices and Costs"), this, SLOT(slotPriceCost()));
    menu->insertItem(tr("Sales History"), this, SLOT(slotSalesHistory()));
    menu->insertItem(tr("Stock Status"), this, SLOT(slotStockStatus()));
    menu->insertItem(tr("On Order"), this, SLOT(slotOnOrder()));
    menu->insertItem(tr("Item Margin"), this, SLOT(slotItemMargin()));
}

void
ItemList::slotInquiry()
{
    Id item_id = currentId();
    if (item_id == INVALID_ID) return;

    Item item;
    _quasar->db()->lookup(item_id, item);

    Inquiry* window = (Inquiry*)(_main->findWindow("Inquiry"));
    if (window == NULL) {
	window = new Inquiry(_main);
	window->show();
    }
    window->setStoreId(_store->getId());
    window->setItemId(item.id());
}

void
ItemList::slotItemHistory()
{
    Id item_id = currentId();
    if (item_id == INVALID_ID) return;

    Item item;
    _quasar->db()->lookup(item_id, item);

    ItemHistory* window = (ItemHistory*)(_main->findWindow("ItemHistory"));
    if (window == NULL) {
	window = new ItemHistory(_main);
	window->show();
    }
    window->setStoreId(_store->getId());
    window->setItem(item.id(), item.number());
}

void
ItemList::slotPriceCost()
{
    Id item_id = currentId();
    if (item_id == INVALID_ID) return;

    PriceList* window = (PriceList*)(_main->findWindow("PriceList"));
    if (window == NULL) {
	window = new PriceList(_main);
	window->show();
    }
    window->setStoreId(_store->getId());
    window->setItemId(item_id);
}

void
ItemList::slotSalesHistory()
{
    Id item_id = currentId();
    if (item_id == INVALID_ID) return;

    Item item;
    _quasar->db()->lookup(item_id, item);

    SalesHistory* window = (SalesHistory*)(_main->findWindow("SalesHistory"));
    if (window == NULL) {
	window = new SalesHistory(_main);
	window->show();
    }
    window->setStoreId(_store->getId());
    window->setItem(item.id(), item.number());
}

void
ItemList::slotStockStatus()
{
    Id item_id = currentId();
    if (item_id == INVALID_ID) return;

    Item item;
    _quasar->db()->lookup(item_id, item);

    StockStatus* window = (StockStatus*)(_main->findWindow("StockStatus"));
    if (window == NULL) {
	window = new StockStatus(_main);
	window->show();
    }
    window->setItem(item.id(), item.number());
}

void
ItemList::slotOnOrder()
{
    Id item_id = currentId();
    if (item_id == INVALID_ID) return;

    Item item;
    _quasar->db()->lookup(item_id, item);

    OnOrder* window = (OnOrder*)(_main->findWindow("OnOrder"));
    if (window == NULL) {
	window = new OnOrder(_main);
	window->show();
    }
    window->setStoreId(_store->getId());
    window->setItem(item.id(), item.number());
}

void
ItemList::slotItemMargin()
{
    Id item_id = currentId();
    if (item_id == INVALID_ID) return;

    Item item;
    _quasar->db()->lookup(item_id, item);

    ItemMargin* window = (ItemMargin*)(_main->findWindow("ItemMargin"));
    if (window == NULL) {
	window = new ItemMargin(_main);
	window->show();
    }
    window->setStoreId(_store->getId());
    window->setItem(item.id(), item.number());
}

void
ItemList::slotStockedChanged()
{
    clearLists();

    int columns = _list->columns();
    if (_show_unstocked->isChecked()) {
	_list->addCheckColumn(tr("Stocked?"));
	resize(width() + _list->columnWidth(columns), height());
    } else {
	int column = _list->findColumn(tr("Stocked?"));
	if (column != -1) {
	    resize(width() - _list->columnWidth(column), height());
	    _list->removeColumn(column);
	}
    }
}

bool
ItemList::isActive(Id item_id)
{
    Item item;
    _quasar->db()->lookup(item_id, item);
    return item.isActive();
}

void
ItemList::setActive(Id item_id, bool active)
{
    if (item_id == INVALID_ID) return;

    Item item;
    _quasar->db()->lookup(item_id, item);

    if (!active && item.isInventoried()) {
	fixed on_hand, total_cost, on_order;
	_quasar->db()->itemGeneral(item.id(), "", INVALID_ID, QDate(),
				   on_hand, total_cost, on_order);
	if (on_hand != 0.0 || total_cost != 0.0) {
	    QString message = tr("This item still has either an on hand\n"
		"quantity or a cost.  It must be adjusted to\n"
		"zero before it can be made inactive.");
	    qApp->beep();
	    QMessageBox::critical(this, tr("Error"), message,
				  QMessageBox::Ok, 0);
	    return;
	}
	if (on_order != 0.0) {
	    QString message = tr("This item is on order.  You must cancel\n"
		"the order before it can be made inactive.");
	    qApp->beep();
	    QMessageBox::critical(this, tr("Error"), message,
				  QMessageBox::Ok, 0);
	    return;
	}
    }

    Item orig = item;
    _quasar->db()->setActive(item, active);
    _quasar->db()->update(orig, item);
}

void
ItemList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    bool showVendor = _show_vendor->isChecked();
    bool showUnstocked = _show_unstocked->isChecked();
    int inactiveColumn = _list->findColumn(tr("Inactive?"));
    int stockedColumn = _list->findColumn(tr("Stocked?"));
    Id item_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    _dept_cache.clear();
    _subdept_cache.clear();

    QString searchNumber = _number->text();
    QString searchDesc = _description->text();

    ItemSelect conditions;
    conditions.activeOnly = !showInactive;
    if (!searchNumber.isEmpty())
	conditions.number = searchNumber;
    if (!searchDesc.isEmpty())
	conditions.description = "%" + searchDesc + "%";
    conditions.group_id = _group->getId();
    conditions.vendor_id = _vendor->getId();
    conditions.dept_id = _dept->getId();
    conditions.subdept_id = _subdept->getId();
    conditions.store_id = _store->getId();
    conditions.location_id = _location->getId();
    conditions.stockedOnly = !showUnstocked;
    conditions.checkOrderNum = true;

    int count;
    _quasar->db()->count(count, conditions);
    if (count > 100) {
	QApplication::restoreOverrideCursor();

	QString message = tr("This will select %1 items\n"
			     "which may be slow.  Continue?").arg(count);
	int ch = QMessageBox::warning(this, tr("Are You Sure?"), message,
				      QMessageBox::Yes, QMessageBox::No);
	if (ch != QMessageBox::Yes) return;

	QApplication::setOverrideCursor(waitCursor);
	qApp->processEvents();
    }

    vector<Item> items;
    _quasar->db()->select(items, conditions);

    for (unsigned int i = 0; i < items.size(); ++i) {
	Item& item = items[i];
	bool stocked = item.stocked(_store->getId());

	QString number;
	if (item.numbers().size() > 0)
	    number = item.numbers()[0].number;

	if (showVendor && item.vendors().size() > 0) {
	    QString vendorNum = item.vendors()[0].number;
	    if (!vendorNum.isEmpty()) {
		number = vendorNum;
	    }
	}

	QString size;
	for (unsigned int i = 0; i < item.numbers().size(); ++i) {
	    if (item.numbers()[i].number != number) continue;
	    size = item.numbers()[i].size;
	    break;
	}
	if (size.isEmpty() && item.sizes().size() == 1)
	    size = item.sizes()[0].name;

	Dept dept;
	findDept(item.deptId(), dept);

	Subdept subdept;
	findSubdept(item.subdeptId(), subdept);

	ListViewItem* lvi = new ListViewItem(_list, item.id());
	lvi->setValue(0, number);
	lvi->setValue(1, item.description());
	lvi->setValue(2, dept.name());
	lvi->setValue(3, subdept.name());
	lvi->setValue(4, size);
	if (showInactive) lvi->setValue(inactiveColumn, !item.isActive());
	if (showUnstocked) lvi->setValue(stockedColumn, stocked);
	if (item.id() == item_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
ItemList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Item List"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
ItemList::performNew()
{
    ItemMaster* master = new ItemMaster(_main, INVALID_ID);
    master->show();
}

void
ItemList::performEdit()
{
    Id item_id = currentId();
    ItemMaster* master = new ItemMaster(_main, item_id);
    master->show();
}

bool
ItemList::findDept(Id dept_id, Dept& dept)
{
    for (unsigned int i = 0; i < _dept_cache.size(); ++i) {
	if (_dept_cache[i].id() == dept_id) {
	    dept = _dept_cache[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(dept_id, dept))
	return false;

    _dept_cache.push_back(dept);
    return true;
}

bool
ItemList::findSubdept(Id subdept_id, Subdept& subdept)
{
    for (unsigned int i = 0; i < _subdept_cache.size(); ++i) {
	if (_subdept_cache[i].id() == subdept_id) {
	    subdept = _subdept_cache[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(subdept_id, subdept))
	return false;

    _subdept_cache.push_back(subdept);
    return true;
}
