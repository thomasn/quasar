// $Id: receive_master.cpp,v 1.92 2005/06/08 07:32:25 bpepers Exp $
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

#include "receive_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "object_cache.h"
#include "receive_payment.h"
#include "order.h"
#include "recurring_master.h"
#include "item_price_select.h"
#include "company.h"
#include "gltx_frame.h"
#include "table.h"
#include "multi_line_edit.h"
#include "date_popup.h"
#include "id_edit.h"
#include "double_edit.h"
#include "money_edit.h"
#include "percent_edit.h"
#include "price_edit.h"
#include "item_edit.h"
#include "vendor_lookup.h"
#include "customer_lookup.h"
#include "item_lookup.h"
#include "account_lookup.h"
#include "tax_lookup.h"
#include "charge_lookup.h"
#include "term_lookup.h"
#include "store_lookup.h"
#include "order_lookup.h"
#include "store.h"
#include "report_defn.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpopupmenu.h>
#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpair.h>

typedef QPair<Id, fixed> InfoPair;

ReceiveMaster::ReceiveMaster(MainWindow* main, Id receive_id)
    : DataWindow(main, "VendorInvoice", receive_id), _cache(_db),
      _item_row(-1), _skip_recalc(false)
{
    _helpSource = "vendor_invoice.html";

    _file->insertItem(tr("Recurring"), this, SLOT(slotRecurring()));

    // Search button
    QPushButton* search = new QPushButton(tr("Search"), _buttons);
    connect(search, SIGNAL(clicked()), SLOT(slotSearch()));

    // Print button
    QPushButton* print = new QPushButton(tr("Print"), _buttons);
    connect(print, SIGNAL(clicked()), SLOT(slotPrint()));

    // Payment button
    _payment = new QPushButton(tr("Payments"), _buttons);
    connect(_payment, SIGNAL(clicked()), SLOT(slotPayment()));

    // Sales history button
    QPushButton* hist = new QPushButton(tr("History"), _buttons);
    connect(hist, SIGNAL(clicked()), SLOT(slotSalesHistory()));

    _gltxFrame = new GltxFrame(main, tr("Invoice No."), _frame);
    _gltxFrame->store->setLength(30);
    _gltxFrame->station->setLength(30);
    _gltxFrame->employee->setLength(30);
    _gltxFrame->hideMemo();
    connect(_gltxFrame->store, SIGNAL(validData()), SLOT(slotStoreChanged()));

    QFrame* top = new QFrame(_frame);
    top->setFrameStyle(QFrame::Raised | QFrame::Panel);

    QLabel* vendorLabel = new QLabel(tr("Vendor:"), top);
    _vendor_id = new LookupEdit(new VendorLookup(main, this), top);
    _vendor_id->setLength(30);
    connect(_vendor_id, SIGNAL(validData()), SLOT(slotVendorChanged()));

    QLabel* vendorAddrLabel = new QLabel(tr("Address:"), top);
    _vendor_addr = new MultiLineEdit(top);
    _vendor_addr->setFixedVisibleLines(4);
    vendorAddrLabel->setBuddy(_vendor_addr);

    QLabel* termLabel = new QLabel(tr("Terms:"), top);
    _term_id = new LookupEdit(new TermLookup(main, this), top);
    _term_id->setLength(30);
    termLabel->setBuddy(_term_id);

    QLabel* shipViaLabel = new QLabel(tr("Ship Via:"), top);
    _ship_via = new LineEdit(top);
    _ship_via->setLength(20);
    shipViaLabel->setBuddy(_ship_via);

    QLabel* invDateLabel = new QLabel(tr("Vendor's Date:"), top);
    _inv_date = new DatePopup(top);
    invDateLabel->setBuddy(_inv_date);

    QLabel* typeLabel = new QLabel(tr("Type:"), top);
    _type = new QComboBox(false, top);
    _type->insertItem(tr("Item"));
    _type->insertItem(tr("Account"));
    typeLabel->setBuddy(_type);
    connect(_type, SIGNAL(activated(int)), SLOT(slotTypeChanged()));

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(3);
    topGrid->setMargin(3);
    topGrid->setRowStretch(3, 1);
    topGrid->setColStretch(2, 1);
    topGrid->addWidget(vendorLabel, 0, 0);
    topGrid->addWidget(_vendor_id, 0, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(vendorAddrLabel, 1, 0, AlignLeft | AlignTop);
    topGrid->addMultiCellWidget(_vendor_addr, 1, 4, 1, 1);
    topGrid->addWidget(termLabel, 0, 3, AlignRight | AlignVCenter);
    topGrid->addWidget(_term_id, 0, 4, AlignLeft | AlignVCenter);
    topGrid->addWidget(shipViaLabel, 1, 3, AlignRight | AlignVCenter);
    topGrid->addWidget(_ship_via, 1, 4, AlignLeft | AlignVCenter);
    topGrid->addWidget(invDateLabel, 2, 3, AlignRight | AlignVCenter);
    topGrid->addWidget(_inv_date, 2, 4, AlignLeft | AlignVCenter);
    topGrid->addWidget(typeLabel, 3, 3, AlignRight | AlignVCenter);
    topGrid->addWidget(_type, 3, 4, AlignLeft | AlignVCenter);

    QFrame* mid = new QFrame(_frame);
    mid->setFrameStyle(QFrame::Raised | QFrame::Panel);

    _items = new Table(mid);
    _items->setVScrollBarMode(QScrollView::AlwaysOn);
    _items->setLeftMargin(fontMetrics().width("99999"));
    _items->setDisplayRows(5);
    connect(_items, SIGNAL(cellMoved(int,int)), SLOT(itemCellMoved(int,int)));
    connect(_items, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(itemCellChanged(int,int,Variant)));
    connect(_items, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(itemFocusNext(bool&,int&,int&,int)));
    connect(_items, SIGNAL(rowInserted(int)), SLOT(itemRowInserted(int)));
    connect(_items, SIGNAL(rowDeleted(int)), SLOT(itemRowDeleted(int)));

    // Lookups
    _lookup = new ItemLookup(_main, this);
    _lookup->purchasedOnly = true;
    TaxLookup* taxLookup = new TaxLookup(_main, this);

    // Add columns
    new LookupColumn(_items, tr("Item Number"), 10, _lookup);
    new TextColumn(_items, tr("Description"), 10);
    new TextColumn(_items, tr("Size"), 10);
    new NumberColumn(_items, tr("Qty"), 6);
    new PriceColumn(_items, tr("Cost"));
    new MoneyColumn(_items, tr("Ext Cost"));
    new LookupColumn(_items, tr("Tax"), 6, taxLookup);

    LineEdit* descEdit = new LineEdit(_items);
    NumberEdit* qtyEdit = new DoubleEdit(_items);
    PriceEdit* costEdit = new PriceEdit(_items);
    NumberEdit* extCostEdit = new MoneyEdit(_items);
    descEdit->setMaxLength(40);
    qtyEdit->setMaxLength(8);
    extCostEdit->setMaxLength(14);

    // Add editors
    _size = new QComboBox(false, _items);
    new LookupEditor(_items, 0, new ItemEdit(_lookup, _items));
    new LineEditor(_items, 1, descEdit);
    new ComboEditor(_items, 2, _size);
    new NumberEditor(_items, 3, qtyEdit);
    new PriceEditor(_items, 4, costEdit);
    new NumberEditor(_items, 5, extCostEdit);
    new LookupEditor(_items, 6, new LookupEdit(taxLookup, _items));

    _items->setMinimumWidth(_items->sizeHint().width());
    QGridLayout* midGrid = new QGridLayout(mid);
    midGrid->setSpacing(3);
    midGrid->setMargin(3);
    midGrid->setColStretch(0, 1);
    midGrid->addWidget(_items, 0, 0);

    _tabs = new QTabWidget(_frame);
    QFrame* totals = new QFrame(_tabs);
    QFrame* comments = new QFrame(_tabs);
    QFrame* taxes = new QFrame(_tabs);
    QFrame* charges = new QFrame(_tabs);
    QFrame* ship = new QFrame(_tabs);
    QFrame* itemInfo = new QFrame(_tabs);
    _ordersTab = new QFrame(_tabs);
    _tabs->addTab(totals, tr("Totals"));
    _tabs->addTab(comments, tr("Comments"));
    _tabs->addTab(taxes, tr("Taxes"));
    _tabs->addTab(charges, tr("Charges"));
    _tabs->addTab(ship, tr("Ship To"));
    _tabs->addTab(itemInfo, tr("Item Info"));
    _tabs->addTab(_ordersTab, tr("Orders"));

    QLabel* itemLabel = new QLabel(tr("Item Total:"), totals);
    _item_amt = new MoneyEdit(totals);
    _item_amt->setFocusPolicy(NoFocus);

    QLabel* chargeLabel = new QLabel(tr("Charge Total:"), totals);
    _charge_amt = new MoneyEdit(totals);
    _charge_amt->setFocusPolicy(NoFocus);

    QLabel* taxAmtLabel = new QLabel(tr("Tax Total:"), totals);
    _tax_amt = new MoneyEdit(totals);
    _tax_amt->setFocusPolicy(NoFocus);

    QLabel* depositAmtLabel = new QLabel(tr("Deposit Total:"), totals);
    _deposit_amt = new MoneyEdit(totals);
    _deposit_amt->setFocusPolicy(NoFocus);

    QLabel* itemQtyLabel = new QLabel(tr("Qty Total:"), totals);
    _item_qty = new DoubleEdit(totals);
    _item_qty->setFocusPolicy(NoFocus);

    QLabel* totalLabel = new QLabel(tr("Total:"), totals);
    _total_amt = new MoneyEdit(totals);
    _total_amt->setFocusPolicy(NoFocus);

    QLabel* paidLabel = new QLabel(tr("Paid:"), totals);
    _paid_amt = new MoneyEdit(totals);
    _paid_amt->setFocusPolicy(NoFocus);

    QLabel* dueLabel = new QLabel(tr("Due:"), totals);
    _due_amt = new MoneyEdit(totals);
    _due_amt->setFocusPolicy(NoFocus);

    QGridLayout* totalGrid = new QGridLayout(totals);
    totalGrid->setSpacing(3);
    totalGrid->setMargin(3);
    totalGrid->setColStretch(2, 1);
    totalGrid->setColStretch(5, 1);
    totalGrid->addWidget(itemLabel, 0, 0, AlignRight | AlignVCenter);
    totalGrid->addWidget(_item_amt, 0, 1, AlignLeft | AlignVCenter);
    totalGrid->addWidget(chargeLabel, 1, 0, AlignRight | AlignVCenter);
    totalGrid->addWidget(_charge_amt, 1, 1, AlignLeft | AlignVCenter);
    totalGrid->addWidget(taxAmtLabel, 0, 3, AlignRight | AlignVCenter);
    totalGrid->addWidget(_tax_amt, 0, 4, AlignLeft | AlignVCenter);
    totalGrid->addWidget(depositAmtLabel, 1, 3, AlignRight | AlignVCenter);
    totalGrid->addWidget(_deposit_amt, 1, 4, AlignLeft | AlignVCenter);
    totalGrid->addWidget(itemQtyLabel, 2, 3, AlignRight | AlignVCenter);
    totalGrid->addWidget(_item_qty, 2, 4, AlignLeft | AlignVCenter);
    totalGrid->addWidget(totalLabel, 0, 6, AlignRight | AlignVCenter);
    totalGrid->addWidget(_total_amt, 0, 7, AlignLeft | AlignVCenter);
    totalGrid->addWidget(paidLabel, 1, 6, AlignRight | AlignVCenter);
    totalGrid->addWidget(_paid_amt, 1, 7, AlignLeft | AlignVCenter);
    totalGrid->addWidget(dueLabel, 2, 6, AlignRight | AlignVCenter);
    totalGrid->addWidget(_due_amt, 2, 7, AlignLeft | AlignVCenter);

    QLabel* commentLabel = new QLabel(tr("Comment:"), comments);
    _comment = new MultiLineEdit(comments);
    _comment->setMinimumWidth(_comment->fontMetrics().width('x') * 70);
    _comment->setMaximumWidth(_comment->fontMetrics().width('x') * 70);
    _comment->setFixedVisibleLines(4);
    commentLabel->setBuddy(_comment);

    QGridLayout* commentGrid = new QGridLayout(comments);
    commentGrid->setSpacing(3);
    commentGrid->setMargin(3);
    commentGrid->setRowStretch(2, 1);
    commentGrid->setColStretch(1, 1);
    commentGrid->addWidget(commentLabel, 0, 0, AlignLeft | AlignVCenter);
    commentGrid->addMultiCellWidget(_comment,0,1,1,1,AlignLeft|AlignVCenter);

    _taxes = new Table(taxes);
    _taxes->setVScrollBarMode(QScrollView::AlwaysOn);
    _taxes->setDisplayRows(3);

    new TextColumn(_taxes, tr("Tax"), 6);
    new TextColumn(_taxes, tr("Name"), 20);
    new MoneyColumn(_taxes, tr("Taxable"));
    new MoneyColumn(_taxes, tr("Tax Amt"));
    new MoneyColumn(_taxes, tr("Inc. Taxable"));
    new MoneyColumn(_taxes, tr("Inc. Tax Amt"));

    QGridLayout* taxGrid = new QGridLayout(taxes);
    taxGrid->setSpacing(3);
    taxGrid->setMargin(3);
    taxGrid->setRowStretch(0, 1);
    taxGrid->setColStretch(0, 1);
    taxGrid->addWidget(_taxes, 0, 0);

    _icharges = new Table(charges);
    _icharges->setVScrollBarMode(QScrollView::AlwaysOn);
    _icharges->setDisplayRows(3);
    _icharges->setLeftMargin(fontMetrics().width("999"));
    connect(_icharges, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(ichargeCellChanged(int,int,Variant)));
    connect(_icharges, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(ichargeFocusNext(bool&,int&,int&,int)));
    connect(_icharges, SIGNAL(rowInserted(int)),SLOT(ichargeRowInserted(int)));
    connect(_icharges, SIGNAL(rowDeleted(int)), SLOT(ichargeRowDeleted(int)));

    // Lookups
    ChargeLookup* ichargeLookup = new ChargeLookup(_main, this);
    TaxLookup* chargeTaxLookup = new TaxLookup(_main, this);

    new LookupColumn(_icharges, tr("Internal Charge"), 20, ichargeLookup);
    new MoneyColumn(_icharges, tr("Amount"), 5);
    new LookupColumn(_icharges, tr("Tax"), 6, chargeTaxLookup);

    new LookupEditor(_icharges, 0, new LookupEdit(ichargeLookup, _icharges));
    new NumberEditor(_icharges, 1, new MoneyEdit(_icharges));
    new LookupEditor(_icharges, 2, new LookupEdit(chargeTaxLookup,_icharges));

    _echarges = new Table(charges);
    _echarges->setVScrollBarMode(QScrollView::AlwaysOn);
    _echarges->setDisplayRows(3);
    _echarges->setLeftMargin(fontMetrics().width("999"));
    connect(_echarges, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(echargeCellChanged(int,int,Variant)));
    connect(_echarges, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(echargeFocusNext(bool&,int&,int&,int)));
    connect(_echarges, SIGNAL(rowInserted(int)),SLOT(echargeRowInserted(int)));
    connect(_echarges, SIGNAL(rowDeleted(int)), SLOT(echargeRowDeleted(int)));

    // Lookups
    ChargeLookup* echargeLookup = new ChargeLookup(_main, this);

    new LookupColumn(_echarges, tr("External Charge"), 20, echargeLookup);
    new MoneyColumn(_echarges,tr("Amount"), 5);

    new LookupEditor(_echarges, 0, new LookupEdit(echargeLookup, _echarges));
    new NumberEditor(_echarges, 1, new MoneyEdit(_echarges));

    QGridLayout* chargeGrid = new QGridLayout(charges);
    chargeGrid->setSpacing(3);
    chargeGrid->setMargin(3);
    chargeGrid->setRowStretch(0, 1);
    chargeGrid->setColStretch(0, 2);
    chargeGrid->setColStretch(1, 1);
    chargeGrid->addWidget(_icharges, 0, 0);
    chargeGrid->addWidget(_echarges, 0, 1);

    QLabel* shipLabel = new QLabel(tr("Ship To:"), ship);
    _ship = new LookupEdit(new CustomerLookup(main, this), ship);
    _ship->setLength(30);
    _ship->setFocusPolicy(ClickFocus);
    shipLabel->setBuddy(_ship);
    connect(_ship, SIGNAL(validData()), SLOT(slotShipToChanged()));

    QLabel* shipAddrLabel = new QLabel(tr("Address:"), ship);
    _ship_addr = new MultiLineEdit(ship);
    _ship_addr->setFocusPolicy(ClickFocus);
    _ship_addr->setFixedVisibleLines(4);
    shipAddrLabel->setBuddy(_ship_addr);

    QGridLayout* shipGrid = new QGridLayout(ship);
    shipGrid->setSpacing(3);
    shipGrid->setMargin(3);
    shipGrid->setRowStretch(2, 1);
    shipGrid->setColStretch(1, 1);
    shipGrid->addWidget(shipLabel, 0, 0);
    shipGrid->addWidget(_ship, 0, 1, AlignLeft | AlignVCenter);
    shipGrid->addWidget(shipAddrLabel, 1, 0);
    shipGrid->addMultiCellWidget(_ship_addr, 1, 2, 1,1,AlignLeft|AlignVCenter);

    _inc_tax = new QCheckBox(tr("Include Tax?"), itemInfo);
    connect(_inc_tax, SIGNAL(toggled(bool)), SLOT(slotIncTaxChanged()));
    _inc_dep = new QCheckBox(tr("Include Deposit?"), itemInfo);
    connect(_inc_dep, SIGNAL(toggled(bool)), SLOT(slotIncDepChanged()));

    QLabel* depositLabel = new QLabel(tr("Deposit:"), itemInfo);
    _deposit = new MoneyEdit(itemInfo);
    _deposit->setLength(10);
    connect(_deposit, SIGNAL(validData()), SLOT(slotDepositChanged()));

    QLabel* orderedLabel = new QLabel(tr("Ordered:"), itemInfo);
    _ordered = new DoubleEdit(itemInfo);
    _ordered->setLength(8);
    _ordered->setFocusPolicy(NoFocus);

    QLabel* lineCostLabel = new QLabel(tr("Line Cost:"), itemInfo);
    _line_cost = new MoneyEdit(itemInfo);
    _line_cost->setLength(10);
    _line_cost->setFocusPolicy(NoFocus);

    QLabel* lineIChargeLabel = new QLabel(tr("Internal Charge:"), itemInfo);
    _line_icharge = new MoneyEdit(itemInfo);
    _line_icharge->setLength(10);
    _line_icharge->setFocusPolicy(NoFocus);

    QLabel* lineEChargeLabel = new QLabel(tr("External Charge:"), itemInfo);
    _line_echarge = new MoneyEdit(itemInfo);
    _line_echarge->setLength(10);
    _line_echarge->setFocusPolicy(NoFocus);

    QGridLayout* itemGrid = new QGridLayout(itemInfo);
    itemGrid->setSpacing(3);
    itemGrid->setMargin(3);
    itemGrid->setRowStretch(3, 1);
    itemGrid->setColStretch(1, 1);
    itemGrid->setColStretch(4, 1);
    itemGrid->addWidget(_inc_tax, 0, 0);
    itemGrid->addWidget(_inc_dep, 1, 0);
    itemGrid->addWidget(depositLabel, 0, 2);
    itemGrid->addWidget(_deposit, 0, 3);
    itemGrid->addWidget(orderedLabel, 1, 2);
    itemGrid->addWidget(_ordered, 1, 3);
    itemGrid->addWidget(lineCostLabel, 0, 5);
    itemGrid->addWidget(_line_cost, 0, 6);
    itemGrid->addWidget(lineIChargeLabel, 1, 5);
    itemGrid->addWidget(_line_icharge, 1, 6);
    itemGrid->addWidget(lineEChargeLabel, 2, 5);
    itemGrid->addWidget(_line_echarge, 2, 6);

    _orders = new Table(_ordersTab);
    _orders->setDisplayRows(3);
    _orders->setVScrollBarMode(QScrollView::AlwaysOn);
    connect(_orders, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(orderCellChanged(int,int,Variant)));
    connect(_orders, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(orderFocusNext(bool&,int&,int&,int)));

    // Lookups
    _orderLookup = new OrderLookup(_main, this);

    // Add columns
    new LookupColumn(_orders, tr("Number"), 15, _orderLookup);
    new DateColumn(_orders, tr("Date"));

    // Add editors
    new LookupEditor(_orders, 0, new LookupEdit(_orderLookup, _orders));

    QGridLayout* orderGrid = new QGridLayout(_ordersTab);
    orderGrid->setSpacing(3);
    orderGrid->setMargin(3);
    orderGrid->setRowStretch(0, 1);
    orderGrid->setColStretch(0, 1);
    orderGrid->setColStretch(1, 1);
    orderGrid->addWidget(_orders, 0, 0);

    // Flags
    _inactive->setText(tr("Voided?"));
    _claim = new QCheckBox(tr("Claim?"), _buttons);
    _claim->setFocusPolicy(ClickFocus);
    connect(_claim, SIGNAL(toggled(bool)), SLOT(slotClaimChanged()));

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(0, 1);
    grid->setRowStretch(2, 1);
    grid->addWidget(_gltxFrame, 0, 0);
    grid->addWidget(top, 1, 0);
    grid->addWidget(mid, 2, 0);
    grid->addWidget(_tabs, 3, 0);

    Company company;
    _quasar->db()->lookup(company);
    if (company.depositAccount() == INVALID_ID) {
	depositAmtLabel->hide();
	_deposit_amt->hide();
	_inc_dep->hide();
	depositLabel->hide();
	_deposit->hide();
    }

    _old_type = Receive::Item;
    _size_width = _items->columnWidth(2);

    setCaption(tr("Vendor Invoice"));
    finalize();
}

ReceiveMaster::~ReceiveMaster()
{
}

void
ReceiveMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);
    _curr = _orig;
    _firstField = _vendor_id;
}

void
ReceiveMaster::newItem()
{
    Receive blank;
    _orig = blank;
    _gltxFrame->defaultData(_orig);
    _orig.setInvoiceDate(QDate::currentDate());

    Company company;
    _quasar->db()->lookup(company);
    _orig.setShipAddress(company.address());

    _curr = _orig;
    _firstField = _vendor_id;
}

void
ReceiveMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _gltxFrame->cloneData(_curr);
    for (unsigned int i = 0; i < _curr.items().size(); ++i)
	_curr.items()[i].ordered = 0.0;
    dataToWidget();
}

bool
ReceiveMaster::fileItem()
{
    if (_curr.number() != "#")
	if (checkGltxUsed(_curr.number(), _curr.dataType(), _curr.id()))
	    return false;

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
ReceiveMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
ReceiveMaster::restoreItem()
{
    _curr = _orig;
}

void
ReceiveMaster::cloneItem()
{
    ReceiveMaster* clone = new ReceiveMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
ReceiveMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
ReceiveMaster::dataToWidget()
{
    _gltxFrame->setData(_curr);
    _vendor_id->setId(_curr.vendorId());
    _vendor_addr->setText(_curr.vendorAddress());
    _ship->setId(_curr.shipId());
    _ship_addr->setText(_curr.shipAddress());
    _term_id->setId(_curr.termsId());
    _type->setCurrentItem(_curr.lineType());
    _ship_via->setText(_curr.shipVia());
    _inv_date->setDate(_curr.invoiceDate());
    _comment->setText(_curr.comment());
    _inactive->setChecked(!_curr.isActive());
    _claim->setChecked(_curr.isClaim());

    // Load vendor
    _quasar->db()->lookup(_curr.vendorId(), _vendor);

    // Clear the caches
    _item_cache.clear();

    // Clear out item info
    clearItems(false);
    slotTypeChanged();

    // Get sign from claim flag
    fixed sign = 1;
    if (_curr.isClaim()) sign = -1;

    // Load the items
    _items->setUpdatesEnabled(false);
    _items->clear();
    _lines.clear();
    _lookup->store_id = _gltxFrame->store->getId();
    const vector<ReceiveItem>& items = _curr.items();
    unsigned int line;
    for (line = 0; line < items.size(); ++line) {
	Id item_id = items[line].item_id;
	Id account_id = items[line].account_id;
	QString number = items[line].number;
	QString description = items[line].description;
	QString size = items[line].size;
	fixed size_qty = items[line].size_qty;
	fixed quantity = items[line].quantity * sign;
	fixed ordered = items[line].ordered;
	fixed received = items[line].received;
	Id cost_id = items[line].cost_id;
	fixed cost_disc = items[line].cost_disc;
	Price cost = items[line].cost;
	fixed ext_cost = items[line].ext_cost;
	fixed ext_base = items[line].ext_base;
	fixed ext_deposit = items[line].ext_deposit;
	fixed ext_tax = items[line].ext_tax;
	fixed int_charges = items[line].int_charges;
	fixed ext_charges = items[line].ext_charges;
	Id tax_id = items[line].tax_id;
	Id item_tax_id = items[line].item_tax_id;
	bool include_tax = items[line].include_tax;
	bool include_deposit = items[line].include_deposit;
	bool open_dept = items[line].open_dept;

	Item item;
	Account account;
	QString item_desc = description;
	if (_curr.lineType() == Receive::Item) {
	    _cache.findItem(item_id, item);
	    if (item_desc.isEmpty())
		item_desc = item.description();
	} else {
	    _cache.findAccount(account_id, account);
	}

	VectorRow* row = new VectorRow(_items->columns());
	if (_curr.lineType() == Receive::Item)
	    row->setValue(0, Plu(item_id, number));
	else
	    row->setValue(0, account_id);
	row->setValue(1, item_desc);
	row->setValue(2, size);
	row->setValue(3, quantity);
	row->setValue(4, cost);
	row->setValue(5, ext_cost);
	row->setValue(6, tax_id);
	_items->appendRow(row);

	ItemLine& line = _lines[_items->rows() - 1];
	line.item = item;
	line.account = account;
	line.number = number;
	line.description = description;
	line.size = size;
	line.size_qty = size_qty;
	line.qty = quantity;
	line.received = received;
	line.ordered = ordered;
	line.cost_id = cost_id;
	line.cost_disc = cost_disc;
	line.cost = cost;
	line.ext_cost = ext_cost;
	line.ext_base = ext_base;
	line.ext_deposit = ext_deposit;
	line.ext_tax = ext_tax;
	line.int_charges = int_charges;
	line.ext_charges = ext_charges;
	line.tax_id = tax_id;
	line.item_tax_id = item_tax_id;
	line.include_tax = include_tax;
	line.include_deposit = include_deposit;
	line.open_dept = open_dept;
    }
    _items->appendRow(new VectorRow(_items->columns()));
    _items->setUpdatesEnabled(true);

    // Load the orders
    _orders->clear();
    const vector<Id>& orders = _curr.orders();
    for (line = 0; line < orders.size(); ++line) {
	Id order_id = orders[line];
	Order order;
	_cache.findOrder(order_id, order);

	VectorRow* row = new VectorRow(_orders->columns());
	row->setValue(0, order_id);
	row->setValue(1, order.date());
	_orders->appendRow(row);
    }
    _orders->appendRow(new VectorRow(_orders->columns()));

    // Load the charges
    _icharges->clear();
    _icharge_lines.clear();
    _echarges->clear();
    _echarge_lines.clear();
    const vector<ReceiveCharge>& charges = _curr.charges();
    for (line = 0; line < charges.size(); ++line) {
	Id charge_id = charges[line].charge_id;
	Id tax_id = charges[line].tax_id;
	fixed amount = charges[line].amount;
	fixed base = charges[line].base;
	bool internal = charges[line].internal;

	Charge charge;
	_cache.findCharge(charge_id, charge);

	if (internal) {
	    VectorRow* row = new VectorRow(_icharges->columns());
	    row->setValue(0, charge_id);
	    row->setValue(1, amount);
	    row->setValue(2, tax_id);
	    _icharges->appendRow(row);

	    ChargeLine& line = _icharge_lines[_icharges->rows() - 1];
	    line.charge = charge;
	    line.amount = amount;
	    line.base = base;
	    line.tax_id = tax_id;
	} else {
	    VectorRow* row = new VectorRow(_echarges->columns());
	    row->setValue(0, charge_id);
	    row->setValue(1, amount);
	    _echarges->appendRow(row);

	    ChargeLine& line = _echarge_lines[_echarges->rows() - 1];
	    line.charge = charge;
	    line.amount = amount;
	    line.base = base;
	    line.tax_id = INVALID_ID;
	}
    }
    _icharges->appendRow(new VectorRow(_icharges->columns()));
    _echarges->appendRow(new VectorRow(_echarges->columns()));

    _skip_recalc = true;
    updateItemInfo(0);
    recalculate();
}

void
ReceiveMaster::printItem(bool ask)
{
    if (ask) {
	QString type = _curr.isClaim() ? "claim" : "invoice";
	QString message = tr("Do you wish to print %1 #%2?").arg(type)
	    .arg(_curr.number());

	int ch = QMessageBox::information(this, tr("Question"), message,
					  QMessageBox::No, QMessageBox::Yes);
	if (ch != QMessageBox::Yes) return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    QString fileName = "receive_print.xml";
    QString filePath;
    if (!_quasar->resourceFetch("reports", fileName, filePath)) {
	QApplication::restoreOverrideCursor();

	QString message = tr("Report %1 not found").arg(fileName);
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    ReportDefn report;
    if (!report.load(filePath)) {
	QApplication::restoreOverrideCursor();

	QString message = tr("Report %1 is invalid").arg(fileName);
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    ParamMap params;
    params["receive_id"] = _curr.id().toString();
    ReportOutput output;
    report.generate(params, output);

    QApplication::restoreOverrideCursor();
    output.print(this);
}

static void
addInfo(vector<InfoPair>& info, Id id, fixed amount)
{
    if (id == INVALID_ID) return;
    for (unsigned int i = 0; i < info.size(); ++i) {
	if (info[i].first == id) {
	    info[i].second += amount;
	    return;
	}
    }
    info.push_back(InfoPair(id, amount));
}

// Set the data object from the widgets.
void
ReceiveMaster::widgetToData()
{
    _gltxFrame->getData(_curr);
    _curr.setVendorId(_vendor_id->getId());
    _curr.setVendorAddress(_vendor_addr->text());
    _curr.setShipId(_ship->getId());
    _curr.setShipAddress(_ship_addr->text());
    _curr.setTermsId(_term_id->getId());
    _curr.setLineType(Receive::LineType(_type->currentItem()));
    _curr.setShipVia(_ship_via->text());
    _curr.setInvoiceDate(_inv_date->getDate());
    _curr.setComment(_comment->text());
    _curr.setClaim(_claim->isChecked());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    vector<AccountLine>& lines = _curr.accounts();
    lines.clear();

    vector<InfoPair> asset_info;
    vector<InfoPair> expense_info;
    vector<InfoPair> tax_info;

    Company company;
    _quasar->db()->lookup(company);
    Id store_id = _curr.storeId();

    // Get sign from claim flag
    fixed sign = 1;
    if (_curr.isClaim()) sign = -1;

    // Process all the items
    vector<ReceiveItem>& items = _curr.items();
    items.clear();
    for (unsigned int i = 0; i < _lines.size(); ++i) {
	const ItemLine& line = _lines[i];
	if (line.item.id() == INVALID_ID && line.account.id() == INVALID_ID)
	    continue;

	fixed inv_cost = line.ext_base + line.int_charges + line.ext_charges;
	if (_curr.lineType() == Receive::Account) {
	    addInfo(asset_info, line.account.id(), inv_cost);
	} else {
	    addInfo(expense_info, company.depositAccount(), line.ext_deposit);
	    if (line.item.isInventoried())
		addInfo(asset_info, line.item.assetAccount(), inv_cost);
	    else
		addInfo(expense_info, line.item.expenseAccount(), inv_cost);
	}

	ReceiveItem item;
	item.item_id = line.item.id();
	item.account_id = line.account.id();
	item.number = line.number;
	item.description = line.description;
	item.size = line.size;
	item.size_qty = line.size_qty;
	item.quantity = line.qty * sign;
	item.ordered = line.ordered;
	item.received = line.received;
	item.inv_cost = inv_cost * sign;
	item.ext_deposit = line.ext_deposit;
	item.cost_id = line.cost_id;
	item.cost_disc = line.cost_disc;
	item.cost = line.cost;
	item.ext_cost = line.ext_cost;
	item.ext_base = line.ext_base;
	item.ext_tax = line.ext_tax;
	item.int_charges = line.int_charges;
	item.ext_charges = line.ext_charges;
	item.tax_id = line.tax_id;
	item.item_tax_id = line.item_tax_id;
	item.include_tax = line.include_tax;
	item.include_deposit = line.include_deposit;
	item.open_dept = line.open_dept;
	items.push_back(item);
    }

    // Process all the taxes
    vector<TaxLine>& taxes = _curr.taxes();
    taxes.clear();
    for (unsigned int i = 0; i < _tax_info.size(); ++i) {
	Id tax_id = _tax_info[i].tax_id;
	fixed taxable = _tax_info[i].taxable;
	fixed amount = _tax_info[i].amount;
	fixed inc_taxable = _tax_info[i].inc_taxable;
	fixed inc_amount = _tax_info[i].inc_amount;

	Tax tax;
	_cache.findTax(tax_id, tax);

	TaxLine line(tax_id, taxable, amount, inc_taxable, inc_amount);
	taxes.push_back(line);
	addInfo(tax_info, tax.paidAccount(), amount + inc_amount);
    }

    // Process internal charges
    vector<ReceiveCharge>& charges = _curr.charges();
    charges.clear();
    for (unsigned int i = 0; i < _icharge_lines.size(); ++i) {
	const Charge& charge = _icharge_lines[i].charge;
	if (charge.id() == INVALID_ID) continue;
	Id tax_id = _icharge_lines[i].tax_id;
	fixed amount = _icharge_lines[i].amount;
	fixed base = _icharge_lines[i].base;

	ReceiveCharge line(charge.id(), tax_id, amount, base, true);
	charges.push_back(line);

	if (charge.allocateMethod() == Charge::NONE)
	    addInfo(expense_info, charge.accountId(), base);
    }

    // Process external charges
    for (unsigned int i = 0; i < _echarge_lines.size(); ++i) {
	const Charge& charge = _echarge_lines[i].charge;
	if (charge.id() == INVALID_ID) continue;
	fixed amount = _echarge_lines[i].amount;
	fixed base = _echarge_lines[i].base;

	ReceiveCharge line(charge.id(), INVALID_ID, amount, base, false);
	charges.push_back(line);

	if (charge.allocateMethod() != Charge::NONE)
	    addInfo(expense_info, charge.accountId(), -base);
    }

    // Post to AP
    if (_curr.vendorId() != INVALID_ID) {
	AccountLine line(_vendor.accountId(), -_curr.total());
	lines.push_back(line);
    }

    // Post the taxes
    for (unsigned int i = 0; i < tax_info.size(); ++i) {
	Id account_id = tax_info[i].first;
	fixed amount = tax_info[i].second * sign;
	if (amount != 0.0)
	    lines.push_back(AccountLine(account_id, amount));
    }

    // Post the asset adjustments
    for (unsigned int i = 0; i < asset_info.size(); ++i) {
	Id account_id = asset_info[i].first;
	fixed amount = asset_info[i].second * sign;
	if (amount != 0.0)
	    lines.push_back(AccountLine(account_id, amount));
    }

    // Post the expenses
    for (unsigned int i = 0; i < expense_info.size(); ++i) {
	Id account_id = expense_info[i].first;
	fixed amount = expense_info[i].second * sign;
	if (amount != 0.0)
	    lines.push_back(AccountLine(account_id, amount));
    }

    // Add card line
    if (_curr.vendorId() != INVALID_ID) {
	_curr.cards().clear();

	CardLine line(_curr.vendorId(), _curr.total());
	_curr.cards().push_back(line);
    }

    // Add the receive orders
    vector<Id>& orders = _curr.orders();
    orders.clear();
    for (int row = 0; row < _orders->rows(); ++row) {
	Id order_id = _orders->cellValue(row, 0).toId();
	if (order_id == INVALID_ID) continue;
	orders.push_back(order_id);
    }
}

void
ReceiveMaster::slotClaimChanged()
{
    if (_claim->isChecked()) {
	setCaption(tr("Vendor Claim"));
	_gltxFrame->numberLabel->setText(tr("Claim No.:"));
	_payment->setText(tr("Allocations"));
	_tabs->setTabEnabled(_ordersTab, false);
    } else {
	setCaption(tr("Vendor Invoice"));
	_gltxFrame->numberLabel->setText(tr("Invoice No.:"));
	_payment->setText(tr("Payments"));
	_tabs->setTabEnabled(_ordersTab, true);
    }
    // TODO: clear items, order, ...
}

void
ReceiveMaster::slotDateChanged()
{
    _inv_date->setDate(_gltxFrame->date->getDate());
}

void
ReceiveMaster::slotVendorChanged()
{
    Vendor vendor;
    if (_quasar->db()->lookup(_vendor_id->getId(), vendor)) {
	_vendor_addr->setText(vendor.address());
	_term_id->setId(vendor.termsId());
    }
    _vendor = vendor;
    _orderLookup->vendor->setId(vendor.id());

    // TODO: remove any orders/slips

    // Recalculate costs
    if (_items->rows() > 0) {
	for (int row = 0; row < _items->rows(); ++row)
	    recalculateCost(row);
	_skip_recalc = true;
	recalculate();
	if (_items->currentRow() != -1)
	    updateItemInfo(_items->currentRow());
    }
}

void
ReceiveMaster::slotStoreChanged()
{
    if (!clearItems(false)) return;
    _lookup->store_id = _gltxFrame->store->getId();

    // TODO: remove any orders/slips
    slotShipToChanged();
}

void
ReceiveMaster::slotShipToChanged()
{
    Id card_id = _ship->getId();
    if (card_id == INVALID_ID) {
	Store store;
	_cache.findStore(_gltxFrame->store->getId(), store);
	if (store.hasAddress()) {
	    _ship_addr->setText(store.address());
	} else {
	    Company company;
	    _quasar->db()->lookup(company);
	    _ship_addr->setText(company.address());
	}
    } else {
	Card* card = _cache.findCard(_ship->getId());
	if (card != NULL)
	    _ship_addr->setText(card->address());
    }
}

bool
ReceiveMaster::clearItems(bool ask)
{
    int item_cnt = 0;
    for (unsigned int i = 0; i < _lines.size(); ++i) {
	const ItemLine& line = _lines[i];
	if (line.item.id() == INVALID_ID && line.account.id() == INVALID_ID)
	    continue;
	++item_cnt;
    }

    if (item_cnt > 0 && ask) {
	QString message = tr("All the current items must be deleted\n"
	    "in order to perform this change.  Are\n"
	    "you sure you want to do this?");
	int choice = QMessageBox::warning(this, tr("Delete Items"), message,
					  QMessageBox::No, QMessageBox::Yes);
	if (choice != QMessageBox::Yes) return false;
    }

    _items->clear();
    _lines.clear();
    _item_row = -1;

    _items->appendRow(new VectorRow(_items->columns()));

    return true;
}

void
ReceiveMaster::slotTypeChanged()
{
    if (!clearItems()) return;
    if (_type->currentItem() == _old_type) return;
    _old_type = _type->currentItem();

    LookupColumn* column = (LookupColumn*)_items->column(0);
    delete column->editor;

    if (_items->columnWidth(2) != 0)
	_size_width = _items->columnWidth(2);

    if (_type->currentItem() == Receive::Account) {
	AccountLookup* acctLookup = new AccountLookup(_main, this);
	_items->setColumnName(0, tr("Account"));
	_items->column(2)->maxWidth = 0;
	_items->setColumnWidth(1, _items->columnWidth(1) + _size_width);
	_items->setColumnWidth(2, 0);
	column->lookup = acctLookup;
	new LookupEditor(_items, 0, new LookupEdit(acctLookup, _items));
    } else {
	_lookup = new ItemLookup(_main, this);
	_lookup->purchasedOnly = true;
	_lookup->store_id = _gltxFrame->store->getId();
	_items->setColumnName(0, tr("Item Number"));
	_items->column(2)->maxWidth = 9999;
	_items->setColumnWidth(1, _items->columnWidth(1) - _size_width);
	_items->setColumnWidth(2, _size_width);
	column->lookup = _lookup;
	new LookupEditor(_items, 0, new ItemEdit(_lookup, _items));
    }
}

void
ReceiveMaster::slotPayment()
{
    if (!saveItem(true)) return;

    ReceivePayment* dialog = new ReceivePayment(this, _main, _curr.id());
    dialog->exec();

    oldItem();
    dataToWidget();
}

void
ReceiveMaster::slotSalesHistory()
{
    if (!_hist.isNull()) return;

    _hist = new SalesHistory(_main);
    if (_item_row != -1) {
	_hist->setStoreId(_gltxFrame->store->getId());
	_hist->setItem(_lines[_item_row].item.id(), _lines[_item_row].number);
    }
    _hist->show();
}

void
ReceiveMaster::slotIncTaxChanged()
{
    if (_item_row == -1) return;
    ItemLine& line = _lines[_item_row];
    if (line.include_tax == _inc_tax->isChecked()) return;

    line.include_tax = _inc_tax->isChecked();

    recalculate();
    updateItemInfo(_item_row);
}

void
ReceiveMaster::slotIncDepChanged()
{
    if (_item_row == -1) return;
    ItemLine& line = _lines[_item_row];
    if (line.include_deposit == _inc_dep->isChecked()) return;

    line.include_deposit = _inc_dep->isChecked();

    recalculate();
    updateItemInfo(_item_row);
}

void
ReceiveMaster::slotDepositChanged()
{
    if (_item_row == -1) return;
    ItemLine& line = _lines[_item_row];

    line.ext_deposit = _deposit->getFixed();

    recalculate();
    updateItemInfo(_item_row);
}

void
ReceiveMaster::updateItemInfo(int row)
{
    _item_row = row;

    // Set to defaults
    if (row == -1) {
	_inc_tax->setChecked(false);
	_inc_dep->setChecked(false);
	_deposit->setText("");
	_ordered->setText("");
	_line_cost->setText("");
	_line_icharge->setText("");
	_line_echarge->setText("");
	return;
    }

    const ItemLine& line = _lines[row];

    _inc_tax->setChecked(line.include_tax);
    _inc_dep->setChecked(line.include_deposit);
    _deposit->setFixed(line.ext_deposit);
    _ordered->setFixed(line.ordered);
    _line_cost->setFixed(line.ext_base);
    _line_icharge->setFixed(line.int_charges);
    _line_echarge->setFixed(line.ext_charges);

    if (!_hist.isNull()) {
	_hist->setStoreId(_gltxFrame->store->getId());
	_hist->setItem(_lines[_item_row].item.id(), _lines[_item_row].number);
    }
}

void
ReceiveMaster::itemCellMoved(int row, int)
{
    if (row == _items->currentRow() && _size->count() > 0)
	return;

    updateItemInfo(row);
    _size->clear();

    if (row == -1) return;

    const ItemLine& line = _lines[row];
    for (unsigned int i = 0; i < line.item.sizes().size(); ++i) {
	_size->insertItem(line.item.sizes()[i].name);
	if (line.item.sizes()[i].name == line.size)
	    _size->setCurrentItem(_size->count() - 1);
    }
}

void
ReceiveMaster::itemCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _items->rows() - 1 && col == 0) {
	Id id = _items->cellValue(row, col).toId();
	if (id != INVALID_ID)
	    _items->appendRow(new VectorRow(_items->columns()));
    }

    // Check for other changes
    int lineType = _type->currentItem();
    ItemLine& line = _lines[row];
    Id id;
    fixed ext_cost;
    switch (col) {
    case 0: // item_id or account_id
	id = _items->cellValue(row, 0).toId();
	if (lineType == Receive::Account) {
	    if (_cache.findAccount(id, line.account)) {
		if (line.qty == 0.0) line.qty = 1.0;
		line.number = "";
		line.description = "";
		line.size = "";
		line.size_qty = 1.0;
		line.ordered = 0.0;
		line.received = line.qty;
		line.cost_id = INVALID_ID;
		line.cost_disc = 0.0;
		line.cost.setNull();
		line.ext_cost = 0.0;
		line.ext_deposit = 0.0;
		line.tax_id = INVALID_ID;
		line.item_tax_id = INVALID_ID;
		line.include_tax = false;
		line.include_deposit = false;
		line.open_dept = false;

		_items->setCellValue(row, 1, "");
		_items->setCellValue(row, 2, line.size);
		_items->setCellValue(row, 3, line.qty);
		_items->setCellValue(row, 4, "");
		_items->setCellValue(row, 5, "");
		_items->setCellValue(row, 6, INVALID_ID);
	    } else {
		if (row != _items->rows() - 1) {
		    line.description = tr("**Unknown Account**");
		    _items->setCellValue(row, 1, tr("**Unknown Account**"));
		}
	    }
	} else {
	    if (_cache.findItem(id, line.item)) {
		QString number = _items->cellValue(row, 0).toPlu().number();
		QString size = line.item.numberSize(number);
		if (size.isEmpty()) size = line.item.purchaseSize();

		_size->clear();
		for (unsigned int i = 0; i < line.item.sizes().size(); ++i) {
		    _size->insertItem(line.item.sizes()[i].name);
		    if (line.item.sizes()[i].name == size)
			_size->setCurrentItem(_size->count() - 1);
		}

		line.number = number;
		line.description = "";
		line.size = size;
		line.size_qty = line.item.sizeQty(size);
		line.qty = 1.0;
		line.ordered = 0.0;
		line.received = 1.0;
		line.tax_id = line.item.purchaseTax();
		line.item_tax_id = line.item.purchaseTax();
		line.include_tax = line.item.costIncludesTax();
		line.include_deposit = line.item.costIncludesDeposit();
		line.open_dept = line.item.isOpenDept();

		_items->setCellValue(row, 1, line.item.description());
		_items->setCellValue(row, 2, line.size);
		_items->setCellValue(row, 3, line.qty);
		_items->setCellValue(row, 5, "");
		_items->setCellValue(row, 6, line.tax_id);
		recalculateCost(row);
	    } else {
		if (row != _items->rows() - 1) {
		    line.description = tr("**Unknown Item**");
		    _items->setCellValue(row, 1, tr("**Unknown Item**"));
		}
	    }
	}
	break;
    case 1: // description
	line.description = _items->cellValue(row, 1).toString();
	break;
    case 2: // size
	line.size = _items->cellValue(row, 2).toString();
	line.size_qty = line.item.sizeQty(line.size);
	recalculateCost(row);
	break;
    case 3: // qty
	line.qty = _items->cellValue(row, 3).toFixed();
	recalculateCost(row);
	break;
    case 4: // cost
	line.cost = _items->cellValue(row, 4).toPrice();
	ext_cost = line.cost.calculate(line.qty);
	if (line.cost_id != INVALID_ID)
	    line.cost_disc = line.ext_cost - ext_cost;
	line.cost_id = INVALID_ID;
	line.ext_cost = ext_cost;
	_items->setCellValue(row, 5, line.ext_cost);
	break;
    case 5: // ext_cost
	ext_cost = _items->cellValue(row, 5).toFixed();
	if (line.cost_id != INVALID_ID)
	    line.cost_disc = line.ext_cost - ext_cost;
	line.ext_cost = ext_cost;
	line.cost_id = INVALID_ID;
	if (line.qty == 0.0)
	    line.cost.setPrice(0.0);
	else
	    line.cost.setPrice(line.ext_cost / line.qty);
	_items->setCellValue(row, 4, line.cost);
	break;
    case 6: // tax_id
	line.tax_id = _items->cellValue(row, 6).toId();
	break;
    }

    recalculate();
    updateItemInfo(row);
}

void
ReceiveMaster::itemFocusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _items->currentRow();
    int col = _items->currentColumn();
    bool isAccount = _type->currentItem() == Receive::Account;

    if (type == Table::MoveNext && col == 0) {
	Id id = _items->cellValue(row, col).toId();
	if (id == INVALID_ID && row == _items->rows() - 1) {
	    leave = true;
	} else {
	    newCol = isAccount ? 1 : 3;
	}
    } else if (type == Table::MoveNext && col == 1 && isAccount) {
	newCol = 3;
    } else if (type == Table::MovePrev && col == 3 && isAccount) {
	newCol = 1;
    } else if (type == Table::MoveNext && col == 5 && !isAccount) {
	newCol = 0;
	newRow = row + 1;
    } else if (type == Table::MovePrev && col == 0) {
	if (row > 0) {
	    newCol = isAccount ? 6 : 5;
	    newRow = row - 1;
	}
    }
}

void
ReceiveMaster::itemRowInserted(int row)
{
    ItemLine line;
    _lines.insert(_lines.begin() + row, line);
}

void
ReceiveMaster::itemRowDeleted(int row)
{
    _lines.erase(_lines.begin() + row);
    recalculate();
}

void
ReceiveMaster::orderCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _orders->rows() - 1 && col == 0) {
	Id id = _orders->cellValue(row, col).toId();
	if (id != INVALID_ID)
	    _orders->appendRow(new VectorRow(_orders->columns()));
    }

    // Check for other changes
    Id order_id;
    Order order;
    switch (col) {
    case 0: // order_id
	order_id = _orders->cellValue(row, col).toId();
	if (_cache.findOrder(order_id, order)) {
	    _orders->setCellValue(row, 1, DateValcon().format(order.date()));
	    importOrder(order_id);
	} else {
	    _orders->setCellValue(row, 1, "");
	}
	break;
    }
}

void
ReceiveMaster::orderFocusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _orders->currentRow();
    int col = _orders->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id id = _orders->cellValue(row, col).toId();
	if (id == INVALID_ID && row == _orders->rows() - 1) {
	    leave = true;
	} else {
	    newRow = row + 1;
	    newCol = 0;
	}
    } else if (type == Table::MovePrev && col == 0) {
	if (row == 0) {
	    leave = true;
	} else {
	    newRow = row - 1;
	    newCol = 0;
	}
    }
}

void
ReceiveMaster::ichargeCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _icharges->rows() - 1 && col == 0) {
	Id id = _icharges->cellValue(row, col).toId();
	if (id != INVALID_ID)
	    _icharges->appendRow(new VectorRow(_icharges->columns()));
    }

    // Check for other changes
    ChargeLine& line = _icharge_lines[row];
    Id charge_id;
    switch (col) {
    case 0: // charge_id
	charge_id = _icharges->cellValue(row, 0).toId();
	if (_cache.findCharge(charge_id, line.charge)) {
	    line.tax_id = line.charge.taxId();
	    line.amount = 0.0;
	    line.base = 0.0;

	    _icharges->setCellValue(row, 1, "");
	    _icharges->setCellValue(row, 2, line.tax_id);
	}
	break;
    case 1: // amount
	line.amount = _icharges->cellValue(row, 1).toFixed();
	_skip_recalc = true;
	break;
    case 2: // tax_id
	line.tax_id = _icharges->cellValue(row, 2).toId();
	break;
    }

    recalculate();
}

void
ReceiveMaster::ichargeFocusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _icharges->currentRow();
    int col = _icharges->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id id = _icharges->cellValue(row, col).toId();
	if (id == INVALID_ID && row == _icharges->rows() - 1) {
	    leave = true;
	}
    } else if (type == Table::MoveNext && col == 1) {
	newCol = 0;
	newRow = row + 1;
    } else if (type == Table::MovePrev && col == 0) {
	if (row > 0) {
	    newCol = 1;
	    newRow = row - 1;
	}
    }
}

void
ReceiveMaster::ichargeRowInserted(int row)
{
    ChargeLine line;
    _icharge_lines.insert(_icharge_lines.begin() + row, line);
}

void
ReceiveMaster::ichargeRowDeleted(int row)
{
    _icharge_lines.erase(_icharge_lines.begin() + row);
    recalculate();
}

void
ReceiveMaster::echargeCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _echarges->rows() - 1 && col == 0) {
	Id id = _echarges->cellValue(row, col).toId();
	if (id != INVALID_ID)
	    _echarges->appendRow(new VectorRow(_echarges->columns()));
    }

    // Check for other changes
    ChargeLine& line = _echarge_lines[row];
    Id charge_id;
    switch (col) {
    case 0: // charge_id
	charge_id = _echarges->cellValue(row, 0).toId();
	if (_cache.findCharge(charge_id, line.charge)) {
	    line.tax_id = INVALID_ID;
	    line.amount = 0.0;
	    line.base = 0.0;

	    _echarges->setCellValue(row, 1, "");
	}
	break;
    case 1: // amount
	line.amount = _echarges->cellValue(row, 1).toFixed();
	_skip_recalc = true;
	break;
    }

    recalculate();
}

void
ReceiveMaster::echargeFocusNext(bool& leave, int&, int&, int type)
{
    int row = _echarges->currentRow();
    int col = _echarges->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id id = _echarges->cellValue(row, col).toId();
	if (id == INVALID_ID && row == _echarges->rows() - 1) {
	    leave = true;
	}
    }
}

void
ReceiveMaster::echargeRowInserted(int row)
{
    ChargeLine line;
    _echarge_lines.insert(_echarge_lines.begin() + row, line);
}

void
ReceiveMaster::echargeRowDeleted(int row)
{
    _echarge_lines.erase(_echarge_lines.begin() + row);
    recalculate();
}

void
ReceiveMaster::recalculate()
{
    static bool in_recalculate = false;
    if (in_recalculate) return;
    in_recalculate = true;

    // No vendor tax exempt yet but the code is here
    Tax exempt;

    // Process items
    fixed item_total = 0.0;
    fixed deposit_total = 0.0;
    fixed total_weight = 0.0;
    fixed item_count = 0.0;
    _tax_info.clear();
    for (unsigned int i = 0; i < _lines.size(); ++i) {
	ItemLine& line = _lines[i];
	if (line.item.id() == INVALID_ID && line.account.id() == INVALID_ID)
	    continue;

	// Calculate base price
	line.ext_base = line.ext_cost;
	if (line.include_deposit)
	    line.ext_base -= line.ext_deposit;

	// Calculate taxes
	fixed ext_tax = 0.0;
	if (line.tax_id != INVALID_ID) {
	    fixed base = line.ext_base;
	    Tax tax;
	    _cache.findTax(line.tax_id, tax);
	    vector<Id> tax_ids;
	    vector<fixed> tax_amts;

	    if (line.include_tax) {
		ext_tax = _db->calculateTaxOff(_cache, tax, base, tax_ids,
					       tax_amts);
		addTaxInc(base, tax_ids, tax_amts);
		line.ext_base -= ext_tax;
	    } else {
		ext_tax = _db->calculateTaxOn(_cache, tax, base, exempt,
					      tax_ids, tax_amts);
		addTax(base, tax_ids, tax_amts);
	    }
	}
	line.ext_tax = ext_tax;

	// Clear out old charges
	_lines[i].int_charges = 0.0;
	_lines[i].ext_charges = 0.0;

	// Add up totals
	item_total += line.ext_base;
	deposit_total += line.ext_deposit;
	total_weight += line.item.weight(line.size) * line.qty;
	item_count += line.qty;
    }

    // Process internal charges
    fixed icharge_total = 0.0;
    for (unsigned int i = 0; i < _icharge_lines.size(); ++i) {
	ChargeLine& cline = _icharge_lines[i];
	const Charge& charge = cline.charge;
	if (charge.id() == INVALID_ID) continue;

	// Check if should calculate
	fixed amount = cline.amount;
	if (charge.calculateMethod() != Charge::MANUAL && !_skip_recalc) {
	    switch (charge.calculateMethod()) {
	    case Charge::COST:
		amount = item_total * charge.amount() / 100.0;
		break;
	    case Charge::WEIGHT:
		amount = total_weight * charge.amount();
		break;
	    default:
		amount = 0.0;
		break;
	    }
	    amount.moneyRound();
	}

	// Calculate tax
	fixed base = amount;
	if (cline.tax_id != INVALID_ID) {
	    Tax tax;
	    _cache.findTax(cline.tax_id, tax);
	    vector<Id> tax_ids;
	    vector<fixed> tax_amts;

	    if (charge.includeTax()) {
		base -= _db->calculateTaxOff(_cache, tax, amount, tax_ids,
					     tax_amts);
		addTaxInc(amount, tax_ids, tax_amts);
	    } else {
		_db->calculateTaxOn(_cache, tax, amount, exempt, tax_ids,
				    tax_amts);
		addTax(amount, tax_ids, tax_amts);
	    }
	}

	cline.amount = amount;
	cline.base = base;

	icharge_total += cline.base;
	_icharges->setCellValue(i, 1, cline.amount);

	// Check if should allocate
	if (charge.allocateMethod() != Charge::NONE) {
	    int last = -1;
	    fixed remain = cline.base;
	    for (unsigned int j = 0; j < _lines.size(); ++j) {
		ItemLine& line = _lines[j];
		if (line.item.id() == INVALID_ID && line.account.id() == INVALID_ID)
		    continue;

		fixed weight = line.item.weight(line.size) * line.qty;
		fixed ratio = 0.0;
		switch (charge.allocateMethod()) {
		case Charge::COST:
		    ratio = line.ext_base / item_total;
		    break;
		case Charge::WEIGHT:
		    ratio = weight / total_weight;
		    break;
		default:
		    break;
		}

		fixed alloc = cline.base * ratio;
		alloc.moneyRound();
		line.int_charges += alloc;
		remain -= alloc;
		last = j;
	    }
	    _lines[last].int_charges += remain;
	}

    }

    // Process external charges
    fixed echarge_total = 0.0;
    for (unsigned int i = 0; i < _echarge_lines.size(); ++i) {
	ChargeLine& cline = _echarge_lines[i];
	const Charge& charge = cline.charge;
	if (charge.id() == INVALID_ID) continue;

	// Check if should calculate
	fixed amount = cline.amount;
	if (charge.calculateMethod() != Charge::MANUAL && !_skip_recalc) {
	    switch (charge.calculateMethod()) {
	    case Charge::COST:
		amount = item_total * charge.amount() / 100.0;
		break;
	    case Charge::WEIGHT:
		amount = total_weight * charge.amount();
		break;
	    default:
		amount = 0.0;
		break;
	    }
	    amount.moneyRound();
	}

	// Calculate tax
	fixed base = amount;
	if (cline.tax_id != INVALID_ID) {
	    Tax tax;
	    _cache.findTax(cline.tax_id, tax);
	    vector<Id> tax_ids;
	    vector<fixed> tax_amts;

	    if (charge.includeTax()) {
		base -= _db->calculateTaxOff(_cache, tax, amount, tax_ids,
					     tax_amts);
	    }
	}

	cline.amount = amount;
	cline.base = base;

	echarge_total += cline.base;
	_echarges->setCellValue(i, 1, cline.amount);

	// Check if should allocate
	if (charge.allocateMethod() != Charge::NONE) {
	    int last = -1;
	    fixed remain = cline.base;
	    for (unsigned int j = 0; j < _lines.size(); ++j) {
		ItemLine& line = _lines[j];
		if (line.item.id() == INVALID_ID && line.account.id() == INVALID_ID)
		    continue;

		fixed weight = line.item.weight(line.size) * line.qty;
		fixed ratio = 0.0;
		switch (charge.allocateMethod()) {
		case Charge::COST:
		    ratio = line.ext_base / item_total;
		    break;
		case Charge::WEIGHT:
		    ratio = weight / total_weight;
		    break;
		default:
		    break;
		}

		fixed alloc = cline.base * ratio;
		alloc.moneyRound();
		line.ext_charges += alloc;
		remain -= alloc;
		last = j;
	    }
	    _lines[last].ext_charges += remain;
	}
    }

    // Round taxes to two decimal places and total up
    _taxes->clear();
    fixed tax_total = 0.0;
    for (unsigned int i = 0; i < _tax_info.size(); ++i) {
	Id tax_id = _tax_info[i].tax_id;
	_tax_info[i].amount.moneyRound();

	tax_total += _tax_info[i].amount + _tax_info[i].inc_amount;

	Tax tax;
	_cache.findTax(tax_id, tax);

	VectorRow* row = new VectorRow(_taxes->columns());
	row->setValue(0, tax.name());
	row->setValue(1, tax.description());
	row->setValue(2, _tax_info[i].taxable);
	row->setValue(3, _tax_info[i].amount);
	row->setValue(4, _tax_info[i].inc_taxable);
	row->setValue(5, _tax_info[i].inc_amount);
	_taxes->appendRow(row);
    }

    fixed paid = _curr.paymentTotal();
    fixed total = item_total + tax_total + deposit_total + icharge_total;

    _item_amt->setFixed(item_total);
    _charge_amt->setFixed(icharge_total);
    _tax_amt->setFixed(tax_total);
    _deposit_amt->setFixed(deposit_total);
    _item_qty->setFixed(item_count);
    _total_amt->setFixed(total);
    _paid_amt->setFixed(paid);
    _due_amt->setFixed(total - paid);

    _skip_recalc = false;
    updateItemInfo(_item_row);

    in_recalculate = false;
}

void
ReceiveMaster::recalculateCost(int row)
{
    ItemLine& line = _lines[row];
    if (line.account.id() == INVALID_ID && line.item.id() == INVALID_ID)
	return;

    // If the type is account, just recalculate ext_cost
    if (_type->currentItem() == Receive::Account) {
	line.ext_cost = line.cost.calculate(line.qty);
	_items->setCellValue(row, 5, line.ext_cost);
	return;
    }

    // Get existing costs used for quantity limit
    vector<Id> cost_ids;
    vector<fixed> recv_qtys;
    for (unsigned int i = 0; i < _lines.size(); ++i) {
	const ItemLine& line = _lines[i];
	if (line.cost_id == INVALID_ID) continue;
	if (int(i) == row) continue;

	bool found = false;
	for (unsigned int j = 0; j < cost_ids.size(); ++j) {
	    if (cost_ids[j] == line.cost_id) {
		found = true;
		recv_qtys[j] += line.qty;
		break;
	    }
	}
	if (!found) {
	    cost_ids.push_back(line.cost_id);
	    recv_qtys.push_back(line.qty);
	}
    }

    vector<ItemPrice> costs;
    findCosts(line.item.id(), costs);
    QDate date = _gltxFrame->date->getDate();
    Id store = _gltxFrame->store->getId();

    ItemPrice cost;
    fixed sell_qty;
    _db->itemBestPrice(line.item, costs, line.size, _vendor, store, date,
		       line.qty, true, true, cost, line.ext_cost, sell_qty,
		       cost_ids, recv_qtys);
    line.ext_deposit = line.item.deposit() * line.size_qty * sell_qty;

    if (sell_qty != line.qty) {
	QString message = tr("The quantity was changed from %1 to %2\n"
			     "to take advantage of a quantity limit cost.")
	    .arg(line.qty.toString()).arg(sell_qty.toString());
	QMessageBox::warning(this, tr("Warning"), message);
    }

    line.qty = sell_qty;
    line.cost_id = cost.id();
    line.cost_disc = 0.0;
    if (cost.method() == ItemPrice::PRICE)
	line.cost = cost.price();
    else
	line.cost = Price(line.ext_cost / sell_qty);

    _items->setCellValue(row, 3, line.qty);
    _items->setCellValue(row, 4, line.cost);
    _items->setCellValue(row, 5, line.ext_cost);

    // If size is wrong, try to find better size
    QString size = line.item.numberSize(line.number);
    if (!size.isEmpty() && line.size != size) {
	for (unsigned int i = 0; i < line.item.numbers().size(); ++i) {
	    if (line.item.numbers()[i].size != line.size) continue;

	    line.number = line.item.numbers()[i].number;
	    _items->setCellValue(row, 0, Plu(line.item.id(), line.number));
	    break;
	}
    }

    // Check if vendor has a prefered order number
    QString orderNumber;
    for (unsigned int i = 0; i < line.item.vendors().size(); ++i) {
	const ItemVendor& info = line.item.vendors()[i];
	if (info.vendor_id != _vendor.id()) continue;
	if (!info.size.isEmpty() && info.size != line.size) continue;
	orderNumber = info.number;
	if (line.number == orderNumber) break;
    }
    if (!orderNumber.isEmpty()) {
	line.number = orderNumber;
	_items->setCellValue(row, 0, Plu(line.item.id(), orderNumber));
    }
}

void
ReceiveMaster::addTax(fixed taxable, vector<Id>& ids, vector<fixed>& amts)
{
    for (unsigned int i = 0; i < ids.size(); ++i) {
	Id tax_id = ids[i];
	fixed tax_amt = amts[i];

	bool found = false;
	for (unsigned int j = 0; j < _tax_info.size(); ++j) {
	    TaxLine& line = _tax_info[j];
	    if (line.tax_id == tax_id) {
		found = true;
		break;
	    }
	}

	if (found) {
	    _tax_info[i].taxable += taxable;
	    _tax_info[i].amount += tax_amt;
	} else {
	    TaxLine line(tax_id, taxable, tax_amt);
	    _tax_info.push_back(line);
	}
    }
}

void
ReceiveMaster::addTaxInc(fixed taxable, vector<Id>& ids, vector<fixed>& amts)
{
    for (unsigned int i = 0; i < ids.size(); ++i) {
	Id tax_id = ids[i];
	fixed tax_amt = amts[i];

	bool found = false;
	for (unsigned int j = 0; j < _tax_info.size(); ++j) {
	    TaxLine& line = _tax_info[j];
	    if (line.tax_id == tax_id) {
		found = true;
		break;
	    }
	}

	if (found) {
	    _tax_info[i].inc_taxable += taxable;
	    _tax_info[i].inc_amount += tax_amt;
	} else {
	    TaxLine line(tax_id, 0.0, 0.0, taxable, tax_amt);
	    _tax_info.push_back(line);
	}
    }
}

bool
ReceiveMaster::findCosts(Id item_id, vector<ItemPrice>& costs)
{
    costs.clear();

    for (unsigned int i = 0; i < _item_cache.size(); ++i) {
	if (_item_cache[i].item_id == item_id) {
	    costs = _item_cache[i].costs;
	    return true;
	}
    }

    Item item;
    if (!_cache.findItem(item_id, item))
	return false;

    ItemCache cache;
    cache.item_id = item.id();

    ItemPriceSelect conditions;
    conditions.activeOnly = true;
    conditions.item = item;
    conditions.costOnly = true;
    if (!_db->select(cache.costs, conditions))
	return false;

    _item_cache.push_back(cache);
    costs = cache.costs;
    return true;
}

ReceiveMaster::ItemLine::ItemLine()
    : tax_id(INVALID_ID), item_tax_id(INVALID_ID), include_tax(false),
      include_deposit(false), open_dept(false)
{
}

ReceiveMaster::ChargeLine::ChargeLine()
    : tax_id(INVALID_ID)
{
}

void
ReceiveMaster::importOrder(Id order_id)
{
    Order order;
    if (!_cache.findOrder(order_id, order))
	return;

    // Ask if should import order items
    QDialog* dialog = new QDialog(this, "AskImport", true);
    QLabel* msg = new QLabel(tr("Do you want to copy the items from the\n"
			     "purchase order into this screen?"), dialog);
    QCheckBox* def = new QCheckBox(tr("Default received to remaining?"),
				   dialog);
    QPushButton* ok = new QPushButton(tr("&OK"), dialog);
    QPushButton* cancel = new QPushButton(tr("&Cancel"), dialog);
    def->setChecked(true);
    ok->setDefault(true);
    ok->setMinimumSize(_cancel->sizeHint());
    cancel->setMinimumSize(cancel->sizeHint());
    dialog->setCaption(tr("Import Order Items"));
    QGridLayout* grid = new QGridLayout(dialog);
    grid->setMargin(6);
    grid->setSpacing(10);
    grid->addMultiCellWidget(msg, 0, 0, 0, 1);
    grid->addMultiCellWidget(def, 1, 1, 0, 1);
    grid->addWidget(ok, 2, 0, AlignCenter);
    grid->addWidget(cancel, 2, 1, AlignCenter);
    connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
    def->setFocus();
    int result = dialog->exec();
    bool use_remain = def->isChecked();
    delete dialog;
    if (result != QDialog::Accepted) return;

    // Set from order
    _term_id->setId(order.termsId());
    _ship_via->setText(order.shipVia());
    _gltxFrame->store->setId(order.storeId());

    // Remove bottom blank line for now
    _items->deleteRow(_items->rows() - 1);
    _icharges->deleteRow(_icharges->rows() - 1);
    _echarges->deleteRow(_echarges->rows() - 1);

    // Bring over items
    const vector<OrderItem>& items = order.items();
    for (unsigned int i = 0; i < items.size(); ++i) {
	Id item_id = items[i].item_id;
	QString number = items[i].number;
	QString description = items[i].description;
	QString size = items[i].size;
	fixed size_qty = items[i].size_qty;
	fixed ordered = items[i].ordered;
	fixed billed = items[i].billed;
	Id cost_id = items[i].cost_id;
	fixed cost_disc = items[i].cost_disc;
	Price cost = items[i].cost;
	Id tax_id = items[i].tax_id;
	Id item_tax_id = items[i].item_tax_id;
	bool include_tax = items[i].include_tax;
	bool include_deposit = items[i].include_deposit;

	fixed remain = ordered - billed;
	if (remain <= 0.0) continue;
	if (!use_remain) remain = 0.0;

	Item item;
	Account account;
	QString item_desc = description;
	_cache.findItem(item_id, item);
	if (item_desc.isEmpty())
	    item_desc = item.description();

	fixed remain_cost = cost.calculate(remain);
	fixed remain_deposit = item.deposit() * size_qty * remain;
	fixed remain_base = remain_cost;
	if (include_deposit) remain_base -= remain_deposit;
	fixed remain_tax = 0.0;
	if (include_tax && tax_id != INVALID_ID) {
	    Tax tax;
	    _cache.findTax(tax_id, tax);
	    remain_tax = _db->calculateTaxOff(_cache, tax, remain_base);
	    remain_base -= remain_tax;
	}

	VectorRow* row = new VectorRow(_items->columns());
	row->setValue(0, Plu(item_id, number));
	row->setValue(1, item_desc);
	row->setValue(2, size);
	row->setValue(3, remain);
	row->setValue(4, cost);
	row->setValue(5, remain_cost);
	row->setValue(6, tax_id);
	_items->appendRow(row);

	ItemLine& line = _lines[_items->rows() - 1];
	line.item = item;
	line.account = account;
	line.number = number;
	line.description = description;
	line.size = size;
	line.size_qty = size_qty;
	line.qty = remain;
	line.ordered = ordered;
	line.received = remain;
	line.cost_id = cost_id;
	line.cost_disc = cost_disc;
	line.cost = cost;
	line.ext_cost = remain_cost;
	line.ext_base = remain_base;
	line.ext_deposit = remain_deposit;
	line.ext_tax = remain_tax;
	line.tax_id = tax_id;
	line.item_tax_id = item_tax_id;
	line.include_tax = include_tax;
	line.include_deposit = include_deposit;
	line.open_dept = item.isOpenDept();
    }

    // Bring over charges
    const vector<OrderCharge>& charges = order.charges();
    for (unsigned int i = 0; i < charges.size(); ++i) {
	Id charge_id = charges[i].charge_id;
	Id tax_id = charges[i].tax_id;
	fixed amount = charges[i].amount;
        bool internal = charges[i].internal;

	Charge charge;
	_cache.findCharge(charge_id, charge);

	if (internal) {
	    VectorRow* row = new VectorRow(_icharges->columns());
	    row->setValue(0, charge_id);
	    row->setValue(1, amount);
	    row->setValue(2, tax_id);
	    _icharges->appendRow(row);

	    ChargeLine& line = _icharge_lines[_icharges->rows() - 1];
	    line.charge = charge;
	    line.tax_id = tax_id;
	    line.amount = amount;
	} else {
	    VectorRow* row = new VectorRow(_echarges->columns());
	    row->setValue(0, charge_id);
	    row->setValue(1, amount);
	    _echarges->appendRow(row);

	    ChargeLine& line = _echarge_lines[_echarges->rows() - 1];
	    line.charge = charge;
	    line.tax_id = INVALID_ID;
	    line.amount = amount;
	}
    }

    _items->appendRow(new VectorRow(_items->columns()));
    _icharges->appendRow(new VectorRow(_icharges->columns()));
    _echarges->appendRow(new VectorRow(_echarges->columns()));

    _skip_recalc = true;
    recalculate();
}

void
ReceiveMaster::slotSearch()
{
    QDialog* dialog = new QDialog(this, "Search", true);
    dialog->setCaption(tr("Item Search"));

    QLabel* numberLabel = new QLabel(tr("Item Number:"), dialog);
    LineEdit* numberWidget = new LineEdit(dialog);
    QLabel* descLabel = new QLabel(tr("Description:"), dialog);
    LineEdit* descWidget = new LineEdit(dialog);
    QCheckBox* startOver = new QCheckBox(tr("Start search at top?"), dialog);

    QFrame* buttons = new QFrame(dialog);
    QPushButton* next = new QPushButton(tr("&Next"), buttons);
    QPushButton* cancel = new QPushButton(tr("&Cancel"), buttons);
    next->setDefault(true);

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setMargin(6);
    buttonGrid->setSpacing(10);
    buttonGrid->addWidget(next, 0, 1);
    buttonGrid->addWidget(cancel, 0, 2);

    QGridLayout* grid = new QGridLayout(dialog);
    grid->setMargin(6);
    grid->setSpacing(10);
    grid->addWidget(numberLabel, 0, 0);
    grid->addWidget(numberWidget, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(descLabel, 1, 0);
    grid->addWidget(descWidget, 1, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(startOver, 2, 2, 0, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(buttons, 3, 3, 0, 1);

    connect(next, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

    _items->setFocus();
    startOver->setChecked(true);

    while (true) {
	if (dialog->exec() != QDialog::Accepted)
	    break;

	QString number = numberWidget->text();
	QString desc = descWidget->text();
	if (number.isEmpty() && desc.isEmpty()) {
	    QString message = tr("No search criteria given");
	    QMessageBox::critical(this, tr("Error"), message);
	    continue;
	}

	int startRow = -1;
	if (!startOver->isChecked())
	    startRow = _items->currentRow();

	int foundRow = -1;
	for (int row = startRow + 1; row < _items->rows(); ++row) {
	    QString rowNumber = _items->cellValue(row, 0).toPlu().number();
	    QString rowDesc = _items->cellValue(row, 1).toString();

	    if (!number.isEmpty() && rowNumber.contains(number, false)) {
		foundRow = row;
		break;
	    }
	    if (!desc.isEmpty() && rowDesc.contains(desc, false)) {
		foundRow = row;
		break;
	    }
	}

	if (foundRow == -1 && startOver->isChecked()) {
	    QString message = tr("No matches found");
	    QMessageBox::critical(this, tr("Error"), message);
	} else if (foundRow == -1 && !startOver->isChecked()) {
	    QString message = tr("No further matches found");
	    int choice = QMessageBox::critical(this, tr("Error"), message,
					       tr("Start Over"), tr("Cancel"));
	    if (choice != 0) break;
	    _items->setCurrentCell(0, 0);
	    startOver->setChecked(true);
	} else {
	    _items->setCurrentCell(foundRow, 0);
	    startOver->setChecked(false);
	}
    }

    delete dialog;
}

void
ReceiveMaster::slotRecurring()
{
    if (!saveItem(true)) return;

    RecurringMaster* screen = new RecurringMaster(_main);
    screen->setGltx(_curr.id());
    screen->show();
}
