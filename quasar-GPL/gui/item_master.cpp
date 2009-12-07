// $Id: item_master.cpp,v 1.96 2005/05/13 22:39:05 bpepers Exp $
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

#include "item_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "object_cache.h"
#include "dept.h"
#include "subdept.h"
#include "price_list.h"
#include "item_margin.h"
#include "stock_status.h"
#include "company.h"
#include "group.h"
#include "item_adjust.h"
#include "item_edit.h"
#include "store_lookup.h"
#include "location_lookup.h"
#include "date_popup.h"
#include "dept_lookup.h"
#include "subdept_lookup.h"
#include "account_lookup.h"
#include "tax_lookup.h"
#include "vendor_lookup.h"
#include "item_lookup.h"
#include "extra_lookup.h"
#include "group_lookup.h"
#include "price_edit.h"
#include "money_edit.h"
#include "percent_edit.h"
#include "double_edit.h"
#include "check_box.h"
#include "account.h"
#include "table.h"
#include "item_select.h"
#include "store_select.h"
#include "extra_select.h"
#include "item_price.h"
#include "tax.h"
#include "store.h"
#include "extra.h"

#include <qapplication.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qframe.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qinputdialog.h>
#include <qmessagebox.h>

ItemMaster::ItemMaster(MainWindow* main, Id item_id)
    : DataWindow(main, "ItemMaster", item_id), _skipUpdates(false)
{
    _helpSource = "item_master.html";

    QPushButton* prices = new QPushButton(tr("Prices"), _buttons);
    connect(prices, SIGNAL(clicked()), SLOT(slotPrices()));

    QPushButton* costs = new QPushButton(tr("Costs"), _buttons);
    connect(costs, SIGNAL(clicked()), SLOT(slotCosts()));

    QPushButton* margin = new QPushButton(tr("Margin"), _buttons);
    connect(margin, SIGNAL(clicked()), SLOT(slotMargin()));

    QPushButton* status = new QPushButton(tr("Stock"), _buttons);
    connect(status, SIGNAL(clicked()), SLOT(slotStockStatus()));

    QPushButton* build = new QPushButton(tr("Build"), _buttons);
    connect(build, SIGNAL(clicked()), SLOT(slotBuild()));

    // Create widgets
    QFrame* top = new QFrame(_frame);

    QLabel* numberLabel = new QLabel(tr("Item Number:"), top);
    _number = new LineEdit(top);
    _number->setLength(18, '9');
    numberLabel->setBuddy(_number);
    connect(_number, SIGNAL(validData()), SLOT(numberChanged()));

    QLabel* descLabel = new QLabel(tr("Description:"), top);
    _desc = new LineEdit(top);
    _desc->setLength(40);
    descLabel->setBuddy(_desc);

    QLabel* deptLabel = new QLabel(tr("Dept:"), top);
    _dept = new LookupEdit(new DeptLookup(main, this), top);
    _dept->setLength(30);
    deptLabel->setBuddy(_dept);
    connect(_dept, SIGNAL(validData()), SLOT(deptChanged()));

    QLabel* subdeptLabel = new QLabel(tr("Subdept:"), top);
    _subLookup = new SubdeptLookup(main, this);
    _subdept = new LookupEdit(_subLookup, top);
    _subdept->setLength(30);
    subdeptLabel->setBuddy(_subdept);
    connect(_subdept, SIGNAL(validData()), SLOT(subdeptChanged()));

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(3);
    topGrid->setMargin(3);
    topGrid->setColStretch(2, 1);
    topGrid->addWidget(numberLabel, 0, 0), AlignLeft | AlignVCenter;
    topGrid->addWidget(_number, 0, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(descLabel, 1, 0), AlignLeft | AlignVCenter;
    topGrid->addWidget(_desc, 1, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(deptLabel, 0, 3, AlignLeft | AlignVCenter);
    topGrid->addWidget(_dept, 0, 4, AlignLeft | AlignVCenter);
    topGrid->addWidget(subdeptLabel, 1, 3, AlignLeft | AlignVCenter);
    topGrid->addWidget(_subdept, 1, 4, AlignLeft | AlignVCenter);

    QHBox* mid = new QHBox(_frame);
    mid->setSpacing(3);

    QGroupBox* type = new QGroupBox(tr("Type"), mid);
    QGridLayout* t_grid = new QGridLayout(type, 2, 2, type->frameWidth()*2);
    t_grid->addRowSpacing(0, type->fontMetrics().height());

    _purchased = new QCheckBox(tr("Purchased?"), type);
    _sold = new QCheckBox(tr("Sold?"), type);
    _inventoried = new QCheckBox(tr("Inventoried?"), type);
    _open_dept = new QCheckBox(tr("Open Department?"), type);

    _expenseLabel = new QLabel(tr("COGS Account:"), type);
    _expenseLookup = new AccountLookup(main, this, Account::COGS);
    _expense_acct = new LookupEdit(_expenseLookup, type);
    _expense_acct->setLength(30);
    _expenseLabel->setBuddy(_expense_acct);

    QLabel* incomeLabel = new QLabel(tr("Income Account:"), type);
    _income_acct = new LookupEdit(new AccountLookup(main, this,
						     Account::Income),
				   type);
    _income_acct->setLength(30);
    incomeLabel->setBuddy(_income_acct);

    QLabel* assetLabel = new QLabel(tr("Asset Account:"), type);
    _asset_acct = new LookupEdit(new AccountLookup(main, this,
						    Account::Inventory),
				  type);
    _asset_acct->setLength(30);
    assetLabel->setBuddy(_asset_acct);

    connect(_purchased, SIGNAL(toggled(bool)), SLOT(typeChanged()));
    connect(_sold, SIGNAL(toggled(bool)), SLOT(typeChanged()));
    connect(_inventoried, SIGNAL(toggled(bool)), SLOT(typeChanged()));

    t_grid->setRowStretch(5, 1);
    t_grid->setColStretch(1, 1);
    t_grid->addWidget(_purchased, 1, 0, AlignLeft | AlignVCenter);
    t_grid->addWidget(_sold, 2, 0, AlignLeft | AlignVCenter);
    t_grid->addWidget(_inventoried, 3, 0, AlignLeft | AlignVCenter);
    t_grid->addWidget(_open_dept, 4, 0, AlignLeft | AlignVCenter);
    t_grid->addWidget(_expenseLabel, 1, 1, AlignRight | AlignVCenter);
    t_grid->addWidget(_expense_acct, 1, 2, AlignLeft | AlignVCenter);
    t_grid->addWidget(incomeLabel, 2, 1, AlignRight | AlignVCenter);
    t_grid->addWidget(_income_acct, 2, 2, AlignLeft | AlignVCenter);
    t_grid->addWidget(assetLabel, 3, 1, AlignRight | AlignVCenter);
    t_grid->addWidget(_asset_acct, 3, 2, AlignLeft | AlignVCenter);

    QGroupBox* groups = new QGroupBox(tr("Groups"), mid);
    QGridLayout* g_grid = new QGridLayout(groups, 2, 1,groups->frameWidth()*2);
    g_grid->addRowSpacing(0, groups->fontMetrics().height());

    _groups = new Table(groups);
    _groups->setVScrollBarMode(QScrollView::AlwaysOn);
    _groups->setDisplayRows(2);
    _groups->setLeftMargin(fontMetrics().width("999"));
    connect(_groups, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(groupCellChanged(int,int,Variant)));

    GroupLookup* groupLookup = new GroupLookup(_main, this, Group::ITEM);
    new LookupColumn(_groups, tr("Group Name"), 20, groupLookup);
    new LookupEditor(_groups, 0, new LookupEdit(groupLookup, _groups));

    g_grid->setRowStretch(1, 1);
    g_grid->addWidget(_groups, 1, 0);

    QFrame* bot = new QFrame(_frame);
    bot->setFrameStyle(QFrame::Box | QFrame::Sunken);

    QLabel* storeLabel = new QLabel(tr("Store:"), bot);
    _store = new LookupEdit(new StoreLookup(_main, this), bot);
    _store->setLength(30);
    storeLabel->setBuddy(_store);
    connect(_store, SIGNAL(validData()), SLOT(storeChanged()));

    QLabel* locationLabel = new QLabel(tr("Location:"), bot);
    _location = new LookupEdit(new LocationLookup(_main, this), bot);
    _location->setLength(30);
    locationLabel->setBuddy(_location);
    connect(_location, SIGNAL(validData()), SLOT(setStoreInfo()));

    QLabel* minLabel = new QLabel(tr("Min:"), bot);
    _minQty = new DoubleEdit(bot);
    _minQty->setLength(8);
    minLabel->setBuddy(_minQty);
    connect(_minQty, SIGNAL(validData()), SLOT(setStoreInfo()));

    QLabel* maxLabel = new QLabel(tr("Max:"), bot);
    _maxQty = new DoubleEdit(bot);
    _maxQty->setLength(8);
    maxLabel->setBuddy(_maxQty);
    connect(_maxQty, SIGNAL(validData()), SLOT(setStoreInfo()));

    _discontinued = new QCheckBox(tr("Discontinued?"), bot);
    _stocked = new QCheckBox(tr("Stocked?"), bot);
    connect(_stocked, SIGNAL(toggled(bool)), SLOT(setStoreInfo()));

    QLabel* onHandLabel = new QLabel(tr("On Hand:"), bot);
    _onHand = new DoubleEdit(bot);
    _onHand->setFocusPolicy(NoFocus);

    QLabel* onOrderLabel = new QLabel(tr("On Order:"), bot);
    _onOrder = new DoubleEdit(bot);
    _onOrder->setFocusPolicy(NoFocus);

    QGridLayout* botGrid = new QGridLayout(bot);
    botGrid->setSpacing(6);
    botGrid->setMargin(3);
    botGrid->setColStretch(1, 2);
    botGrid->addWidget(storeLabel, 0, 0);
    botGrid->addWidget(_store, 0, 1, AlignLeft | AlignVCenter);
    botGrid->addWidget(locationLabel, 1, 0);
    botGrid->addWidget(_location, 1, 1, AlignLeft | AlignVCenter);
    botGrid->addWidget(minLabel, 2, 0);
    botGrid->addWidget(_minQty, 2, 1, AlignLeft | AlignVCenter);
    botGrid->addWidget(maxLabel, 3, 0);
    botGrid->addWidget(_maxQty, 3, 1, AlignLeft | AlignVCenter);
    botGrid->addMultiCellWidget(_discontinued, 0,0,3,4,AlignLeft|AlignVCenter);
    botGrid->addMultiCellWidget(_stocked, 1,1,3,4,AlignLeft|AlignVCenter);
    botGrid->addWidget(onHandLabel, 2, 3);
    botGrid->addWidget(_onHand, 2, 4, AlignLeft | AlignVCenter);
    botGrid->addWidget(onOrderLabel, 3, 3);
    botGrid->addWidget(_onOrder, 3, 4, AlignLeft | AlignVCenter);

    _tabs = new QTabWidget(_frame);
    QFrame* simple = new QFrame(_tabs);
    QFrame* size_num = new QFrame(_tabs);
    QFrame* flags = new QFrame(_tabs);
    QFrame* storesTab = new QFrame(_tabs);
    QFrame* costsTab = new QFrame(_tabs);
    _vendorsTab = new QFrame(_tabs);
    _buildTab = new QFrame(_tabs);
    QFrame* extraTab = new QFrame(_tabs);
    _tabs->addTab(simple, tr("General"));
    _tabs->addTab(size_num, tr("Sizes && Numbers"));
    _tabs->addTab(flags, tr("Flags"));
    _tabs->addTab(storesTab, tr("Store"));
    _tabs->addTab(costsTab, tr("Costs && Prices"));
    _tabs->addTab(_vendorsTab, tr("Vendors"));
    _tabs->addTab(_buildTab, tr("Kit"));
    _tabs->addTab(extraTab, tr("Extra Info"));

    QHBox* info = new QHBox(simple);
    info->setSpacing(3);

    QGroupBox* purchase = new QGroupBox(tr("Purchase Information"), info);
    QGridLayout* p_grid = new QGridLayout(purchase, 2, 2,
					  purchase->frameWidth() * 2);
    p_grid->addRowSpacing(0, purchase->fontMetrics().height());

    QLabel* purchSizeLabel = new QLabel(tr("Default Size:"), purchase);
    _purch_size = new LineEdit(purchase);
    _purch_size->setLength(8);
    purchSizeLabel->setBuddy(_purch_size);
    connect(_purch_size, SIGNAL(validData()), SLOT(purchaseSizeChanged()));

    QLabel* purchQtyLabel = new QLabel(tr("Size Qty:"), purchase);
    _purch_qty = new DoubleEdit(purchase);
    _purch_qty->setLength(8);
    purchQtyLabel->setBuddy(_purch_qty);
    connect(_purch_qty, SIGNAL(validData()), SLOT(purchaseQtyChanged()));

    QLabel* costLabel = new QLabel(tr("Cost:"), purchase);
    _cost = new PriceEdit(purchase);
    costLabel->setBuddy(_cost);
    connect(_cost, SIGNAL(validData()), SLOT(costChanged()));

    QLabel* purchTaxLabel = new QLabel(tr("Purchase Tax:"), purchase);
    _purch_tax = new LookupEdit(new TaxLookup(main, this), purchase);
    _purch_tax->setLength(6);
    purchTaxLabel->setBuddy(_purch_tax);
    connect(_purch_tax, SIGNAL(validData()), SLOT(recalculateMargin()));

    QLabel* vendorLabel = new QLabel(tr("Default Vendor:"), purchase);
    _vendor = new LookupEdit(new VendorLookup(main, this), purchase);
    _vendor->setLength(30);
    vendorLabel->setBuddy(_vendor);
    connect(_vendor, SIGNAL(validData()), SLOT(vendorChanged()));

    QLabel* vendorNumLabel = new QLabel(tr("Order Number:"), purchase);
    _vendor_num = new LineEdit(purchase);
    _vendor_num->setLength(18, '9');
    vendorNumLabel->setBuddy(_vendor_num);
    connect(_vendor_num, SIGNAL(validData()), SLOT(vendorNumChanged()));

    p_grid->setRowStretch(6, 1);
    p_grid->setColStretch(1, 1);
    p_grid->addWidget(purchSizeLabel, 1, 0, AlignLeft | AlignVCenter);
    p_grid->addWidget(_purch_size, 1, 1, AlignLeft | AlignVCenter);
    p_grid->addWidget(purchQtyLabel, 1, 2, AlignLeft | AlignVCenter);
    p_grid->addWidget(_purch_qty, 1, 3, AlignLeft | AlignVCenter);
    p_grid->addWidget(costLabel, 2, 0, AlignLeft | AlignVCenter);
    p_grid->addMultiCellWidget(_cost, 2, 2, 1, 3);
    p_grid->addWidget(purchTaxLabel, 3, 0, AlignLeft | AlignVCenter);
    p_grid->addMultiCellWidget(_purch_tax, 3, 3, 1, 3, AlignLeft|AlignVCenter);
    p_grid->addWidget(vendorLabel, 4, 0, AlignLeft | AlignVCenter);
    p_grid->addMultiCellWidget(_vendor, 4, 4, 1, 3, AlignLeft|AlignVCenter);
    p_grid->addWidget(vendorNumLabel, 5, 0, AlignLeft | AlignVCenter);
    p_grid->addMultiCellWidget(_vendor_num, 5, 5, 1, 3,AlignLeft|AlignVCenter);

    QGroupBox* sales = new QGroupBox(tr("Sales Information"), info);
    QGridLayout* s_grid = new QGridLayout(sales, 2, 2,
					  sales->frameWidth() * 2);
    s_grid->addRowSpacing(0, sales->fontMetrics().height());

    QLabel* sellSizeLabel = new QLabel(tr("Default Size:"), sales);
    _sell_size = new LineEdit(sales);
    _sell_size->setLength(8);
    sellSizeLabel->setBuddy(_sell_size);
    connect(_sell_size, SIGNAL(validData()), SLOT(sellSizeChanged()));

    QLabel* sellQtyLabel = new QLabel(tr("Size Qty:"), sales);
    _sell_qty = new DoubleEdit(sales);
    _sell_qty->setLength(8);
    sellQtyLabel->setBuddy(_sell_qty);
    connect(_sell_qty, SIGNAL(validData()), SLOT(sellQtyChanged()));

    QLabel* priceLabel = new QLabel(tr("Price:"), sales);
    _price = new PriceEdit(sales);
    priceLabel->setBuddy(_price);
    connect(_price, SIGNAL(validData()), SLOT(priceChanged()));

    QLabel* marginLabel = new QLabel(tr("Margin:"), sales);
    _margin = new PercentEdit(sales);
    _margin->setLength(10);
    marginLabel->setBuddy(_margin);
    connect(_margin, SIGNAL(validData()), SLOT(marginChanged()));

    QLabel* profitLabel = new QLabel(tr("Profit:"), sales);
    _profit = new MoneyEdit(sales);
    _profit->setLength(12);
    profitLabel->setBuddy(_profit);
    connect(_profit, SIGNAL(validData()), SLOT(profitChanged()));

    QLabel* targetLabel = new QLabel(tr("Target GM:"), sales);
    _target_gm = new PercentEdit(sales);
    _target_gm->setLength(10);
    targetLabel->setBuddy(_target_gm);
    connect(_target_gm, SIGNAL(validData()), SLOT(targetChanged()));

    QLabel* allowLabel = new QLabel(tr("Variance:"), sales);
    _allowed_var = new PercentEdit(sales);
    _allowed_var->setLength(10);
    allowLabel->setBuddy(_allowed_var);
    connect(_allowed_var, SIGNAL(validData()), SLOT(allowedChanged()));

    QLabel* sellTaxLabel = new QLabel(tr("Selling Tax:"), sales);
    _sell_tax = new LookupEdit(new TaxLookup(main, this), sales);
    _sell_tax->setLength(6);
    sellTaxLabel->setBuddy(_sell_tax);
    connect(_sell_tax, SIGNAL(validData()), SLOT(recalculateMargin()));

    _depositLabel = new QLabel(tr("Deposit:"), sales);
    _deposit = new MoneyEdit(sales);
    _depositLabel->setBuddy(_deposit);
    connect(_deposit, SIGNAL(validData()), SLOT(recalculateMargin()));

    s_grid->setRowStretch(6, 1);
    s_grid->setColStretch(1, 1);
    s_grid->addWidget(sellSizeLabel, 1, 0, AlignLeft | AlignVCenter);
    s_grid->addWidget(_sell_size, 1, 1, AlignLeft | AlignVCenter);
    s_grid->addWidget(sellQtyLabel, 1, 2, AlignLeft | AlignVCenter);
    s_grid->addWidget(_sell_qty, 1, 3, AlignLeft | AlignVCenter);
    s_grid->addWidget(priceLabel, 2, 0, AlignLeft | AlignVCenter);
    s_grid->addMultiCellWidget(_price, 2, 2, 1, 3);
    s_grid->addWidget(marginLabel, 3, 0, AlignLeft | AlignVCenter);
    s_grid->addWidget(_margin, 3, 1, AlignLeft | AlignVCenter);
    s_grid->addWidget(profitLabel, 3, 2, AlignLeft | AlignVCenter);
    s_grid->addWidget(_profit, 3, 3, AlignLeft | AlignVCenter);
    s_grid->addWidget(targetLabel, 4, 0, AlignLeft | AlignVCenter);
    s_grid->addWidget(_target_gm, 4, 1, AlignLeft | AlignVCenter);
    s_grid->addWidget(allowLabel, 4, 2, AlignLeft | AlignVCenter);
    s_grid->addWidget(_allowed_var, 4, 3, AlignLeft | AlignVCenter);
    s_grid->addWidget(sellTaxLabel, 5, 0, AlignLeft | AlignVCenter);
    s_grid->addWidget(_sell_tax, 5, 1, AlignLeft | AlignVCenter);
    s_grid->addWidget(_depositLabel, 5, 2, AlignLeft | AlignVCenter);
    s_grid->addWidget(_deposit, 5, 3, AlignLeft | AlignVCenter);

    QHBox* adv = new QHBox(size_num);
    adv->setSpacing(3);

    _sizes = new Table(adv);
    _sizes->setVScrollBarMode(QScrollView::AlwaysOn);
    _sizes->setDisplayRows(2);
    _sizes->setLeftMargin(fontMetrics().width("999"));
    connect(_sizes, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(sizeCellChanged(int,int,Variant)));
    connect(_sizes, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(sizeFocusNext(bool&,int&,int&,int)));
    connect(_sizes, SIGNAL(rowDeleted(int)), SLOT(sizeRowDeleted(int)));

    new TextColumn(_sizes, tr("Size Name"), 8);
    new NumberColumn(_sizes, tr("Quantity"), 8);
    new NumberColumn(_sizes, tr("Weight"), 8);
    new NumberColumn(_sizes, tr("Multiple"), 8);

    new LineEditor(_sizes, 0, new LineEdit(_sizes));
    new NumberEditor(_sizes, 1, new DoubleEdit(_sizes));
    new NumberEditor(_sizes, 2, new DoubleEdit(_sizes));
    new NumberEditor(_sizes, 3, new DoubleEdit(_sizes));

    _numbers = new Table(adv);
    _numbers->setVScrollBarMode(QScrollView::AlwaysOn);
    _numbers->setDisplayRows(2);
    _numbers->setLeftMargin(fontMetrics().width("999"));
    connect(_numbers, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(numberCellChanged(int,int,Variant)));
    connect(_numbers, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(numberFocusNext(bool&,int&,int&,int)));
    connect(_numbers, SIGNAL(rowDeleted(int)), SLOT(numberRowDeleted(int)));
    connect(_numbers, SIGNAL(rowInserted(int)), SLOT(numberRowInserted(int)));

    new TextColumn(_numbers, tr("Item Number"), 18);
    new TextColumn(_numbers, tr("Size"), 8);

    _number_size = new QComboBox(_numbers);
    new LineEditor(_numbers, 0, new LineEdit(_numbers));
    new ComboEditor(_numbers, 1, _number_size);

    _discountable = new QCheckBox(tr("Discountable?"), flags);
    _qty_enforced = new QCheckBox(tr("Qty Enforced?"), flags);
    _qty_decimals = new QCheckBox(tr("Qty Decimals?"), flags);
    _qty_price = new QCheckBox(tr("Qty From Price?"), flags);
    _weighed = new QCheckBox(tr("Weighed?"), flags);

    _cost_inc_tax = new QCheckBox(tr("Cost Includes Tax?"), flags);
    _price_inc_tax = new QCheckBox(tr("Price Includes Tax?"), flags);
    connect(_cost_inc_tax, SIGNAL(toggled(bool)), SLOT(recalculateMargin()));
    connect(_price_inc_tax, SIGNAL(toggled(bool)), SLOT(recalculateMargin()));

    _cost_inc_dep = new QCheckBox(tr("Cost Includes Deposit?"), flags);
    _price_inc_dep = new QCheckBox(tr("Price Includes Deposit?"), flags);
    connect(_cost_inc_dep, SIGNAL(toggled(bool)), SLOT(recalculateMargin()));
    connect(_price_inc_dep, SIGNAL(toggled(bool)), SLOT(recalculateMargin()));

    QLabel* costDiscLabel = new QLabel(tr("Cost Discount:"), flags);
    _costDisc = new PercentEdit(flags);
    _costDisc->setLength(10);
    costDiscLabel->setBuddy(_costDisc);
    connect(_costDisc, SIGNAL(validData()), SLOT(recalculateMargin()));

    _stores = new Table(storesTab);
    _stores->setVScrollBarMode(QScrollView::AlwaysOn);
    _stores->setDisplayRows(3);
    _stores->setLeftMargin(fontMetrics().width("999"));
    connect(_stores, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(storesCellChanged(int,int,Variant)));
    connect(_stores, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(storesFocusNext(bool&,int&,int&,int)));
    connect(_stores, SIGNAL(rowDeleted(int)), SLOT(storesRowDeleted(int)));
    connect(_stores, SIGNAL(rowInserted(int)), SLOT(storesRowInserted(int)));

    StoreLookup* storeLookup = new StoreLookup(_main, this);
    LocationLookup* locLookup = new LocationLookup(_main, this);

    new LookupColumn(_stores, tr("Store"), 30, storeLookup);
    new LookupColumn(_stores, tr("Location"), 30, locLookup);
    new CheckColumn(_stores, tr("Stocked?"));
    new NumberColumn(_stores, tr("Min"));
    new NumberColumn(_stores, tr("Max"));

    new LookupEditor(_stores, 1, new LookupEdit(locLookup, _stores));
    new CheckEditor(_stores, 2, new CheckBox(_stores));
    new NumberEditor(_stores, 3, new DoubleEdit(_stores));
    new NumberEditor(_stores, 4, new DoubleEdit(_stores));

    _costs = new Table(costsTab);
    _costs->setVScrollBarMode(QScrollView::AlwaysOn);
    _costs->setDisplayRows(3);
    _costs->setLeftMargin(fontMetrics().width("999"));
    connect(_costs, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(costsCellChanged(int,int,Variant)));
    connect(_costs, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(costsFocusNext(bool&,int&,int&,int)));
    connect(_costs, SIGNAL(rowDeleted(int)), SLOT(costsRowDeleted(int)));
    connect(_costs, SIGNAL(rowInserted(int)), SLOT(costsRowInserted(int)));

    storeLookup = new StoreLookup(_main, this);

    new LookupColumn(_costs, tr("Store"), 15, storeLookup);
    new TextColumn(_costs, tr("Size"), 8);
    new PriceColumn(_costs, tr("Cost"), 6);
    new PriceColumn(_costs, tr("Price"), 6);
    new MoneyColumn(_costs, tr("Profit"), 6);
    new PercentColumn(_costs, tr("Margin"));
    new PercentColumn(_costs, tr("Target"));
    new PercentColumn(_costs, tr("Allow"));

    new PriceEditor(_costs, 2, new PriceEdit(_costs));
    new PriceEditor(_costs, 3, new PriceEdit(_costs));
    new NumberEditor(_costs, 4, new MoneyEdit(_costs));
    new NumberEditor(_costs, 5, new PercentEdit(_costs));
    new NumberEditor(_costs, 6, new PercentEdit(_costs));
    new NumberEditor(_costs, 7, new PercentEdit(_costs));

    _vendors = new Table(_vendorsTab);
    _vendors->setVScrollBarMode(QScrollView::AlwaysOn);
    _vendors->setDisplayRows(2);
    _vendors->setLeftMargin(fontMetrics().width("999"));
    connect(_vendors, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(vendorCellChanged(int,int,Variant)));
    connect(_vendors, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(vendorFocusNext(bool&,int&,int&,int)));
    connect(_vendors, SIGNAL(rowDeleted(int)), SLOT(vendorRowDeleted(int)));
    connect(_vendors, SIGNAL(rowInserted(int)), SLOT(vendorRowInserted(int)));

    CardLookup* vendorLookup1 = new VendorLookup(_main, this);

    new LookupColumn(_vendors, tr("Vendor"), 20, vendorLookup1);
    new TextColumn(_vendors, tr("Order Number"), 18);
    new TextColumn(_vendors, tr("Size"), 8);

    LineEdit* vendorNumEdit = new LineEdit(_vendors);
    vendorNumEdit->setMaxLength(18);
    _ordernum_size = new QComboBox(_vendors);

    new LookupEditor(_vendors, 0, new LookupEdit(vendorLookup1, _vendors));
    new LineEditor(_vendors, 1, vendorNumEdit);
    new ComboEditor(_vendors, 2, _ordernum_size);

    QLabel* buildQtyLabel = new QLabel(tr("Build Quantity:"), _buildTab);
    _build_qty = new DoubleEdit(_buildTab);

    QLabel* buildCostLabel = new QLabel(tr("Build Cost:"), _buildTab);
    _build_cost = new MoneyEdit(_buildTab);
    _build_cost->setFocusPolicy(NoFocus);

    _build = new Table(_buildTab);
    _build->setVScrollBarMode(QScrollView::AlwaysOn);
    _build->setDisplayRows(2);
    _build->setLeftMargin(fontMetrics().width("999"));
    connect(_build, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(buildCellChanged(int,int,Variant)));
    connect(_build, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(buildFocusNext(bool&,int&,int&,int)));

    ItemLookup* buildLookup = new ItemLookup(_main, this);

    new LookupColumn(_build, tr("Component"), 18, buildLookup);
    new TextColumn(_build, tr("Description"), 20);
    new TextColumn(_build, tr("Size"), 10);
    new NumberColumn(_build, tr("Quantity"), 6);
    new MoneyColumn(_build, tr("Cost"), 10);

    new LookupEditor(_build, 0, new ItemEdit(buildLookup, _build));
    new LineEditor(_build, 2, new LineEdit(_build));
    new NumberEditor(_build, 3, new DoubleEdit(_build));

    QGridLayout* buildGrid = new QGridLayout(_buildTab);
    buildGrid->setSpacing(6);
    buildGrid->setMargin(3);
    buildGrid->setRowStretch(1, 1);
    buildGrid->setColStretch(2, 1);
    buildGrid->addWidget(buildQtyLabel, 0, 0);
    buildGrid->addWidget(_build_qty, 0, 1, AlignLeft | AlignVCenter);
    buildGrid->addWidget(buildCostLabel, 0, 3);
    buildGrid->addWidget(_build_cost, 0, 4, AlignLeft | AlignVCenter);
    buildGrid->addMultiCellWidget(_build, 1, 1, 0, 4);

    _extra = new Table(extraTab);
    _extra->setVScrollBarMode(QScrollView::AlwaysOn);
    _extra->setDisplayRows(2);
    _extra->setLeftMargin(fontMetrics().width("999"));
    connect(_extra, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(extraFocusNext(bool&,int&,int&,int)));

    ExtraLookup* extraLookup = new ExtraLookup(_main, this, "Item");

    new LookupColumn(_extra, tr("Data Name"), 30, extraLookup);
    new TextColumn(_extra, tr("Value"), 30);

    new LineEditor(_extra, 1, new LineEdit(_extra));

    QPushButton* createData = new QPushButton(tr("Create Data"), extraTab);
    QPushButton* deleteData = new QPushButton(tr("Delete Data"), extraTab);
    QPushButton* renameData = new QPushButton(tr("Rename Data"), extraTab);

    connect(createData, SIGNAL(clicked()), SLOT(slotCreateData()));
    connect(deleteData, SIGNAL(clicked()), SLOT(slotDeleteData()));
    connect(renameData, SIGNAL(clicked()), SLOT(slotRenameData()));

    QGridLayout* extraGrid = new QGridLayout(extraTab);
    extraGrid->setSpacing(6);
    extraGrid->setMargin(3);
    extraGrid->setRowStretch(0, 1);
    extraGrid->setColStretch(0, 1);
    extraGrid->addMultiCellWidget(_extra, 0, 0, 0, 3);
    extraGrid->addWidget(createData, 1, 1);
    extraGrid->addWidget(deleteData, 1, 2);
    extraGrid->addWidget(renameData, 1, 3);

    _open_inv = new QGroupBox(tr("Opening Inventory"), _frame);
    QGridLayout* i_grid = new QGridLayout(_open_inv, 2, 2,
					  _open_inv->frameWidth() * 2);
    i_grid->addRowSpacing(0, _open_inv->fontMetrics().height());

    _asOfLabel = new QLabel(tr("As of"), _open_inv);
    _as_of = new DatePopup(_open_inv);
    _asOfLabel->setBuddy(_as_of);

    QLabel* openSizeLabel = new QLabel(tr("Size:"), _open_inv);
    _open_size = new QComboBox(_open_inv);
    openSizeLabel->setBuddy(_open_size);
    connect(_open_size, SIGNAL(activated(int)), SLOT(openSizeChanged()));

    QLabel* openQtyLabel = new QLabel(tr("Quantity:"), _open_inv);
    _open_qty = new DoubleEdit(_open_inv);
    _open_qty->setLength(12);
    openQtyLabel->setBuddy(_open_qty);
    connect(_open_qty, SIGNAL(validData()), SLOT(openQtyChanged()));

    QLabel* openCostLabel = new QLabel(tr("Total Cost:"), _open_inv);
    _open_cost = new MoneyEdit(_open_inv);
    _open_cost->setLength(12);
    openCostLabel->setBuddy(_open_cost);

    i_grid->setColStretch(3, 1);
    i_grid->addWidget(_asOfLabel, 1, 0, AlignLeft);
    i_grid->addWidget(_as_of, 2, 0, AlignLeft | AlignVCenter);
    i_grid->addWidget(openSizeLabel, 1, 1, AlignLeft);
    i_grid->addWidget(_open_size, 2, 1, AlignLeft | AlignVCenter);
    i_grid->addWidget(openQtyLabel, 1, 2, AlignLeft);
    i_grid->addWidget(_open_qty, 2, 2, AlignLeft | AlignVCenter);
    i_grid->addWidget(openCostLabel, 1, 3, AlignLeft);
    i_grid->addWidget(_open_cost, 2, 3, AlignLeft | AlignVCenter);

    _quasar->db()->lookup(_company);
    if (_id != INVALID_ID || _company.historicalBalancing() == INVALID_ID)
	_open_inv->hide();
    if (_company.depositAccount() == INVALID_ID) {
	_depositLabel->hide();
	_deposit->hide();
	_cost_inc_dep->hide();
	_price_inc_dep->hide();
    }

    QGridLayout* simpleGrid = new QGridLayout(simple);
    simpleGrid->setSpacing(6);
    simpleGrid->setMargin(3);
    simpleGrid->addWidget(info, 0, 0);

    QGridLayout* sizesGrid = new QGridLayout(size_num);
    sizesGrid->setSpacing(6);
    sizesGrid->setMargin(3);
    sizesGrid->addWidget(adv, 0, 0);

    QGridLayout* flagsGrid = new QGridLayout(flags);
    flagsGrid->setSpacing(6);
    flagsGrid->setMargin(3);
    flagsGrid->setRowStretch(5, 1);
    flagsGrid->setColStretch(1, 1);
    flagsGrid->setColStretch(3, 1);
    flagsGrid->addWidget(_discountable, 0, 0);
    flagsGrid->addWidget(_cost_inc_tax, 0, 2);
    flagsGrid->addWidget(_qty_enforced, 1, 0);
    flagsGrid->addWidget(_price_inc_tax, 1, 2);
    flagsGrid->addWidget(_qty_decimals, 2, 0);
    flagsGrid->addWidget(_cost_inc_dep, 2, 2);
    flagsGrid->addWidget(_qty_price, 3, 0);
    flagsGrid->addWidget(_price_inc_dep, 3, 2);
    flagsGrid->addWidget(_weighed, 4, 0);
    flagsGrid->addWidget(costDiscLabel, 0, 4);
    flagsGrid->addWidget(_costDisc, 0, 5);

    QGridLayout* storeGrid = new QGridLayout(storesTab);
    storeGrid->setSpacing(6);
    storeGrid->setMargin(3);
    storeGrid->addWidget(_stores, 0, 0);

    QGridLayout* costGrid = new QGridLayout(costsTab);
    costGrid->setSpacing(6);
    costGrid->setMargin(3);
    costGrid->addWidget(_costs, 0, 0);

    QGridLayout* vendorGrid = new QGridLayout(_vendorsTab);
    vendorGrid->setSpacing(6);
    vendorGrid->setMargin(3);
    vendorGrid->setColStretch(0, 2);
    vendorGrid->setColStretch(1, 1);
    vendorGrid->addWidget(_vendors, 0, 0);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(6);
    grid->setMargin(3);
    grid->addWidget(top, 0, 0);
    grid->addWidget(mid, 1, 0);
    grid->addWidget(bot, 2, 0);
    grid->addWidget(_tabs, 3, 0);
    grid->addWidget(_open_inv, 4, 0);

    setCaption(tr("Item Master"));
    finalize();
}

ItemMaster::~ItemMaster()
{
}

void
ItemMaster::slotPrices()
{
    if (!saveItem(true)) return;
    PriceList* list = new PriceList(_main);
    list->setItemId(_curr.id());
    list->show();
}

void
ItemMaster::slotCosts()
{
    if (!saveItem(true)) return;
    PriceList* list = new PriceList(_main, false);
    list->setItemId(_curr.id());
    list->show();
}

void
ItemMaster::slotMargin()
{
    if (!saveItem(true)) return;
    ItemMargin* screen = new ItemMargin(_main);
    screen->setItem(_curr.id(), _curr.number());
    screen->show();
}

void
ItemMaster::slotStockStatus()
{
    if (!saveItem(true)) return;
    StockStatus* status = new StockStatus(_main);
    status->setItem(_curr.id(), _curr.number());
    status->show();
}

void
ItemMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    if (_orig.deposit() != 0.0) {
	_depositLabel->show();
	_deposit->show();
    }

    _curr = _orig;
    _firstField = _number;
}

void
ItemMaster::newItem()
{
    Item blank;
    _orig = blank;

    _as_of->setDate(QDate::currentDate());
    _open_qty->setText("");
    _open_cost->setText("");

    if (_company.historicalBalancing() != INVALID_ID)
	_open_inv->show();

    _curr = _orig;
    _firstField = _number;
}

void
ItemMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _curr.numbers().clear();
    for (unsigned int i = 0; i < _curr.vendors().size(); ++i)
	_curr.vendors()[i].number = "";
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
}

bool
ItemMaster::fileItem()
{
    if (_orig.id() == INVALID_ID) {
	if (!_quasar->db()->create(_curr)) return false;

	QString size = _open_size->currentText();
	fixed size_qty = _curr.sizeQty(size);
	fixed qty = _open_qty->getFixed();
	fixed ext_cost = _open_cost->getFixed();
	fixed ext_deposit = _curr.deposit() * size_qty * qty;
	fixed total = ext_cost + ext_deposit;

	if (!size.isEmpty() && qty != 0.0 && total != 0.0) {
	    Id store_id = _quasar->defaultStore();
	    QDate date = _as_of->getDate();
	    if (date.isNull()) date = QDate::currentDate();

	    ItemAdjust adjustment;
	    adjustment.setPostDate(date);
	    adjustment.setPostTime(QTime(0, 0, 0));
	    adjustment.setMemo(tr("Open balance"));
	    adjustment.setStoreId(store_id);

	    // Add account lines
	    AccountLine costLine(_curr.assetAccount(), ext_cost);
	    adjustment.accounts().push_back(costLine);
	    if (ext_deposit != 0.0) {
		AccountLine depLine(_company.depositAccount(), ext_deposit);
		adjustment.accounts().push_back(depLine);
	    }

	    // Add item lines
	    ItemLine adj_line;
	    adj_line.item_id = _curr.id();
	    adj_line.number = _curr.number();
	    adj_line.size = size;
	    adj_line.size_qty = size_qty;
	    adj_line.quantity = qty;
	    adj_line.inv_cost = ext_cost;
	    adj_line.ext_deposit = ext_deposit;
	    adjustment.items().push_back(adj_line);

	    // Set expense
	    Id expense_id = _company.historicalBalancing();
	    adjustment.setAccountId(expense_id);
	    adjustment.accounts().push_back(AccountLine(expense_id, -total));

	    // Post item adjustment
	    if (!_quasar->db()->create(adjustment)) {
		QString message = tr("Failed creating open balance transaction");
		QMessageBox::critical(this, tr("Error"), message);
	    }
	}
    } else {
	if (!_quasar->db()->update(_orig, _curr)) return false;
    }

    _orig = _curr;
    _id = _curr.id();

    return true;
}

bool
ItemMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
ItemMaster::restoreItem()
{
    _curr = _orig;
}

void
ItemMaster::cloneItem()
{
    ItemMaster* clone = new ItemMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
ItemMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
ItemMaster::dataToWidget()
{
    Id store_id = _quasar->defaultStore();
    _ignoreSubdept = true;
    _purch_row = -1;
    _sell_row = -1;

    _desc->setText(_curr.description());
    _dept->setId(_curr.deptId());
    _subdept->setId(_curr.subdeptId());
    _subLookup->dept->setId(_curr.deptId());
    _purchased->setChecked(_curr.isPurchased());
    _sold->setChecked(_curr.isSold());
    _inventoried->setChecked(_curr.isInventoried());
    _open_dept->setChecked(_curr.isOpenDept());
    _expense_acct->setId(_curr.expenseAccount());
    _income_acct->setId(_curr.incomeAccount());
    _asset_acct->setId(_curr.assetAccount());
    _discontinued->setChecked(_curr.isDiscontinued());
    _purch_size->setText(_curr.purchaseSize());
    _purch_qty->setFixed(_curr.sizeQty(_curr.purchaseSize()));
    _purch_tax->setId(_curr.purchaseTax());
    _sell_size->setText(_curr.sellSize());
    _sell_qty->setFixed(_curr.sizeQty(_curr.sellSize()));
    _sell_tax->setId(_curr.sellTax());
    _deposit->setFixed(_curr.deposit());
    _store->setId(store_id);
    _cost_inc_tax->setChecked(_curr.costIncludesTax());
    _price_inc_tax->setChecked(_curr.priceIncludesTax());
    _cost_inc_dep->setChecked(_curr.costIncludesDeposit());
    _price_inc_dep->setChecked(_curr.priceIncludesDeposit());
    _discountable->setChecked(_curr.isDiscountable());
    _qty_enforced->setChecked(_curr.isQuantityEnforced());
    _qty_decimals->setChecked(_curr.isQuantityDecimals());
    _qty_price->setChecked(_curr.isQtyFromPrice());
    _weighed->setChecked(_curr.isWeighed());
    _build_qty->setFixed(_curr.buildQty());
    _costDisc->setFixed(_curr.costDiscount());
    _inactive->setChecked(!_curr.isActive());

    // Item numbers
    _number->clear();
    _numbers->clear();
    unsigned int i;
    for (i = 0; i < _curr.numbers().size(); ++i) {
	const ItemPlu& line = _curr.numbers()[i];

	VectorRow* row = new VectorRow(_numbers->columns());
	row->setValue(0, line.number);
	row->setValue(1, line.size);
	_numbers->appendRow(row);

	if (i == 0) _number->setText(line.number);
    }
    _numbers->appendRow(new VectorRow(_numbers->columns()));

    // Get list of all stores
    StoreSelect conditions;
    conditions.activeOnly = true;
    vector<Store> stores;
    _quasar->db()->select(stores, conditions);

    // Load stores/costs information for all store/size combinations
    _stores->clear();
    _costs->clear();
    for (i = 0; i < stores.size(); ++i) {
	Id store_id = stores[i].id();

	VectorRow* row = new VectorRow(_stores->columns());
	row->setValue(0, store_id);
	row->setValue(1, _curr.locationId(store_id));
	row->setValue(2, _curr.stocked(store_id));
	row->setValue(3, _curr.minQty(store_id));
	row->setValue(4, _curr.maxQty(store_id));
	_stores->appendRow(row);

	for (unsigned int j = 0; j < _curr.sizes().size(); ++j) {
	    const QString& size = _curr.sizes()[j].name;

	    row = new VectorRow(_costs->columns());
	    row->setValue(0, store_id);
	    row->setValue(1, size);
	    row->setValue(2, _curr.cost(store_id, size));
	    row->setValue(3, _curr.price(store_id, size));
	    row->setValue(6, _curr.targetGM(store_id, size));
	    row->setValue(7, _curr.allowedVariance(store_id, size));
	    _costs->appendRow(row);
	}
    }
    _stores->setCurrentCell(0, 1);
    _costs->setCurrentCell(0, 2);

    // Vendors
    _vendor->setId(INVALID_ID);
    _vendor_num->clear();
    _vendors->clear();
    for (i = 0; i < _curr.vendors().size(); ++i) {
	const ItemVendor& line = _curr.vendors()[i];

	VectorRow* row = new VectorRow(_vendors->columns());
	row->setValue(0, line.vendor_id);
	row->setValue(1, line.number);
	row->setValue(2, line.size);
	_vendors->appendRow(row);

	if (i == 0) {
	    _vendor->setId(line.vendor_id);
	    _vendor_num->setText(line.number);
	}
    }
    _vendors->appendRow(new VectorRow(_vendors->columns()));

    // Set groups
    _groups->clear();
    for (i = 0; i < _curr.groups().size(); ++i) {
	VectorRow* row = new VectorRow(_groups->columns());
	row->setValue(0, _curr.groups()[i]);
	_groups->appendRow(row);
    }
    _groups->appendRow(new VectorRow(_groups->columns()));

    // Set sizes
    _sizes->clear();
    for (i = 0; i < _curr.sizes().size(); ++i) {
	VectorRow* row = new VectorRow(_sizes->columns());
	row->setValue(0, _curr.sizes()[i].name);
	row->setValue(1, _curr.sizes()[i].qty);
	row->setValue(2, _curr.sizes()[i].weight);
	row->setValue(3, _curr.sizes()[i].multiple);
	_sizes->appendRow(row);
    }
    _sizes->appendRow(new VectorRow(_sizes->columns()));
    sizesChanged();

    // Figure out regular cost
    _purch_row = findStoreRow(_curr.purchaseSize());
    if (_purch_row == -1)
	_cost->clear();
    else
	_cost->setPrice(_costs->cellValue(_purch_row, 2).toPrice());

    // Figure out regular price
    _sell_row = findStoreRow(_curr.sellSize());
    if (_sell_row == -1) {
	_price->clear();
	_profit->clear();
	_margin->clear();
	_target_gm->clear();
	_allowed_var->clear();
    } else {
	_price->setPrice(_costs->cellValue(_sell_row, 3).toPrice());
	_profit->setFixed(_costs->cellValue(_sell_row, 4).toFixed());
	_margin->setFixed(_costs->cellValue(_sell_row, 5).toFixed());
	_target_gm->setFixed(_costs->cellValue(_sell_row, 6).toFixed());
	_allowed_var->setFixed(_costs->cellValue(_sell_row, 7).toFixed());
    }

    // Set components
    _build->clear();
    fixed buildCost = 0.0;
    for (i = 0; i < _curr.items().size(); ++i) {
	const Component& line = _curr.items()[i];

	Item item;
	_quasar->db()->lookup(line.item_id, item);

	QString size = line.size;
	fixed qty = line.qty;
	fixed cost = item.cost(store_id, size, qty);
	buildCost += cost;

	VectorRow* row = new VectorRow(_build->columns());
	row->setValue(0, Plu(line.item_id, line.number));
	row->setValue(1, item.description());
	row->setValue(2, size);
	row->setValue(3, qty);
	row->setValue(4, cost);
	_build->appendRow(row);
    }
    _build->appendRow(new VectorRow(_build->columns()));
    _build_cost->setFixed(buildCost);

    // Extra info
    _extra->clear();
    ExtraSelect extraSelect;
    extraSelect.activeOnly = true;
    extraSelect.table = "Item";
    vector<Extra> extras;
    _quasar->db()->select(extras, extraSelect);
    for (i = 0; i < extras.size(); ++i) {
	Id extra_id = extras[i].id();
	QString value = _curr.value(extra_id);

	VectorRow* row = new VectorRow(_extra->columns());
	row->setValue(0, extra_id);
	row->setValue(1, value);
	_extra->appendRow(row);
    }
    if (_extra->rows() > 0)
	_extra->setCurrentCell(0, 1);

    typeChanged();
    storeChanged();
    recalculateMargin();
    _ignoreSubdept = false;
}

// Set the data object from the widgets.  Return true if no errors.
void
ItemMaster::widgetToData()
{
    bool purchased = _purchased->isChecked();
    bool sold = _sold->isChecked();
    bool inventoried = _inventoried->isChecked();

    _curr.setDescription(_desc->text());
    _curr.setDeptId(_dept->getId());
    _curr.setSubdeptId(_subdept->getId());
    _curr.setPurchased(purchased);
    _curr.setSold(sold);
    _curr.setInventoried(inventoried);
    _curr.setDiscontinued(_discontinued->isChecked());
    _curr.setCostIncludesTax(_cost_inc_tax->isChecked());
    _curr.setPriceIncludesTax(_price_inc_tax->isChecked());
    _curr.setCostIncludesDeposit(_cost_inc_dep->isChecked());
    _curr.setPriceIncludesDeposit(_price_inc_dep->isChecked());
    _curr.setDiscountable(_discountable->isChecked());
    _curr.setQuantityEnforced(_qty_enforced->isChecked());
    _curr.setQuantityDecimals(_qty_decimals->isChecked());
    _curr.setQtyFromPrice(_qty_price->isChecked());
    _curr.setWeighed(_weighed->isChecked());
    _curr.setBuildQty(_build_qty->getFixed());
    _curr.setDeposit(_deposit->getFixed());
    _curr.setCostDiscount(_costDisc->getFixed());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    _curr.groups().clear();
    int row;
    for (row = 0; row < _groups->rows(); ++row) {
	Id group_id = _groups->cellValue(row, 0).toId();
	if (group_id != INVALID_ID)
	    _curr.groups().push_back(group_id);
    }

    _curr.sizes().clear();
    for (row = 0; row < _sizes->rows(); ++row) {
	ItemSize line;
	line.name = _sizes->cellValue(row, 0).toString();
	line.qty = _sizes->cellValue(row, 1).toFixed();
	line.weight = _sizes->cellValue(row, 2).toFixed();
	line.multiple = _sizes->cellValue(row, 3).toFixed();
	if (!line.name.isEmpty())
	    _curr.sizes().push_back(line);
    }

    _curr.numbers().clear();
    for (row = 0; row < _numbers->rows(); ++row) {
	QString number = _numbers->cellValue(row, 0).toString();
	QString size = _numbers->cellValue(row, 1).toString();
	if (!number.isEmpty())
	    _curr.numbers().push_back(ItemPlu(number, size));
    }

    // NOTE: only load stores/costs if there are sizes.  This is to
    // counter-act the dataToWidget code that fills in all possible
    // stores.  Without this check when you just try to code the item
    // master screen without making any change, it is seen as a change.
    _curr.stores().clear();
    _curr.costs().clear();
    if (_curr.sizes().size() != 0) {
	for (row = 0; row < _stores->rows(); ++row) {
	    ItemStore line;
	    line.store_id = _stores->cellValue(row, 0).toId();
	    line.location_id = _stores->cellValue(row, 1).toId();
	    line.stocked = _stores->cellValue(row, 2).toBool();
	    line.min_qty = _stores->cellValue(row, 3).toFixed();
	    line.max_qty = _stores->cellValue(row, 4).toFixed();

	    if (line.store_id != INVALID_ID)
		_curr.stores().push_back(line);
	}

	for (row = 0; row < _costs->rows(); ++row) {
	    ItemCost line;
	    line.store_id = _costs->cellValue(row, 0).toId();
	    line.size = _costs->cellValue(row, 1).toString();
	    line.cost = _costs->cellValue(row, 2).toPrice();
	    line.price = _costs->cellValue(row, 3).toPrice();
	    line.target_gm = _costs->cellValue(row, 6).toFixed();
	    line.allowed_var = _costs->cellValue(row, 7).toFixed();

	    if (line.store_id != INVALID_ID)
		_curr.costs().push_back(line);
	}
    }

    _curr.vendors().clear();
    for (row = 0; row < _vendors->rows(); ++row) {
	Id vendor_id = _vendors->cellValue(row, 0).toId();
	QString number = _vendors->cellValue(row, 1).toString();
	QString size = _vendors->cellValue(row, 2).toString();
	if (vendor_id != INVALID_ID)
	    _curr.vendors().push_back(ItemVendor(vendor_id, number, size));
    }

    // Taxes and sizes set based on flags
    if (purchased) {
	_curr.setPurchaseTax(_purch_tax->getId());
	_curr.setPurchaseSize(_purch_size->text());
    } else {
	_curr.setPurchaseTax(INVALID_ID);
	_curr.setPurchaseSize("");
    }
    if (sold) {
	_curr.setSellTax(_sell_tax->getId());
	_curr.setSellSize(_sell_size->text());
    } else {
	_curr.setSellTax(INVALID_ID);
	_curr.setSellSize("");
    }

    _curr.items().clear();
    for (row = 0; row < _build->rows(); ++row) {
	Id item_id = _build->cellValue(row, 0).toPlu().itemId();
	QString number = _build->cellValue(row, 0).toPlu().number();
	QString size = _build->cellValue(row, 2).toString();
	fixed qty = _build->cellValue(row, 3).toFixed();
	if (item_id != INVALID_ID)
	    _curr.items().push_back(Component(item_id, number, size, qty));
    }

    _curr.extra().clear();
    for (row = 0; row < _extra->rows(); ++row) {
	Id extra_id = _extra->cellValue(row, 0).toId();
	QString value = _extra->cellValue(row, 1).toString();
	_curr.setValue(extra_id, value);
    }

    // Ledger accounts set based on flags
    if (sold) {
	_curr.setIncomeAccount(_income_acct->getId());
    } else {
	_curr.setIncomeAccount(INVALID_ID);
    }
    if (inventoried) {
	_curr.setAssetAccount(_asset_acct->getId());
    } else {
	_curr.setAssetAccount(INVALID_ID);
    }
    if ((inventoried && sold) || (purchased && !inventoried)) {
	_curr.setExpenseAccount(_expense_acct->getId());
    } else {
	_curr.setExpenseAccount(INVALID_ID);
    }

    // No open dept unless sold, purchased, and inventoried
    if (sold && inventoried && purchased)
	_curr.setOpenDept(_open_dept->isChecked());
    else
	_curr.setOpenDept(false);
}

// Validate the widget values.
bool
ItemMaster::dataValidate()
{
    if (_orig.id() == INVALID_ID) {
	fixed qty = _open_qty->getFixed();
	fixed ext_cost = _open_cost->getFixed();

	if (qty != 0.0 || ext_cost != 0.0) {
	    if (_company.historicalBalancing() == INVALID_ID)
		return error(tr("No historical balance account setup"),
			     _open_qty);
	}
    }

    return DataWindow::dataValidate();
}

void
ItemMaster::addSize(const QString& size, fixed qty)
{
    int row;
    for (row = 0; row < _sizes->rows(); ++row) {
	if (_sizes->cellValue(row, 0).toString() == size) {
	    _sizes->setCellValue(row, 1, qty);
	    _sizes->setCellValue(row, 2, Variant::null);
	    _sizes->setCellValue(row, 3, Variant::null);
	    return;
	}
    }

    if (size.isEmpty()) {
	qDebug("Empty size in addSize!");
	return;
    }

    int rows = _sizes->rows();
    if (rows != 0 && _sizes->cellValue(rows - 1, 0).toString().isEmpty()) {
	_sizes->setCellValue(rows - 1, 0, size);
	_sizes->setCellValue(rows - 1, 1, qty);
	_sizes->setCellValue(rows - 1, 2, Variant::null);
	_sizes->setCellValue(rows - 1, 3, Variant::null);
	_sizes->appendRow(new VectorRow(_sizes->columns()));
    } else {
	VectorRow* row = new VectorRow(_sizes->columns());
	row->setValue(0, size);
	row->setValue(1, qty);
	row->setValue(2, Variant::null);
	row->setValue(3, Variant::null);
	_sizes->appendRow(row);
    }
    sizesChanged();
    addCostRows(size);
}

void
ItemMaster::addCostRows(const QString& size)
{
    if (_costs->rows() == 0) {
	StoreSelect conditions;
	conditions.activeOnly = true;
	vector<Store> stores;
	_quasar->db()->select(stores, conditions);

	for (unsigned int i = 0; i < stores.size(); ++i) {
	    Id store_id = stores[i].id();

	    VectorRow* row = new VectorRow(_costs->columns());
	    row->setValue(0, store_id);
	    row->setValue(1, size);
	    row->setValue(6, _target_gm->getFixed());
	    row->setValue(7, _allowed_var->getFixed());
	    _costs->appendRow(row);
	}
    } else {
	// Add row for each store
	Id last_store = _costs->cellValue(0, 0).toId();
	for (int i = 0; i < _costs->rows(); ++i) {
	    Id store_id = _costs->cellValue(i, 0).toId();
	    if (store_id == last_store) continue;

	    VectorRow* row = new VectorRow(_costs->columns());
	    row->setValue(0, last_store);
	    row->setValue(1, size);
	    row->setValue(6, _target_gm->getFixed());
	    row->setValue(7, _allowed_var->getFixed());
	    _costs->insertRow(i, row);

	    last_store = store_id;
	}

	// Add last row
	VectorRow* row = new VectorRow(_costs->columns());
	row->setValue(0, last_store);
	row->setValue(1, size);
	row->setValue(6, _target_gm->getFixed());
	row->setValue(7, _allowed_var->getFixed());
	_costs->appendRow(row);
    }
}

void
ItemMaster::numberChanged()
{
    if (_numbers->rows() == 0)
	_numbers->appendRow(new VectorRow(_numbers->columns()));

    _numbers->setCellValue(0, 0, _number->text());

    for (int row = _numbers->rows() - 1; row >= 1; --row)
	if (_numbers->cellValue(row, 0).toString() == _number->text())
	    _numbers->deleteRow(row);

    if (_numbers->rows() == 1)
	_numbers->appendRow(new VectorRow(_numbers->columns()));
}

void
ItemMaster::storeChanged()
{
    Id store_id = _store->getId();
    for (int row = 0; row < _stores->rows(); ++row) {
	if (_stores->cellValue(row, 0).toId() != store_id)
	    continue;

	_skipUpdates = true;
	_location->setId(_stores->cellValue(row, 1).toId());
	_stocked->setChecked(_stores->cellValue(row, 2).toBool());
	_minQty->setFixed(_stores->cellValue(row, 3).toFixed());
	_maxQty->setFixed(_stores->cellValue(row, 4).toFixed());
	_skipUpdates = false;
	break;
    }

    // Set on hand and on order
    if (_curr.id() == INVALID_ID || !_curr.isInventoried()) {
	_onHand->setText("");
	_onOrder->setText("");
    } else {
	QDate date = QDate::currentDate();
	fixed onHand, totalCost, onOrder;
	_quasar->db()->itemGeneral(_curr.id(), "", store_id, date,
				   onHand, totalCost, onOrder);

	fixed sellSizeQty = _curr.sizeQty(_curr.sellSize());
	if (sellSizeQty != 0) {
	    onHand = onHand / sellSizeQty;
	    onOrder = onOrder / sellSizeQty;
	}

	_onHand->setFixed(onHand);
	_onOrder->setFixed(onOrder);
    }
}

void
ItemMaster::setStoreInfo()
{
    if (_skipUpdates) return;

    Id store_id = _store->getId();
    for (int row = 0; row < _stores->rows(); ++row) {
	if (_stores->cellValue(row, 0).toId() != store_id)
	    continue;

	_stores->setCellValue(row, 1, _location->getId());
	_stores->setCellValue(row, 2, _stocked->isChecked());
	_stores->setCellValue(row, 3, _minQty->getFixed());
	_stores->setCellValue(row, 4, _maxQty->getFixed());
	break;
    }
}

void
ItemMaster::purchaseSizeChanged()
{
    QString size = _purch_size->text();
    fixed qty;
    if (sizeExists(size, qty)) {
	_purch_qty->setFixed(qty);
	recalculateMargin();
    } else {
	qty = _purch_qty->getFixed();
	addSize(size, qty);
    }

    _purch_row = findStoreRow(size);
    if (_purch_row == -1)
	_cost->clear();
    else
	_cost->setPrice(_costs->cellValue(_purch_row, 2).toPrice());

    if (_sell_size->text().isEmpty() && !size.isEmpty()) {
	_sell_size->setText(size);
	sellSizeChanged();
    }
}

void
ItemMaster::purchaseQtyChanged()
{
    QString size = _purch_size->text();
    fixed qty = _purch_qty->getFixed();
    addSize(size, qty);

    if (_sell_size->text() == size)
	_sell_qty->setFixed(qty);

    recalculateMargin();
}

void
ItemMaster::sellSizeChanged()
{
    QString size = _sell_size->text();
    fixed qty;
    if (sizeExists(size, qty)) {
	_sell_qty->setFixed(qty);
	recalculateMargin();
    } else {
	qty = _sell_qty->getFixed();
	addSize(size, qty);
    }

    _sell_row = findStoreRow(size);
    if (_sell_row == -1) {
	_price->clear();
	_target_gm->clear();
	_allowed_var->clear();
    } else {
	_price->setPrice(_costs->cellValue(_sell_row, 3).toPrice());
	_target_gm->setFixed(_costs->cellValue(_sell_row, 6).toFixed());
	_allowed_var->setFixed(_costs->cellValue(_sell_row, 7).toFixed());
    }

    if (_purch_size->text().isEmpty() && !size.isEmpty()) {
	_purch_size->setText(size);
	purchaseSizeChanged();
    }
}

void
ItemMaster::sellQtyChanged()
{
    QString size = _sell_size->text();
    fixed qty = _sell_qty->getFixed();
    addSize(size, qty);

    if (_purch_size->text() == size)
	_purch_qty->setFixed(qty);

    recalculateMargin();
}

void
ItemMaster::sizesChanged()
{
    QString openSize = _open_size->currentText();

    _number_size->clear();
    _ordernum_size->clear();
    _open_size->clear();

    _number_size->insertItem("");
    _ordernum_size->insertItem("");
    for (int row = 0; row < _sizes->rows(); ++row) {
	QString name = _sizes->cellValue(row, 0).toString();
	if (name.isEmpty()) continue;
	_number_size->insertItem(name);
	_ordernum_size->insertItem(name);
	_open_size->insertItem(name);

	if (name == openSize)
	    _open_size->setCurrentItem(_open_size->count() - 1);
    }

    if (_open_size->count() == 0)
	_open_size->insertItem("        ");
}

void
ItemMaster::typeChanged()
{
    bool purchased = _purchased->isChecked();
    bool sold = _sold->isChecked();
    bool inventoried = _inventoried->isChecked();

    // Set accounts based on flags
    _asset_acct->setEnabled(inventoried);
    _income_acct->setEnabled(sold);
    _expense_acct->setEnabled((inventoried&&sold)||(purchased&&!inventoried));
    _open_dept->setEnabled(sold && inventoried && purchased);

    // Set purchasing info based on flags
    _purch_size->setEnabled(purchased);
    _purch_qty->setEnabled(purchased);
    _cost->setEnabled(purchased);
    _purch_tax->setEnabled(purchased);
    _vendor->setEnabled(purchased);
    _vendor_num->setEnabled(purchased);
    _tabs->setTabEnabled(_vendorsTab, purchased);
    _cost_inc_tax->setEnabled(purchased);
    _cost_inc_dep->setEnabled(purchased);
    _minQty->setEnabled(purchased);
    _maxQty->setEnabled(purchased);
    _costDisc->setEnabled(purchased);

    // Set selling info based on flags
    _sell_size->setEnabled(sold);
    _sell_qty->setEnabled(sold);
    _price->setEnabled(sold);
    _margin->setEnabled(sold && purchased);
    _profit->setEnabled(sold && purchased);
    _target_gm->setEnabled(sold && purchased);
    _allowed_var->setEnabled(sold && purchased);
    _sell_tax->setEnabled(sold);
    _price_inc_tax->setEnabled(sold);
    _price_inc_dep->setEnabled(sold);

    // Build info tab
    _tabs->setTabEnabled(_buildTab, inventoried && !purchased);

    // Open inventory fields
    _asOfLabel->setEnabled(inventoried);
    _as_of->setEnabled(inventoried);
    _open_size->setEnabled(inventoried);
    _open_qty->setEnabled(inventoried);
    _open_cost->setEnabled(inventoried);

    QString typeName;
    if (inventoried && sold) {
	_expenseLabel->setText(tr("COGS Account:"));
	typeName = Account::typeName(Account::COGS);
    } else {
	_expenseLabel->setText(tr("Expense Account:"));
	typeName = Account::typeName(Account::Expense);
    }
    _expenseLookup->type->setCurrentText(typeName);
}

void
ItemMaster::deptChanged()
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
ItemMaster::subdeptChanged()
{
    if (_ignoreSubdept) return;

    Id subdept_id = _subdept->getId();
    if (subdept_id == INVALID_ID) return;

    Subdept subdept;
    if (!_quasar->db()->lookup(subdept_id, subdept)) return;

    _dept->setId(subdept.deptId());
    _subLookup->dept->setId(subdept.deptId());
    _purchased->setChecked(subdept.isPurchased());
    _sold->setChecked(subdept.isSold());
    _inventoried->setChecked(subdept.isInventoried());
    _sell_tax->setId(subdept.sellTax());
    _purch_tax->setId(subdept.purchaseTax());
    _expense_acct->setId(subdept.expenseAccount());
    _income_acct->setId(subdept.incomeAccount());
    _asset_acct->setId(subdept.assetAccount());
    _target_gm->setFixed(subdept.targetGM());
    _allowed_var->setFixed(subdept.allowedVariance());
    _discountable->setChecked(subdept.isDiscountable());

    typeChanged();
    targetChanged();
    allowedChanged();
}

void
ItemMaster::recalculateMargin()
{
    static bool in_recalc = false;
    if (in_recalc) return;
    in_recalc = true;

    for (int row = 0; row < _costs->rows(); ++row) {
	QString sell_size = _costs->cellValue(row, 1).toString();
	Price price = _costs->cellValue(row, 3).toPrice();
	if (price.isNull()) {
	    _costs->setCellValue(row, 4, Variant::null);
	    _costs->setCellValue(row, 5, Variant::null);
	    if (row == _sell_row) {
		_profit->setText("");
		_margin->setText("");
	    }
	    continue;
	}

	QString purch_size = sell_size;
	Price cost = _costs->cellValue(row, 2).toPrice();
	if (cost.isNull()) {
	    purch_size = _purch_size->text();
	    cost = _cost->getPrice();
	}

	fixed sell_qty, purch_qty;
	sizeExists(sell_size, sell_qty);
	sizeExists(purch_size, purch_qty);

	ObjectCache cache(_db);

	// Adjust cost for included deposit/tax
	fixed line_cost = cost.price();
	if (_cost_inc_dep->isChecked())
	    line_cost -= _deposit->getFixed() * purch_qty;
	if (_cost_inc_tax->isChecked()) {
	    Tax tax;
	    _quasar->db()->lookup(_purch_tax->getId(), tax);
	    line_cost -= _quasar->db()->calculateTaxOff(cache, tax, line_cost);
	}
	if (_costDisc->getFixed() != 0.0) {
	    fixed discount = line_cost * _costDisc->getFixed() / 100.0;
	    discount.moneyRound();
	    line_cost -= discount;
	}

	// Adjust price for included deposit/tax
	fixed line_price = price.price();
	if (_price_inc_dep->isChecked())
	    line_price -= _deposit->getFixed() * sell_qty;
	if (_price_inc_tax->isChecked()) {
	    Tax tax;
	    _quasar->db()->lookup(_sell_tax->getId(), tax);
	    line_price -= _quasar->db()->calculateTaxOff(cache, tax,line_price);
	}

	// Adjust for different sizes
	if (purch_qty != sell_qty)
	    line_cost = line_cost * sell_qty / purch_qty;

	fixed margin = 0.0;
	fixed profit = line_price - line_cost;
	if (line_price != 0.0) margin = profit * 100.0 / line_price;

	margin.moneyRound();
	profit.moneyRound();

	// Check if profit changed
	if (profit != _costs->cellValue(row, 4).toFixed()) {
	    if (profit == 0.0) {
		_costs->setCellValue(row, 4, Variant::null);
		if (row == _sell_row) _profit->setText("");
	    } else {
		_costs->setCellValue(row, 4, profit);
		if (row == _sell_row) _profit->setFixed(profit);
	    }
	}

	// Check if margin changed
	if (margin != _costs->cellValue(row, 5).toFixed()) {
	    if (margin == 0.0) {
		_costs->setCellValue(row, 5, Variant::null);
		if (row == _sell_row) _margin->setText("");
	    } else {
		_costs->setCellValue(row, 5, margin);
		if (row == _sell_row) _margin->setFixed(margin);
	    }
	}
    }

    in_recalc = false;
}

void
ItemMaster::costChanged()
{
    if (_purch_row != -1)
	_costs->setCellValue(_purch_row, 2, _cost->getPrice());
    recalculateMargin();
}

void
ItemMaster::vendorChanged()
{
    if (_vendors->rows() == 0)
	_vendors->appendRow(new VectorRow(_vendors->columns()));

    _vendors->setCellValue(0, 0, _vendor->getId());

    if (_vendors->rows() == 1)
	_vendors->appendRow(new VectorRow(_vendors->columns()));
}

void
ItemMaster::vendorNumChanged()
{
    if (_vendors->rows() == 0)
	_vendors->appendRow(new VectorRow(_vendors->columns()));

    _vendors->setCellValue(0, 1, _vendor_num->text());

    if (_vendors->rows() == 1)
	_vendors->appendRow(new VectorRow(_vendors->columns()));
}

void
ItemMaster::priceChanged()
{
    if (_sell_row != -1)
	_costs->setCellValue(_sell_row, 3, _price->getPrice());
    recalculateMargin();
}

void
ItemMaster::marginChanged()
{
    if (_sell_row != -1)
	_costs->setCellValue(_sell_row, 5, _margin->getFixed());

    fixed cost = _cost->getPrice().price();
    fixed margin = _margin->getFixed() / 100.0;
    if (margin == 1.0) return;

    ObjectCache cache(_db);

    // Adjust cost for included deposit/tax
    if (_cost_inc_dep->isChecked())
	cost -= _deposit->getFixed() * _purch_qty->getFixed();
    if (_cost_inc_tax->isChecked()) {
	Tax tax;
	_quasar->db()->lookup(_purch_tax->getId(), tax);
	cost -= _quasar->db()->calculateTaxOff(cache, tax, cost);
    }
    if (_costDisc->getFixed() != 0.0) {
	fixed discount = cost * _costDisc->getFixed() / 100.0;
	discount.moneyRound();
	cost -= discount;
    }

    // Adjust for different sizes
    if (_purch_size->text() != _sell_size->text()) {
	fixed purch_qty = _purch_qty->getFixed();
	fixed sell_qty = _sell_qty->getFixed();
	cost = cost * sell_qty / purch_qty;
    }

    fixed price = cost / (1.0 - margin);
    price.moneyRound();

    // Adjust price for included deposit/tax
    if (_price_inc_tax->isChecked()) {
	Tax tax;
	_quasar->db()->lookup(_sell_tax->getId(), tax);
	price += _quasar->db()->calculateTaxOn(cache, tax, price);
    }
    if (_price_inc_dep->isChecked())
	price += _deposit->getFixed() * _sell_qty->getFixed();

    price.moneyRound();
    price = _company.priceRound(price);

    Price new_price(price);
    if (_price->getPrice() != new_price) {
	_price->setPrice(new_price);
	priceChanged();
    }
}

void
ItemMaster::profitChanged()
{
    if (_sell_row != -1)
	_costs->setCellValue(_sell_row, 4, _profit->getFixed());

    fixed cost = _cost->getPrice().calculate(1.0);
    fixed profit = _profit->getFixed();

    ObjectCache cache(_db);

    // Adjust cost for included deposit/tax
    if (_cost_inc_dep->isChecked())
	cost -= _deposit->getFixed() * _purch_qty->getFixed();
    if (_cost_inc_tax->isChecked()) {
	Tax tax;
	_quasar->db()->lookup(_purch_tax->getId(), tax);
	cost -= _quasar->db()->calculateTaxOff(cache, tax, cost);
    }
    if (_costDisc->getFixed() != 0.0) {
	fixed discount = cost * _costDisc->getFixed() / 100.0;
	discount.moneyRound();
	cost -= discount;
    }

    // Adjust for different sizes
    if (_purch_size->text() != _sell_size->text()) {
	fixed purch_qty = _purch_qty->getFixed();
	fixed sell_qty = _sell_qty->getFixed();
	cost = cost * sell_qty / purch_qty;
    }

    fixed price = cost + profit;
    price.moneyRound();

    // Adjust price for included deposit/tax
    if (_price_inc_tax->isChecked()) {
	Tax tax;
	_quasar->db()->lookup(_sell_tax->getId(), tax);
	price += _quasar->db()->calculateTaxOn(cache, tax, price);
    }
    if (_price_inc_dep->isChecked())
	price += _deposit->getFixed() * _sell_qty->getFixed();

    price.moneyRound();
    price = _company.priceRound(price);

    Price new_price(price);
    if (_price->getPrice() != new_price) {
	_price->setPrice(new_price);
	priceChanged();
    }
}

void
ItemMaster::targetChanged()
{
    if (_sell_row != -1)
	_costs->setCellValue(_sell_row, 6, _target_gm->getFixed());
}

void
ItemMaster::allowedChanged()
{
    if (_sell_row != -1)
	_costs->setCellValue(_sell_row, 7, _allowed_var->getFixed());
}

void
ItemMaster::openSizeChanged()
{
    openQtyChanged();
}

void
ItemMaster::openQtyChanged()
{
    QString size = _open_size->currentText();
    fixed qty = _open_qty->getFixed();
    fixed ext_cost = 0.0;

    int row = findStoreRow(size);
    if (row == -1) {
	Price cost = _cost->getPrice();
	ext_cost = cost.calculate(qty);
	fixed open_size_qty = 1.0;
	fixed purch_size_qty = 1.0;
	sizeExists(size, open_size_qty);
	sizeExists(_purch_size->text(), purch_size_qty);
	ext_cost = ext_cost * open_size_qty / purch_size_qty;
    } else {
	Price cost = _costs->cellValue(row, 2).toPrice();
	ext_cost = cost.calculate(qty);
    }

    _open_cost->setFixed(ext_cost);
}

void
ItemMaster::groupCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _groups->rows() - 1 && col == 0) {
	Id group_id = _groups->cellValue(row, col).toId();
	if (group_id != INVALID_ID)
	    _groups->appendRow(new VectorRow(_groups->columns()));
    }
}

int
ItemMaster::findStoreRow(const QString& size)
{
    if (size.stripWhiteSpace().isEmpty()) return -1;
    Id store_id = _store->getId();

    for (int row = 0; row < _costs->rows(); ++row) {
	if (_costs->cellValue(row, 0).toId() != store_id) continue;
	if (_costs->cellValue(row, 1).toString() != size) continue;
	return row;
    }
    return -1;
}

bool
ItemMaster::sizeExists(const QString& size, fixed& qty)
{
    qty = 1.0;
    for (int row = 0; row < _sizes->rows(); ++row) {
	if (_sizes->cellValue(row, 0).toString() == size) {
	    qty = _sizes->cellValue(row, 1).toFixed();
	    return true;
	}
    }
    return false;
}

bool
ItemMaster::sizeExists(const QString& size)
{
    fixed qty;
    return sizeExists(size, qty);
}

void
ItemMaster::sizeRowDeleted(int)
{
    // Check size in numbers still exists
    int i;
    for (i = _numbers->rows() - 1; i >= 0; --i) {
	QString size = _numbers->cellValue(i, 1).toString();
	if (size.isEmpty()) continue;
	if (!sizeExists(size)) _numbers->deleteRow(i);
    }

    // Check size in order numbers
    for (i = _vendors->rows() - 1; i >= 0; --i) {
	QString size = _vendors->cellValue(i, 2).toString();
	if (size.isEmpty()) continue;
	if (!sizeExists(size)) _vendors->setCellValue(i, 2, "");
    }

    // Check default purchase size still exists
    if (!sizeExists(_purch_size->text())) {
	_purch_size->setText(_sizes->cellValue(0, 0).toString());
	purchaseSizeChanged();
    }

    // Check default selling size still exists
    if (!sizeExists(_sell_size->text())) {
	_sell_size->setText(_sizes->cellValue(0, 0).toString());
	sellSizeChanged();
    }

    // Check size in stores still exists
    for (i = _costs->rows() - 1; i >= 0; --i) {
	QString size = _costs->cellValue(i, 1).toString();
	if (!sizeExists(size)) _costs->deleteRow(i);
    }

    sizesChanged();
}

void
ItemMaster::sizeCellChanged(int row, int col, Variant old)
{
    // If not blank and last row, append row
    if (row == _sizes->rows() - 1 && col == 0) {
	QString name = _sizes->cellValue(row, col).toString();
	if (!name.isEmpty())
	    _sizes->appendRow(new VectorRow(_sizes->columns()));
    }

    QString size;
    fixed qty;
    switch (col) {
    case 0:
	size = _sizes->cellValue(row, 0).toString();
	if (old.toString().isEmpty()) {
	    addCostRows(size);
	} else {
	    int row;
	    for (row = 0; row < _numbers->rows(); ++row) {
		if (_numbers->cellValue(row, 1) == old)
		    _numbers->setCellValue(row, 1, size);
	    }
	    for (row = 0; row < _vendors->rows(); ++row) {
		if (_vendors->cellValue(row, 2) == old)
		    _vendors->setCellValue(row, 2, size);
	    }
	    for (row = 0; row < _costs->rows(); ++row) {
		if (_costs->cellValue(row, 1) == old)
		    _costs->setCellValue(row, 1, size);
	    }
	    if (_purch_size->text() == old.toString()) {
		_purch_size->setText(size);
		purchaseSizeChanged();
	    }
	    if (_sell_size->text() == old.toString()) {
		_sell_size->setText(size);
		sellSizeChanged();
	    }
	}
	sizesChanged();
	break;
    case 1:
	size = _sizes->cellValue(row, 0).toString();
	qty = _sizes->cellValue(row, 1).toFixed();
	if (size == _purch_size->text())
	    _purch_qty->setFixed(qty);
	if (size == _sell_size->text())
	    _sell_qty->setFixed(qty);
	recalculateMargin();
	break;
    }
}

void
ItemMaster::sizeFocusNext(bool& leave, int&, int&, int type)
{
    int row = _sizes->currentRow();
    int col = _sizes->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	QString name = _sizes->cellValue(row, col).toString();
	if (name.isEmpty() && row == _sizes->rows() - 1)
	    leave = true;
    }
}

void
ItemMaster::numberRowDeleted(int row)
{
    if (row != 0) return;
    if (_numbers->rows() > 0)
	_number->setText(_numbers->cellValue(0, 0).toString());
}

void
ItemMaster::numberRowInserted(int row)
{
    if (row != 0) return;
    _number->setText(_numbers->cellValue(0, 0).toString());
}

void
ItemMaster::numberCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _numbers->rows() - 1 && col == 0) {
	QString number = _numbers->cellValue(row, col).toString();
	if (!number.isEmpty())
	    _numbers->appendRow(new VectorRow(_numbers->columns()));
    }

    if (row == 0 && col == 0)
	_number->setText(_numbers->cellValue(0, 0).toString());
}

void
ItemMaster::numberFocusNext(bool& leave, int&, int&, int type)
{
    int row = _numbers->currentRow();
    int col = _numbers->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	QString number = _numbers->cellValue(row, col).toString();
	if (number.isEmpty() && row == _numbers->rows() - 1)
	    leave = true;
    }
}

void
ItemMaster::vendorCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _vendors->rows() - 1 && col == 0) {
	Id vendor_id = _vendors->cellValue(row, col).toId();
	if (vendor_id != INVALID_ID)
	    _vendors->appendRow(new VectorRow(_vendors->columns()));
    }

    if (row == 0) {
	_vendor->setId(_vendors->cellValue(0, 0).toId());
	_vendor_num->setText(_vendors->cellValue(0, 1).toString());
    }
}

void
ItemMaster::vendorFocusNext(bool& leave, int&, int&, int type)
{
    int row = _vendors->currentRow();
    int col = _vendors->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id vendor_id = _vendors->cellValue(row, col).toId();
	if (vendor_id == INVALID_ID && row == _vendors->rows() - 1)
	    leave = true;
    }
}

void
ItemMaster::vendorRowDeleted(int row)
{
    if (row != 0) return;
    if (_vendors->rows() > 0) {
	_vendor->setId(_vendors->cellValue(0, 0).toId());
	_vendor_num->setText(_vendors->cellValue(0, 1).toString());
    }
}

void
ItemMaster::vendorRowInserted(int row)
{
    if (row != 0) return;
    _vendor->setId(_vendors->cellValue(0, 0).toId());
    _vendor_num->setText(_vendors->cellValue(0, 1).toString());
}

void
ItemMaster::storesCellChanged(int row, int, Variant)
{
    if (_stores->cellValue(row, 0).toId() != _store->getId())
	return;

    Id store_id = _store->getId();
    for (int i = 0; i < _stores->rows(); ++i) {
	if (_stores->cellValue(i, 0).toId() != store_id)
	    continue;

	_skipUpdates = true;
	_location->setId(_stores->cellValue(i, 1).toId());
	_stocked->setChecked(_stores->cellValue(i, 2).toBool());
	_minQty->setFixed(_stores->cellValue(i, 3).toFixed());
	_maxQty->setFixed(_stores->cellValue(i, 4).toFixed());
	_skipUpdates = false;
	break;
    }
}

void
ItemMaster::storesFocusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _stores->currentRow();
    int col = _stores->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	newCol = 1;
    } else if (type == Table::MoveNext && col == 2) {
	newCol = 1;
	newRow = row + 1;
    } else if (type == Table::MovePrev && col == 1) {
	if (row > 0) {
	    newCol = 2;
	    newRow = row - 1;
	} else {
	    leave = true;
	}
    }
}

void
ItemMaster::storesRowInserted(int)
{
}

void
ItemMaster::storesRowDeleted(int)
{
}

void
ItemMaster::costsCellChanged(int row, int col, Variant)
{
    QString size = _costs->cellValue(row, 1).toString();
    Price cost = _costs->cellValue(row, 2).toPrice();
    Price price = _costs->cellValue(row, 3).toPrice();
    fixed profit = _costs->cellValue(row, 4).toFixed();
    fixed margin = _costs->cellValue(row, 5).toFixed();
    fixed target = _costs->cellValue(row, 6).toFixed();
    fixed allowed = _costs->cellValue(row, 7).toFixed();

    switch (col) {
    case 2: // cost
	if (row == _purch_row)
	    _cost->setPrice(cost);
	recalculateMargin();
	break;
    case 3: // price
	if (row == _sell_row)
	    _price->setPrice(price);
	recalculateMargin();
	break;
    case 4: // profit
	if (_costs->cellValue(row, 4).isNull()) {
	    _costs->setCellValue(row, 3, Variant::null);
	    _costs->setCellValue(row, 5, Variant::null);
	} else {
	    fixed line_cost, purch_qty, sell_qty;
	    if (cost.isNull()) {
		line_cost = _cost->getPrice().calculate(1.0);
		purch_qty = _purch_qty->getFixed();
	    } else {
		line_cost = cost.price();
		sizeExists(size, purch_qty);
	    }
	    sizeExists(size, sell_qty);

	    ObjectCache cache(_db);

	    // Adjust cost for included deposit/tax
	    if (_cost_inc_dep->isChecked())
		line_cost -= _deposit->getFixed() * purch_qty;
	    if (_cost_inc_tax->isChecked()) {
		Tax tax;
		_quasar->db()->lookup(_purch_tax->getId(), tax);
		line_cost -= _quasar->db()->calculateTaxOff(cache, tax,
							    line_cost);
	    }
	    if (_costDisc->getFixed() != 0.0) {
		fixed discount = line_cost * _costDisc->getFixed() / 100.0;
		discount.moneyRound();
		line_cost -= discount;
	    }

	    // Adjust for different sizes
	    if (purch_qty != sell_qty)
		line_cost = line_cost * sell_qty / purch_qty;

	    fixed line_price = line_cost + profit;
	    line_price.moneyRound();

	    // Adjust price for included deposit/tax
	    if (_price_inc_tax->isChecked()) {
		Tax tax;
		_quasar->db()->lookup(_sell_tax->getId(), tax);
		line_price += _quasar->db()->calculateTaxOn(cache, tax,
							    line_price);
	    }
	    if (_price_inc_dep->isChecked())
		line_price += _deposit->getFixed() * sell_qty;

	    line_price.moneyRound();
	    line_price = _company.priceRound(line_price);

	    _costs->setCellValue(row, 3, Price(line_price));
	}
	recalculateMargin();
	break;
    case 5: // margin
	if (_costs->cellValue(row, 5).isNull()) {
	    _costs->setCellValue(row, 3, Variant::null);
	    _costs->setCellValue(row, 4, Variant::null);
	} else {
	    fixed line_cost, purch_qty, sell_qty;
	    if (cost.isNull()) {
		line_cost = _cost->getPrice().calculate(1.0);
		purch_qty = _purch_qty->getFixed();
	    } else {
		line_cost = cost.price();
		sizeExists(size, purch_qty);
	    }
	    sizeExists(size, sell_qty);

	    ObjectCache cache(_db);

	    // Adjust cost for included deposit/tax
	    if (_cost_inc_dep->isChecked())
		line_cost -= _deposit->getFixed() * purch_qty;
	    if (_cost_inc_tax->isChecked()) {
		Tax tax;
		_quasar->db()->lookup(_purch_tax->getId(), tax);
		line_cost -= _quasar->db()->calculateTaxOff(cache, tax,
							    line_cost);
	    }
	    if (_costDisc->getFixed() != 0.0) {
		fixed discount = line_cost * _costDisc->getFixed() / 100.0;
		discount.moneyRound();
		line_cost -= discount;
	    }

	    // Adjust for different sizes
	    if (purch_qty != sell_qty)
		line_cost = line_cost * sell_qty / purch_qty;

	    fixed line_price = 0.0;
	    if (margin != 100.0)
		line_price = line_cost / (1.0 - margin / 100.0);
	    line_price.moneyRound();

	    // Adjust price for included deposit/tax
	    if (_price_inc_tax->isChecked()) {
		Tax tax;
		_quasar->db()->lookup(_sell_tax->getId(), tax);
		line_price += _quasar->db()->calculateTaxOn(cache, tax,
							    line_price);
	    }
	    if (_price_inc_dep->isChecked())
		line_price += _deposit->getFixed() * sell_qty;

	    line_price.moneyRound();
	    line_price = _company.priceRound(line_price);

	    _costs->setCellValue(row, 3, Price(line_price));
	}
	recalculateMargin();
	break;
    case 6: // target
	if (row == _sell_row)
	    _target_gm->setFixed(target);
	break;
    case 7: // allowed variance
	if (row == _sell_row)
	    _allowed_var->setFixed(allowed);
	break;
    }
}

void
ItemMaster::costsFocusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _costs->currentRow();
    int col = _costs->currentColumn();

    if (type == Table::MoveNext && col == 7) {
	newCol = 2;
	newRow = row + 1;
    } else if (type == Table::MovePrev && col == 2) {
	if (row > 0) {
	    newCol = 7;
	    newRow = row - 1;
	} else {
	    leave = true;
	}
    }
}

void
ItemMaster::costsRowInserted(int row)
{
    if (row <= _purch_row)
	++_purch_row;
    if (row <= _sell_row)
	++_sell_row;
}

void
ItemMaster::costsRowDeleted(int row)
{
    if (row < _purch_row)
	--_purch_row;
    else if (row == _purch_row) {
	_purch_row = findStoreRow(_purch_size->text());
	if (_purch_row == -1)
	    _cost->clear();
	else
	    _cost->setPrice(_costs->cellValue(_purch_row, 2).toPrice());
    }

    if (row < _sell_row)
	--_sell_row;
    else if (row == _sell_row) {
	_sell_row = findStoreRow(_sell_size->text());
	if (_sell_row == -1)
	    _price->clear();
	else
	    _price->setPrice(_costs->cellValue(_sell_row, 3).toPrice());
    }
}

void
ItemMaster::buildCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _build->rows() - 1 && col == 0) {
	Id item_id = _build->cellValue(row, col).toId();
	if (item_id != INVALID_ID)
	    _build->appendRow(new VectorRow(_build->columns()));
    }

    Id store_id = _quasar->defaultStore();
    Id item_id = _build->cellValue(row, 0).toPlu().itemId();
    QString number = _build->cellValue(row, 0).toPlu().number();
    QString size = _build->cellValue(row, 2).toString();
    fixed qty = _build->cellValue(row, 3).toFixed();

    Item item;
    switch (col) {
    case 0:
	if (_quasar->db()->lookup(item_id, item)) {
	    QString size = item.numberSize(number);
	    if (size.isEmpty()) size = item.sellSize();
	    fixed cost = item.cost(store_id, size, 1.0);

	    _build->setCellValue(row, 1, item.description());
	    _build->setCellValue(row, 2, size);
	    _build->setCellValue(row, 3, 1.0);
	    _build->setCellValue(row, 4, cost);
	}
	break;
    case 2: // size
	if (_quasar->db()->lookup(item_id, item)) {
	    fixed cost = item.cost(store_id, size, qty);
	    _build->setCellValue(row, 4, cost);
	}
	break;
    case 3: // qty
	if (_quasar->db()->lookup(item_id, item)) {
	    fixed cost = item.cost(store_id, size, qty);
	    _build->setCellValue(row, 4, cost);
	}
	break;
    }

    fixed buildCost = 0.0;
    for (int i = 0; i < _build->rows(); ++i)
	if (_build->cellValue(i, 0).toId() != INVALID_ID)
	    buildCost += _build->cellValue(i, 4).toFixed();
    _build_cost->setFixed(buildCost);
}

void
ItemMaster::buildFocusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _build->currentRow();
    int col = _build->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id item_id = _build->cellValue(row, 0).toId();
	if (item_id == INVALID_ID && row == _build->rows() - 1)
	    leave = true;
	else
	    newCol = 3;
    } else if (type == Table::MoveNext && col == 3) {
	if (row == _build->rows() - 1)
	    leave = true;
	else
	    newRow = row + 1;
	newCol = 0;
    } else if (type == Table::MovePrev && col == 0) {
	newCol = 3;
    } else if (type == Table::MovePrev && col == 2) {
	newCol = 0;
    }
}

void
ItemMaster::extraFocusNext(bool&, int& newRow, int& newCol, int type)
{
    int row = _extra->currentRow();
    int col = _extra->currentColumn();

    if (type == Table::MoveNext && col == 1) {
	newCol = 1;
    } else if (type == Table::MovePrev && col == 1) {
	newRow = row - 1;
	newCol = 1;
    }
}

void
ItemMaster::slotBuild()
{
    if (!saveItem(true)) return;
    if (_curr.items().size() == 0) return;

    Id store_id = _quasar->defaultStore();

    QDialog* dialog = new QDialog(this, "BuildKits", true);
    dialog->setCaption(tr("Item Build"));

    fixed groupQty = _curr.buildQty();
    QString msg = tr("This item builds in groups of %1.  How\n"
		     "many groups do you wish to build?")
	.arg(groupQty.toString());
    QLabel* msgLabel = new QLabel(msg, dialog);

    QLabel* qtyLabel = new QLabel(tr("How Many:"), dialog);
    NumberEdit* qtyEdit = new DoubleEdit(dialog);

    QLabel* storeLabel = new QLabel(tr("Store:"), dialog);
    LookupWindow* lookup = new StoreLookup(_main, this);
    lookup->setAllowNew(false);
    LookupEdit* storeEdit = new LookupEdit(lookup, dialog);

    QFrame* buttons = new QFrame(dialog);
    QPushButton* ok = new QPushButton(tr("OK"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);

    ok->connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    cancel->connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
    ok->setDefault(true);

    QGridLayout* grid1 = new QGridLayout(buttons);
    grid1->setSpacing(6);
    grid1->setMargin(6);
    grid1->setColStretch(0, 1);
    grid1->addWidget(ok, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
    grid1->addWidget(cancel, 0, 1, Qt::AlignRight | Qt::AlignVCenter);

    QGridLayout* grid = new QGridLayout(dialog);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->addMultiCellWidget(msgLabel, 0, 0, 0, 1);
    grid->addWidget(qtyLabel, 1, 0);
    grid->addWidget(qtyEdit, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(storeLabel, 2, 0);
    grid->addWidget(storeEdit, 2, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(buttons, 3, 3, 0, 1);

    qtyEdit->setFixed(1.0);
    qtyEdit->selectAll();
    storeEdit->setId(store_id);

    qtyEdit->setFocus();
    if (dialog->exec() != QDialog::Accepted) {
	delete dialog;
	return;
    }

    fixed build_qty = qtyEdit->getFixed();
    store_id = storeEdit->getId();
    delete dialog;

    if (build_qty <= 0.0) {
	QMessageBox::critical(this, tr("Error"), tr("Invalid build quantity"));
	return;
    }
    if (store_id == INVALID_ID) {
	QMessageBox::critical(this, tr("Error"), tr("Store is required"));
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    ItemAdjust tx;
    tx.setPostDate(QDate::currentDate());
    tx.setPostTime(QTime::currentTime());
    tx.setStoreId(store_id);
    QString memo = tr("Auto Build; ") + _curr.description();
    tx.setMemo(memo.left(40));

    // Adjust out the component items
    fixed total_cost = 0.0;
    for (unsigned int i = 0; i < _curr.items().size(); ++i) {
	Id item_id = _curr.items()[i].item_id;
	QString number = _curr.items()[i].number;
	QString size = _curr.items()[i].size;
	fixed qty = _curr.items()[i].qty * build_qty;

	Item item;
	_quasar->db()->lookup(item_id, item);

	fixed ext_cost;
	_quasar->db()->itemSellingCost(item, size, store_id, qty, 0, ext_cost);

	fixed size_qty = item.sizeQty(size);
	ItemLine line;
	line.item_id = item.id();
	line.number = number;
	line.size = size;
	line.size_qty = size_qty;
	line.quantity = -qty;
	line.inv_cost = -ext_cost;
	tx.items().push_back(line);

	tx.accounts().push_back(AccountLine(item.assetAccount(), -ext_cost));
	total_cost += ext_cost;
    }

    // Adjust in the current items
    QString number = _curr.number();
    QString size = _curr.sellSize();
    fixed size_qty = _curr.sizeQty(size);
    fixed total_qty = build_qty * _curr.buildQty();
    ItemLine line;
    line.item_id = _curr.id();
    line.number = number;
    line.size = size;
    line.size_qty = size_qty;
    line.quantity = total_qty;
    line.inv_cost = total_cost;
    tx.items().push_back(line);

    tx.accounts().push_back(AccountLine(_curr.assetAccount(), total_cost));

    bool good = _quasar->db()->create(tx);
    QApplication::restoreOverrideCursor();

    if (!good) {
	QMessageBox::critical(this, tr("Error"),
			      tr("Failed to create adjustment"));
	return;
    }

    QString message = tr("Created adjustment #%1").arg(tx.number());
    QMessageBox::information(this, tr("Finished"), message);
}

void
ItemMaster::slotCreateData()
{
    bool ok = false;
    QString message = tr("Enter the new data name:");
    QString text = QInputDialog::getText(tr("Create Data"), message,
					 QLineEdit::Normal, QString::null,
					 &ok, this);
    if (!ok || text.isEmpty()) return;

    Extra extra;
    extra.setTable("Item");
    extra.setName(text);
    if (!_quasar->db()->create(extra)) {
	QMessageBox::critical(this, tr("Error"),
			      tr("Creating new data failed."));
	return;
    }

    VectorRow* row = new VectorRow(_extra->columns());
    row->setValue(0, extra.id());
    row->setValue(1, "");
    _extra->appendRow(row);
}

void
ItemMaster::slotDeleteData()
{
    int row = _extra->currentRow();
    if (row == -1) return;

    Id extra_id = _extra->cellValue(row, 0).toId();
    if (extra_id == INVALID_ID) return;

    Extra extra;
    if (!_quasar->db()->lookup(extra_id, extra)) {
	QMessageBox::critical(this, tr("Error"), tr("Data lookup failed."));
	return;
    }

    QString message = tr("Are you sure you want to delete the\n"
			 "%1 data?  It will be removed from\n"
			 "all items.").arg(extra.name());
    int choice = QMessageBox::warning(this, tr("Delete?"), message,
				      QMessageBox::No, QMessageBox::Yes);
    if (choice != QMessageBox::Yes)
	return;

    if (!_quasar->db()->remove(extra)) {
	QMessageBox::critical(this, tr("Error"), tr("Deleting data failed."));
	return;
    }

    _extra->deleteRow(row);
}

void
ItemMaster::slotRenameData()
{
    int row = _extra->currentRow();
    if (row == -1) return;

    Id extra_id = _extra->cellValue(row, 0).toId();
    if (extra_id == INVALID_ID) return;

    Extra extra;
    if (!_quasar->db()->lookup(extra_id, extra)) {
	QMessageBox::critical(this, tr("Error"), tr("Data lookup failed."));
	return;
    }

    bool ok = false;
    QString message = tr("Enter the new name for %1:").arg(extra.name());
    QString text = QInputDialog::getText(tr("Rename Data"), message,
					 QLineEdit::Normal, QString::null,
					 &ok, this);
    if (!ok || text.isEmpty()) return;

    Extra orig = extra;
    extra.setName(text);
    if (!_quasar->db()->update(orig, extra)) {
	QMessageBox::critical(this, tr("Error"), tr("Renaming data failed."));
	return;
    }

    _extra->setCellValue(row, 0, extra.id());
}
