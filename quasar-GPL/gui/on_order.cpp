// $Id: on_order.cpp,v 1.24 2005/03/13 23:13:45 bpepers Exp $
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

#include "on_order.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "item_edit.h"
#include "vendor_lookup.h"
#include "store_lookup.h"
#include "dept_lookup.h"
#include "subdept_lookup.h"
#include "item_lookup.h"
#include "order_master.h"
#include "list_view_item.h"
#include "vendor.h"
#include "order.h"
#include "order_select.h"
#include "item.h"
#include "subdept.h"
#include "store.h"
#include "money_edit.h"
#include "double_edit.h"
#include "grid.h"
#include "text_frame.h"
#include "line_frame.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qtimer.h>

OnOrder::OnOrder(MainWindow* main)
    : QuasarWindow(main, "OnOrder")
{
    _helpSource = "on_order.html";

    QFrame* frame = new QFrame(this);

    QLabel* vendorLabel = new QLabel(tr("Vendor:"), frame);
    _vendor = new LookupEdit(new VendorLookup(main, this), frame);
    _vendor->setLength(30);
    vendorLabel->setBuddy(_vendor);

    QPushButton* allVendors = new QPushButton(tr("All Vendors"), frame);
    connect(allVendors, SIGNAL(clicked()), SLOT(slotAllVendors()));

    QLabel* storeLabel = new QLabel(tr("Store:"), frame);
    _store = new LookupEdit(new StoreLookup(main, this), frame);
    _store->setLength(30);
    storeLabel->setBuddy(_store);
    connect(_store, SIGNAL(validData()), SLOT(slotStoreChanged()));

    QLabel* deptLabel = new QLabel(tr("Department:"), frame);
    _dept = new LookupEdit(new DeptLookup(main, this), frame);
    _dept->setLength(30);
    deptLabel->setBuddy(_dept);

    QLabel* subdeptLabel = new QLabel(tr("Subdepartment:"), frame);
    _subdept = new LookupEdit(new SubdeptLookup(main, this), frame);
    _subdept->setLength(30);
    subdeptLabel->setBuddy(_subdept);

    QLabel* itemLabel = new QLabel(tr("Item:"), frame);
    _lookup = new ItemLookup(main, this);
    _lookup->purchasedOnly = true;
    _item = new ItemEdit(_lookup, frame);
    _item->setLength(18, '9');
    itemLabel->setBuddy(_item);
    connect(_item, SIGNAL(validData()), SLOT(slotItemChanged()));

    QLabel* sizeLabel = new QLabel(tr("Size:"), frame);
    _size = new QComboBox(false, frame);
    sizeLabel->setBuddy(_size);
    _size->insertItem(tr("All Sizes"));
    connect(_size, SIGNAL(activated(int)), SLOT(slotSizeChanged()));

    _list = new ListView(frame);
    _list->addTextColumn(tr("Vendor"), 20);
    _list->addTextColumn(tr("Order#"), 8, AlignRight);
    _list->addTextColumn(tr("Store"), 6, AlignRight);
    _list->addTextColumn(tr("Item Number"), 18, AlignRight);
    _list->addTextColumn(tr("Description"), 20);
    _list->addTextColumn(tr("Size"), 8);
    _list->addNumberColumn(tr("On Order"));
    _list->addMoneyColumn(tr("Amount"));
    _list->setAllColumnsShowFocus(true);
    _list->setShowSortIndicator(true);
    connect(_list, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotPickLine()));

    QFrame* total = new QFrame(frame);
    QLabel* qtyLabel = new QLabel(tr("Total Quantity:"), total);
    _totalQty = new DoubleEdit(total);
    _totalQty->setLength(20, '9');
    _totalQty->setFocusPolicy(NoFocus);

    QLabel* costLabel = new QLabel(tr("Total Cost:"), total);
    _totalCost = new MoneyEdit(total);
    _totalCost->setLength(20, '9');
    _totalCost->setFocusPolicy(NoFocus);

    QGridLayout* totalGrid = new QGridLayout(total);
    totalGrid->setSpacing(3);
    totalGrid->setColStretch(0, 1);
    totalGrid->addWidget(qtyLabel, 0, 0, AlignRight|AlignVCenter);
    totalGrid->addWidget(_totalQty, 0, 1);
    totalGrid->addWidget(costLabel, 1, 0, AlignRight|AlignVCenter);
    totalGrid->addWidget(_totalCost, 1, 1);

    QFrame* buttons = new QFrame(frame);
    QPushButton* refresh = new QPushButton(tr("&Refresh"), buttons);
    connect(refresh, SIGNAL(clicked()), SLOT(slotRefresh()));
    refresh->setMinimumSize(refresh->sizeHint());

    QPushButton* print = new QPushButton(tr("&Print"), buttons);
    connect(print, SIGNAL(clicked()), SLOT(slotPrint()));
    print->setMinimumSize(refresh->sizeHint());

    QPushButton* ok = new QPushButton(tr("&Close"), buttons);
    connect(ok, SIGNAL(clicked()), SLOT(close()));
    ok->setMinimumSize(refresh->sizeHint());

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(2, 1);
    buttonGrid->addWidget(refresh, 0, 0, AlignLeft | AlignVCenter);
    buttonGrid->addWidget(print, 0, 1, AlignLeft | AlignVCenter);
    buttonGrid->addWidget(ok, 0, 2, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setRowStretch(4, 1);
    grid->setColStretch(2, 1);
    grid->addWidget(vendorLabel, 0, 0, AlignLeft | AlignVCenter);
    grid->addWidget(_vendor, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(allVendors, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(storeLabel, 3, 0, AlignRight | AlignVCenter);
    grid->addWidget(_store, 3, 1, AlignLeft | AlignVCenter);
    grid->addWidget(deptLabel, 0, 2, AlignRight | AlignVCenter);
    grid->addWidget(_dept, 0, 3, AlignLeft | AlignVCenter);
    grid->addWidget(subdeptLabel, 1, 2, AlignRight | AlignVCenter);
    grid->addWidget(_subdept, 1, 3, AlignLeft | AlignVCenter);
    grid->addWidget(itemLabel, 2, 2, AlignRight | AlignVCenter);
    grid->addWidget(_item, 2, 3, AlignLeft | AlignVCenter);
    grid->addWidget(sizeLabel, 3, 2, AlignRight | AlignVCenter);
    grid->addWidget(_size, 3, 3, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_list, 4, 4, 0, 3);
    grid->addMultiCellWidget(total, 5, 5, 0, 3);
    grid->addMultiCellWidget(buttons, 6, 6, 0, 3);

    _vendor->setFocus();
    _store->setId(_quasar->defaultStore());

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    setCentralWidget(frame);
    setCaption(tr("On Order"));
    finalize();
}

OnOrder::~OnOrder()
{
}

void
OnOrder::setStoreId(Id store_id)
{
    _store->setId(store_id);
}

void
OnOrder::setItem(Id item_id, const QString& number)
{
    _item->setValue(Plu(item_id, number));
    slotItemChanged();
    slotRefresh();
}

void
OnOrder::slotStoreChanged()
{
    _lookup->store_id = _store->getId();
    _item->setId(INVALID_ID);
}

void
OnOrder::slotItemChanged()
{
    _size->clear();
    _size->insertItem(tr("All Sizes"));

    Item item;
    if (_quasar->db()->lookup(_item->getId(), item)) {
	QString number = _item->value().toPlu().number();
	QString size = item.numberSize(number);
	for (unsigned int i = 0; i < item.sizes().size(); ++i) {
	    _size->insertItem(item.sizes()[i].name);
	    if (size == item.sizes()[i].name)
		_size->setCurrentItem(_size->count() - 1);
	}
    }
}

void
OnOrder::slotSizeChanged()
{
    // TODO: find better number?
}

void
OnOrder::slotAllVendors()
{
    if (_vendor->getId() == INVALID_ID)
	return;
    _vendor->setId(INVALID_ID);
}

void
OnOrder::slotPickLine()
{
    ListViewItem* item = (ListViewItem*)_list->currentItem();
    if (item == NULL) return;

    OrderMaster* edit = new OrderMaster(_main, item->id);
    edit->show();
}

void
OnOrder::slotPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("On Order"));
    Grid* header = new Grid(1, grid);
    grid->set(0, 0, 1, grid->columns(), header, Grid::AllSides);

    header->setColumnWeight(0, 1);
    TextFrame* text = new TextFrame(tr("On Order"), header);
    text->setFont(Font("Times", 24));
    header->set(USE_NEXT, 0, text);
    if (_vendor->getId() != INVALID_ID) {
	Card card;
	_quasar->db()->lookup(_vendor->getId(), card);
	text = new TextFrame(card.nameFL(), header);
	text->setFont(Font("Times", 14));
	header->set(USE_NEXT, 0, " ");
	header->set(USE_NEXT, 0, text);
    }

    // Add totals line
    grid->set(grid->rows(), 0, " ");
    grid->set(USE_NEXT, 0, tr("Total"));
    grid->set(USE_CURR, 4, _totalQty->text());
    grid->set(USE_CURR, 5, _totalCost->text());

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
OnOrder::slotRefresh()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    OrderSelect conditions;
    conditions.vendor_id = _vendor->getId();
    conditions.store_id = _store->getId();
    conditions.activeOnly = true;

    vector<Order> orders;
    _quasar->db()->select(orders, conditions);

    Item item;
    _quasar->db()->lookup(_item->getId(), item);

    Id itemMatch = item.id();
    Id subdeptMatch = _subdept->getId();
    Id deptMatch = _dept->getId();
    QString sizeMatch = _size->currentText();

    fixed totalQty = 0.0;
    fixed totalCost = 0.0;

    _list->clear();
    for (unsigned int i = 0; i < orders.size(); ++i) {
	const Order& order = orders[i];

	Vendor vendor;
	_quasar->db()->lookup(order.vendorId(), vendor);

	Store store;
	_quasar->db()->lookup(order.storeId(), store);

	for (unsigned int j = 0; j < order.items().size(); ++j) {
	    const OrderItem& line = order.items()[j];

	    if (itemMatch != INVALID_ID && line.item_id != itemMatch)
		continue;

	    Item item;
	    _quasar->db()->lookup(line.item_id, item);

	    if (subdeptMatch != INVALID_ID && item.subdeptId() != subdeptMatch)
		continue;

	    if (deptMatch != INVALID_ID) {
		Subdept subdept;
		_quasar->db()->lookup(item.subdeptId(), subdept);
		if (subdept.deptId() != deptMatch)
		    continue;
	    }

	    if (sizeMatch != tr("All Sizes") && line.size != sizeMatch)
		continue;

	    QString description = line.description;
	    if (description.isEmpty())
		description = item.description();

	    fixed remain = line.ordered - line.billed;
	    fixed base_cost = 0.0;
	    if (line.ordered != 0.0)
	        base_cost = line.ext_base / line.ordered;
	    fixed ext_cost = base_cost * remain;
	    ext_cost.moneyRound();

	    ListViewItem* itm = new ListViewItem(_list, order.id());
	    itm->setValue(0, vendor.name());
	    itm->setValue(1, order.number());
	    itm->setValue(2, store.number());
	    itm->setValue(3, item.number());
	    itm->setValue(4, description);
	    itm->setValue(5, line.size);
	    itm->setValue(6, remain);
	    itm->setValue(7, ext_cost);

	    totalQty += remain;
	    totalCost += ext_cost;
	}
    }

    _totalQty->setFixed(totalQty);
    _totalCost->setFixed(totalCost);

    QApplication::restoreOverrideCursor();
}
