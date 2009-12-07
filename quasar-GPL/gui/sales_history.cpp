// $Id: sales_history.cpp,v 1.35 2005/05/12 07:57:57 bpepers Exp $
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

#include "sales_history.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "item.h"
#include "list_view_item.h"
#include "item_edit.h"
#include "date_popup.h"
#include "item_lookup.h"
#include "store_lookup.h"
#include "integer_edit.h"
#include "double_edit.h"
#include "money_valcon.h"
#include "percent_valcon.h"
#include "grid.h"
#include "text_frame.h"
#include "line_frame.h"
#include "combo_box.h"

#include <qapplication.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtimer.h>

SalesHistory::SalesHistory(MainWindow* main)
    : QuasarWindow(main, "SalesHistory"), _grid(NULL)
{
    _helpSource = "sales_history.html";

    QFrame* frame = new QFrame(this);

    QGroupBox* select = new QGroupBox(tr("Data Selection"), frame);
    QGridLayout* grid1 = new QGridLayout(select, 2, 1, select->frameWidth()*2);
    grid1->addRowSpacing(0, select->fontMetrics().height());

    _lookup = new ItemLookup(_main, this);
    _lookup->soldOnly = true;
    _lookup->store_id = _quasar->defaultStore();

    QLabel* itemLabel = new QLabel(tr("Item Number:"), select);
    _item = new ItemEdit(_lookup, select);
    _item->setLength(18, '9');
    itemLabel->setBuddy(_item);

    QLabel* descLabel = new QLabel(tr("Description:"), select);
    _desc = new LineEdit(select);
    _desc->setLength(30);
    _desc->setFocusPolicy(NoFocus);

    QLabel* storeLabel = new QLabel(tr("Store:"), select);
    _store = new LookupEdit(new StoreLookup(_main, this), select);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    QLabel* sizeLabel = new QLabel(tr("Size:"), select);
    _size = new ComboBox(false, select);
    sizeLabel->setBuddy(_size);
    _size->insertItem(tr("All Sizes"));

    QLabel* dateLabel = new QLabel(tr("Date:"), select);
    _date = new DatePopup(select);
    dateLabel->setBuddy(_date);

    grid1->setColStretch(2, 1);
    grid1->addWidget(itemLabel, 1, 0);
    grid1->addWidget(_item, 1, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(descLabel, 2, 0);
    grid1->addWidget(_desc, 2, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(storeLabel, 3, 0);
    grid1->addWidget(_store, 3, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(sizeLabel, 1, 2, AlignRight | AlignVCenter);
    grid1->addWidget(_size, 1, 3, AlignLeft | AlignVCenter);
    grid1->addWidget(dateLabel, 2, 2, AlignRight | AlignVCenter);
    grid1->addWidget(_date, 2, 3, AlignLeft | AlignVCenter);

    QGroupBox* format = new QGroupBox(tr("Data Format"), frame);
    QGridLayout* grid2 = new QGridLayout(format, 2, 1, format->frameWidth()*2);
    grid2->addRowSpacing(0, format->fontMetrics().height());

    QLabel* qtySizeLabel = new QLabel(tr("Quantity Size:"), format);
    _qtySize = new ComboBox(false, format);
    qtySizeLabel->setBuddy(_qtySize);
    _qtySize->insertItem("EACH");

    QLabel* periodLabel = new QLabel(tr("Period:"), format);
    _period = new ComboBox(false, format);
    periodLabel->setBuddy(_period);

    _period->insertItem(tr("Month"));
    _period->insertItem(tr("Week"));
    _period->insertItem(tr("Day"));

    QLabel* countLabel = new QLabel(tr("Count:"), format);
    _count = new IntegerEdit(format);
    _count->setLength(4);
    countLabel->setBuddy(_count);

    grid2->setColStretch(2, 1);
    grid2->addWidget(qtySizeLabel, 1, 0);
    grid2->addWidget(_qtySize, 1, 1, AlignLeft | AlignVCenter);
    grid2->addWidget(periodLabel, 1, 2, AlignRight | AlignVCenter);
    grid2->addWidget(_period, 1, 3, AlignLeft | AlignVCenter);
    grid2->addWidget(countLabel, 2, 2, AlignRight | AlignVCenter);
    grid2->addWidget(_count, 2, 3, AlignLeft | AlignVCenter);

    _list = new ListView(frame);
    _list->addTextColumn(tr("Period"), 10);
    _list->addDateColumn(tr("Start Date"));
    _list->addNumberColumn(tr("Quantity"));
    _list->addMoneyColumn(tr("Total Cost"));
    _list->addMoneyColumn(tr("Total Price"));
    _list->addMoneyColumn(tr("Profit"));
    _list->addPercentColumn(tr("Margin"));
    _list->setSorting(1, false);
    _list->setShowSortIndicator(true);

    QFrame* info = new QFrame(frame);

    QLabel* minLabel = new QLabel(tr("Min:"), info);
    _minQty = new DoubleEdit(info);
    _minQty->setFocusPolicy(NoFocus);

    QLabel* maxLabel = new QLabel(tr("Max:"), info);
    _maxQty = new DoubleEdit(info);
    _maxQty->setFocusPolicy(NoFocus);

    QLabel* onHandLabel = new QLabel(tr("On Hand:"), info);
    _onHand = new DoubleEdit(info);
    _onHand->setFocusPolicy(NoFocus);

    QLabel* onOrderLabel = new QLabel(tr("On Order:"), info);
    _onOrder = new DoubleEdit(info);
    _onOrder->setFocusPolicy(NoFocus);

    QGridLayout* infoGrid = new QGridLayout(info);
    infoGrid->setSpacing(6);
    infoGrid->setMargin(6);
    infoGrid->setColStretch(2, 1);
    infoGrid->addWidget(minLabel, 0, 0);
    infoGrid->addWidget(_minQty, 0, 1, AlignLeft | AlignVCenter);
    infoGrid->addWidget(onHandLabel, 0, 3);
    infoGrid->addWidget(_onHand, 0, 4, AlignLeft | AlignVCenter);
    infoGrid->addWidget(maxLabel, 1, 0);
    infoGrid->addWidget(_maxQty, 1, 1, AlignLeft | AlignVCenter);
    infoGrid->addWidget(onOrderLabel, 1, 3);
    infoGrid->addWidget(_onOrder, 1, 4, AlignLeft | AlignVCenter);

    QFrame* box = new QFrame(frame);

    QPushButton* refresh = new QPushButton(tr("&Refresh"), box);
    refresh->setMinimumSize(refresh->sizeHint());
    connect(refresh, SIGNAL(clicked()), SLOT(slotRefresh()));

    QPushButton* print = new QPushButton(tr("&Print"), box);
    print->setMinimumSize(refresh->sizeHint());
    connect(print, SIGNAL(clicked()), SLOT(slotPrint()));

    QPushButton* close = new QPushButton(tr("Cl&ose"), box);
    close->setMinimumSize(refresh->sizeHint());
    connect(close, SIGNAL(clicked()), SLOT(close()));

    QGridLayout* boxGrid = new QGridLayout(box);
    boxGrid->setSpacing(6);
    boxGrid->setMargin(6);
    boxGrid->setColStretch(2, 1);
    boxGrid->addWidget(refresh, 0, 0, AlignLeft | AlignVCenter);
    boxGrid->addWidget(print, 0, 1, AlignLeft | AlignVCenter);
    boxGrid->addWidget(close, 0, 2, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setRowStretch(2, 1);
    grid->addWidget(select, 0, 0);
    grid->addWidget(format, 1, 0);
    grid->addWidget(_list, 2, 0);
    grid->addWidget(info, 3, 0);
    grid->addWidget(box, 4, 0);

    connect(_item, SIGNAL(validData()), SLOT(slotItemChanged()));
    connect(_store, SIGNAL(validData()), SLOT(slotStoreChanged()));
    connect(_size, SIGNAL(activated(int)), SLOT(slotRefresh()));
    connect(_date, SIGNAL(validData()), SLOT(slotRefresh()));
    connect(_qtySize, SIGNAL(activated(int)), SLOT(slotRefresh()));
    connect(_period, SIGNAL(activated(int)), SLOT(slotRefresh()));
    connect(_count, SIGNAL(validData()), SLOT(slotRefresh()));

    _store->setId(_quasar->defaultStore());
    _date->setDate(QDate::currentDate());
    _count->setInt(12);
    _item->setFocus();

    setCentralWidget(frame);
    setCaption(tr("Sales History"));
    finalize();
}

SalesHistory::~SalesHistory()
{
}

void
SalesHistory::setStoreId(Id store_id)
{
    _store->setId(store_id);
}

void
SalesHistory::setItem(Id item_id, const QString& number)
{
    if (_item->getId() == item_id) return;
    _item->setValue(Plu(item_id, number));
    slotItemChanged();
}

void
SalesHistory::addLine(const QString& period, QDate date, fixed qty,
		      fixed cost, fixed price, fixed sizeQty)
{
    ListViewItem* lvi = new ListViewItem(_list);

    fixed profit = price - cost;
    fixed margin = 0.0;
    if (price != 0.0)
	margin = (profit) / price * 100.0;

    lvi->setValue(0, period);
    lvi->setValue(1, date);
    if (sizeQty == 1.0)
	lvi->setValue(2, qty);
    else
	lvi->setValue(2, qty / sizeQty);
    lvi->setValue(3, cost);
    lvi->setValue(4, price);
    lvi->setValue(5, profit);
    lvi->setValue(6, margin);

    if (period == tr("Total"))
	Font::defaultFont = Font("helvetica", 10.0, true);

    _grid->set(USE_NEXT, 0, period);
    _grid->set(USE_CURR, 1, DateValcon().format(date));
    _grid->set(USE_CURR, 2, NumberValcon().format(qty));
    _grid->set(USE_CURR, 3, MoneyValcon().format(cost));
    _grid->set(USE_CURR, 4, MoneyValcon().format(price));
    _grid->set(USE_CURR, 5, MoneyValcon().format(profit));
    _grid->set(USE_CURR, 6, PercentValcon().format(margin));
}

void
SalesHistory::slotItemChanged()
{
    _desc->setText("");
    _size->clear();
    _qtySize->clear();
    _list->clear();
    _size->insertItem(tr("All Sizes"));
    _minQty->setText("");
    _maxQty->setText("");
    _onHand->setText("");
    _onOrder->setText("");
    delete _grid;
    _grid = NULL;

    Id item_id = _item->getId();
    if (item_id == INVALID_ID) return;

    Item item;
    _quasar->db()->lookup(item_id, item);

    _desc->setText(item.description());
    for (unsigned i = 0; i < item.sizes().size(); ++i) {
	QString size = item.sizes()[i].name;
	_size->insertItem(size);
	_qtySize->insertItem(size);
    }
    _qtySize->setCurrentItem(item.sellSize());

    QString size = "";
    Id store_id = _store->getId();
    QDate date = _date->getDate();

    fixed onHand, onOrder, totalCost;
    _quasar->db()->itemGeneral(item.id(), size, store_id, date,
			       onHand, totalCost, onOrder);

    fixed sellSizeQty = item.sizeQty(item.sellSize());
    if (sellSizeQty != 0) {
	onHand = onHand / sellSizeQty;
	onOrder = onOrder / sellSizeQty;
    }

    _minQty->setFixed(item.minQty(store_id));
    _maxQty->setFixed(item.maxQty(store_id));
    _onHand->setFixed(onHand);
    _onOrder->setFixed(onOrder);

    slotRefresh();
}

void
SalesHistory::slotStoreChanged()
{
    _lookup->store_id = _store->getId();
    slotRefresh();
}

void
SalesHistory::slotRefresh()
{
    _list->clear();
    delete _grid;
    _grid = NULL;

    Id item_id = _item->getId();
    if (item_id == INVALID_ID) return;

    Item item;
    _quasar->db()->lookup(item_id, item);

    int count = _count->getInt();
    if (count <= 0) return;

    Id store = _store->getId();

    QString size = _size->currentText();
    if (size == tr("All Sizes")) size = "";
    fixed sizeQty = item.sizeQty(_qtySize->currentText());

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    vector<QDate> from;
    vector<QDate> to;
    vector<fixed> qtys;
    vector<fixed> costs;
    vector<fixed> prices;

    int year = _date->getDate().year();
    int month = _date->getDate().month();
    int day = _date->getDate().day();
    int dow = _date->getDate().dayOfWeek();

    if (_period->currentItem() == 0)
	day = 1;
    else if (_period->currentItem() == 1)
	day -= dow;

    // Setup value ranges
    int line;
    for (line = 0; line < count; ++line) {
	QDate start(year, month, day);
	QDate end = start;

	switch (_period->currentItem()) {
	case 0: // Month
	    end = end + start.daysInMonth() - 1;
	    --month;
	    break;
	case 1: // Week
	    end = end + 6;
	    day -= 7;
	    break;
	case 2: // Day
	    --day;
	    break;
	}

	from.push_back(start);
	to.push_back(end);

	if (day < 1) {
	    --month;
	    if (month < 1) {
		--year;
		month = 12;
	    }
	    day += QDate(year, month, 1).daysInMonth();
	}
	if (month < 1) {
	    --year;
	    month = 12;
	}

	qtys.push_back(0.0);
	costs.push_back(0.0);
	prices.push_back(0.0);
    }

    for (unsigned int i = 0; i < from.size(); ++i) {
	fixed qty, cost, price;
	QDate start = from[i];
	QDate end = to[i];

	if (end > _date->getDate())
	    end = _date->getDate();
	_quasar->db()->itemSold(item.id(), size, store, start, end,
				qty, cost, price);

	qtys[i] += qty;
	costs[i] += cost;
	prices[i] += price;
    }

    // Setup grid
    _grid = new Grid(7);
    Grid* header = new Grid(1, _grid);
    TextFrame* text;
    text = new TextFrame(tr("Sales History"), header);
    text->setFont(Font("Times", 24));
    header->set(USE_NEXT, 0, text);
    text = new TextFrame(item.number() + " " + item.description(), header);
    text->setFont(Font("Times", 18));
    header->set(USE_NEXT, 0, text);
    header->setColumnWeight(0, 1);

    _grid->set(0, 0, 1, _grid->columns(), header, Grid::AllSides);
    _grid->set(USE_NEXT, 0, "");
    for (int column = 0; column < 7; ++column) {
	_grid->setColumnSticky(column, column < 2 ? Grid::Left : Grid::Right);
	_grid->setColumnPad(column, 5);
    }
    _grid->set(2, 0, tr("Period"));
    _grid->set(3, 0, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 1, tr("Start Date"));
    _grid->set(3, 1, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 2, tr("Quantity"));
    _grid->set(3, 2, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 3, tr("Total Cost"));
    _grid->set(3, 3, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 4, tr("Total Price"));
    _grid->set(3, 4, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 5, tr("Profit"));
    _grid->set(3, 5, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 6, tr("Margin"));
    _grid->set(3, 6, new LineFrame(_grid), Grid::LeftRight);
    _grid->setHeaderRows(_grid->rows());

    // Print out totals
    fixed total_qty = 0.0;
    fixed total_cost = 0.0;
    fixed total_price = 0.0;
    for (line = 0; line < count; ++line) {
	QDate date = from[line];
	fixed qty = qtys[line];
	fixed cost = costs[line];
	fixed price = prices[line];

	QString period = "";
	switch (_period->currentItem()) {
	case 0: // Month
	    period = QDate::monthName(date.month());
	    break;
	case 1: // Week
	    period = tr("Week ") + QString::number(line + 1);
	    break;
	case 2: // Day
	    period = QDate::dayName(date.dayOfWeek());
	    break;
	}

	addLine(period, date, qty, cost, price, sizeQty);

	total_qty += qty;
	total_cost += cost;
	total_price += price;
    }

    addLine(tr("Total"), QDate(), total_qty, total_cost, total_price,
	    sizeQty);

    QApplication::restoreOverrideCursor();
}

void
SalesHistory::slotPrint()
{
    if (_grid == NULL) return;
    _grid->print(this);
}
