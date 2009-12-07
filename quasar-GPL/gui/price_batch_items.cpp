// $Id: price_batch_items.cpp,v 1.7 2005/05/13 18:57:56 bpepers Exp $
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

#include "price_batch_items.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "object_cache.h"
#include "item_select.h"
#include "order.h"
#include "receive.h"
#include "lookup_edit.h"
#include "dept_lookup.h"
#include "subdept_lookup.h"
#include "group_lookup.h"
#include "location_lookup.h"
#include "order_lookup.h"
#include "receive_lookup.h"
#include "group.h"
#include "tax.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <algorithm>

PriceBatchItems::PriceBatchItems(QWidget* parent, MainWindow* main,Id store_id)
    : QDialog(parent, "PriceBatchItems", true), _store_id(store_id),
      _quasar(main->quasar())
{
    setCaption(tr("Add Items"));

    _useItems = new QRadioButton(tr("From Item File:"), this);
    _useOrder = new QRadioButton(tr("From Purchase Order:"), this);
    _useInvoice = new QRadioButton(tr("From Vendor Invoice:"), this);

    QButtonGroup* group = new QButtonGroup(this);
    group->hide();
    group->insert(_useItems);
    group->insert(_useOrder);
    group->insert(_useInvoice);
    connect(group, SIGNAL(clicked(int)), SLOT(slotTypeChanged(int)));

    QLabel* deptLabel = new QLabel(tr("Department:"), this);
    DeptLookup* deptLookup = new DeptLookup(main, this);
    _dept = new LookupEdit(deptLookup, this);

    QLabel* subdeptLabel = new QLabel(tr("Subdepartment:"), this);
    SubdeptLookup* subdeptLookup = new SubdeptLookup(main, this);
    _subdept = new LookupEdit(subdeptLookup, this);

    QLabel* groupLabel = new QLabel(tr("Group:"), this);
    GroupLookup* groupLookup = new GroupLookup(main, this);
    groupLookup->type->setCurrentItem(Group::typeName(Group::ITEM));
    _group = new LookupEdit(groupLookup, this);

    QLabel* locationLabel = new QLabel(tr("Location:"), this);
    LocationLookup* locationLookup = new LocationLookup(main, this);
    locationLookup->store_id = store_id;
    _location = new LookupEdit(locationLookup, this);

    QLabel* orderLabel = new QLabel(tr("Purchase Order:"), this);
    OrderLookup* orderLookup = new OrderLookup(main, this);
    orderLookup->store_id = store_id;
    _order = new LookupEdit(orderLookup, this);

    QLabel* invoiceLabel = new QLabel(tr("Vendor Invoice:"), this);
    ReceiveLookup* invoiceLookup = new ReceiveLookup(main, this);
    invoiceLookup->store_id = store_id;
    _invoice = new LookupEdit(invoiceLookup, this);

    QFrame* buttons = new QFrame(this);
    QPushButton* ok = new QPushButton(tr("OK"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);

    ok->setMinimumSize(cancel->sizeHint());
    cancel->setMinimumSize(cancel->sizeHint());
    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    ok->setDefault(true);

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(3);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(ok, 0, 1);
    buttonGrid->addWidget(cancel, 0, 2);

    QGridLayout* grid = new QGridLayout(this);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->addColSpacing(0, 30);
    grid->addMultiCellWidget(_useItems, 0, 0, 0, 2, AlignLeft | AlignVCenter);
    grid->addWidget(deptLabel, 1, 1);
    grid->addWidget(_dept, 1, 2);
    grid->addWidget(subdeptLabel, 2, 1);
    grid->addWidget(_subdept, 2, 2);
    grid->addWidget(groupLabel, 1, 4);
    grid->addWidget(_group, 1, 5);
    grid->addWidget(locationLabel, 2, 4);
    grid->addWidget(_location, 2, 5);
    grid->addMultiCellWidget(_useOrder, 3, 3, 0, 2, AlignLeft | AlignVCenter);
    grid->addWidget(orderLabel, 4, 1);
    grid->addWidget(_order, 4, 2);
    grid->addMultiCellWidget(_useInvoice, 5, 5, 0, 2, AlignLeft|AlignVCenter);
    grid->addWidget(invoiceLabel, 6, 1);
    grid->addWidget(_invoice, 6, 2);
    grid->addMultiCellWidget(buttons, 7, 7, 0, 5);

    _useItems->setChecked(true);
    slotTypeChanged(0);
}

PriceBatchItems::~PriceBatchItems()
{
}

void
PriceBatchItems::slotTypeChanged(int index)
{
    _dept->setEnabled(index == 0);
    _subdept->setEnabled(index == 0);
    _group->setEnabled(index == 0);
    _location->setEnabled(index == 0);
    _order->setEnabled(index == 1);
    _invoice->setEnabled(index == 2);
}

void
PriceBatchItems::accept()
{
    if (_useOrder->isChecked() && _order->getId() == INVALID_ID) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), tr("An order is required"));
	_order->setFocus();
	return;
    }

    if (_useInvoice->isChecked() && _invoice->getId() == INVALID_ID) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), tr("An invoice is required"));
	_invoice->setFocus();
	return;
    }

    QDialog::accept();
}

// Used to sort item's in number order
static bool operator<(const Item& lhs, const Item& rhs) {
    QString ln = QString("%1").arg(lhs.number().lower(), 20);
    QString rn = QString("%1").arg(rhs.number().lower(), 20);
    return ln < rn;
}

void
PriceBatchItems::getItems(vector<Item>& items, vector<QString>& numbers,
			  vector<QString>& descs, vector<QString>& sizes,
			  vector<fixed>& costs, vector<Price>& prices)
{
    items.clear();
    numbers.clear();
    descs.clear();
    sizes.clear();
    costs.clear();
    prices.clear();

    ObjectCache cache(_quasar->db());

    if (_useItems->isChecked()) {
	ItemSelect conditions;
	conditions.dept_id = _dept->getId();
	conditions.subdept_id = _subdept->getId();
	conditions.group_id = _group->getId();
	conditions.location_id = _location->getId();
	conditions.soldOnly = true;
	conditions.stockedOnly = true;
	conditions.activeOnly = true;
	conditions.store_id = _store_id;

	_quasar->db()->select(items, conditions);

	// Sort by number
	std::sort(items.begin(), items.end());

	for (unsigned int i = 0; i < items.size(); ++i) {
	    const Item& item = items[i];
	    QString size = item.sellSize();
	    fixed cost = item.cost(_store_id, size).calculate(1.0);
	    Price price = item.price(_store_id, size);

	    if (cost == 0) {
		Price purch_cost = item.cost(_store_id, item.purchaseSize());
		fixed purch_qty = item.sizeQty(item.purchaseSize());
		fixed size_qty = item.sizeQty(size);
		cost = purch_cost.calculate(1.0) * size_qty / purch_qty;
	    }

	    // Remove deposit/tax included in cost
	    if (item.costIncludesDeposit() && cost != 0.0)
		cost -= item.deposit() * item.sizeQty(size);
	    if (item.costIncludesTax() && cost != 0.0) {
		Tax purchaseTax;
		cache.findTax(item.purchaseTax(), purchaseTax);
		cost -= _quasar->db()->calculateTaxOff(cache, purchaseTax,
						       cost);
	    }
	    if (item.costDiscount() != 0.0) {
		fixed discount = cost * item.costDiscount() / 100.0;
		discount.moneyRound();
		cost -= discount;
	    }

	    numbers.push_back(item.number());
	    descs.push_back(item.description());
	    sizes.push_back(size);
	    costs.push_back(cost);
	    prices.push_back(price);
	}
    } else if (_useOrder->isChecked()) {
	Order order;
	_quasar->db()->lookup(_order->getId(), order);

	for (unsigned int i = 0; i < order.items().size(); ++i) {
	    const OrderItem& line = order.items()[i];
	    Id item_id = line.item_id;
	    QString number = line.number;
	    QString description = line.description;
	    QString orderSize = line.size;
	    fixed ext_cost = line.ext_base + line.int_charges+line.ext_charges;
	    fixed qty = line.ordered;
	    if (qty <= 0.0) continue;

	    Item item;
	    cache.findItem(item_id, item);
	    QString size = item.sellSize();
	    Price price = item.price(_store_id, size);

	    fixed size_qty = item.sizeQty(orderSize);
	    fixed cost = ext_cost / qty / size_qty * item.sizeQty(size);

	    if (description.isEmpty())
		description = item.description();

	    items.push_back(item);
	    numbers.push_back(item.number());
	    descs.push_back(description);
	    sizes.push_back(size);
	    costs.push_back(cost);
	    prices.push_back(price);
	}
    } else {
	Receive receive;
	_quasar->db()->lookup(_invoice->getId(), receive);

	for (unsigned int i = 0; i < receive.items().size(); ++i) {
	    const ReceiveItem& line = receive.items()[i];
	    Id item_id = line.item_id;
	    QString number = line.number;
	    QString description = line.description;
	    QString invoiceSize = line.size;
	    fixed ext_cost = line.inv_cost;
	    fixed qty = line.quantity;
	    if (qty <= 0.0) continue;

	    Item item;
	    cache.findItem(item_id, item);
	    QString size = item.sellSize();
	    Price price = item.price(_store_id, size);

	    fixed size_qty = item.sizeQty(size);
	    fixed cost = ext_cost / qty / size_qty * item.sizeQty(size);

	    if (description.isEmpty())
		description = item.description();

	    items.push_back(item);
	    numbers.push_back(item.number());
	    descs.push_back(description);
	    sizes.push_back(size);
	    costs.push_back(cost);
	    prices.push_back(price);
	}
    }
}
