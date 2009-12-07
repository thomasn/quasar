// $Id: sales_report.cpp,v 1.27 2004/10/09 00:37:57 bpepers Exp $
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

#include "sales_report.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "item_edit.h"
#include "customer_lookup.h"
#include "group_lookup.h"
#include "store_lookup.h"
#include "dept_lookup.h"
#include "subdept_lookup.h"
#include "item_lookup.h"
#include "date_range.h"
#include "date_popup.h"
#include "list_view_item.h"
#include "customer.h"
#include "item.h"
#include "subdept.h"
#include "store.h"
#include "group.h"
#include "invoice_select.h"
#include "money_edit.h"
#include "double_edit.h"
#include "percent_edit.h"
#include "grid.h"
#include "text_frame.h"
#include "line_frame.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlayout.h>

SalesReport::SalesReport(MainWindow* main)
    : QuasarWindow(main, "SalesReport")
{
    _helpSource = "sales_report.html";

    QFrame* frame = new QFrame(this);

    QLabel* customerLabel = new QLabel(tr("Customer:"), frame);
    _customer = new LookupEdit(new CustomerLookup(main, this), frame);
    _customer->setLength(30);
    customerLabel->setBuddy(_customer);

    QLabel* groupLabel = new QLabel(tr("Group:"), frame);
    _group = new LookupEdit(new GroupLookup(main, this, Group::CUSTOMER),
			    frame);
    _group->setLength(30);
    groupLabel->setBuddy(_group);

    QLabel* rangeLabel = new QLabel(tr("Range:"), frame);
    _range = new DateRange(frame);
    rangeLabel->setBuddy(_range);

    QLabel* fromLabel = new QLabel(tr("From:"), frame);
    _from = new DatePopup(frame);
    fromLabel->setBuddy(_from);

    QLabel* toLabel = new QLabel(tr("To:"), frame);
    _to = new DatePopup(frame);
    toLabel->setBuddy(_to);

    _range->setFromPopup(_from);
    _range->setToPopup(_to);
    _range->setCurrentItem(DateRange::Today);

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
    _lookup->soldOnly = true;
    _item = new ItemEdit(_lookup, frame);
    _item->setLength(30);
    itemLabel->setBuddy(_item);
    connect(_item, SIGNAL(validData()), SLOT(slotItemChanged()));

    QLabel* sizeLabel = new QLabel(tr("Size:"), frame);
    _size = new QComboBox(false, frame);
    _size->insertItem(tr("All Sizes"));
    sizeLabel->setBuddy(_size);
    connect(_size, SIGNAL(activated(int)), SLOT(slotSizeChanged()));

    QLabel* storeLabel = new QLabel(tr("Store:"), frame);
    _store = new LookupEdit(new StoreLookup(main, this), frame);
    _store->setLength(30);
    storeLabel->setBuddy(_store);
    connect(_store, SIGNAL(validData()), SLOT(slotStoreChanged()));

    _list = new ListView(frame);
    _list->addTextColumn(tr("Customer"), 24);
    _list->addDateColumn(tr("Date"));
    _list->addTextColumn(tr("Tx #"), 10, AlignRight);
    _list->addTextColumn(tr("Store"), 6, AlignRight);
    _list->addTextColumn(tr("Item"), 12, AlignRight);
    _list->addTextColumn(tr("Size"), 8);
    _list->addTextColumn(tr("Description"), 16);
    _list->addNumberColumn(tr("Quantity"));
    _list->addMoneyColumn(tr("Price"));
    _list->setAllColumnsShowFocus(true);
    _list->setShowSortIndicator(true);
    connect(_list, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotPickLine()));

    // Reset widths to try and make as small as possible
    int costWidth = _list->fontMetrics().width("$9,999,999.99");
    _list->setColumnWidth(2, _list->fontMetrics().width(tr("  Tx #  ")));
    _list->setColumnWidth(7, _list->fontMetrics().width(tr(" Quantity ")));
    _list->setColumnWidth(8, costWidth);

    QFrame* total = new QFrame(frame);
    QLabel* qtyLabel = new QLabel(tr("Total Quantity:"), total);
    _totalQty = new DoubleEdit(total);
    _totalQty->setLength(20, '9');
    _totalQty->setFocusPolicy(NoFocus);

    QLabel* priceLabel = new QLabel(tr("Total Price:"), total);
    _totalPrice = new MoneyEdit(total);
    _totalPrice->setLength(20, '9');
    _totalPrice->setFocusPolicy(NoFocus);

    QGridLayout* totalGrid = new QGridLayout(total);
    totalGrid->setSpacing(3);
    totalGrid->setColStretch(0, 1);
    totalGrid->addWidget(qtyLabel, 0, 0, AlignRight|AlignVCenter);
    totalGrid->addWidget(_totalQty, 0, 1);
    totalGrid->addWidget(priceLabel, 1, 0, AlignRight|AlignVCenter);
    totalGrid->addWidget(_totalPrice, 1, 1);

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
    grid->setRowStretch(5, 1);
    grid->setColStretch(2, 1);
    grid->addWidget(customerLabel, 0, 0, AlignLeft | AlignVCenter);
    grid->addWidget(_customer, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(groupLabel, 1, 0, AlignLeft | AlignVCenter);
    grid->addWidget(_group, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(rangeLabel, 2, 0, AlignLeft | AlignVCenter);
    grid->addWidget(_range, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(fromLabel, 3, 0, AlignLeft | AlignVCenter);
    grid->addWidget(_from, 3, 1, AlignLeft | AlignVCenter);
    grid->addWidget(toLabel, 4, 0, AlignLeft | AlignVCenter);
    grid->addWidget(_to, 4, 1, AlignLeft | AlignVCenter);
    grid->addWidget(deptLabel, 0, 2, AlignRight | AlignVCenter);
    grid->addWidget(_dept, 0, 3, AlignLeft | AlignVCenter);
    grid->addWidget(subdeptLabel, 1, 2, AlignRight | AlignVCenter);
    grid->addWidget(_subdept, 1, 3, AlignLeft | AlignVCenter);
    grid->addWidget(itemLabel, 2, 2, AlignRight | AlignVCenter);
    grid->addWidget(_item, 2, 3, AlignLeft | AlignVCenter);
    grid->addWidget(sizeLabel, 3, 2, AlignRight | AlignVCenter);
    grid->addWidget(_size, 3, 3, AlignLeft | AlignVCenter);
    grid->addWidget(storeLabel, 4, 2, AlignRight | AlignVCenter);
    grid->addWidget(_store, 4, 3, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_list, 5, 5, 0, 3);
    grid->addMultiCellWidget(total, 6, 6, 0, 3);
    grid->addMultiCellWidget(buttons, 7, 7, 0, 3);

    _customer->setFocus();
    _store->setId(_quasar->defaultStore());

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    setCentralWidget(frame);
    setCaption(tr("Customer Sales"));
    finalize();
}

SalesReport::~SalesReport()
{
}

void
SalesReport::setStoreId(Id store_id)
{
    _store->setId(store_id);
}

void
SalesReport::setCardId(Id card_id)
{
    _customer->setId(card_id);
    slotRefresh();
}

void
SalesReport::slotStoreChanged()
{
    _lookup->store_id = _store->getId();
    _item->setId(INVALID_ID);
}

void
SalesReport::slotItemChanged()
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
SalesReport::slotSizeChanged()
{
    // TODO: change number if better for size
}

void
SalesReport::slotPickLine()
{
    ListViewItem* item = (ListViewItem*)_list->currentItem();
    if (item == NULL) return;

    Gltx gltx;
    Id gltx_id = item->id;
    if (!_quasar->db()->lookup(gltx_id, gltx)) return;

    QWidget* edit = editGltx(gltx, _main);
    if (edit != NULL) edit->show();
}

void
SalesReport::slotPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Customer Sales"));
    Grid* header = new Grid(1, grid);
    grid->set(0, 0, 1, grid->columns(), header, Grid::AllSides);

    header->setColumnWeight(0, 1);
    TextFrame* text = new TextFrame(tr("Customer Sales"), header);
    text->setFont(Font("Times", 24));
    header->set(USE_NEXT, 0, text);
    if (_customer->getId() != INVALID_ID) {
	Card card;
	_quasar->db()->lookup(_customer->getId(), card);
	text = new TextFrame(card.nameFL(), header);
	text->setFont(Font("Times", 14));
	header->set(USE_NEXT, 0, " ");
	header->set(USE_NEXT, 0, text);
    }
    if (_group->getId() != INVALID_ID) {
	Group group;
	_quasar->db()->lookup(_group->getId(), group);
	text = new TextFrame(group.name(), header);
	text->setFont(Font("Times", 14));
	header->set(USE_NEXT, 0, " ");
	header->set(USE_NEXT, 0, text);
    }
    if (_dept->getId() != INVALID_ID) {
	Dept dept;
	_quasar->db()->lookup(_dept->getId(), dept);
	text = new TextFrame("Department " + dept.name(), header);
	text->setFont(Font("Times", 14));
	header->set(USE_NEXT, 0, " ");
	header->set(USE_NEXT, 0, text);
    }
    if (_subdept->getId() != INVALID_ID) {
	Subdept subdept;
	_quasar->db()->lookup(_subdept->getId(), subdept);
	text = new TextFrame("Subdepartment " + subdept.name(), header);
	text->setFont(Font("Times", 14));
	header->set(USE_NEXT, 0, " ");
	header->set(USE_NEXT, 0, text);
    }
    if (_item->getId() != INVALID_ID) {
	text = new TextFrame("Item " + _item->value().toString(), header);
	text->setFont(Font("Times", 14));
	header->set(USE_NEXT, 0, " ");
	header->set(USE_NEXT, 0, text);
    }
    if (!_from->getDate().isNull() || !_to->getDate().isNull()) {
	QString message;
	if (_from->getDate() == _to->getDate())
	    message = tr("On ") + _from->getDate().toString();
	else if (_from->getDate().isNull())
	    message = tr("On/Before ") + _to->getDate().toString();
	else if (_to->getDate().isNull())
	    message = tr("On/After ") + _from->getDate().toString();
	else
	    message = _from->getDate().toString() + tr(" to ") +
		_to->getDate().toString();
	text = new TextFrame(message, header);
	text->setFont(Font("Times", 14));
	header->set(USE_NEXT, 0, " ");
	header->set(USE_NEXT, 0, text);
    }

    // Add totals line
    grid->set(grid->rows(), 0, " ");
    grid->set(USE_NEXT, 0, tr("Total"));
    grid->set(USE_CURR, 14, _totalQty->text());
    grid->set(USE_CURR, 16, _totalPrice->text());

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
SalesReport::slotRefresh()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    _list->clear();
    _qtyTotal = 0.0;
    _priceTotal = 0.0;

    // Load invoices
    vector<Invoice> invoices;
    InvoiceSelect conditions;
    conditions.card_id = _customer->getId();
    conditions.card_group_id = _group->getId();
    conditions.start_date = _from->getDate();
    conditions.end_date = _to->getDate();
    conditions.store_id = _store->getId();
    conditions.activeOnly = true;
    _quasar->db()->select(invoices, conditions);

    // Add invoices
    unsigned int i;
    for (i = 0; i < invoices.size(); ++i)
	addInvoice(invoices[i]);

    _totalQty->setFixed(_qtyTotal);
    _totalPrice->setFixed(_priceTotal);

    QApplication::restoreOverrideCursor();
}

void
SalesReport::addInvoice(const Invoice& invoice)
{
    Customer customer;
    _quasar->db()->lookup(invoice.cardId(), customer);

    Item item;
    _quasar->db()->lookup(_item->getId(), item);

    Store store;
    _quasar->db()->lookup(invoice.storeId(), store);

    Id itemMatch = _item->getId();
    Id subdeptMatch = _subdept->getId();
    Id deptMatch = _dept->getId();
    QString sizeMatch = _size->currentText();

    for (unsigned int i = 0; i < invoice.items().size(); ++i) {
	const InvoiceItem& line = invoice.items()[i];
	if (line.voided) continue;

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
	if (description.isEmpty()) {
	    description = item.description();
	}

	fixed ext_price = line.ext_base;
	if (line.include_tax) ext_price += line.ext_tax;
	if (line.include_deposit) ext_price += line.ext_deposit;

	int next = 0;
	ListViewItem* itm = new ListViewItem(_list, invoice.id());
	itm->setValue(next++, customer.name());
	itm->setValue(next++, invoice.postDate());
	itm->setValue(next++, invoice.number());
	itm->setValue(next++, store.number());
	itm->setValue(next++, item.number());
	itm->setValue(next++, line.size);
	itm->setValue(next++, description);
	itm->setValue(next++, line.quantity);
	itm->setValue(next++, ext_price);

	_qtyTotal += line.quantity;
	_priceTotal += ext_price;
    }
}
