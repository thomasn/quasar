// $Id: quote_master.cpp,v 1.27 2005/05/13 22:18:47 bpepers Exp $
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

#include "quote_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "object_cache.h"
#include "invoice_master.h"
#include "item_price_select.h"
#include "company.h"
#include "table.h"
#include "date_popup.h"
#include "multi_line_edit.h"
#include "id_edit.h"
#include "double_edit.h"
#include "money_edit.h"
#include "percent_edit.h"
#include "price_edit.h"
#include "item_edit.h"
#include "customer_lookup.h"
#include "employee_lookup.h"
#include "item_lookup.h"
#include "account_lookup.h"
#include "tax_lookup.h"
#include "term_lookup.h"
#include "store_lookup.h"
#include "discount_lookup.h"
#include "account.h"
#include "report_defn.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qpopupmenu.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qmessagebox.h>

QuoteMaster::QuoteMaster(MainWindow* main, Id quote_id)
    : DataWindow(main, "CustomerQuote", quote_id), _item_row(-1)
{
    _helpSource = "customer_quote.html";

    // Print button
    QPushButton* print = new QPushButton(tr("Print"), _buttons);
    connect(print, SIGNAL(clicked()), SLOT(slotPrint()));

    // Invoice button
    QPushButton* invoice = new QPushButton(tr("Invoice"), _buttons);
    connect(invoice, SIGNAL(clicked()), SLOT(slotInvoice()));

    // Stock button
    QPushButton* status = new QPushButton(tr("Stock"), _buttons);
    connect(status, SIGNAL(clicked()), SLOT(slotStockStatus()));

    QFrame* top = new QFrame(_frame);
    top->setFrameStyle(QFrame::Raised | QFrame::Panel);

    QLabel* custLabel = new QLabel(tr("Customer:"), top);
    _cust_id = new LookupEdit(new CustomerLookup(main, this), top);
    _cust_id->setLength(30);
    custLabel->setBuddy(_cust_id);
    connect(_cust_id, SIGNAL(validData()), SLOT(slotCustomerChanged()));

    QLabel* customerAddrLabel = new QLabel(tr("Address:"), top);
    _cust_addr = new MultiLineEdit(top);
    _cust_addr->setFixedVisibleLines(4);
    customerAddrLabel->setBuddy(_cust_addr);

    QLabel* numberLabel = new QLabel(tr("Quote No.:"), top);
    _number = new LineEdit(14, top);
    numberLabel->setBuddy(_number);

    QLabel* typeLabel = new QLabel(tr("Type:"), top);
    _type = new QComboBox(top);
    typeLabel->setBuddy(_type);
    connect(_type, SIGNAL(activated(int)), SLOT(slotTypeChanged()));

    _type->insertItem(tr("Item"));
    _type->insertItem(tr("Account"));

    QLabel* referenceLabel = new QLabel(tr("Reference #:"), top);
    _reference = new LineEdit(top);
    _reference->setLength(14, '9');
    referenceLabel->setBuddy(_reference);

    QLabel* invoiceLabel = new QLabel(tr("Invoice:"), top);
    _invoice = new LineEdit(top);
    _invoice->setEnabled(false);
    invoiceLabel->setBuddy(_invoice);

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

    QLabel* expiryLabel = new QLabel(tr("Expire Date:"), top);
    _expiry = new DatePopup(top);
    expiryLabel->setBuddy(_expiry);

    QLabel* employeeLabel = new QLabel(tr("Employee:"), top);
    _employee = new LookupEdit(new EmployeeLookup(main, this), top);
    _employee->setLength(20);
    employeeLabel->setBuddy(_employee);

    QLabel* storeLabel = new QLabel(tr("Store:"), top);
    StoreLookup* locLookup = new StoreLookup(_main, this);
    locLookup->sellOnly = true;
    _store = new LookupEdit(locLookup, top);
    _store->setLength(30);
    storeLabel->setBuddy(_store);
    connect(_store, SIGNAL(validData()), SLOT(slotStoreChanged()));

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(3);
    topGrid->setMargin(3);
    topGrid->setColStretch(4, 1);
    topGrid->setRowStretch(5, 1);
    topGrid->addWidget(custLabel, 0, 0, AlignLeft | AlignVCenter);
    topGrid->addMultiCellWidget(_cust_id, 0, 0, 1, 3, AlignLeft|AlignVCenter);
    topGrid->addWidget(customerAddrLabel, 1, 0, AlignLeft | AlignTop);
    topGrid->addMultiCellWidget(_cust_addr, 1, 3, 1, 3);
    topGrid->addWidget(numberLabel, 4, 0, AlignLeft | AlignVCenter);
    topGrid->addWidget(_number, 4, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(typeLabel, 4, 2, AlignRight | AlignVCenter);
    topGrid->addWidget(_type, 4, 3);
    topGrid->addWidget(referenceLabel, 5, 0, AlignLeft | AlignVCenter);
    topGrid->addWidget(_reference, 5, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(invoiceLabel, 5, 2, AlignRight | AlignVCenter);
    topGrid->addWidget(_invoice, 5, 3);
    topGrid->addWidget(dateLabel, 0, 4, AlignRight | AlignVCenter);
    topGrid->addWidget(_date, 0, 5, AlignLeft | AlignVCenter);
    topGrid->addWidget(shipViaLabel, 1, 4, AlignRight | AlignVCenter);
    topGrid->addWidget(_ship_via, 1, 5, AlignLeft | AlignVCenter);
    topGrid->addWidget(termLabel, 2, 4, AlignRight | AlignVCenter);
    topGrid->addWidget(_term_id, 2, 5, AlignLeft | AlignVCenter);
    topGrid->addWidget(expiryLabel, 3, 4, AlignRight | AlignVCenter);
    topGrid->addWidget(_expiry, 3, 5, AlignLeft | AlignVCenter);
    topGrid->addWidget(employeeLabel, 4, 4, AlignRight | AlignVCenter);
    topGrid->addWidget(_employee, 4, 5, AlignLeft | AlignVCenter);
    topGrid->addWidget(storeLabel, 5, 4, AlignRight | AlignVCenter);
    topGrid->addWidget(_store, 5, 5, AlignLeft|AlignVCenter);

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
    _lookup->soldOnly = true;
    TaxLookup* taxLookup = new TaxLookup(_main, this);

    // Add columns
    new LookupColumn(_items, tr("Item Number"), 12, _lookup);
    new TextColumn(_items, tr("Description"), 14);
    new TextColumn(_items, tr("Size"), 8);
    new NumberColumn(_items, tr("Qty"), 6);
    new PriceColumn(_items, tr("Price"));
    new MoneyColumn(_items, tr("Ext Price"));
    new TextColumn(_items, tr("Discount"), 9, AlignRight);
    new LookupColumn(_items, tr("Tax"), 6, taxLookup);

    LineEdit* descEdit = new LineEdit(_items);
    NumberEdit* qtyEdit = new DoubleEdit(_items);
    PriceEdit* priceEdit = new PriceEdit(_items);
    NumberEdit* extPriceEdit = new MoneyEdit(_items);
    descEdit->setMaxLength(40);
    qtyEdit->setMaxLength(8);
    extPriceEdit->setMaxLength(14);

    // Add editors
    _size = new QComboBox(_items);
    new LookupEditor(_items, 0, new ItemEdit(_lookup, _items));
    new LineEditor(_items, 1, descEdit);
    new ComboEditor(_items, 2, _size);
    new NumberEditor(_items, 3, qtyEdit);
    new PriceEditor(_items, 4, priceEdit);
    new NumberEditor(_items, 5, extPriceEdit);
    new LookupEditor(_items, 7, new LookupEdit(taxLookup, _items));

    _items->setMinimumWidth(_items->sizeHint().width());
    QGridLayout* midGrid = new QGridLayout(mid);
    midGrid->setSpacing(3);
    midGrid->setMargin(3);
    midGrid->setColStretch(0, 1);
    midGrid->addWidget(_items, 0, 0);

    QTabWidget* tabs = new QTabWidget(_frame);
    QFrame* totals = new QFrame(tabs);
    QFrame* comments = new QFrame(tabs);
    QFrame* taxes = new QFrame(tabs);
    QFrame* ship = new QFrame(tabs);
    QFrame* itemInfo = new QFrame(tabs);
    QFrame* discs = new QFrame(tabs);
    tabs->addTab(totals, tr("Totals"));
    tabs->addTab(comments, tr("Comments"));
    tabs->addTab(taxes, tr("Taxes"));
    tabs->addTab(ship, tr("Ship To"));
    tabs->addTab(itemInfo, tr("Item Info"));
    tabs->addTab(discs, tr("Discounts"));

    QLabel* itemLabel = new QLabel(tr("Item Total:"), totals);
    _item_amt = new MoneyEdit(totals);
    _item_amt->setFocusPolicy(NoFocus);

    QLabel* ldiscLabel = new QLabel(tr("Line Discounts:"), totals);
    _ldisc_amt = new MoneyEdit(totals);
    _ldisc_amt->setFocusPolicy(NoFocus);

    QLabel* tdiscLabel = new QLabel(tr("Trans. Discounts:"), totals);
    _tdisc_amt = new MoneyEdit(totals);
    _tdisc_amt->setFocusPolicy(NoFocus);

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
    totalGrid->addWidget(itemLabel, 0, 0, AlignRight | AlignVCenter);
    totalGrid->addWidget(_item_amt, 0, 1, AlignLeft | AlignVCenter);
    totalGrid->addWidget(ldiscLabel, 1, 0, AlignRight | AlignVCenter);
    totalGrid->addWidget(_ldisc_amt, 1, 1, AlignLeft | AlignVCenter);
    totalGrid->addWidget(tdiscLabel, 2, 0, AlignRight | AlignVCenter);
    totalGrid->addWidget(_tdisc_amt, 2, 1, AlignLeft | AlignVCenter);
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

    QLabel* shipLabel = new QLabel(tr("Ship To:"), ship);
    _ship = new LookupEdit(new CustomerLookup(main, this), ship);
    _ship->setLength(30);
    shipLabel->setBuddy(_ship);
    connect(_ship, SIGNAL(validData()), SLOT(slotShipToChanged()));

    QLabel* shipAddrLabel = new QLabel(tr("Address:"), ship);
    _ship_addr = new MultiLineEdit(ship);
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
    _discountable = new QCheckBox(tr("Discountable?"), itemInfo);
    connect(_discountable, SIGNAL(toggled(bool)), SLOT(slotDiscableChanged()));

    QLabel* depositLabel = new QLabel(tr("Deposit:"), itemInfo);
    _deposit = new MoneyEdit(itemInfo);
    _deposit->setLength(10);
    connect(_deposit, SIGNAL(validData()), SLOT(slotDepositChanged()));

    QLabel* lineDiscLabel = new QLabel(tr("Discount:"), itemInfo);
    DiscountLookup* discLookup = new DiscountLookup(_main, this);
    discLookup->lineOnly = true;
    _line_disc = new LookupEdit(discLookup, itemInfo);
    _line_disc->setLength(20);
    connect(_line_disc, SIGNAL(validData()), SLOT(slotDiscountChanged()));

    _percent = new QRadioButton(tr("Percent:"), itemInfo);
    _percentAmt = new PercentEdit(itemInfo);
    connect(_percentAmt, SIGNAL(validData()), SLOT(slotDiscountChanged()));

    _dollar = new QRadioButton(tr("Dollar:"), itemInfo);
    _dollarAmt = new MoneyEdit(itemInfo);
    connect(_dollarAmt, SIGNAL(validData()), SLOT(slotDiscountChanged()));

    QButtonGroup* methods = new QButtonGroup(this);
    methods->hide();
    methods->insert(_percent);
    methods->insert(_dollar);
    connect(methods, SIGNAL(clicked(int)), SLOT(slotDiscountChanged()));

    QLabel* discountLabel = new QLabel(tr("Line Disc.:"), itemInfo);
    _discount = new MoneyEdit(itemInfo);
    _discount->setLength(10);
    _discount->setFocusPolicy(NoFocus);

    QLabel* lineTotalLabel = new QLabel(tr("Line Total:"), itemInfo);
    _line_total = new MoneyEdit(itemInfo);
    _line_total->setLength(10);
    _line_total->setFocusPolicy(NoFocus);

    QLabel* txDiscountLabel = new QLabel(tr("Trans. Disc.:"), itemInfo);
    _tx_disc = new MoneyEdit(itemInfo);
    _tx_disc->setLength(10);
    _tx_disc->setFocusPolicy(NoFocus);

    QGridLayout* itemGrid = new QGridLayout(itemInfo);
    itemGrid->setSpacing(3);
    itemGrid->setMargin(3);
    itemGrid->setRowStretch(3, 1);
    itemGrid->setColStretch(1, 1);
    itemGrid->setColStretch(4, 1);
    itemGrid->setColStretch(7, 1);
    itemGrid->addWidget(_discountable, 0, 0);
    itemGrid->addWidget(_inc_tax, 1, 0);
    itemGrid->addWidget(_inc_dep, 2, 0);
    itemGrid->addWidget(depositLabel, 0, 2);
    itemGrid->addWidget(_deposit, 0, 3);
    itemGrid->addWidget(lineDiscLabel, 0, 5);
    itemGrid->addWidget(_line_disc, 0, 6);
    itemGrid->addWidget(_percent, 1, 5);
    itemGrid->addWidget(_percentAmt, 1, 6);
    itemGrid->addWidget(_dollar, 2, 5);
    itemGrid->addWidget(_dollarAmt, 2, 6);
    itemGrid->addWidget(discountLabel, 0, 8);
    itemGrid->addWidget(_discount, 0, 9);
    itemGrid->addWidget(lineTotalLabel, 1, 8);
    itemGrid->addWidget(_line_total, 1, 9);
    itemGrid->addWidget(txDiscountLabel, 2, 8);
    itemGrid->addWidget(_tx_disc, 2, 9);

    _discs = new Table(discs);
    _discs->setVScrollBarMode(QScrollView::AlwaysOn);
    _discs->setDisplayRows(3);
    connect(_discs, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(discCellChanged(int,int,Variant)));
    connect(_discs, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(discFocusNext(bool&,int&,int&,int)));
    connect(_discs, SIGNAL(rowInserted(int)), SLOT(discRowInserted(int)));
    connect(_discs, SIGNAL(rowDeleted(int)), SLOT(discRowDeleted(int)));

    DiscountLookup* txDiscLookup = new DiscountLookup(_main, this);
    txDiscLookup->txOnly = true;

    new LookupColumn(_discs, tr("Discount"), 20, txDiscLookup);
    new PercentColumn(_discs, tr("Percent"));
    new MoneyColumn(_discs, tr("Dollars"));
    new MoneyColumn(_discs, tr("Total"));

    new LookupEditor(_discs, 0, new LookupEdit(txDiscLookup, _discs));
    new NumberEditor(_discs, 1, new PercentEdit(_discs));
    new NumberEditor(_discs, 2, new MoneyEdit(_discs));

    QGridLayout* discGrid = new QGridLayout(discs);
    discGrid->setSpacing(3);
    discGrid->setMargin(3);
    discGrid->setRowStretch(0, 1);
    discGrid->setColStretch(0, 1);
    discGrid->addWidget(_discs, 0, 0);

    // Flags
    _inactive->setText(tr("Voided?"));

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(0, 1);
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

    setName("CustomerQuote");
    setCaption(tr("Customer Quote"));
    _helpSource = "customer_quote.html";

    _old_type = Quote::Item;
    _size_width = _items->columnWidth(2);

    finalize();
}

QuoteMaster::~QuoteMaster()
{
    if (!_stock.isNull())
	delete _stock;
}

void
QuoteMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _cust_id;
}

void
QuoteMaster::newItem()
{
    Quote blank;
    _orig = blank;
    _orig.setDate(QDate::currentDate());
    _orig.setStoreId(_quasar->defaultStore());
    _orig.setEmployeeId(_quasar->user().defaultEmployee());

    _curr = _orig;
    _firstField = _cust_id;
}

void
QuoteMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _curr.setNumber("#");
    _curr.setInvoiceId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
}

bool
QuoteMaster::fileItem()
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
QuoteMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
QuoteMaster::restoreItem()
{
    _curr = _orig;
}

void
QuoteMaster::cloneItem()
{
    QuoteMaster* clone = new QuoteMaster(_main, INVALID_ID);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
QuoteMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
QuoteMaster::dataToWidget()
{
    _cust_id->setId(_curr.customerId());
    _cust_addr->setText(_curr.customerAddress());
    _ship->setId(_curr.shipId());
    _ship_addr->setText(_curr.shipAddress());
    _term_id->setId(_curr.termsId());
    _store->setId(_curr.storeId());
    _number->setText(_curr.number());
    _type->setCurrentItem(_curr.lineType());
    _date->setDate(_curr.date());
    _reference->setText(_curr.reference());
    _ship_via->setText(_curr.shipVia());
    _expiry->setDate(_curr.expiryDate());
    _employee->setId(_curr.employeeId());
    _comment->setText(_curr.comment());
    _inactive->setChecked(!_curr.isActive());

    // Load customer
    _quasar->db()->lookup(_curr.customerId(), _customer);

    // Set the invoice
    if (_curr.invoiceId() == INVALID_ID) {
	_invoice->setText("");
    } else {
	Invoice invoice;
	_quasar->db()->lookup(_curr.invoiceId(), invoice);
	_invoice->setText(invoice.number());
    }

    // Clear the caches
    _item_cache.clear();
    _tax_cache.clear();
    _discount_cache.clear();

    // Clear out item info
    clearItems(false);
    slotTypeChanged();

    // Load the items
    _items->setUpdatesEnabled(false);
    _items->clear();
    _lines.clear();
    _lookup->store_id = _store->getId();
    const vector<InvoiceItem>& items = _curr.items();
    unsigned int i;
    for (i = 0; i < items.size(); ++i) {
	Id item_id = items[i].item_id;
	Id account_id = items[i].account_id;
	QString number = items[i].number;
	QString description = items[i].description;
	QString size = items[i].size;
	fixed size_qty = items[i].size_qty;
	fixed quantity = items[i].quantity;
	Id price_id = items[i].price_id;
	fixed price_disc = items[i].price_disc;
	Price price = items[i].price;
	fixed ext_price = items[i].ext_price;
	fixed ext_base = items[i].ext_base;
	fixed ext_deposit = items[i].ext_deposit;
	fixed ext_tax = items[i].ext_tax;
	Id tax_id = items[i].tax_id;
	Id item_tax_id = items[i].item_tax_id;
	bool include_tax = items[i].include_tax;
	bool include_deposit = items[i].include_deposit;
	bool discountable = items[i].discountable;
	bool open_dept = items[i].open_dept;
	InvoiceDisc line_disc = items[i].line_disc;

	Item item;
	Account account;
	QString item_desc = description;
	if (_curr.lineType() == Quote::Item) {
	    findItem(item_id, item);
	    if (item_desc.isEmpty())
		item_desc = item.description();
	} else {
	    _quasar->db()->lookup(account_id, account);
	}

	QString disc_text = "";
	if (line_disc.discount_id != INVALID_ID) {
	    if (line_disc.method == Discount::PERCENT)
		disc_text = PercentValcon().format(line_disc.amount);
	    else
		disc_text = MoneyValcon().format(line_disc.amount);
	}

	VectorRow* row = new VectorRow(_items->columns());
	if (_curr.lineType() == Quote::Item)
	    row->setValue(0, Plu(item_id, number));
	else
	    row->setValue(0, account_id);
	row->setValue(1, item_desc);
	row->setValue(2, size);
	row->setValue(3, quantity);
	row->setValue(4, price);
	row->setValue(5, ext_price);
	row->setValue(6, disc_text);
	row->setValue(7, tax_id);
	_items->appendRow(row);

	ItemLine& line = _lines[_items->rows() - 1];
	line.item = item;
	line.account = account;
	line.number = number;
	line.description = description;
	line.size = size;
	line.size_qty = size_qty;
	line.qty = quantity;
	line.price_id = price_id;
	line.price_disc = price_disc;
	line.price = price;
	line.ext_price = ext_price;
	line.ext_base = ext_base;
	line.ext_deposit = ext_deposit;
	line.ext_tax = ext_tax;
	line.tax_id = tax_id;
	line.item_tax_id = item_tax_id;
	line.include_tax = include_tax;
	line.include_deposit = include_deposit;
	line.discountable = discountable;
	line.open_dept = open_dept;
	line.line_disc.discount_id = line_disc.discount_id;
	line.line_disc.method = line_disc.method;
	line.line_disc.amount = line_disc.amount;
	line.line_disc.account_id = line_disc.account_id;
	line.line_disc.total_amt = line_disc.total_amt;
    }
    _items->appendRow(new VectorRow(_items->columns()));
    _items->setUpdatesEnabled(true);

    // Load the discounts
    _discs->clear();
    _discounts.clear();
    const vector<InvoiceDisc>& discounts = _curr.discounts();
    for (i = 0; i < discounts.size(); ++i) {
	Id discount_id = discounts[i].discount_id;
	int method = discounts[i].method;
	fixed amount = discounts[i].amount;
	Id account_id = discounts[i].account_id;
	fixed total_amt = discounts[i].total_amt;

	VectorRow* row = new VectorRow(_discs->columns());
	row->setValue(0, discount_id);
	if (method == Discount::PERCENT)
	    row->setValue(1, amount);
	else
	    row->setValue(2, amount);
	row->setValue(3, total_amt);
	_discs->appendRow(row);

	DiscountLine& line = _discounts[_discs->rows() - 1];
	line.discount_id = discount_id;
	line.method = method;
	line.amount = amount;
	line.account_id = account_id;
	line.total_amt = total_amt;
    }
    _discs->appendRow(new VectorRow(_discs->columns()));

    updateItemInfo(0);
    recalculate();
}

void
QuoteMaster::printItem(bool ask)
{
    if (ask) {
	QString message = tr("Do you wish to print quote #%1?")
		.arg(_curr.number());
	int ch = QMessageBox::information(this, tr("Question"), message,
					  QMessageBox::No, QMessageBox::Yes);
	if (ch != QMessageBox::Yes) return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    QString fileName = "quote_print.xml";
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
    params["quote_id"] = _curr.id().toString();
    ReportOutput output;
    report.generate(params, output);

    QApplication::restoreOverrideCursor();
    output.print(this);
}

// Set the data object from the widgets.
void
QuoteMaster::widgetToData()
{
    _curr.setCustomerId(_cust_id->getId());
    _curr.setCustomerAddress(_cust_addr->text());
    _curr.setShipId(_ship->getId());
    _curr.setShipAddress(_ship_addr->text());
    _curr.setTermsId(_term_id->getId());
    _curr.setStoreId(_store->getId());
    _curr.setNumber(_number->text());
    _curr.setLineType(Quote::LineType(_type->currentItem()));
    _curr.setDate(_date->getDate());
    _curr.setReference(_reference->text());
    _curr.setShipVia(_ship_via->text());
    _curr.setExpiryDate(_expiry->getDate());
    _curr.setEmployeeId(_employee->getId());
    _curr.setComment(_comment->text());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    Company company;
    _quasar->db()->lookup(company);
    Id store_id = _curr.storeId();

    // Process all the items
    vector<InvoiceItem>& items = _curr.items();
    items.clear();
    unsigned int i;
    for (i = 0; i < _lines.size(); ++i) {
	const ItemLine& line = _lines[i];
	if (line.item.id() == INVALID_ID && line.account.id() == INVALID_ID)
	    continue;

	fixed sale_price = line.ext_base;
	if (line.line_disc.account_id == INVALID_ID) {
	    sale_price -= line.line_disc.total_amt;
	} else {
	    // do nothing
	}
	for (unsigned int j = 0; j < line.tdisc_nums.size(); ++j) {
	    DiscountLine& disc = _discounts[line.tdisc_nums[j]];
	    if (disc.account_id != INVALID_ID) continue;
	    sale_price -= line.tdisc_amts[j];
	}

	InvoiceItem item;
	item.item_id = line.item.id();
	item.account_id = line.account.id();
	item.number = line.number;
	item.description = line.description;
	item.size = line.size;
	item.size_qty = line.size_qty;
	item.quantity = line.qty;
	item.ext_deposit = line.ext_deposit;
	item.price_id = line.price_id;
	item.price_disc = line.price_disc;
	item.price = line.price;
	item.sale_price = sale_price;
	item.ext_price = line.ext_price;
	item.ext_base = line.ext_base;
	item.ext_tax = line.ext_tax;
	item.tax_id = line.tax_id;
	item.item_tax_id = line.item_tax_id;
	item.include_tax = line.include_tax;
	item.include_deposit = line.include_deposit;
	item.discountable = line.discountable;
	item.open_dept = line.open_dept;
	item.line_disc.discount_id = line.line_disc.discount_id;
	item.line_disc.method = line.line_disc.method;
	item.line_disc.amount = line.line_disc.amount;
	item.line_disc.account_id = line.line_disc.account_id;
	item.line_disc.total_amt = line.line_disc.total_amt;
	item.tdisc_nums = line.tdisc_nums;
	item.tdisc_amts = line.tdisc_amts;
	items.push_back(item);
    }

    // Process all the discounts
    vector<InvoiceDisc>& discounts = _curr.discounts();
    discounts.clear();
    for (i = 0; i < _discounts.size(); ++i) {
	const DiscountLine& line = _discounts[i];
	if (line.discount_id == INVALID_ID) continue;

	InvoiceDisc item;
	item.discount_id = line.discount_id;
	item.method = line.method;
	item.amount = line.amount;
	item.account_id = line.account_id;
	item.total_amt = line.total_amt;
	discounts.push_back(item);
    }

    // Process all the taxes
    vector<TaxLine>& taxes = _curr.taxes();
    taxes.clear();
    for (i = 0; i < _tax_info.size(); ++i) {
	Id tax_id = _tax_info[i].tax_id;
	fixed taxable = _tax_info[i].taxable;
	fixed amount = _tax_info[i].amount;
	fixed inc_taxable = _tax_info[i].inc_taxable;
	fixed inc_amount = _tax_info[i].inc_amount;

	Tax tax;
	findTax(tax_id, tax);

	TaxLine line(tax_id, taxable, amount, inc_taxable, inc_amount);
	taxes.push_back(line);
    }
}

void
QuoteMaster::slotCustomerChanged()
{
    Customer customer;
    if (_quasar->db()->lookup(_cust_id->getId(), customer)) {
	_cust_addr->setText(customer.address());
	_term_id->setId(customer.termsId());
    }
    _customer = customer;

    // Recalculate prices
    if (_items->rows() > 0) {
	for (int row = 0; row < _items->rows(); ++row)
	    recalculatePrice(row);
	recalculate();
	if (_items->currentRow() != -1)
	    updateItemInfo(_items->currentRow());
    }
}

void
QuoteMaster::slotStoreChanged()
{
    // Recalculate prices
    if (_items->rows() > 0) {
	for (int row = 0; row < _items->rows(); ++row) {
	    ItemLine& line = _lines[row];
	    if (line.item.id() == INVALID_ID) continue;

	    // Check that item is stocked in new store
	    if (!line.item.stocked(_store->getId())) {
		QString message = tr("Item #%1 is not stocked in the new\n"
				     "store and will be removed from the "
				     "order.").arg(line.number);
		QMessageBox::warning(this, tr("Warning"), message);
		_items->deleteRow(row);
		--row;
		continue;
	    }

	    recalculatePrice(row);
	}
	recalculate();
	if (_items->currentRow() != -1)
	    updateItemInfo(_items->currentRow());
    }

    _lookup->store_id = _store->getId();
}

void
QuoteMaster::slotShipToChanged()
{
    Id card_id = _ship->getId();
    if (card_id == INVALID_ID) {
	_ship_addr->setText(_cust_addr->text());
    } else {
	Card card;
	_quasar->db()->lookup(_ship->getId(), card);
	_ship_addr->setText(card.address());
    }
}

bool
QuoteMaster::clearItems(bool ask)
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
QuoteMaster::slotTypeChanged()
{
    if (!clearItems()) return;
    if (_type->currentItem() == _old_type) return;
    _old_type = _type->currentItem();

    LookupColumn* column = (LookupColumn*)_items->column(0);
    delete column->editor;

    if (_items->columnWidth(2) != 0)
	_size_width = _items->columnWidth(2);

    if (_type->currentItem() == Quote::Account) {
	AccountLookup* acctLookup = new AccountLookup(_main, this);
	_items->setColumnName(0, tr("Account"));
	_items->column(2)->maxWidth = 0;
	_items->setColumnWidth(1, _items->columnWidth(1) + _size_width);
	_items->setColumnWidth(2, 0);
	column->lookup = acctLookup;
	new LookupEditor(_items, 0, new LookupEdit(acctLookup, _items));
    } else {
	_lookup = new ItemLookup(_main, this);
	_lookup->soldOnly = true;
	_lookup->store_id = _store->getId();
	_items->setColumnName(0, tr("Item Number"));
	_items->column(2)->maxWidth = 9999;
	_items->setColumnWidth(1, _items->columnWidth(1) - _size_width);
	_items->setColumnWidth(2, _size_width);
	column->lookup = _lookup;
	new LookupEditor(_items, 0, new ItemEdit(_lookup, _items));
    }
}

void
QuoteMaster::slotInvoice()
{
    if (_inactive->isChecked()) {
	QMessageBox::critical(this, tr("Error"), tr("Quote is voided"));
	return;
    }

    // Save as voided
    _inactive->setChecked(true);
    if (!saveItem(true)) return;

    // Create invoice screen
    InvoiceMaster* master = new InvoiceMaster(_main);
    master->fromQuote(_curr.id());
    master->show();

    slotCancel();
}

void
QuoteMaster::slotStockStatus()
{
    if (!_stock.isNull()) return;

    _stock = new StockStatus(_main);
    if (_item_row != -1) {
	Id item_id = _lines[_item_row].item.id();
	QString number = _lines[_item_row].number;
	_stock->setItem(item_id, number);
    }
    _stock->show();
}

void
QuoteMaster::slotIncTaxChanged()
{
    if (_item_row == -1) return;
    ItemLine& line = _lines[_item_row];
    if (line.include_tax == _inc_tax->isChecked()) return;

    line.include_tax = _inc_tax->isChecked();

    recalculate();
    updateItemInfo(_item_row);
}

void
QuoteMaster::slotIncDepChanged()
{
    if (_item_row == -1) return;
    ItemLine& line = _lines[_item_row];
    if (line.include_deposit == _inc_dep->isChecked()) return;

    line.include_deposit = _inc_dep->isChecked();

    recalculate();
    updateItemInfo(_item_row);
}

void
QuoteMaster::slotDiscableChanged()
{
    if (_item_row == -1) return;
    ItemLine& line = _lines[_item_row];
    if (line.discountable == _discountable->isChecked()) return;

    line.discountable = _discountable->isChecked();

    recalculate();
    updateItemInfo(_item_row);
}

void
QuoteMaster::slotDepositChanged()
{
    if (_item_row == -1) return;
    ItemLine& line = _lines[_item_row];

    line.ext_deposit = _deposit->getFixed();

    recalculate();
    updateItemInfo(_item_row);
}

void
QuoteMaster::slotDiscountChanged()
{
    if (_item_row == -1) return;
    DiscountLine& disc = _lines[_item_row].line_disc;

    if (_line_disc->getId() != disc.discount_id) {
	disc.discount_id = _line_disc->getId();
	if (disc.discount_id != INVALID_ID) {
	    Discount discount;
	    findDiscount(disc.discount_id, discount);
	    disc.method = discount.method();
	    disc.amount = discount.amount();
	    disc.account_id = discount.accountId();
	} else {
	    disc.method = Discount::PERCENT;
	    disc.amount = 0.0;
	    disc.account_id = INVALID_ID;
	}
    } else {
	if (_percent->isChecked()) {
	    disc.method = Discount::PERCENT;
	    disc.amount = _percentAmt->getFixed();
	} else {
	    disc.method = Discount::DOLLAR;
	    disc.amount = _dollarAmt->getFixed();
	}
    }

    QString disc_text = "";
    if (disc.discount_id != INVALID_ID && disc.amount != 0.0) {
	if (disc.method == Discount::PERCENT)
	    disc_text = PercentValcon().format(disc.amount);
	else
	    disc_text = MoneyValcon().format(disc.amount);
    }
    _items->setCellValue(_item_row, 6, disc_text);

    recalculate();
    updateItemInfo(_item_row);
}

void
QuoteMaster::updateItemInfo(int row)
{
    _item_row = row;

    // Set to defaults
    if (row == -1) {
	_discountable->setChecked(false);
	_inc_tax->setChecked(false);
	_inc_dep->setChecked(false);
	_deposit->setText("");
	_line_disc->setId(INVALID_ID);
	_line_disc->setEnabled(false);
	_percent->setChecked(true);
	_percentAmt->setText("");
	_percentAmt->setEnabled(false);
	_dollarAmt->setText("");
	_dollarAmt->setEnabled(false);
	_discount->setText("");
	_line_total->setText("");
	_tx_disc->setText("");
	return;
    }

    const ItemLine& line = _lines[row];
    const DiscountLine& disc = line.line_disc;

    _inc_tax->setChecked(line.include_tax);
    _inc_dep->setChecked(line.include_deposit);
    _discountable->setChecked(line.discountable);
    _deposit->setFixed(line.ext_deposit);
    _line_disc->setId(disc.discount_id);
    _discount->setFixed(line.line_disc.total_amt);
    _line_total->setFixed(line.ext_base - line.line_disc.total_amt);

    fixed tdisc_amt = 0.0;
    for (unsigned int i = 0; i < line.tdisc_amts.size(); ++i)
	tdisc_amt += line.tdisc_amts[i];
    _tx_disc->setFixed(tdisc_amt);

    if (disc.discount_id == INVALID_ID) {
	_percentAmt->setText("");
	_dollarAmt->setText("");
    } else {
	if (disc.method == Discount::PERCENT) {
	    _percent->setChecked(true);
	    _percentAmt->setFixed(disc.amount);
	    _dollarAmt->setText("");
	} else {
	    _dollar->setChecked(true);
	    _dollarAmt->setFixed(disc.amount);
	    _percentAmt->setText("");
	}
    }

    _line_disc->setEnabled(line.discountable);
    _percentAmt->setEnabled(line.discountable && _percent->isChecked());
    _dollarAmt->setEnabled(line.discountable && _dollar->isChecked());

    if (!_stock.isNull()) {
	Id item_id = _lines[_item_row].item.id();
	QString number = _lines[_item_row].number;
	_stock->setItem(item_id, number);
    }
}

void
QuoteMaster::itemCellMoved(int row, int)
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
QuoteMaster::itemCellChanged(int row, int col, Variant)
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
    fixed ext_price;
    switch (col) {
    case 0: // item_id or account_id
	id = _items->cellValue(row, 0).toId();
	if (lineType == Quote::Account) {
	    if (_quasar->db()->lookup(id, line.account)) {
		if (line.qty == 0.0) line.qty = 1.0;
		line.number = "";
		line.description = "";
		line.size = "";
		line.size_qty = 1.0;
		line.price_id = INVALID_ID;
		line.price_disc = 0.0;
		line.price.setNull();
		line.ext_price = 0.0;
		line.ext_deposit = 0.0;
		line.tax_id = INVALID_ID;
		line.item_tax_id = INVALID_ID;
		line.include_tax = false;
		line.include_deposit = false;
		line.discountable = true;
		line.open_dept = false;
		line.line_disc.discount_id = INVALID_ID;

		_items->setCellValue(row, 1, "");
		_items->setCellValue(row, 2, line.size);
		_items->setCellValue(row, 3, line.qty);
		_items->setCellValue(row, 4, "");
		_items->setCellValue(row, 5, "");
		_items->setCellValue(row, 6, "");
		_items->setCellValue(row, 7, INVALID_ID);
	    } else {
		if (row != _items->rows() - 1) {
		    line.description = tr("**Unknown Account**");
		    _items->setCellValue(row, 1, tr("**Unknown Account**"));
		}
	    }
	} else {
	    Plu plu = _items->cellValue(row, 0).toPlu();
	    if (findItem(plu.itemId(), line.item)) {
		QString number = plu.number();
		QString size = line.item.numberSize(number);
		if (size.isEmpty()) size = line.item.sellSize();

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
		line.tax_id = line.item.sellTax();
		line.item_tax_id = line.item.sellTax();
		line.include_tax = line.item.priceIncludesTax();
		line.include_deposit = line.item.priceIncludesDeposit();
		line.line_disc.discount_id = INVALID_ID;
		line.open_dept = line.item.isOpenDept();

		_items->setCellValue(row, 1, line.item.description());
		_items->setCellValue(row, 2, line.size);
		_items->setCellValue(row, 3, line.qty);
		_items->setCellValue(row, 5, "");
		_items->setCellValue(row, 6, "");
		_items->setCellValue(row, 7, line.tax_id);
		recalculatePrice(row);
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
	recalculatePrice(row);
	break;
    case 3: // qty
	line.qty = _items->cellValue(row, 3).toFixed();
	recalculatePrice(row);
	break;
    case 4: // price
	line.price = _items->cellValue(row, 4).toPrice();
	ext_price = line.price.calculate(line.qty);
	if (line.price_id != INVALID_ID)
	    line.price_disc = line.ext_price - ext_price;
	line.price_id = INVALID_ID;
	line.ext_price = ext_price;
	_items->setCellValue(row, 5, line.ext_price);
	break;
    case 5: // ext_price
	ext_price = _items->cellValue(row, 5).toFixed();
	if (line.price_id != INVALID_ID)
	    line.price_disc = line.ext_price - ext_price;
	line.ext_price = ext_price;
	line.price_id = INVALID_ID;
	if (line.qty == 0.0)
	    line.price.setPrice(0.0);
	else
	    line.price.setPrice(line.ext_price / line.qty);
	_items->setCellValue(row, 4, line.price);
	break;
    case 7: // tax_id
	line.tax_id = _items->cellValue(row, 7).toId();
	break;
    }

    recalculate();
    updateItemInfo(row);
}

void
QuoteMaster::itemFocusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _items->currentRow();
    int col = _items->currentColumn();
    bool isAccount = _type->currentItem() == Quote::Account;

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
QuoteMaster::itemRowInserted(int row)
{
    ItemLine line;
    _lines.insert(_lines.begin() + row, line);
}

void
QuoteMaster::itemRowDeleted(int row)
{
    _lines.erase(_lines.begin() + row);
    recalculate();
}

void
QuoteMaster::discCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _discs->rows() - 1 && col == 0) {
	Variant id = _discs->cellValue(row, col);
	if (!id.isNull() && id.toId() != INVALID_ID)
	    _discs->appendRow(new VectorRow(_discs->columns()));
    }

    // Check for other changes
    DiscountLine& line = _discounts[row];
    fixed amount;
    switch (col) {
    case 0: // discount_id
	line.discount_id = _discs->cellValue(row, 0).toId();
	if (line.discount_id != INVALID_ID) {
	    Discount discount;
	    findDiscount(line.discount_id, discount);
	    line.method = discount.method();
	    line.amount = discount.amount();
	    line.account_id = discount.accountId();

	    if (discount.method() == Discount::PERCENT) {
		_discs->setCellValue(row, 1, discount.amount());
		_discs->setCellValue(row, 2, "");
	    } else {
		_discs->setCellValue(row, 1, "");
		_discs->setCellValue(row, 2, discount.amount());
	    }
	} else {
	    line.method = Discount::PERCENT;
	    line.amount = 0.0;
	    line.account_id = INVALID_ID;

	    _discs->setCellValue(row, 1, "");
	    _discs->setCellValue(row, 2, "");
	}
	_discs->setCellValue(row, 3, "");
	break;
    case 1: // percent
	amount = _discs->cellValue(row, 1).toFixed();
	if (amount != 0.0) {
	    line.method = Discount::PERCENT;
	    line.amount = amount;
	    _discs->setCellValue(row, 2, "");
	}
	break;
    case 2: // dollar
	amount = _discs->cellValue(row, 2).toFixed();
	if (amount != 0.0) {
	    line.method = Discount::DOLLAR;
	    line.amount = amount;
	    _discs->setCellValue(row, 1, "");
	}
	break;
    }

    recalculate();
}

void
QuoteMaster::discFocusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _discs->currentRow();
    int col = _discs->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Variant val = _discs->cellValue(row, col);
	Id id = val.toId();
	if ((val.isNull() || id == INVALID_ID) && row == _discs->rows() - 1) {
	    leave = true;
	}
    } else if (type == Table::MoveNext && col == 2) {
	newCol = 0;
	newRow = row + 1;
    } else if (type == Table::MovePrev && col == 0) {
	if (row > 0) {
	    newCol = 2;
	    newRow = row - 1;
	}
    }
}

void
QuoteMaster::discRowInserted(int row)
{
    DiscountLine line;
    _discounts.insert(_discounts.begin() + row, line);
}

void
QuoteMaster::discRowDeleted(int row)
{
    _discounts.erase(_discounts.begin() + row);
    recalculate();
}

void
QuoteMaster::recalculate()
{
    static bool in_recalculate = false;
    if (in_recalculate) return;
    in_recalculate = true;

    ObjectCache cache(_db);

    // Get customer tax exempt
    Tax exempt;
    findTax(_customer.taxExemptId(), exempt);

    // Total up discountable, calculate line discount, and clear tx discounts
    fixed discountable = 0.0;
    unsigned int i;
    for (i = 0; i < _lines.size(); ++i) {
	ItemLine& line = _lines[i];
	if (line.item.id() == INVALID_ID && line.account.id() == INVALID_ID)
	    continue;

	// Calculate base price and tax
	line.ext_base = line.ext_price;
	if (line.include_deposit)
	    line.ext_base -= line.ext_deposit;
	if (line.include_tax && line.tax_id != INVALID_ID) {
	    Tax tax;
	    findTax(line.tax_id, tax);
	    line.ext_tax = _quasar->db()->calculateTaxOff(cache, tax,
							  line.ext_base);
	    line.ext_base -= line.ext_tax;
	}

	// Clear old discount information
	line.line_disc.total_amt = 0.0;
	line.tdisc_nums.clear();
	line.tdisc_amts.clear();
	if (!line.discountable) continue;

	// Calculate line discount
	DiscountLine& disc = line.line_disc;
	if (disc.discount_id != INVALID_ID) {
	    if (disc.method == Discount::PERCENT) {
		disc.total_amt = line.ext_base * disc.amount / 100.0;
	    } else {
		disc.total_amt = disc.amount * line.qty;
	    }
	    disc.total_amt.moneyRound();
	}

	discountable += line.ext_base - disc.total_amt;
    }

    // Calculate transaction discounts and distribute over items
    fixed remain_disc = discountable;
    for (i = 0; i < _discounts.size(); ++i) {
	DiscountLine& discount = _discounts[i];
	if (discount.discount_id == INVALID_ID) continue;

	// Calculate discount amount
	if (discount.method == Discount::PERCENT) {
	    discount.total_amt = remain_disc * discount.amount / 100.0;
	    discount.total_amt.moneyRound();
	} else {
	    discount.total_amt = discount.amount;
	    if (discount.total_amt > remain_disc)
		discount.total_amt = remain_disc;
	}
	_discs->setCellValue(i, 3, discount.total_amt);
	remain_disc -= discount.total_amt;

	// Distribute back over items
	fixed remain = discount.total_amt;
	int last = -1;
	for (unsigned int j = 0; j < _lines.size(); ++j) {
	    ItemLine& line = _lines[j];
	    if (line.item.id() == INVALID_ID && line.account.id() == INVALID_ID)
		continue;
	    if (!line.discountable) continue;

	    fixed line_amt = line.ext_base - line.line_disc.total_amt;
	    fixed alloc = line_amt * discount.total_amt / discountable;
	    alloc.moneyRound();

	    line.tdisc_nums.push_back(i);
	    line.tdisc_amts.push_back(alloc);

	    remain -= alloc;
	    last = j;
	}

	// If didn't divide out perfectly, add remaining to last line
	if (remain != 0.0 && last != -1) {
	    _lines[last].tdisc_amts[0] += remain;
	}
    }

    // Total items/discount/deposits and taxable
    fixed item_total = 0.0;
    fixed deposit_total = 0.0;
    fixed ldisc_total = 0.0;
    fixed tdisc_total = 0.0;
    _tax_info.clear();
    for (i = 0; i < _lines.size(); ++i) {
	ItemLine& line = _lines[i];
	if (line.item.id() == INVALID_ID && line.account.id() == INVALID_ID)
	    continue;

	fixed ldisc_amt = line.line_disc.total_amt;
	fixed tdisc_amt = 0.0;
	for (unsigned int j = 0; j < line.tdisc_amts.size(); ++j)
	    tdisc_amt += line.tdisc_amts[j];

	// Calculate base price and prepare tax info
	fixed base = line.ext_base - ldisc_amt - tdisc_amt;
	Tax tax;
	findTax(line.tax_id, tax);
	vector<Id> tax_ids;
	vector<fixed> tax_amts;

	// Calculate taxes
	if (line.include_tax && ldisc_amt == 0.0 && tdisc_amt == 0.0 &&
		line.tax_id == line.item_tax_id && exempt.id() == INVALID_ID) {
	    base = line.ext_price;
	    if (line.include_deposit)
		base -= line.ext_deposit;
	    line.ext_tax = _quasar->db()->calculateTaxOff(cache, tax, base,
							  tax_ids, tax_amts);
	    addTaxInc(base, tax_ids, tax_amts);
	    base -= line.ext_tax;
	} else {
	    line.ext_tax = _quasar->db()->calculateTaxOn(cache, tax, base,
							 exempt, tax_ids,
							 tax_amts);
	    addTax(base, tax_ids, tax_amts);
	}

	if (int(i) == _item_row)
	    updateItemInfo(i);

	item_total += line.ext_base;
	deposit_total += line.ext_deposit;
	ldisc_total += ldisc_amt;
	tdisc_total += tdisc_amt;
    }

    // Round taxes to two decimal places and total up
    _taxes->clear();
    fixed tax_total = 0.0;
    for (i = 0; i < _tax_info.size(); ++i) {
	Id tax_id = _tax_info[i].tax_id;

	// Check for tax exemptions
	if (exempt.id() != INVALID_ID) {
	    bool found = false;
	    if (tax_id == exempt.id()) {
		found = true;
	    } else if (exempt.isGroup()) {
		for (unsigned int j = 0; j < exempt.group_ids().size(); ++j) {
		    if (tax_id == exempt.group_ids()[j]) {
			found = true;
			break;
		    }
		}
	    }
	    if (found) {
		_tax_info[i].amount = 0.0;
		_tax_info[i].inc_amount = 0.0;
	    }
	}

	// NOTE: don't need to round inc_amount since its always returned
	// rounded from the tax calculation.
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

    // Update totals
    fixed total = item_total - ldisc_total - tdisc_total + tax_total +
	deposit_total;
    _item_amt->setFixed(item_total);
    _ldisc_amt->setFixed(ldisc_total);
    _tdisc_amt->setFixed(tdisc_total);
    _tax_amt->setFixed(tax_total);
    _deposit_amt->setFixed(deposit_total);
    _total_amt->setFixed(total);

    in_recalculate = false;
}

void
QuoteMaster::recalculatePrice(int row)
{
    ItemLine& line = _lines[row];
    if (line.account.id() == INVALID_ID && line.item.id() == INVALID_ID)
	return;

    // Get quantity and sign
    fixed qty = fabs(line.qty);
    fixed sign = line.qty.sign();

    // If the type is account, just recalculate ext_price
    if (_type->currentItem() == Quote::Account) {
	line.ext_price = line.price.calculate(qty) * sign;
	_items->setCellValue(row, 5, line.ext_price);
	return;
    }

    // Get existing prices use for quantity limit
    vector<Id> price_ids;
    vector<fixed> sold_qtys;
    for (unsigned int i = 0; i < _lines.size(); ++i) {
	const ItemLine& line = _lines[i];
	if (line.price_id == INVALID_ID) continue;
	if (int(i) == row) continue;
	if (line.qty.sign() != sign) continue;

	bool found = false;
	for (unsigned int j = 0; j < price_ids.size(); ++j) {
	    if (price_ids[j] == line.price_id) {
		found = true;
		sold_qtys[j] += fabs(line.qty);
		break;
	    }
	}
	if (!found) {
	    price_ids.push_back(line.price_id);
	    sold_qtys.push_back(line.qty * sign);
	}
    }

    vector<ItemPrice> prices;
    findPrices(line.item.id(), prices);
    QDate date = _date->getDate();
    Id store = _store->getId();

    ItemPrice price;
    fixed sell_price;
    fixed sell_qty;
    fixed sell_cost;
    fixed sell_deposit;
    _quasar->db()->itemBestPrice(line.item, prices, line.size, _customer,
				 store, date, qty, true, false, price,
				 sell_price, sell_qty, price_ids, sold_qtys);
    _quasar->db()->itemSellingCost(line.item, line.size, store, sell_qty,
				   sell_price, sell_cost);
    sell_deposit = line.item.deposit() * line.size_qty * sell_qty;

    if (sell_qty != qty) {
	QString message = tr("The quantity was changed from %1 to %2\n"
			     "to take advantage of a quantity limit price.")
	    .arg(qty.toString()).arg(sell_qty.toString());
	QMessageBox::warning(this, tr("Warning"), message);
    }

    line.qty = sell_qty * sign;
    line.ext_price = sell_price * sign;
    line.ext_deposit = sell_deposit * sign;
    line.discountable = price.isDiscountable();

    line.price_id = price.id();
    line.price_disc = 0.0;
    if (price.method() == ItemPrice::PRICE)
	line.price = price.price();
    else {
	fixed price = sell_price / sell_qty;
	price.moneyRound();
	line.price = Price(price);
    }

    _items->setCellValue(row, 3, line.qty);
    _items->setCellValue(row, 4, line.price);
    _items->setCellValue(row, 5, line.ext_price);

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
}

void
QuoteMaster::addTax(fixed taxable, vector<Id>& ids, vector<fixed>& amts)
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
QuoteMaster::addTaxInc(fixed taxable, vector<Id>& ids, vector<fixed>& amts)
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
QuoteMaster::findItem(Id item_id, Item& item)
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
    conditions.priceOnly = true;
    _quasar->db()->select(cache.prices, conditions);

    item = cache.item;
    _item_cache.push_back(cache);
    return true;
}

bool
QuoteMaster::findPrices(Id item_id, vector<ItemPrice>& prices)
{
    prices.clear();
    for (unsigned int i = 0; i < _item_cache.size(); ++i) {
	if (_item_cache[i].item.id() == item_id) {
	    prices = _item_cache[i].prices;
	    return true;
	}
    }
    return false;
}

bool
QuoteMaster::findTax(Id tax_id, Tax& tax)
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
QuoteMaster::findDiscount(Id discount_id, Discount& discount)
{
    for (unsigned int i = 0; i < _discount_cache.size(); ++i) {
	if (_discount_cache[i].id() == discount_id) {
	    discount = _discount_cache[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(discount_id, discount))
	return false;

    _discount_cache.push_back(discount);
    return true;
}

QuoteMaster::DiscountLine::DiscountLine()
    : discount_id(INVALID_ID), method(Discount::PERCENT), amount(0.0),
      account_id(INVALID_ID)
{
}

QuoteMaster::ItemLine::ItemLine()
    : tax_id(INVALID_ID), item_tax_id(INVALID_ID), include_tax(false),
      include_deposit(false), discountable(false)
{
}
