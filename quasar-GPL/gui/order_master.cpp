// $Id: order_master.cpp,v 1.68 2005/05/13 22:26:22 bpepers Exp $
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

#include "order_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "object_cache.h"
#include "order_select.h"
#include "order_template.h"
#include "order_template_lookup.h"
#include "item_price_select.h"
#include "company.h"
#include "table.h"
#include "date_popup.h"
#include "double_edit.h"
#include "money_edit.h"
#include "percent_edit.h"
#include "price_edit.h"
#include "multi_line_edit.h"
#include "item_edit.h"
#include "vendor_lookup.h"
#include "customer_lookup.h"
#include "item_lookup.h"
#include "tax_lookup.h"
#include "charge_lookup.h"
#include "term_lookup.h"
#include "store_lookup.h"
#include "store.h"
#include "report_defn.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qmessagebox.h>

OrderMaster::OrderMaster(MainWindow* main, Id order_id)
    : DataWindow(main, "OrderMaster", order_id), _item_row(-1),
      _skip_recalc(false)
{
    _helpSource = "order_master.html";

    // Search button
    QPushButton* search = new QPushButton(tr("Search"), _buttons);
    connect(search, SIGNAL(clicked()), SLOT(slotSearch()));

    // Print button
    _print = new QPushButton(tr("Print"), _buttons);
    connect(_print, SIGNAL(clicked()), SLOT(slotPrint()));

    // Sales history button
    QPushButton* hist = new QPushButton(tr("History"), _buttons);
    connect(hist, SIGNAL(clicked()), SLOT(slotSalesHistory()));

    // Template button
    QPushButton* temp = new QPushButton(tr("Template"), _buttons);
    connect(temp, SIGNAL(clicked()), SLOT(slotTemplate()));

    QFrame* top = new QFrame(_frame);
    top->setFrameStyle(QFrame::Raised | QFrame::Panel);

    QLabel* vendorLabel = new QLabel(tr("Vendor:"), top);
    _vend_id = new LookupEdit(new VendorLookup(main, this), top);
    _vend_id->setLength(30);
    vendorLabel->setBuddy(_vend_id);
    connect(_vend_id, SIGNAL(validData()), SLOT(slotVendorChanged()));

    QLabel* vendorAddrLabel = new QLabel(tr("Address:"), top);
    _vendor_addr = new MultiLineEdit(top);
    _vendor_addr->setFocusPolicy(ClickFocus);
    _vendor_addr->setFixedVisibleLines(4);
    vendorAddrLabel->setBuddy(_vendor_addr);

    QLabel* numberLabel = new QLabel(tr("Order No.:"), top);
    _number = new LineEdit(top);
    _number->setFocusPolicy(ClickFocus);
    _number->setLength(14, '9');
    numberLabel->setBuddy(_number);

    QLabel* storeLabel = new QLabel(tr("Store:"), top);
    _store = new LookupEdit(new StoreLookup(_main, this), top);
    _store->setFocusPolicy(ClickFocus);
    _store->setLength(30);
    storeLabel->setBuddy(_store);
    connect(_store, SIGNAL(validData()), SLOT(slotStoreChanged()));

    QLabel* dateLabel = new QLabel(tr("Date:"), top);
    _date = new DatePopup(top);
    dateLabel->setBuddy(_date);

    QLabel* shipViaLabel = new QLabel(tr("Ship Via:"), top);
    _ship_via = new LineEdit(top);
    _ship_via->setLength(20);
    shipViaLabel->setBuddy(_ship_via);

    QLabel* termLabel = new QLabel(tr("Terms:"), top);
    _term_id = new LookupEdit(new TermLookup(main, this), top);
    _term_id->setLength(30);
    termLabel->setBuddy(_term_id);

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(3);
    topGrid->setMargin(3);
    topGrid->setColStretch(2, 1);
    topGrid->setRowStretch(2, 1);
    topGrid->addWidget(vendorLabel, 0, 0, AlignLeft | AlignVCenter);
    topGrid->addWidget(_vend_id, 0, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(vendorAddrLabel, 1, 0, AlignLeft | AlignTop);
    topGrid->addMultiCellWidget(_vendor_addr, 1, 3, 1, 1);
    topGrid->addWidget(numberLabel, 4, 0, AlignLeft | AlignVCenter);
    topGrid->addWidget(_number, 4, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(storeLabel, 5, 0, AlignLeft | AlignVCenter);
    topGrid->addWidget(_store, 5, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(dateLabel, 1, 2, AlignRight | AlignVCenter);
    topGrid->addWidget(_date, 1, 3, AlignLeft | AlignVCenter);
    topGrid->addWidget(shipViaLabel, 2, 2, AlignRight | AlignVCenter);
    topGrid->addWidget(_ship_via, 2, 3, AlignLeft | AlignVCenter);
    topGrid->addWidget(termLabel, 3, 2, AlignRight | AlignVCenter);
    topGrid->addWidget(_term_id, 3, 3, AlignLeft | AlignVCenter);

    QFrame* mid = new QFrame(_frame);
    mid->setFrameStyle(QFrame::Raised | QFrame::Panel);

    _items = new Table(mid);
    _items->setVScrollBarMode(QScrollView::AlwaysOn);
    _items->setLeftMargin(fontMetrics().width("99999"));
    _items->setDisplayRows(5);
    connect(_items, SIGNAL(cellMoved(int,int)), SLOT(itemCellMoved(int,int)));
    connect(_items, SIGNAL(cellValidate(int,int,Variant,bool&)),
	    SLOT(itemCellValidate(int,int,Variant,bool&)));
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
    new LookupColumn(_items, tr("Item Number"), 18, _lookup);
    new TextColumn(_items, tr("Description"), 20);
    new TextColumn(_items, tr("Size"), 10);
    new NumberColumn(_items, tr("Quantity"), 6);
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
    new NumberEditor(_items, 5,extCostEdit);
    new LookupEditor(_items, 6, new LookupEdit(taxLookup, _items));

    QGridLayout* midGrid = new QGridLayout(mid);
    midGrid->setSpacing(3);
    midGrid->setMargin(3);
    midGrid->setColStretch(0, 1);
    midGrid->addWidget(_items, 0, 0);

    QTabWidget* tabs = new QTabWidget(_frame);
    QFrame* totals = new QFrame(tabs);
    QFrame* comments = new QFrame(tabs);
    QFrame* taxes = new QFrame(tabs);
    QFrame* charges = new QFrame(tabs);
    QFrame* ship = new QFrame(tabs);
    QFrame* itemInfo = new QFrame(tabs);
    tabs->addTab(totals, tr("Totals"));
    tabs->addTab(comments, tr("Comments"));
    tabs->addTab(taxes, tr("Taxes"));
    tabs->addTab(charges, tr("Charges"));
    tabs->addTab(ship, tr("Ship To"));
    tabs->addTab(itemInfo, tr("Item Info"));

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

    QLabel* totalLabel = new QLabel(tr("Total:"), totals);
    _total_amt = new MoneyEdit(totals);
    _total_amt->setFocusPolicy(NoFocus);

    QGridLayout* totalGrid = new QGridLayout(totals);
    totalGrid->setSpacing(3);
    totalGrid->setMargin(3);
    totalGrid->setColStretch(2, 1);
    totalGrid->setColStretch(5, 1);
    totalGrid->setRowStretch(3, 1);
    totalGrid->addWidget(itemLabel, 0, 0, AlignRight | AlignVCenter);
    totalGrid->addWidget(_item_amt, 0, 1, AlignLeft | AlignVCenter);
    totalGrid->addWidget(chargeLabel, 1, 0, AlignRight | AlignVCenter);
    totalGrid->addWidget(_charge_amt, 1, 1, AlignLeft | AlignVCenter);
    totalGrid->addWidget(taxAmtLabel, 0, 3, AlignRight | AlignVCenter);
    totalGrid->addWidget(_tax_amt, 0, 4, AlignLeft | AlignVCenter);
    totalGrid->addWidget(depositAmtLabel, 1, 3, AlignRight | AlignVCenter);
    totalGrid->addWidget(_deposit_amt, 1, 4, AlignLeft | AlignVCenter);
    totalGrid->addWidget(totalLabel, 0, 6, AlignRight | AlignVCenter);
    totalGrid->addWidget(_total_amt, 0, 7, AlignLeft | AlignVCenter);

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
    new MoneyColumn(_taxes, tr("Taxable"), 6);
    new MoneyColumn(_taxes, tr("Tax Amt"), 6);
    new MoneyColumn(_taxes, tr("Inc. Taxable"), 6);
    new MoneyColumn(_taxes, tr("Inc. Tax Amt"), 6);

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
    new MoneyColumn(_echarges, tr("Amount"), 5);

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

    QLabel* receivedLabel = new QLabel(tr("Received:"), itemInfo);
    _received = new DoubleEdit(itemInfo);
    _received->setLength(8);
    _received->setFocusPolicy(NoFocus);

    _inc_tax = new QCheckBox(tr("Include Tax?"), itemInfo);
    connect(_inc_tax, SIGNAL(toggled(bool)), SLOT(slotIncTaxChanged()));

    QLabel* taxLabel = new QLabel(tr("Tax:"), itemInfo);
    _tax = new MoneyEdit(itemInfo);
    _tax->setLength(10);
    _tax->setFocusPolicy(NoFocus);

    _inc_dep = new QCheckBox(tr("Include Deposit?"), itemInfo);
    connect(_inc_dep, SIGNAL(toggled(bool)), SLOT(slotIncDepChanged()));

    QLabel* depositLabel = new QLabel(tr("Deposit:"), itemInfo);
    _deposit = new MoneyEdit(itemInfo);
    _deposit->setLength(10);
    connect(_deposit, SIGNAL(validData()), SLOT(slotDepositChanged()));

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
    itemGrid->setColStretch(7, 1);
    itemGrid->addWidget(receivedLabel, 0, 0);
    itemGrid->addWidget(_received, 1, 0);
    itemGrid->addMultiCellWidget(_inc_tax, 0, 0, 2, 3);
    itemGrid->addWidget(taxLabel, 1, 2);
    itemGrid->addWidget(_tax, 1, 3);
    itemGrid->addMultiCellWidget(_inc_dep, 0, 0, 5, 6);
    itemGrid->addWidget(depositLabel, 1, 5);
    itemGrid->addWidget(_deposit, 1, 6);
    itemGrid->addWidget(lineCostLabel, 0, 8);
    itemGrid->addWidget(_line_cost, 0, 9);
    itemGrid->addWidget(lineIChargeLabel, 1, 8);
    itemGrid->addWidget(_line_icharge, 1, 9);
    itemGrid->addWidget(lineEChargeLabel, 2, 8);
    itemGrid->addWidget(_line_echarge, 2, 9);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(1, 1);
    grid->addWidget(top, 0, 0);
    grid->addWidget(mid, 1, 0);
    grid->addWidget(tabs, 2, 0);

    Company company;
    _quasar->db()->lookup(company);
    if (company.depositAccount() == INVALID_ID) {
	depositAmtLabel->hide();
	_deposit_amt->hide();
	_inc_dep->hide();
	depositLabel->hide();
	_deposit->hide();
    }

    setCaption(tr("Purchase Order"));
    finalize();
}

OrderMaster::~OrderMaster()
{
}

void
OrderMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _vend_id;
}

void
OrderMaster::newItem()
{
    Order blank;
    _orig = blank;
    _orig.setNumber("#");
    _orig.setDate(QDate::currentDate());
    _orig.setStoreId(_quasar->defaultStore());

    Company company;
    _quasar->db()->lookup(company);
    _orig.setShipAddress(company.address());

    _curr = _orig;
    _firstField = _vend_id;
}

void
OrderMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _curr.setNumber("#");
    for (unsigned int i = 0; i < _curr.items().size(); ++i)
	_curr.items()[i].billed = 0.0;
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
}

bool
OrderMaster::fileItem()
{
    QString number = _curr.number();
    if (number != "" && _orig.number() != number) {
	vector<Order> orders;
	OrderSelect conditions;
	conditions.number = number;
	_quasar->db()->select(orders, conditions);
	if (orders.size() != 0) {
	    QString message = tr("This order number is already used\n"
		"for another order. Are you sure\n"
		"you want to file this order?");
	    int ch = QMessageBox::warning(this, tr("Warning"), message,
					  QMessageBox::No, QMessageBox::Yes);
	    if (ch != QMessageBox::Yes)
		return false;
	}
    }

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
OrderMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
OrderMaster::restoreItem()
{
    _curr = _orig;
}

void
OrderMaster::cloneItem()
{
    OrderMaster* clone = new OrderMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
OrderMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
OrderMaster::dataToWidget()
{
    _vend_id->setId(_curr.vendorId());
    _vendor_addr->setText(_curr.vendorAddress());
    _ship->setId(_curr.shipId());
    _ship_addr->setText(_curr.shipAddress());
    _term_id->setId(_curr.termsId());
    _store->setId(_curr.storeId());
    _number->setText(_curr.number());
    _date->setDate(_curr.date());
    _ship_via->setText(_curr.shipVia());
    _comment->setText(_curr.comment());
    _inactive->setChecked(!_curr.isActive());

    // Load vendor
    _quasar->db()->lookup(_curr.vendorId(), _vendor);

    // Clear the caches
    _item_cache.clear();
    _tax_cache.clear();

    // Load the items
    _items->setUpdatesEnabled(false);
    _items->clear();
    _lines.clear();
    _lookup->store_id = _store->getId();
    unsigned int i;
    for (i = 0; i < _curr.items().size(); ++i) {
	const OrderItem& line = _curr.items()[i];

	Item item;
	findItem(line.item_id, item);

	QString item_desc = line.description;
	if (item_desc.isEmpty())
	    item_desc = item.description();

	VectorRow* row = new VectorRow(_items->columns());
	row->setValue(0, Plu(line.item_id, line.number));
	row->setValue(1, item_desc);
	row->setValue(2, line.size);
	row->setValue(3, line.ordered);
	row->setValue(4, line.cost);
	row->setValue(5, line.ext_cost);
	row->setValue(6, line.tax_id);
	_items->appendRow(row);

	ItemLine& iline = _lines[_items->rows() - 1];
	iline.item = item;
	iline.number = line.number;
	iline.description = line.description;
	iline.size = line.size;
	iline.size_qty = line.size_qty;
	iline.ordered = line.ordered;
	iline.billed = line.billed;
	iline.cost_id = line.cost_id;
	iline.cost_disc = line.cost_disc;
	iline.cost = line.cost;
	iline.ext_cost = line.ext_cost;
	iline.ext_base = line.ext_base;
	iline.ext_deposit = line.ext_deposit;
	iline.ext_tax = line.ext_tax;
	iline.int_charges = line.int_charges;
	iline.ext_charges = line.ext_charges;
	iline.tax_id = line.tax_id;
	iline.item_tax_id = line.item_tax_id;
	iline.include_tax = line.include_tax;
	iline.include_deposit = line.include_deposit;
    }
    _items->appendRow(new VectorRow(_items->columns()));
    _items->setUpdatesEnabled(true);

    // Load the charges
    _icharges->clear();
    _icharge_lines.clear();
    _echarges->clear();
    _echarge_lines.clear();
    for (i = 0; i < _curr.charges().size(); ++i) {
	const OrderCharge& line = _curr.charges()[i];

	Charge charge;
	findCharge(line.charge_id, charge);

	if (line.internal) {
	    VectorRow* row = new VectorRow(_icharges->columns());
	    row->setValue(0, line.charge_id);
	    row->setValue(1, line.amount);
	    row->setValue(2, line.tax_id);
	    _icharges->appendRow(row);

	    ChargeLine& cline = _icharge_lines[_icharges->rows() - 1];
	    cline.charge = charge;
	    cline.amount = line.amount;
	    cline.base = line.base;
	    cline.tax_id = line.tax_id;
	} else {
	    VectorRow* row = new VectorRow(_echarges->columns());
	    row->setValue(0, line.charge_id);
	    row->setValue(1, line.amount);
	    _echarges->appendRow(row);

	    ChargeLine& cline = _echarge_lines[_echarges->rows() - 1];
	    cline.charge = charge;
	    cline.amount = line.amount;
	    cline.base = line.base;
	    cline.tax_id = INVALID_ID;
	}
    }
    _icharges->appendRow(new VectorRow(_icharges->columns()));
    _echarges->appendRow(new VectorRow(_echarges->columns()));

    updateItemInfo(0);
    recalculate();
}

void
OrderMaster::printItem(bool ask)
{
    if (ask) {
	QString message = tr("Do you wish to print order #%1?")
		.arg(_curr.number());
	int ch = QMessageBox::information(this, tr("Question"), message,
					  QMessageBox::No, QMessageBox::Yes);
	if (ch != QMessageBox::Yes) return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    QString fileName = "order_print.xml";
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
    params["order_id"] = _curr.id().toString();
    ReportOutput output;
    report.generate(params, output);

    QApplication::restoreOverrideCursor();
    output.print(this);
}

// Set the data object from the widgets.
void
OrderMaster::widgetToData()
{
    _curr.setVendorId(_vend_id->getId());
    _curr.setVendorAddress(_vendor_addr->text());
    _curr.setShipId(_ship->getId());
    _curr.setShipAddress(_ship_addr->text());
    _curr.setTermsId(_term_id->getId());
    _curr.setStoreId(_store->getId());
    _curr.setNumber(_number->text());
    _curr.setDate(_date->getDate());
    _curr.setShipVia(_ship_via->text());
    _curr.setComment(_comment->text());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    // Process all the items
    vector<OrderItem>& items = _curr.items();
    items.clear();
    unsigned int i;
    for (i = 0; i < _lines.size(); ++i) {
	const ItemLine& line = _lines[i];
	if (line.item.id() == INVALID_ID) continue;

	OrderItem item;
	item.item_id = line.item.id();
	item.number = line.number;
	item.description = line.description;
	item.size = line.size;
	item.size_qty = line.size_qty;
	item.ordered = line.ordered;
	item.billed = line.billed;
	item.cost_id = line.cost_id;
	item.cost_disc = line.cost_disc;
	item.cost = line.cost;
	item.ext_cost = line.ext_cost;
	item.ext_base = line.ext_base;
	item.ext_deposit = line.ext_deposit;
	item.ext_tax = line.ext_tax;
	item.int_charges = line.int_charges;
	item.ext_charges = line.ext_charges;
	item.tax_id = line.tax_id;
	item.item_tax_id = line.item_tax_id;
	item.include_tax = line.include_tax;
	item.include_deposit = line.include_deposit;
	items.push_back(item);
    }

    // Process all the taxes
    vector<OrderTax>& taxes = _curr.taxes();
    taxes.clear();
    for (i = 0; i < _tax_info.size(); ++i) {
	Id tax_id = _tax_info[i].tax_id;
	fixed taxable = _tax_info[i].taxable;
	fixed amount = _tax_info[i].amount;;
	fixed inc_taxable = _tax_info[i].inc_taxable;
	fixed inc_amount = _tax_info[i].inc_amount;

	OrderTax line(tax_id, taxable, amount, inc_taxable, inc_amount);
	taxes.push_back(line);
    }

    // Process internal charges
    vector<OrderCharge>& charges = _curr.charges();
    charges.clear();
    for (i = 0; i < _icharge_lines.size(); ++i) {
	const Charge& charge = _icharge_lines[i].charge;
	if (charge.id() == INVALID_ID) continue;
	Id tax_id = _icharge_lines[i].tax_id;
	fixed amount = _icharge_lines[i].amount;
	fixed base = _icharge_lines[i].base;

	OrderCharge line(charge.id(), tax_id, amount, base, true);
	charges.push_back(line);
    }

    // Process external charges
    for (i = 0; i < _echarge_lines.size(); ++i) {
	const Charge& charge = _echarge_lines[i].charge;
	if (charge.id() == INVALID_ID) continue;
	fixed amount = _echarge_lines[i].amount;
	fixed base = _echarge_lines[i].base;

	OrderCharge line(charge.id(), INVALID_ID, amount, base, false);
	charges.push_back(line);
    }
}

void
OrderMaster::slotVendorChanged()
{
    Vendor vendor;
    if (_quasar->db()->lookup(_vend_id->getId(), vendor)) {
	_vendor_addr->setText(vendor.address());
	_term_id->setId(vendor.termsId());
    }
    _vendor = vendor;

    // Recalculate costs
    if (_items->rows() > 0) {
	for (int row = 0; row < _items->rows(); ++row)
	    recalculateCost(row);
	recalculate();
	if (_items->currentRow() != -1)
	    updateItemInfo(_items->currentRow());
    }
}

void
OrderMaster::slotStoreChanged()
{
    // Recalculate costs
    if (_items->rows() > 0) {
	for (int row = 0; row < _items->rows(); ++row) {
	    ItemLine& line = _lines[row];
	    if (line.item.id() == INVALID_ID) continue;

	    // Check that item is stocked in new store
	    if (!line.item.stocked(_store->getId())) {
		QString message = tr("Item #%1 is not stocked in "
		    "the new\nstore and will be removed from the "
		    "order.").arg(line.number);
		QMessageBox::warning(this, tr("Warning"), message);
		_items->deleteRow(row);
		--row;
		continue;
	    }

	    recalculateCost(row);
	}
	recalculate();
	if (_items->currentRow() != -1)
	    updateItemInfo(_items->currentRow());
    }

    _lookup->store_id = _store->getId();
    slotShipToChanged();
}

void
OrderMaster::slotShipToChanged()
{
    Id card_id = _ship->getId();
    if (card_id == INVALID_ID) {
	Store store;
	_quasar->db()->lookup(_store->getId(), store);
	if (store.hasAddress()) {
	    _ship_addr->setText(store.address());
	} else {
	    Company company;
	    _quasar->db()->lookup(company);
	    _ship_addr->setText(company.address());
	}
    } else {
	Card card;
	_quasar->db()->lookup(_ship->getId(), card);
	_ship_addr->setText(card.address());
    }
}

void
OrderMaster::slotSearch()
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
OrderMaster::slotSalesHistory()
{
    if (!_hist.isNull()) return;

    _hist = new SalesHistory(_main);
    if (_item_row != -1) {
	_hist->setStoreId(_store->getId());
	_hist->setItem(_lines[_item_row].item.id(), _lines[_item_row].number);
    }
    _hist->show();
}

void
OrderMaster::slotTemplate()
{
    OrderTemplateLookup* lookup = new OrderTemplateLookup(_main, this);
    lookup->refresh();
    if (!lookup->exec()) return;

    OrderTemplate templ;
    _quasar->db()->lookup(lookup->getId(), templ);

    // Set vendor if not set
    _vend_id->setId(templ.vendorId());
    slotVendorChanged();

    // Remove bottom blank lines for now
    _items->deleteRow(_items->rows() - 1);
    _icharges->deleteRow(_icharges->rows() - 1);
    _echarges->deleteRow(_echarges->rows() - 1);

    // Bring over items
    const vector<TemplateItem>& items = templ.items();
    unsigned int i;
    for (i = 0; i < items.size(); ++i) {
	Id item_id = items[i].item_id;
	QString number = items[i].number;
	QString size = items[i].size;

	Item item;
	findItem(item_id, item);

	VectorRow* row = new VectorRow(_items->columns());
	row->setValue(0, Plu(item_id, number));
	row->setValue(1, item.description());
	row->setValue(2, size);
	row->setValue(3, 0.0);
	row->setValue(4, 0.0);
	row->setValue(5, 0.0);
	row->setValue(6, item.purchaseTax());
	_items->appendRow(row);

	ItemLine& iline = _lines[_items->rows() - 1];
	iline.item = item;
	iline.number = number;
	iline.description = item.description();
	iline.size = size;
	iline.size_qty = item.sizeQty(size);
	iline.ordered = 0.0;
	iline.billed = 0.0;
	iline.tax_id = item.purchaseTax();
	iline.item_tax_id = item.purchaseTax();
	iline.include_tax = item.costIncludesTax();
	iline.include_deposit = item.costIncludesDeposit();
	recalculateCost(_items->rows() - 1);
    }

    // Bring over charges
    const vector<TemplateCharge>& charges = templ.charges();
    for (i = 0; i < charges.size(); ++i) {
	Id charge_id = charges[i].charge_id;
	Id tax_id = charges[i].tax_id;
	fixed amount = charges[i].amount;
	bool internal = charges[i].internal;

	Charge charge;
	findCharge(charge_id, charge);

	if (internal) {
	    VectorRow* row = new VectorRow(_icharges->columns());
	    row->setValue(0, charge_id);
	    row->setValue(1, amount);
	    row->setValue(2, tax_id);
	    _icharges->appendRow(row);


	    ChargeLine& cline = _icharge_lines[_icharges->rows() - 1];
	    cline.charge = charge;
	    cline.amount = amount;
	    cline.tax_id = tax_id;
	} else {
	    VectorRow* row = new VectorRow(_echarges->columns());
	    row->setValue(0, charge_id);
	    row->setValue(1, amount);
	    _echarges->appendRow(row);

	    ChargeLine& cline = _echarge_lines[_echarges->rows() - 1];
	    cline.charge = charge;
	    cline.amount = amount;
	    cline.tax_id = INVALID_ID;
	}
    }

    _items->appendRow(new VectorRow(_items->columns()));
    _icharges->appendRow(new VectorRow(_icharges->columns()));
    _echarges->appendRow(new VectorRow(_echarges->columns()));

    recalculate();
}

void
OrderMaster::slotIncTaxChanged()
{
    if (_item_row == -1) return;
    ItemLine& line = _lines[_item_row];
    if (line.include_tax == _inc_tax->isChecked()) return;

    line.include_tax = _inc_tax->isChecked();

    recalculate();
}

void
OrderMaster::slotIncDepChanged()
{
    if (_item_row == -1) return;
    ItemLine& line = _lines[_item_row];
    if (line.include_deposit == _inc_dep->isChecked()) return;

    line.include_deposit = _inc_dep->isChecked();

    recalculate();
}

void
OrderMaster::slotDepositChanged()
{
    if (_item_row == -1) return;
    ItemLine& line = _lines[_item_row];

    line.ext_deposit = _deposit->getFixed();

    recalculate();
}

void
OrderMaster::updateItemInfo(int row)
{
    _item_row = row;

    // Set to defaults
    if (row == -1) {
	_received->setText("");
	_inc_tax->setChecked(false);
	_tax->setText("");
	_inc_dep->setChecked(false);
	_deposit->setText("");
	_line_cost->setText("");
	_line_icharge->setText("");
	_line_echarge->setText("");
	return;
    }

    const ItemLine& line = _lines[row];

    _received->setFixed(line.billed);
    _inc_tax->setChecked(line.include_tax);
    _tax->setFixed(line.ext_tax);
    _inc_dep->setChecked(line.include_deposit);
    _deposit->setFixed(line.ext_deposit);
    _line_cost->setFixed(line.ext_base);
    _line_icharge->setFixed(line.int_charges);
    _line_echarge->setFixed(line.ext_charges);

    if (!_hist.isNull()) {
	_hist->setStoreId(_store->getId());
	_hist->setItem(_lines[_item_row].item.id(), _lines[_item_row].number);
    }
}

void
OrderMaster::itemCellMoved(int row, int)
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
OrderMaster::itemCellValidate(int, int col, Variant value, bool& ok)
{
    static bool inValidate = false;
    if (inValidate) return;
    if (col != 0) return;

    Plu plu = value.toPlu();
    if (plu.itemId() == INVALID_ID) return;

    Item item;
    if (!findItem(plu.itemId(), item)) return;
    if (!item.isDiscontinued()) return;

    inValidate = true;
    QMessageBox::critical(this, tr("Error"), tr("Item is discontinued"));
    ok = false;
    inValidate = false;
}

void
OrderMaster::itemCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _items->rows() - 1 && col == 0) {
	Id item_id = _items->cellValue(row, col).toId();
	if (item_id != INVALID_ID)
	    _items->appendRow(new VectorRow(_items->columns()));
    }

    // Check for other changes
    ItemLine& line = _lines[row];
    Plu plu;
    Id item_id;
    fixed ext_cost;
    switch (col) {
    case 0: // item_id
	plu = _items->cellValue(row, 0).toPlu();
	if (findItem(plu.itemId(), line.item)) {
	    QString number = plu.number();
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
	    line.ordered = 1.0;
	    line.billed = 0.0;
	    line.tax_id = line.item.purchaseTax();
	    line.item_tax_id = line.item.purchaseTax();
	    line.include_tax = line.item.costIncludesTax();
	    line.include_deposit = line.item.costIncludesDeposit();

	    _items->setCellValue(row, 1, line.item.description());
	    _items->setCellValue(row, 2, line.size);
	    _items->setCellValue(row, 3, line.ordered);
	    _items->setCellValue(row, 5, "");
	    _items->setCellValue(row, 6, line.tax_id);
	    recalculateCost(row);
	} else {
	    if (row != _items->rows() - 1) {
		line.description = tr("**Unknown Item**");
		_items->setCellValue(row, 1, tr("**Unknown Item**"));
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
	line.ordered = _items->cellValue(row, 3).toFixed();
	recalculateCost(row);
	break;
    case 4: // cost
	line.cost = _items->cellValue(row, 4).toPrice();
	ext_cost = line.cost.calculate(line.ordered);
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
	if (line.ordered == 0.0)
	    line.cost.setPrice(0.0);
	else
	    line.cost.setPrice(line.ext_cost / line.ordered);
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
OrderMaster::itemFocusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _items->currentRow();
    int col = _items->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id item_id = _items->cellValue(row, 0).toId();
	if (item_id == INVALID_ID && row == _items->rows() - 1) {
	    leave = true;
	} else if (item_id == INVALID_ID) {
	    newCol = 0;
	} else {
	    newCol = 3;
	}
    } else if (type == Table::MovePrev && col == 2) {
	newCol = 0;
    } else if (type == Table::MoveNext && col == 5) {
	newCol = 0;
	newRow = row + 1;
    } else if (type == Table::MovePrev && col == 0) {
	if (row > 0) {
	    newCol = 5;
	    newRow = row - 1;
	}
    }
}

void
OrderMaster::itemRowInserted(int row)
{
    ItemLine line;
    _lines.insert(_lines.begin() + row, line);
}

void
OrderMaster::itemRowDeleted(int row)
{
    _lines.erase(_lines.begin() + row);
    recalculate();
}

void
OrderMaster::ichargeCellChanged(int row, int col, Variant)
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
	if (findCharge(charge_id, line.charge)) {
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
OrderMaster::ichargeFocusNext(bool& leave, int& newRow, int& newCol, int type)
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
OrderMaster::ichargeRowInserted(int row)
{
    ChargeLine line;
    _icharge_lines.insert(_icharge_lines.begin() + row, line);
}

void
OrderMaster::ichargeRowDeleted(int row)
{
    _icharge_lines.erase(_icharge_lines.begin() + row);
    recalculate();
}

void
OrderMaster::echargeCellChanged(int row, int col, Variant)
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
	if (findCharge(charge_id, line.charge)) {
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
OrderMaster::echargeFocusNext(bool& leave, int&, int&, int type)
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
OrderMaster::echargeRowInserted(int row)
{
    ChargeLine line;
    _echarge_lines.insert(_echarge_lines.begin() + row, line);
}

void
OrderMaster::echargeRowDeleted(int row)
{
    _echarge_lines.erase(_echarge_lines.begin() + row);
    recalculate();
}

void
OrderMaster::recalculate()
{
    static bool in_recalculate = false;
    if (in_recalculate) return;
    in_recalculate = true;

    ObjectCache cache(_db);

    // No vendor tax exempt yet but the code is here
    Tax exempt;

    // Process items
    fixed item_total = 0.0;
    fixed deposit_total = 0.0;
    fixed total_weight = 0.0;
    _tax_info.clear();
    unsigned int i;
    for (i = 0; i < _lines.size(); ++i) {
	ItemLine& line = _lines[i];
	if (line.item.id() == INVALID_ID) continue;

	// Calculate base price
	line.ext_base = line.ext_cost;
	if (line.include_deposit)
	    line.ext_base -= line.ext_deposit;

	// Calculate taxes
	fixed ext_tax = 0.0;
	if (line.tax_id != INVALID_ID) {
	    fixed base = line.ext_base;
	    Tax tax;
	    findTax(line.tax_id, tax);
	    vector<Id> tax_ids;
	    vector<fixed> tax_amts;

	    if (line.include_tax) {
		ext_tax = _db->calculateTaxOff(cache, tax, base, tax_ids,
					       tax_amts);
		addTaxInc(base, tax_ids, tax_amts);
		line.ext_base -= ext_tax;
	    } else {
		ext_tax = _db->calculateTaxOn(cache, tax, base, exempt, tax_ids,
					      tax_amts);
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
	total_weight += line.item.weight(line.size) * line.ordered;;
    }

    // Process internal charges
    fixed icharge_total = 0.0;
    for (i = 0; i < _icharge_lines.size(); ++i) {
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
	    findTax(cline.tax_id, tax);
	    vector<Id> tax_ids;
	    vector<fixed> tax_amts;

	    if (charge.includeTax()) {
		base -= _db->calculateTaxOff(cache, tax, amount, tax_ids,
					     tax_amts);
		addTaxInc(amount, tax_ids, tax_amts);
	    } else {
		_db->calculateTaxOn(cache, tax, amount, exempt, tax_ids,
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
		if (line.item.id() == INVALID_ID) continue;

		fixed weight = line.item.weight(line.size) * line.ordered;
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
    for (i = 0; i < _echarge_lines.size(); ++i) {
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
	    findTax(cline.tax_id, tax);
	    vector<Id> tax_ids;
	    vector<fixed> tax_amts;

	    if (charge.includeTax()) {
		base -= _db->calculateTaxOff(cache, tax, amount, tax_ids,
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
		if (line.item.id() == INVALID_ID) continue;

		fixed weight = line.item.weight(line.size) * line.ordered;
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
    for (i = 0; i < _tax_info.size(); ++i) {
	Id tax_id = _tax_info[i].tax_id;
	_tax_info[i].amount.moneyRound();

	tax_total += _tax_info[i].amount + _tax_info[i].inc_amount;

	Tax tax;
	findTax(tax_id, tax);

	VectorRow* row = new VectorRow(_taxes->columns());
	row->setValue(0, tax.name());
	row->setValue(1, tax.description());
	row->setValue(2, _tax_info[i].taxable);
	row->setValue(3, _tax_info[i].amount);
	row->setValue(4, _tax_info[i].inc_taxable);
	row->setValue(5, _tax_info[i].inc_amount);
	_taxes->appendRow(row);
    }

    fixed total = item_total + tax_total + deposit_total + icharge_total;

    _item_amt->setFixed(item_total);
    _charge_amt->setFixed(icharge_total);
    _tax_amt->setFixed(tax_total);
    _deposit_amt->setFixed(deposit_total);
    _total_amt->setFixed(total);

    _skip_recalc = false;
    updateItemInfo(_item_row);

    in_recalculate = false;
}

void
OrderMaster::recalculateCost(int row)
{
    ItemLine& line = _lines[row];
    if (line.item.id() == INVALID_ID)
	return;

    // Get existing costs use for quantity limit
    vector<Id> cost_ids;
    vector<fixed> recv_qtys;
    unsigned int i;
    for (i = 0; i < _lines.size(); ++i) {
	const ItemLine& line = _lines[i];
	if (line.cost_id == INVALID_ID) continue;
	if (int(i) == row) continue;

	bool found = false;
	for (unsigned int j = 0; j < cost_ids.size(); ++j) {
	    if (cost_ids[j] == line.cost_id) {
		found = true;
		recv_qtys[j] += line.ordered;
		break;
	    }
	}
	if (!found) {
	    cost_ids.push_back(line.cost_id);
	    recv_qtys.push_back(line.ordered);
	}
    }

    vector<ItemPrice> costs;
    findCosts(line.item.id(), costs);
    QDate date = _date->getDate();
    Id store = _store->getId();

    ItemPrice cost;
    fixed sell_qty;
    _quasar->db()->itemBestPrice(line.item, costs, line.size, _vendor,
				 store, date, line.ordered, true, true,
				 cost, line.ext_cost, sell_qty, cost_ids,
				 recv_qtys);
    line.ext_deposit = line.item.deposit() * line.size_qty * sell_qty;

    if (sell_qty != line.ordered) {
	QString message = tr("The quantity was changed from %1 to %2\n"
			     "to take advantage of a quantity limit "
			     "cost.").arg(line.ordered.toString())
	    .arg(sell_qty.toString());
	QMessageBox::warning(this, tr("Warning"), message);
    }

    line.ordered = sell_qty;
    line.cost_id = cost.id();
    line.cost_disc = 0.0;
    if (cost.method() == ItemPrice::PRICE)
	line.cost = cost.price();
    else
	line.cost = Price(line.ext_cost / sell_qty);

    _items->setCellValue(row, 3, line.ordered);
    _items->setCellValue(row, 4, line.cost);
    _items->setCellValue(row, 5, line.ext_cost);

    // If size is wrong, try to find better number for size
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
    for (i = 0; i < line.item.vendors().size(); ++i) {
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
OrderMaster::addTax(fixed taxable, vector<Id>& ids, vector<fixed>& amts)
{
    for (unsigned int i = 0; i < ids.size(); ++i) {
	Id tax_id = ids[i];
	fixed tax_amt = amts[i];

	bool found = false;
	for (unsigned int j = 0; j < _tax_info.size(); ++j) {
	    OrderTax& line = _tax_info[j];
	    if (line.tax_id == tax_id) {
		found = true;
		break;
	    }
	}

	if (found) {
	    _tax_info[i].taxable += taxable;
	    _tax_info[i].amount += tax_amt;
	} else {
	    OrderTax line(tax_id, taxable, tax_amt);
	    _tax_info.push_back(line);
	}
    }
}

void
OrderMaster::addTaxInc(fixed taxable, vector<Id>& ids, vector<fixed>& amts)
{
    for (unsigned int i = 0; i < ids.size(); ++i) {
	Id tax_id = ids[i];
	fixed tax_amt = amts[i];

	bool found = false;
	for (unsigned int j = 0; j < _tax_info.size(); ++j) {
	    OrderTax& line = _tax_info[j];
	    if (line.tax_id == tax_id) {
		found = true;
		break;
	    }
	}

	if (found) {
	    _tax_info[i].inc_taxable += taxable;
	    _tax_info[i].inc_amount += tax_amt;
	} else {
	    OrderTax line(tax_id, 0.0, 0.0, taxable, tax_amt);
	    _tax_info.push_back(line);
	}
    }
}

bool
OrderMaster::findItem(Id item_id, Item& item)
{
    for (unsigned int i = 0; i < _item_cache.size(); ++i) {
	if (_item_cache[i].item.id() == item_id) {
	    item = _item_cache[i].item;
	    return true;
	}
    }

    ItemCache cache;
    if (!_quasar->db()->lookup(item_id, cache.item))
	return false;

    ItemPriceSelect conditions;
    conditions.activeOnly = true;
    conditions.item = cache.item;
    conditions.costOnly = true;
    _quasar->db()->select(cache.costs, conditions);

    _item_cache.push_back(cache);
    item = cache.item;
    return true;
}

bool
OrderMaster::findCosts(Id item_id, vector<ItemPrice>& costs)
{
    costs.clear();
    for (unsigned int i = 0; i < _item_cache.size(); ++i) {
	if (_item_cache[i].item.id() == item_id) {
	    costs = _item_cache[i].costs;
	    return true;
	}
    }
    return false;
}

bool
OrderMaster::findTax(Id tax_id, Tax& tax)
{
    if (tax_id == INVALID_ID) return false;

    for (unsigned int i = 0; i < _tax_cache.size(); ++i) {
	if (_tax_cache[i].id() == tax_id) {
	    tax = _tax_cache[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(tax_id, tax))
	return false;

    _tax_cache.push_back(tax);
    return true;
}

bool
OrderMaster::findCharge(Id charge_id, Charge& charge)
{
    if (charge_id == INVALID_ID) return false;

    for (unsigned int i = 0; i < _charge_cache.size(); ++i) {
	if (_charge_cache[i].id() == charge_id) {
	    charge = _charge_cache[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(charge_id, charge))
	return false;

    _charge_cache.push_back(charge);
    return true;
}

OrderMaster::ItemLine::ItemLine()
    : tax_id(INVALID_ID), item_tax_id(INVALID_ID), include_tax(false),
      include_deposit(false)
{
}

OrderMaster::ChargeLine::ChargeLine()
    : tax_id(INVALID_ID)
{
}
