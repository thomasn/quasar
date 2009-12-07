// $Id: slip_master.cpp,v 1.33 2005/03/13 23:13:45 bpepers Exp $
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

#include "slip_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "object_cache.h"
#include "slip_select.h"
#include "receive_select.h"
#include "order_select.h"
#include "receive.h"
#include "order.h"
#include "table.h"
#include "date_popup.h"
#include "integer_edit.h"
#include "double_edit.h"
#include "item_edit.h"
#include "vendor_lookup.h"
#include "item_lookup.h"
#include "store_lookup.h"
#include "store.h"
#include "combo_box.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>

SlipMaster::SlipMaster(MainWindow* main, Id slip_id)
    : DataWindow(main, "SlipMaster", slip_id), _item_row(-1)
{
    _helpSource = "slip_master.html";

    QPushButton* post = new QPushButton(tr("Post"), _buttons);
    connect(post, SIGNAL(clicked()), SLOT(slotPost()));

    QFrame* top = new QFrame(_frame);
    top->setFrameStyle(QFrame::Raised | QFrame::Panel);

    QLabel* numberLabel = new QLabel(tr("Slip #:"), top);
    _number = new LineEdit(top);
    _number->setLength(14, '9');
    numberLabel->setBuddy(_number);

    QLabel* vendorLabel = new QLabel(tr("Vendor:"), top);
    _vend_id = new LookupEdit(new VendorLookup(main, this), top);
    _vend_id->setLength(30);
    vendorLabel->setBuddy(_vend_id);
    connect(_vend_id, SIGNAL(validData()), SLOT(slotVendorChanged()));

    QLabel* orderNumLabel = new QLabel(tr("Order #:"), top);
    _orderNum = new LineEdit(top);
    _orderNum->setMinCharWidth(14, '9');
    orderNumLabel->setBuddy(_orderNum);

    QLabel* invoiceNumLabel = new QLabel(tr("Invoice #:"), top);
    _invoiceNum = new LineEdit(top);
    _invoiceNum->setLength(14, '9');
    invoiceNumLabel->setBuddy(_invoiceNum);

    QLabel* storeLabel = new QLabel(tr("Store:"), top);
    _store = new LookupEdit(new StoreLookup(_main, this), top);
    _store->setFocusPolicy(ClickFocus);
    _store->setLength(30);
    storeLabel->setBuddy(_store);
    connect(_store, SIGNAL(validData()), SLOT(slotStoreChanged()));

    QLabel* shipDateLabel = new QLabel(tr("Ship Date:"), top);
    _shipDate = new DatePopup(top);
    shipDateLabel->setBuddy(_shipDate);

    QLabel* waybillLabel = new QLabel(tr("Waybill:"), top);
    _waybill = new LineEdit(top);
    _waybill->setLength(20);
    waybillLabel->setBuddy(_waybill);

    QLabel* carrierLabel = new QLabel(tr("Carrier:"), top);
    _carrier = new LineEdit(top);
    _carrier->setLength(20);
    carrierLabel->setBuddy(_carrier);

    QLabel* piecesLabel = new QLabel(tr("# Pieces:"), top);
    _pieces = new IntegerEdit(top);
    piecesLabel->setBuddy(_pieces);

    QLabel* statusLabel = new QLabel(tr("Status:"), top);
    _status = new ComboBox(top);
    statusLabel->setBuddy(_status);

    QLabel* postDateLabel = new QLabel(tr("Posted On:"), top);
    _postDate = new DatePopup(top);
    _postDate->setFocusPolicy(NoFocus);
    postDateLabel->setBuddy(_postDate);

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(3);
    topGrid->setMargin(3);
    topGrid->setColStretch(2, 1);
    topGrid->addColSpacing(2, 20);
    topGrid->addWidget(numberLabel, 0, 0);
    topGrid->addWidget(_number, 0, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(vendorLabel, 1, 0);
    topGrid->addWidget(_vend_id, 1, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(orderNumLabel, 2, 0);
    topGrid->addWidget(_orderNum, 2, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(invoiceNumLabel, 3, 0);
    topGrid->addWidget(_invoiceNum, 3, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(storeLabel, 4, 0);
    topGrid->addWidget(_store, 4, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(shipDateLabel, 0, 3);
    topGrid->addWidget(_shipDate, 0, 4, AlignLeft | AlignVCenter);
    topGrid->addWidget(waybillLabel, 1, 3);
    topGrid->addWidget(_waybill, 1, 4, AlignLeft | AlignVCenter);
    topGrid->addWidget(carrierLabel, 2, 3);
    topGrid->addWidget(_carrier, 2, 4, AlignLeft | AlignVCenter);
    topGrid->addWidget(piecesLabel, 3, 3);
    topGrid->addWidget(_pieces, 3, 4, AlignLeft | AlignVCenter);
    topGrid->addWidget(statusLabel, 4, 3);
    topGrid->addWidget(_status, 4, 4, AlignLeft | AlignVCenter);
    topGrid->addWidget(postDateLabel, 5, 3);
    topGrid->addWidget(_postDate, 5, 4, AlignLeft | AlignVCenter);

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

    // Lookups
    _lookup = new ItemLookup(_main, this);
    _lookup->purchasedOnly = true;

    // Add columns
    new LookupColumn(_items, tr("Item Number"), 18, _lookup);
    new TextColumn(_items, tr("Description"), 30);
    new TextColumn(_items, tr("Size"), 8);
    new NumberColumn(_items, tr("Quantity"));

    LineEdit* descEdit = new LineEdit(_items);
    NumberEdit* qtyEdit = new DoubleEdit(_items);
    descEdit->setMaxLength(40);
    qtyEdit->setMaxLength(8);

    // Add editors
    _size = new ComboBox(false, _items);
    new LookupEditor(_items, 0, new ItemEdit(_lookup, _items));
    new LineEditor(_items, 1, descEdit);
    new ComboEditor(_items, 2, _size);
    new NumberEditor(_items, 3, qtyEdit);

    QGridLayout* midGrid = new QGridLayout(mid);
    midGrid->setSpacing(3);
    midGrid->setMargin(3);
    midGrid->setColStretch(0, 1);
    midGrid->addWidget(_items, 0, 0);

    QFrame* totals = new QFrame(_frame);

    QLabel* itemCntLabel = new QLabel(tr("Item Count:"), totals);
    _item_cnt = new DoubleEdit(totals);
    _item_cnt->setFocusPolicy(NoFocus);

    QGridLayout* totalGrid = new QGridLayout(totals);
    totalGrid->setSpacing(3);
    totalGrid->setMargin(3);
    totalGrid->setColStretch(0, 1);
    totalGrid->setRowStretch(1, 1);
    totalGrid->addWidget(itemCntLabel, 0, 1, AlignRight | AlignVCenter);
    totalGrid->addWidget(_item_cnt, 0, 2, AlignLeft | AlignVCenter);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(1, 1);
    grid->addWidget(top, 0, 0);
    grid->addWidget(mid, 1, 0);
    grid->addWidget(totals, 2, 0);

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    _status->insertItem("Pending");
    _status->insertItem("Received");
    _status->insertItem("Posted");

    setCaption(tr("Packing Slip Master"));
    finalize();
}

SlipMaster::~SlipMaster()
{
}

void
SlipMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _number;
}

void
SlipMaster::newItem()
{
    Slip blank;
    _orig = blank;
    _orig.setNumber("#");
    _orig.setShipDate(QDate::currentDate());
    _orig.setStoreId(_quasar->defaultStore());
    _orig.setNumPieces(1);

    _curr = _orig;
    _firstField = _number;
}

void
SlipMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _curr.setNumber("#");
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
}

bool
SlipMaster::fileItem()
{
    if (_orig.status() != "Posted" && _curr.status() == "Posted") {
	QMessageBox::critical(this, tr("Error"), "Can't set status to posted");
	return false;
    }
    if (_orig.status() == "Posted" && _curr.status() != "Posted") {
	QMessageBox::critical(this, tr("Error"), "Can't change status once posted");
	return false;
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
SlipMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
SlipMaster::restoreItem()
{
    _curr = _orig;
}

void
SlipMaster::cloneItem()
{
    SlipMaster* clone = new SlipMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
SlipMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
SlipMaster::dataToWidget()
{
    _number->setText(_curr.number());
    _vend_id->setId(_curr.vendorId());
    _orderNum->setText(_curr.orders().join(","));
    _invoiceNum->setText(_curr.invoiceNumber());
    _store->setId(_curr.storeId());
    _shipDate->setDate(_curr.shipDate());
    _waybill->setText(_curr.waybill());
    _carrier->setText(_curr.carrier());
    _pieces->setFixed(_curr.numPieces());
    _status->setCurrentItem(_curr.status());
    _postDate->setDate(_curr.postDate());
    _inactive->setChecked(!_curr.isActive());

    _lookup->store_id = _curr.storeId();

    // Clear the caches
    _item_cache.clear();

    // Load the items
    _items->setUpdatesEnabled(false);
    _items->clear();
    for (unsigned int i = 0; i < _curr.items().size(); ++i) {
	const SlipItem& line = _curr.items()[i];

	Item item;
	findItem(line.item_id, item);

	VectorRow* row = new VectorRow(_items->columns());
	row->setValue(0, Plu(line.item_id, line.number));
	row->setValue(1, item.description());
	row->setValue(2, line.size);
	row->setValue(3, line.quantity);
	_items->appendRow(row);
    }
    _items->appendRow(new VectorRow(_items->columns()));
    _items->setUpdatesEnabled(true);

    updateItemInfo(0);
    recalculate();
}

// Set the data object from the widgets.
void
SlipMaster::widgetToData()
{
    _curr.setNumber(_number->text());
    _curr.setVendorId(_vend_id->getId());
    _curr.orders() = QStringList::split(",", _orderNum->text());
    _curr.setInvoiceNumber(_invoiceNum->text());
    _curr.setStoreId(_store->getId());
    _curr.setShipDate(_shipDate->getDate());
    _curr.setWaybill(_waybill->text());
    _curr.setCarrier(_carrier->text());
    _curr.setNumPieces(_pieces->getInt());
    _curr.setStatus(_status->currentText());
    _curr.setPostDate(_postDate->getDate());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    // Process all the items
    _curr.items().clear();
    for (int row = 0; row < _items->rows(); ++row) {
        Plu plu = _items->cellValue(row, 0).toPlu();
	QString size = _items->cellValue(row, 2).toString();
	fixed quantity = _items->cellValue(row, 3).toFixed();
	if (plu.itemId() == INVALID_ID && plu.number().isEmpty()) continue;

	Item item;
	findItem(plu.itemId(), item);

	SlipItem line;
	line.item_id = plu.itemId();
	line.number = plu.number();
	line.size = size;
	line.size_qty = item.sizeQty(size);
	line.quantity = quantity;
	_curr.items().push_back(line);
    }
}

void
SlipMaster::slotVendorChanged()
{
    Vendor vendor;
    _quasar->db()->lookup(_vend_id->getId(), vendor);
    _vendor = vendor;
}

void
SlipMaster::slotStoreChanged()
{
    // Check items
    if (_items->rows() > 0) {
	for (int row = 0; row < _items->rows(); ++row) {
	    Plu plu = _items->cellValue(row, 0).toPlu();
	    if (plu.itemId() == INVALID_ID && plu.number().isEmpty())
		continue;

	    Item item;
	    findItem(plu.itemId(), item);

	    // Check that item is stocked in new store
	    if (!item.stocked(_store->getId())) {
		QString message = tr("Item #%1 is not stocked in the new\n"
				     "store and will be removed from the "
				     "slip.").arg(plu.number());
		QMessageBox::warning(this, tr("Warning"), message);
		_items->deleteRow(row);
		--row;
		continue;
	    }
	}
	if (_items->currentRow() != -1)
	    updateItemInfo(_items->currentRow());
    }

    _lookup->store_id = _store->getId();
}

void
SlipMaster::slotPost()
{
    if (!saveItem(true)) return;

    if (_curr.status() == "Posted") {
	QMessageBox::critical(this, tr("Error"), "Slip is already posted");
	return;
    }

    if (_curr.status() == "Pending") {
	QMessageBox::critical(this, tr("Error"), "Slip is pending");
	return;
    }

    int choice = QMessageBox::warning(this, tr("Post Packing Slip"),
			tr("Are you sure you wish to post the packing\n"
			   "slip and create a vendor invoice?"),
			QMessageBox::Yes, QMessageBox::No);
    if (choice != QMessageBox::Yes)
	return;

    bool create;
    if (!postSlip(_quasar, _curr, create)) {
	while (true) {
	    QString* message = _db->nextError();
	    if (message == NULL) break;
	    QMessageBox::critical(this,tr("Error"),"Post failed: " + *message);
	    delete message;
	}
	return;
    }

    QString message;
    if (create)
	message = tr("Created vendor invoice #%1").arg(_curr.invoiceNumber());
    else
	message = tr("Updated vendor invoice #%1").arg(_curr.invoiceNumber());
    QMessageBox::information(this, tr("Post complete"), message);

    slotCancel();
}

bool
SlipMaster::postSlip(QuasarClient* quasar, Slip& slip, bool& create)
{
    Slip origSlip = slip;

    // Look for existing invoice
    Receive invoice, orig;
    if (!slip.invoiceNumber().isEmpty()) {
	ReceiveSelect conditions;
	conditions.vendor_id = slip.vendorId();
	conditions.number = slip.invoiceNumber();

	vector<Receive> invoices;
	quasar->db()->select(invoices, conditions);

	if (invoices.size() > 0)
	    invoice = invoices[0];
    }

    // Look for existing orders
    vector<Order> orders;
    vector<Id> order_ids;
    for (unsigned int i = 0; i < slip.orders().size(); ++i) {
	Order order;
	if (quasar->db()->lookup(slip.orders()[i], order)) {
	    orders.push_back(order);
	    order_ids.push_back(order.id());
	}
    }

    Vendor vendor;
    quasar->db()->lookup(slip.vendorId(), vendor);
    Id store_id = slip.storeId();

    // Initialize new invoice if not using old one
    if (invoice.id() == INVALID_ID) {
	invoice.setNumber(slip.invoiceNumber());
	invoice.setPostDate(QDate::currentDate());
	invoice.setPostTime(QTime::currentTime());
	invoice.setStoreId(store_id);
	invoice.setStationId(quasar->defaultStation());
	invoice.setEmployeeId(quasar->defaultEmployee());
	invoice.setVendorId(slip.vendorId());
	invoice.setVendorAddress(vendor.address());
	invoice.setInvoiceDate(QDate::currentDate());
	invoice.orders() = order_ids;

	if (invoice.number().isEmpty())
	    invoice.setNumber("#");
    }

    // Add items from packing slip to invoice
    orig = invoice;
    for (unsigned int i = 0; i < slip.items().size(); ++i) {
	const SlipItem& line1 = slip.items()[i];

	Item item;
	quasar->db()->lookup(line1.item_id, item);

	// Cost in vendor invoice comes from the slip, the purchase order,
	// or the vendor replacement cost
	fixed ext_cost = line1.ext_cost;
	if (ext_cost == 0.0) {
	    bool found = false;
	    for (unsigned int j = 0; j < orders.size(); ++j) {
		const Order& order = orders[j];
		for (unsigned int k = 0; k < order.items().size(); ++k) {
		    const OrderItem& line = order.items()[k];
		    if (line.item_id != line1.item_id) continue;
		    if (line.ordered == 0) continue;

		    ext_cost = line.ext_cost * line1.quantity / line.ordered;
		    ext_cost.moneyRound();
		    found = true;
		    break;
		}

		if (found) break;
	    }

	    if (!found) {
		ItemPrice regular;
		regular.setPrice(item.cost(store_id, line1.size));
		ext_cost = regular.calculate(line1.quantity);
	    }
	}

	ReceiveItem line2;
	line2.item_id = line1.item_id;
	line2.number = line1.number;
	line2.size = line1.size;
	line2.size_qty = line1.size_qty;
	line2.quantity = line1.quantity;
	line2.voided = false;
	line2.cost = Price(ext_cost / line1.quantity);
	line2.ext_deposit = item.deposit() * line1.size_qty * line1.quantity;
	line2.ext_cost = ext_cost;
	line2.tax_id = item.purchaseTax();
	line2.item_tax_id = item.purchaseTax();
	line2.include_tax = item.costIncludesTax();
	line2.include_deposit = item.costIncludesDeposit();
	invoice.items().push_back(line2);
    }

    ObjectCache cache(quasar->db());
    quasar->db()->prepare(invoice, cache);

    // Create new invoice or update existing one
    create = (invoice.id() == INVALID_ID);
    if (create) {
	if (!quasar->db()->create(invoice))
	    return false;
    } else {
	if (!quasar->db()->update(orig, invoice))
	    return false;
    }

    // Set status and posted date
    slip.setStatus("Posted");
    slip.setPostDate(QDate::currentDate());
    if (!quasar->db()->update(origSlip, slip))
	return false;

    return true;
}

void
SlipMaster::updateItemInfo(int row)
{
    _item_row = row;
}

void
SlipMaster::itemCellMoved(int row, int)
{
    if (row == _items->currentRow()) return;
    _size->clear();
    if (row == -1) return;

    Id item_id = _items->cellValue(row, 0).toId();
    Item item;
    if (!findItem(item_id, item))
	return;

    QString size = _items->cellValue(row, 2).toString();
    for (unsigned int i = 0; i < item.sizes().size(); ++i) {
	_size->insertItem(item.sizes()[i].name);
	if (item.sizes()[i].name == size)
	    _size->setCurrentItem(_size->count() - 1);
    }
}

void
SlipMaster::itemCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _items->rows() - 1 && col == 0) {
	Id id = _items->cellValue(row, col).toId();
	if (id != INVALID_ID)
	    _items->appendRow(new VectorRow(_items->columns()));
    }

    // Check for other changes
    Plu plu;
    Item item;
    switch (col) {
    case 0: // item_id
	plu = _items->cellValue(row, 0).toPlu();
	if (findItem(plu.itemId(), item)) {
	    QString number = plu.number();
	    QString size = item.numberSize(number);
	    if (size.isEmpty()) size = item.purchaseSize();

	    _size->clear();
	    for (unsigned int i = 0; i < item.sizes().size(); ++i) {
		_size->insertItem(item.sizes()[i].name);
		if (item.sizes()[i].name == size)
		    _size->setCurrentItem(_size->count() - 1);
	    }

	    _items->setCellValue(row, 1, item.description());
	    _items->setCellValue(row, 2, size);
	    _items->setCellValue(row, 3, 1.0);
	} else {
	    if (row != _items->rows() - 1) {
		_items->setCellValue(row, 1, tr("**Unknown Item**"));
	    }
	}
	break;
    case 2: // size
	plu = _items->cellValue(row, 0).toPlu();
	if (findItem(plu.itemId(), item)) {
	    QString size = _items->cellValue(row, 2).toString();

	    fixed on_hand, total_cost, on_order;
	    Id store_id = _store->getId();
	    QDate date = QDate::currentDate();
	    _quasar->db()->itemGeneral(item.id(), "", store_id, date,
				       on_hand, total_cost, on_order);

	    _items->setCellValue(row, 3, on_hand / item.sizeQty(size));
	}
	break;
    }

    recalculate();
    updateItemInfo(row);
}

void
SlipMaster::itemFocusNext(bool& leave, int&, int& newCol, int type)
{
    int row = _items->currentRow();
    int col = _items->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id id = _items->cellValue(row, col).toId();
	if (id == INVALID_ID && row == _items->rows() - 1) {
	    leave = true;
	} else if (id == INVALID_ID) {
	    newCol = 0;
	} else {
	    newCol = 3;
	}
    } else if (type == Table::MovePrev && col == 2) {
	newCol = 0;
    }
}

void
SlipMaster::recalculate()
{
    static bool in_recalculate = false;
    if (in_recalculate) return;
    in_recalculate = true;

    // Calculate item count
    fixed item_cnt = 0.0;
    for (int row = 0; row < _items->rows(); ++row) {
        Plu plu = _items->cellValue(row, 0).toPlu();
	fixed quantity = _items->cellValue(row, 3).toFixed();
	if (plu.itemId() == INVALID_ID && plu.number().isEmpty()) continue;

	item_cnt += quantity;
    }
    _item_cnt->setFixed(item_cnt);

    in_recalculate = false;
}

bool
SlipMaster::findItem(Id item_id, Item& item)
{
    for (unsigned int i = 0; i < _item_cache.size(); ++i) {
	if (_item_cache[i].id() == item_id) {
	    item = _item_cache[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(item_id, item))
	return false;

    _item_cache.push_back(item);
    return true;
}
