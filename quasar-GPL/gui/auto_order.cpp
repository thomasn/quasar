// $Id: auto_order.cpp,v 1.27 2005/06/22 15:48:09 bpepers Exp $
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

#include "auto_order.h"
#include "main_window.h"
#include "quasar_db.h"
#include "object_cache.h"
#include "order_master.h"
#include "item.h"
#include "item_select.h"
#include "vendor.h"
#include "order.h"
#include "order_template.h"
#include "group.h"
#include "date_popup.h"
#include "integer_edit.h"
#include "percent_edit.h"
#include "lookup_edit.h"
#include "vendor_lookup.h"
#include "store_lookup.h"
#include "dept_lookup.h"
#include "subdept_lookup.h"
#include "group_lookup.h"
#include "order_template_lookup.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qprogressdialog.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <algorithm>

AutoOrder::AutoOrder(MainWindow* main)
    : QuasarWindow(main, "AutoOrder")
{
    _helpSource = "auto_order.html";

    QFrame* frame = new QFrame(this);

    QLabel* vendorLabel = new QLabel(tr("Order for Vendor:"), frame);
    _vendor = new LookupEdit(new VendorLookup(main, this), frame);
    _vendor->setLength(30);
    vendorLabel->setBuddy(_vendor);

    QLabel* storeLabel = new QLabel(tr("Order for Store:"), frame);
    _store = new LookupEdit(new StoreLookup(main, this), frame);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    QGroupBox* select = new QGroupBox(tr("Item Selection"), frame);
    QGridLayout* selectGrid = new QGridLayout(select, 2, 3,
					      select->frameWidth() * 2);

    _itemSelect = new QRadioButton(tr("Item File:"), select);

    QLabel* deptLabel = new QLabel(tr("Department:"), select);
    _dept = new LookupEdit(new DeptLookup(_main, this), select);
    _dept->setLength(30);
    deptLabel->setBuddy(_dept);

    QLabel* subdeptLabel = new QLabel(tr("Subdepartment:"), select);
    _subdept = new LookupEdit(new SubdeptLookup(_main, this), select);
    _subdept->setLength(30);
    subdeptLabel->setBuddy(_subdept);

    QLabel* groupLabel = new QLabel(tr("Group:"), select);
    _group = new LookupEdit(new GroupLookup(_main, this, Group::ITEM), select);
    _group->setLength(30);
    groupLabel->setBuddy(_group);

    _templateSelect = new QRadioButton(tr("Order Template:"), select);

    QLabel* templateLabel = new QLabel(tr("Templete:"), select);
    _template = new LookupEdit(new OrderTemplateLookup(_main, this), select);
    _template->setLength(30);
    templateLabel->setBuddy(_template);

    QButtonGroup* buttonGroup1 = new QButtonGroup(this);
    buttonGroup1->hide();
    buttonGroup1->insert(_itemSelect);
    buttonGroup1->insert(_templateSelect);
    connect(buttonGroup1, SIGNAL(clicked(int)), SLOT(slotSelectChanged(int)));

    selectGrid->setSpacing(3);
    selectGrid->setMargin(3);
    selectGrid->setColStretch(3, 1);
    selectGrid->addRowSpacing(0, select->fontMetrics().height());
    selectGrid->addColSpacing(0, 20);
    selectGrid->addMultiCellWidget(_itemSelect, 1, 1, 0, 2, AlignLeft);
    selectGrid->addWidget(deptLabel, 2, 1);
    selectGrid->addWidget(_dept, 2, 2, AlignLeft | AlignVCenter);
    selectGrid->addWidget(subdeptLabel, 3, 1);
    selectGrid->addWidget(_subdept, 3, 2, AlignLeft | AlignVCenter);
    selectGrid->addWidget(groupLabel, 4, 1);
    selectGrid->addWidget(_group, 4, 2, AlignLeft | AlignVCenter);
    selectGrid->addMultiCellWidget(_templateSelect, 5, 5, 0, 2, AlignLeft);
    selectGrid->addWidget(templateLabel, 6, 1);
    selectGrid->addWidget(_template, 6, 2, AlignLeft | AlignVCenter);

    QGroupBox* method = new QGroupBox(tr("Calculation Method"), frame);
    QGridLayout* methodGrid = new QGridLayout(method, 2, 3,
					      method->frameWidth() * 2);

    _minMaxMethod = new QRadioButton(tr("Use Min/Max"), method);

    _salesMethod = new QRadioButton(tr("Use Sales History:"), method);

    QLabel* endDateLabel = new QLabel(tr("End Date:"), method);
    _endDate = new DatePopup(method);
    endDateLabel->setBuddy(_endDate);

    QLabel* historyLabel = new QLabel(tr("History of:"), method);
    _historyCnt = new IntegerEdit(method);
    _historyCnt->setLength(8, '9');
    _historyType = new QComboBox(method);
    _historyType->insertItem(tr("days"));
    _historyType->insertItem(tr("weeks"));
    _historyType->insertItem(tr("months"));
    historyLabel->setBuddy(_historyCnt);

    QLabel* percentLabel = new QLabel(tr("Sales Change:"), method);
    _percent = new PercentEdit(method);
    _percent->setLength(8, '9');
    _percentType = new QComboBox(method);
    _percentType->insertItem(tr("increase"));
    _percentType->insertItem(tr("decrease"));
    percentLabel->setBuddy(_percent);

    QLabel* orderLabel = new QLabel(tr("Order for:"), method);
    _orderCnt = new IntegerEdit(method);
    _orderCnt->setLength(8, '9');
    _orderType = new QComboBox(method);
    _orderType->insertItem(tr("days"));
    _orderType->insertItem(tr("weeks"));
    _orderType->insertItem(tr("months"));
    orderLabel->setBuddy(_orderCnt);

    QButtonGroup* buttonGroup2 = new QButtonGroup(this);
    buttonGroup2->hide();
    buttonGroup2->insert(_minMaxMethod);
    buttonGroup2->insert(_salesMethod);
    connect(buttonGroup2, SIGNAL(clicked(int)), SLOT(slotMethodChanged(int)));

    methodGrid->setSpacing(3);
    methodGrid->setMargin(3);
    methodGrid->setColStretch(4, 1);
    methodGrid->addRowSpacing(0, method->fontMetrics().height());
    methodGrid->addColSpacing(0, 20);
    methodGrid->addMultiCellWidget(_minMaxMethod, 1, 1, 0, 3, AlignLeft);
    methodGrid->addMultiCellWidget(_salesMethod, 2, 2, 0, 3, AlignLeft);
    methodGrid->addWidget(endDateLabel, 3, 1);
    methodGrid->addWidget(_endDate, 3, 2, AlignLeft | AlignVCenter);
    methodGrid->addWidget(historyLabel, 4, 1);
    methodGrid->addWidget(_historyCnt, 4, 2, AlignLeft | AlignVCenter);
    methodGrid->addWidget(_historyType, 4, 3, AlignLeft | AlignVCenter);
    methodGrid->addWidget(percentLabel, 5, 1);
    methodGrid->addWidget(_percent, 5, 2, AlignLeft | AlignVCenter);
    methodGrid->addWidget(_percentType, 5, 3, AlignLeft | AlignVCenter);
    methodGrid->addWidget(orderLabel, 6, 1);
    methodGrid->addWidget(_orderCnt, 6, 2, AlignLeft | AlignVCenter);
    methodGrid->addWidget(_orderType, 6, 3, AlignLeft | AlignVCenter);

    QFrame* buttons = new QFrame(frame);

    QPushButton* ok = new QPushButton(tr("&OK"), buttons);
    connect(ok, SIGNAL(clicked()), SLOT(slotOk()));

    QPushButton* cancel = new QPushButton(tr("&Cancel"), buttons);
    connect(cancel, SIGNAL(clicked()), SLOT(close()));

    ok->setMinimumSize(cancel->sizeHint());
    cancel->setMinimumSize(cancel->sizeHint());

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(ok, 0, 1, AlignRight | AlignVCenter);
    buttonGrid->addWidget(cancel, 0, 2, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->addWidget(vendorLabel, 0, 0, AlignLeft | AlignVCenter);
    grid->addWidget(_vendor, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(storeLabel, 1, 0, AlignLeft | AlignVCenter);
    grid->addWidget(_store, 1, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(select, 3, 3, 0, 1);
    grid->addMultiCellWidget(method, 4, 4, 0, 1);
    grid->addMultiCellWidget(buttons, 5, 5, 0, 1);

    _vendor->setFocus();
    _store->setId(_main->defaultStore());
    _itemSelect->setChecked(true);
    _minMaxMethod->setChecked(true);
    _endDate->setDate(QDate::currentDate());
    _historyCnt->setInt(0);
    _percent->setFixed(0.0);
    _orderCnt->setInt(0);
    slotSelectChanged(0);
    slotMethodChanged(0);

    setCentralWidget(frame);
    setCaption(tr("Auto Order"));
    finalize();
}

AutoOrder::~AutoOrder()
{
}

void
AutoOrder::slotSelectChanged(int index)
{
    switch (index) {
    case 0:
	_dept->setEnabled(true);
	_subdept->setEnabled(true);
	_group->setEnabled(true);
	_template->setEnabled(false);
	break;
    case 1:
	_dept->setEnabled(false);
	_subdept->setEnabled(false);
	_group->setEnabled(false);
	_template->setEnabled(true);
	break;
    }
}

void
AutoOrder::slotMethodChanged(int index)
{
    switch (index) {
    case 0:
	_endDate->setEnabled(false);
	_historyCnt->setEnabled(false);
	_historyType->setEnabled(false);
	_percent->setEnabled(false);
	_percentType->setEnabled(false);
	_orderCnt->setEnabled(false);
	_orderType->setEnabled(false);
	break;
    case 1:
	_endDate->setEnabled(true);
	_historyCnt->setEnabled(true);
	_historyType->setEnabled(true);
	_percent->setEnabled(true);
	_percentType->setEnabled(true);
	_orderCnt->setEnabled(true);
	_orderType->setEnabled(true);
	break;
    }
}

struct OrderInfo {
    Item item;
    QString number;
    QString size;
    fixed qty;
};

// Used to sort item's in number order
static bool operator<(const OrderInfo& lhs, const OrderInfo& rhs) {
    QString ln = QString("%1").arg(lhs.number.lower(), 20);
    QString rn = QString("%1").arg(rhs.number.lower(), 20);
    return ln < rn;
}

void
addTaxInc(Order& order, fixed taxable, vector<Id>& ids, vector<fixed>& amts)
{
    for (unsigned int i = 0; i < ids.size(); ++i) {
	Id tax_id = ids[i];
	fixed tax_amt = amts[i];

	bool found = false;
	for (unsigned int j = 0; j < order.taxes().size(); ++j) {
	    if (order.taxes()[j].tax_id == tax_id) {
		order.taxes()[j].inc_taxable += taxable;
		order.taxes()[j].inc_amount += tax_amt;
		found = true;
		break;
	    }
	}

	if (!found) {
	    OrderTax tax_line(tax_id, 0.0, 0.0, taxable, tax_amt);
	    order.taxes().push_back(tax_line);
	}
    }
}

void
addTax(Order& order, fixed taxable, vector<Id>& ids, vector<fixed>& amts)
{
    for (unsigned int i = 0; i < ids.size(); ++i) {
	Id tax_id = ids[i];
	fixed tax_amt = amts[i];

	bool found = false;
	for (unsigned int j = 0; j < order.taxes().size(); ++j) {
	    if (order.taxes()[j].tax_id == tax_id) {
		order.taxes()[j].taxable += taxable;
		order.taxes()[j].amount += tax_amt;
		found = true;
		break;
	    }
	}

	if (!found) {
	    OrderTax tax_line(tax_id, taxable, tax_amt);
	    order.taxes().push_back(tax_line);
	}
    }
}

void
AutoOrder::slotOk()
{
    Id vendor_id = _vendor->getId();
    Id store_id = _store->getId();
    QDate date = QDate::currentDate();

    if (vendor_id == INVALID_ID) {
	QMessageBox::critical(this, tr("Error"), tr("Vendor required"));
	_vendor->setFocus();
	return;
    }
    if (store_id == INVALID_ID) {
	QMessageBox::critical(this, tr("Error"), tr("Store required"));
	_store->setFocus();
	return;
    }
    if (_templateSelect->isChecked() && _template->getId() == INVALID_ID) {
	QMessageBox::critical(this, tr("Error"), tr("Template required"));
	_template->setFocus();
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    vector<OrderInfo> lines;
    vector<OrderCharge> charges;

    if (_itemSelect->isChecked()) {
	ItemSelect conditions;
	conditions.activeOnly = true;
	conditions.vendor_id = vendor_id;
	conditions.store_id = store_id;
	conditions.dept_id = _dept->getId();
	conditions.subdept_id = _subdept->getId();
	conditions.group_id = _group->getId();
	conditions.stockedOnly = true;
	conditions.purchasedOnly = true;
	vector<Item> items;
	_db->select(items, conditions);

	for (unsigned int i = 0; i < items.size(); ++i) {
	    const Item& item = items[i];
	    if (item.isDiscontinued()) continue;

	    QString size = item.purchaseSize();
	    QString number = item.orderNumber(vendor_id, size);
	    if (number.isEmpty()) number = item.number(size);
	    if (number.isEmpty()) number = item.number();

	    OrderInfo info;
	    info.item = item;
	    info.number = number;
	    info.size = size;
	    info.qty = 0.0;
	    lines.push_back(info);
	}

	std::sort(lines.begin(), lines.end());
    } else if (_templateSelect->isChecked()) {
	OrderTemplate temp;
	_db->lookup(_template->getId(), temp);

	// Bring over items
	for (unsigned int i = 0; i < temp.items().size(); ++i) {
	    Item item;
	    _db->lookup(temp.items()[i].item_id, item);
	    if (item.isDiscontinued()) continue;

	    QString number = temp.items()[i].number;
	    QString size = item.numberSize(number);
	    if (size.isEmpty()) size = item.purchaseSize();

	    OrderInfo info;
	    info.item = item;
	    info.number = number;
	    info.size = size;
	    info.qty = 0.0;
	    lines.push_back(info);
	}

	// Bring over charges
	for (unsigned int i = 0; i < temp.charges().size(); ++i) {
	    Id charge_id = temp.charges()[i].charge_id;
	    Id tax_id = temp.charges()[i].tax_id;
	    fixed amount = temp.charges()[i].amount;
	    bool internal = temp.charges()[i].internal;

	    OrderCharge info;
	    info.charge_id = charge_id;
	    info.tax_id = tax_id;
	    info.amount = amount;
	    info.internal = internal;
	    charges.push_back(info);
	}
    }

    for (unsigned int i = 0; i < lines.size(); ++i) {
	const Item& item = lines[i].item;
	QString size = lines[i].size;

	// Calculate order quantity
	fixed orderQty = calculateOrderQty(item);
	if (orderQty <= 0.0) continue;

	// Round up to next even number
	fixed sizeQty = item.sizeQty(size);
	fixed qty = (orderQty / sizeQty + .49).toInt();

	// Round up using order multiple
	fixed orderMultiple = item.orderMultiple(size);
	fixed mult = qty / orderMultiple;
	if (mult != mult.toInt())
	    qty = (mult.toInt() + 1) * orderMultiple;

	lines[i].qty = qty;
    }

    Vendor vendor;
    _db->lookup(vendor_id, vendor);

    Order order;
    order.setNumber("#");
    order.setVendorId(vendor_id);
    order.setVendorAddress(vendor.address());
    order.setStoreId(store_id);
    order.setTermsId(vendor.termsId());
    order.setDate(date);
    order.charges() = charges;

    // No vendor tax exempt yet but the code is here
    Tax exempt;

    ObjectCache cache(_db);

    // Add items
    fixed item_total = 0.0;
    fixed total_weight = 0.0;
    for (unsigned int i = 0; i < lines.size(); ++i) {
	const OrderInfo& info = lines[i];
	const Item& item = info.item;
	fixed ordered = info.qty;
	if (ordered == 0.0) continue;

	QString number = info.number;
	QString size = info.size;
	fixed size_qty = item.sizeQty(size);
	Price cost = item.cost(store_id, size);
	// TODO: check for promo costs
	fixed ext_cost = cost.calculate(ordered);
	fixed ext_deposit = item.deposit() * ordered * size_qty;
	fixed ext_base = ext_cost;
	if (item.costIncludesDeposit())
	    ext_base -= ext_deposit;

	// Calculate taxes
	fixed ext_tax = 0.0;
	if (item.purchaseTax() != INVALID_ID) {
	    Tax tax;
	    _db->lookup(item.purchaseTax(), tax);
	    vector<Id> tax_ids;
	    vector<fixed> tax_amts;

	    if (item.costIncludesTax()) {
		ext_tax = _db->calculateTaxOff(cache, tax, ext_base, tax_ids,
					       tax_amts);
		ext_base -= ext_tax;
		addTaxInc(order, ext_base, tax_ids, tax_amts);
	    } else {
		ext_tax = _db->calculateTaxOn(cache, tax, ext_base, Tax(),
					      tax_ids, tax_amts);
		addTax(order, ext_base, tax_ids, tax_amts);
	    }
	}

	OrderItem line;
	line.item_id = item.id();
	line.number = number;
	line.size = size;
	line.size_qty = size_qty;
	line.ordered = ordered;
	line.cost = cost;
	line.ext_cost = ext_cost;
	line.ext_base = ext_base;
	line.ext_deposit = ext_deposit;
	line.ext_tax = ext_tax;
	line.tax_id = item.purchaseTax();
	line.item_tax_id = item.purchaseTax();
	line.include_tax = item.costIncludesTax();
	line.include_deposit = item.costIncludesDeposit();
	order.items().push_back(line);

	item_total += ext_base;
	total_weight += item.weight(size) * ordered;;
    }

    // Check for no items
    if (order.items().size() == 0) {
	QApplication::restoreOverrideCursor();
	QString message = tr("No items to order");
	QMessageBox::information(this, tr("Information"), message);
	return;
    }

    // Process internal charges
    fixed icharge_total = 0.0;
    for (unsigned int i = 0; i < charges.size(); ++i) {
	OrderCharge& cline = charges[i];
	if (!cline.internal) continue;

	Charge charge;
	_db->lookup(cline.charge_id, charge);
	if (charge.id() == INVALID_ID) continue;

	// Check if should calculate
	fixed amount = cline.amount;
	if (charge.calculateMethod() != Charge::MANUAL) {
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
	    _db->lookup(cline.tax_id, tax);
	    vector<Id> tax_ids;
	    vector<fixed> tax_amts;

	    if (charge.includeTax()) {
		base -= _db->calculateTaxOff(cache, tax, amount, tax_ids,
					     tax_amts);
		addTaxInc(order, amount, tax_ids, tax_amts);
	    } else {
		_db->calculateTaxOn(cache, tax, amount, exempt, tax_ids,
				    tax_amts);
		addTax(order, amount, tax_ids, tax_amts);
	    }
	}

	cline.amount = amount;
	cline.base = base;
	icharge_total += cline.base;

	// Check if should allocate
	if (charge.allocateMethod() != Charge::NONE) {
	    int last = -1;
	    fixed remain = cline.base;
	    for (unsigned int j = 0; j < order.items().size(); ++j) {
		OrderItem& line = order.items()[j];
		if (line.item_id == INVALID_ID) continue;

		Item item;
		_db->lookup(line.item_id, item);

		fixed weight = item.weight(line.size) * line.ordered;
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
	    if (remain != 0.0 && last != -1)
		order.items()[last].int_charges += remain;
	}
    }

    // Process external charges
    fixed echarge_total = 0.0;
    for (unsigned int i = 0; i < charges.size(); ++i) {
	OrderCharge& cline = charges[i];
	if (cline.internal) continue;

	Charge charge;
	_db->lookup(cline.charge_id, charge);
	if (charge.id() == INVALID_ID) continue;

	// Check if should calculate
	fixed amount = cline.amount;
	if (charge.calculateMethod() != Charge::MANUAL) {
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
	    _db->lookup(cline.tax_id, tax);
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

	// Check if should allocate
	if (charge.allocateMethod() != Charge::NONE) {
	    int last = -1;
	    fixed remain = cline.base;
	    for (unsigned int j = 0; j < order.items().size(); ++j) {
		OrderItem& line = order.items()[j];
		if (line.item_id == INVALID_ID) continue;

		Item item;
		_db->lookup(line.item_id, item);

		fixed weight = item.weight(line.size) * line.ordered;
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
	    if (remain != 0.0 && last != -1)
		order.items()[last].ext_charges += remain;
	}
    }

    // Round taxes to two decimal places
    for (unsigned int i = 0; i < order.taxes().size(); ++i)
	order.taxes()[i].amount.moneyRound();

    if (!_db->create(order)) {
	QApplication::restoreOverrideCursor();
	QMessageBox::critical(this, tr("Error"), tr("Creating order failed"));
	return;
    }

    OrderMaster* master = new OrderMaster(_main, order.id());
    master->show();

    QApplication::restoreOverrideCursor();
    close();
}

fixed
AutoOrder::calculateOrderQty(const Item& item)
{
    Id store_id = _store->getId();
    QDate date = QDate::currentDate();
    fixed sellSizeQty = item.sizeQty(item.sellSize());
    fixed minQty = item.minQty(store_id) * sellSizeQty;

    if (_minMaxMethod->isChecked()) {
	fixed maxQty = item.maxQty(store_id) * sellSizeQty;
	if (minQty == 0.0 || maxQty == 0.0)
	    return 0.0;

	fixed onHand, onOrder, totalCost;
	_db->itemGeneral(item.id(), "", store_id, date, onHand, totalCost,
			 onOrder);
	if (onHand + onOrder >= minQty)
	    return 0.0;

	return maxQty - onHand - onOrder;
    }

    fixed onHand, onOrder, totalCost;
    _db->itemGeneral(item.id(), "", store_id, date, onHand, totalCost,
		     onOrder);

    int historyCnt = _historyCnt->getInt();
    if (_historyType->currentItem() == 1)
	historyCnt *= 7;
    if (_historyType->currentItem() == 2)
	historyCnt *= 30; // TODO: fix

    int orderCnt = _orderCnt->getInt();
    if (_orderType->currentItem() == 1)
	orderCnt *= 7;
    if (_orderType->currentItem() == 2)
	orderCnt *= 30; // TODO: fix

    QDate end = _endDate->getDate();
    QDate start = end.addDays(-historyCnt + 1);
    fixed soldQty, soldCost, soldPrice;
    _db->itemSold(item.id(), "", store_id, start, end, soldQty, soldCost,
		  soldPrice);

    // Modify sold qty based on percent change
    fixed percent = _percent->getFixed();
    if (percent != 0.0) {
	if (_percentType->currentItem() == 0) {
	    soldQty += soldQty * percent / 100.0;
	} else {
	    soldQty -= soldQty * percent / 100.0;
	}
    }

    // Calculate qty we want in the end and the qty to order to make it so
    fixed qty = (soldQty * orderCnt / historyCnt + .5).toInt();
    fixed orderQty = qty - onHand - onOrder;

    // If new quantity after ordering is less than min, order to min
    if (orderQty + onHand + onOrder < minQty && minQty != 0.0)
	orderQty = minQty - onHand - onOrder;

    return orderQty;
}
