// $Id: item_history.cpp,v 1.18 2005/03/13 23:13:45 bpepers Exp $
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

#include "item_history.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "inquiry.h"
#include "item.h"
#include "list_view_item.h"
#include "item_edit.h"
#include "date_range.h"
#include "date_popup.h"
#include "item_lookup.h"
#include "store_lookup.h"
#include "integer_edit.h"
#include "money_valcon.h"
#include "grid.h"
#include "text_frame.h"
#include "line_frame.h"

#include <qapplication.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtimer.h>

ItemHistory::ItemHistory(MainWindow* main)
    : QuasarWindow(main, "ItemHistory"), _grid(NULL), _last(NULL)
{
    _helpSource = "item_history.html";

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
    _size = new QComboBox(false, select);
    sizeLabel->setBuddy(_size);
    _size->insertItem(tr("All Sizes"));

    QLabel* dateLabel = new QLabel(tr("Dates:"), select);
    _dateRange = new DateRange(select);
    dateLabel->setBuddy(_dateRange);

    QLabel* fromLabel = new QLabel(tr("From:"), select);
    _from = new DatePopup(select);
    fromLabel->setBuddy(_from);

    QLabel* toLabel = new QLabel(tr("To:"), select);
    _to = new DatePopup(select);
    toLabel->setBuddy(_to);

    grid1->setColStretch(2, 1);
    grid1->addColSpacing(2, 40);
    grid1->setColStretch(5, 1);
    grid1->addColSpacing(5, 40);
    grid1->addWidget(itemLabel, 1, 0);
    grid1->addWidget(_item, 1, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(descLabel, 2, 0);
    grid1->addWidget(_desc, 2, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(storeLabel, 3, 0);
    grid1->addWidget(_store, 3, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(sizeLabel, 1, 3, AlignRight | AlignVCenter);
    grid1->addWidget(_size, 1, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(dateLabel, 1, 6, AlignRight | AlignVCenter);
    grid1->addWidget(_dateRange, 1, 7, AlignLeft | AlignVCenter);
    grid1->addWidget(fromLabel, 2, 6, AlignRight | AlignVCenter);
    grid1->addWidget(_from, 2, 7, AlignLeft | AlignVCenter);
    grid1->addWidget(toLabel, 3, 6, AlignRight | AlignVCenter);
    grid1->addWidget(_to, 3, 7, AlignLeft | AlignVCenter);

    QGroupBox* format = new QGroupBox(tr("Data Format"), frame);
    QGridLayout* grid2 = new QGridLayout(format, 2, 1, format->frameWidth()*2);
    grid2->addRowSpacing(0, format->fontMetrics().height());

    QLabel* qtySizeLabel = new QLabel(tr("Quantity Size:"), format);
    _qtySize = new QComboBox(false, format);
    qtySizeLabel->setBuddy(_qtySize);
    _qtySize->insertItem("EACH");

    grid2->setColStretch(2, 1);
    grid2->addWidget(qtySizeLabel, 1, 0);
    grid2->addWidget(_qtySize, 1, 1, AlignLeft | AlignVCenter);

    _list = new ListView(frame);
    _list->addTextColumn(tr("Date"), 18);
    _list->addNumberColumn(tr("Total Qty"));
    _list->addMoneyColumn(tr("Total Cost"));
    _list->addNumberColumn(tr("Sold Qty"));
    _list->addMoneyColumn(tr("Sold Cost"));
    _list->addMoneyColumn(tr("Sold Price"));
    _list->addNumberColumn(tr("Recv Qty"));
    _list->addMoneyColumn(tr("Recv Cost"));
    _list->addNumberColumn(tr("Adj Qty"));
    _list->addMoneyColumn(tr("Adj Cost"));
    connect(_list, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotDoubleClick()));

    // Reset widths to try and make as small as possible
    int costWidth = _list->fontMetrics().width("$9,999,999.99");
    _list->setColumnWidth(1, _list->fontMetrics().width(" Total Qty "));
    _list->setColumnWidth(2, costWidth);
    _list->setColumnWidth(3, _list->fontMetrics().width(" Sold Qty "));
    _list->setColumnWidth(4, costWidth);
    _list->setColumnWidth(5, costWidth);
    _list->setColumnWidth(6, _list->fontMetrics().width(" Recv Qty "));
    _list->setColumnWidth(7, costWidth);
    _list->setColumnWidth(8, _list->fontMetrics().width(" Adj Qty "));
    _list->setColumnWidth(9, costWidth);
    _list->setSorting(-1);
    _list->setAllColumnsShowFocus(true);

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
    grid->addWidget(box, 3, 0);

    _dateRange->setFromPopup(_from);
    _dateRange->setToPopup(_to);
    _dateRange->setCurrentItem(DateRange::Today);
    _item->setFocus();

    connect(_item, SIGNAL(validData()), SLOT(slotItemChanged()));
    connect(_store, SIGNAL(validData()), SLOT(slotStoreChanged()));
    connect(_size, SIGNAL(activated(int)), SLOT(slotRefresh()));
    connect(_from, SIGNAL(validData()), SLOT(slotRefresh()));
    connect(_to, SIGNAL(validData()), SLOT(slotRefresh()));
    connect(_qtySize, SIGNAL(activated(int)), SLOT(slotRefresh()));

    _store->setId(_quasar->defaultStore());
    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    setCentralWidget(frame);
    setCaption(tr("Item History"));
    finalize();
}

ItemHistory::~ItemHistory()
{
}

void
ItemHistory::setStoreId(Id store_id)
{
    _store->setId(store_id);
}

void
ItemHistory::setItem(Id item_id, const QString& number)
{
    _item->setValue(Plu(item_id, number));
    slotItemChanged();
}

void
ItemHistory::addLine(QString date, fixed qty, fixed cost, fixed sold_qty,
		     fixed sold_cost, fixed sold_price, fixed recv_qty,
		     fixed recv_cost, fixed adj_qty, fixed adj_cost,
		     fixed sizeQty)
{
    ListViewItem* lvi = new ListViewItem(_list, _last);
    _last = lvi;

    if (sizeQty > 1.0) {
	qty = qty / sizeQty;
	sold_qty = sold_qty / sizeQty;
	recv_qty = recv_qty / sizeQty;
	adj_qty = adj_qty / sizeQty;
	qty.moneyRound();
	sold_qty.moneyRound();
	recv_qty.moneyRound();
	adj_qty.moneyRound();
    }

    lvi->setValue(0, date);
    lvi->setValue(1, qty);
    lvi->setValue(2, cost);
    lvi->setValue(3, sold_qty);
    lvi->setValue(4, sold_cost);
    lvi->setValue(5, sold_price);
    lvi->setValue(6, recv_qty);
    lvi->setValue(7, recv_cost);
    lvi->setValue(8, adj_qty);
    lvi->setValue(9, adj_cost);
}

void
ItemHistory::slotItemChanged()
{
    _desc->setText("");
    _size->clear();
    _qtySize->clear();
    _list->clear();
    _last = NULL;
    _size->insertItem(tr("All Sizes"));
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

    slotRefresh();
}

void
ItemHistory::slotStoreChanged()
{
    _lookup->store_id = _store->getId();
    _item->setId(INVALID_ID);
    slotRefresh();
}

static fixed
sum(vector<fixed>& amts)
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < amts.size(); ++i)
        total += amts[i];
    return total;
}

void
ItemHistory::slotRefresh()
{
    _list->clear();
    _last = NULL;
    delete _grid;
    _grid = NULL;

    Id item_id = _item->getId();
    if (item_id == INVALID_ID) return;

    Item item;
    _quasar->db()->lookup(item_id, item);

    Id store = _store->getId();
    QDate from = _from->getDate();
    QDate to = _to->getDate();

    QString size = _size->currentText();
    if (size == tr("All Sizes")) size = "";
    fixed sizeQty = item.sizeQty(_qtySize->currentText());

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    fixed openQty = 0.0;
    fixed openCost = 0.0;
    fixed openSoldQty = 0.0;
    fixed openSoldCost = 0.0;
    fixed openSoldPrice = 0.0;
    fixed openRecvQty = 0.0;
    fixed openRecvCost = 0.0;
    fixed openAdjQty = 0.0;
    fixed openAdjCost = 0.0;

    vector<Id> item_ids;
    vector<QString> sizes;
    vector<Id> stores;
    vector<QDate> dates;
    vector<fixed> qtys;
    vector<fixed> costs;
    vector<fixed> orders;
    vector<fixed> sold_qtys;
    vector<fixed> sold_costs;
    vector<fixed> sold_prices;
    vector<fixed> recv_qtys;
    vector<fixed> recv_costs;
    vector<fixed> adj_qtys;
    vector<fixed> adj_costs;

    // Get initial balance
    if (!from.isNull()) {
	_quasar->db()->itemTotals(item.id(), size, store, from - 1,
				  item_ids, sizes, stores, qtys, costs,
				  orders, sold_qtys, sold_costs, sold_prices,
				  recv_qtys, recv_costs, adj_qtys, adj_costs);
	if (qtys.size() > 0) {
	    openQty = sum(qtys);
	    openCost = sum(costs);
	    openSoldQty = sum(sold_qtys);
	    openSoldCost = sum(sold_costs);
	    openSoldPrice = sum(sold_prices);
	    openRecvQty = sum(recv_qtys);
	    openRecvCost = sum(recv_costs);
	    openAdjQty = sum(adj_qtys);
	    openAdjCost = sum(adj_costs);
	}

	// Add opening balances
	addLine(tr("Opening"), openQty, openCost, openSoldQty, openSoldCost,
		openSoldPrice, openRecvQty, openRecvCost, openAdjQty,
		openAdjCost, sizeQty);
    }

    // Select data
    _quasar->db()->itemTotals(item.id(), size, store, from, to,
			      dates, qtys, costs, sold_qtys, sold_costs,
			      sold_prices, recv_qtys, recv_costs, adj_qtys,
			      adj_costs);

    // Load up list
    for (unsigned int i = 0; i < dates.size(); ++i) {
	addLine(DateValcon().format(dates[i]), qtys[i], costs[i],
		sold_qtys[i], sold_costs[i], sold_prices[i], recv_qtys[i],
		recv_costs[i], adj_qtys[i], adj_costs[i], sizeQty);

	openQty += qtys[i];
	openCost += costs[i];
	openSoldQty += sold_qtys[i];
	openSoldCost += sold_costs[i];
	openSoldPrice += sold_prices[i];
	openRecvQty += recv_qtys[i];
	openRecvCost += recv_costs[i];
	openAdjQty += adj_qtys[i];
	openAdjCost += adj_costs[i];
    }

    // Add closing balances
    addLine(tr("Closing"), openQty, openCost, openSoldQty, openSoldCost,
	    openSoldPrice, openRecvQty, openRecvCost, openAdjQty,
	    openAdjCost, sizeQty);

    _grid = Grid::buildGrid(_list, tr("Item History"));

    // New header
    Grid* header = new Grid(1, _grid);
    header->setColumnWeight(0, 1);
    TextFrame* text;
    text = new TextFrame(tr("Item History"), header);
    text->setFont(Font("Times", 24));
    header->set(USE_NEXT, 0, text);
    text = new TextFrame(item.number() + " " + item.description(), header);
    text->setFont(Font("Times", 18));
    header->set(USE_NEXT, 0, text);
    if (!from.isNull() || !to.isNull()) {
	QString msg = "";
	if (!from.isNull()) msg += tr("From: ") + from.toString();
	if (!to.isNull()) {
	    if (!msg.isNull()) msg += "    ";
	    msg += tr("To: ") + to.toString();
	}
	text = new TextFrame(msg, header);
	header->set(USE_NEXT, 0, text);
    }
    _grid->set(0, 0, 1, _grid->columns(), header, Grid::AllSides);

    QApplication::restoreOverrideCursor();
}

void
ItemHistory::slotPrint()
{
    if (_grid == NULL) return;
    _grid->print(this);
}

void
ItemHistory::slotDoubleClick()
{
    ListViewItem* item = (ListViewItem*)_list->currentItem();
    if (item == NULL) return;
    if (item->text(0) == tr("Opening")) return;
    if (item->text(0) == tr("Closing")) return;

    DateValcon valcon;
    if (!valcon.parse(item->text(0))) return;
    QDate date = valcon.getDate();

    Inquiry* window = (Inquiry*)_main->findWindow("Inquiry");
    if (window == NULL) {
	window = new Inquiry(_main);
	window->show();
    }
    window->setStoreId(_store->getId());
    window->setCurrentDates(date, date);
    window->setItemId(_item->getId());
}
