// $Id: item_adjustment.cpp,v 1.59 2005/04/01 22:13:37 bpepers Exp $
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

#include "item_adjustment.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "gltx_frame.h"
#include "table.h"
#include "double_edit.h"
#include "money_edit.h"
#include "item_edit.h"
#include "item_lookup.h"
#include "account_lookup.h"
#include "adjust_reason_lookup.h"
#include "item.h"
#include "item_price.h"
#include "adjust_reason.h"
#include "report_defn.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qvbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpair.h>

typedef QPair<Id, fixed> InfoPair;

ItemAdjustment::ItemAdjustment(MainWindow* main, Id adjustment_id)
    : DataWindow(main, "ItemAdjustment", adjustment_id)
{
    _helpSource = "item_adjustment.html";

    // Search button
    QPushButton* search = new QPushButton(tr("Search"), _buttons);
    connect(search, SIGNAL(clicked()), SLOT(slotSearch()));

    // Print button
    QPushButton* print = new QPushButton(tr("Print"), _buttons);
    connect(print, SIGNAL(clicked()), SLOT(slotPrint()));

    // Get the company for deposit info
    _quasar->db()->lookup(_company);

    // Create widgets
    _gltxFrame = new GltxFrame(main, tr("Adjustment No."), _frame);
    connect(_gltxFrame->store, SIGNAL(validData()), SLOT(slotStoreChanged()));

    _items = new Table(_frame);
    _items->setVScrollBarMode(QScrollView::AlwaysOn);
    _items->setLeftMargin(fontMetrics().width("99999"));
    _items->setDisplayRows(6);
    connect(_items, SIGNAL(cellMoved(int,int)), SLOT(cellMoved(int,int)));
    connect(_items, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(cellChanged(int,int,Variant)));
    connect(_items, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(focusNext(bool&,int&,int&,int)));
    connect(_items, SIGNAL(rowInserted(int)), SLOT(rowInserted(int)));
    connect(_items, SIGNAL(rowDeleted(int)), SLOT(rowDeleted(int)));

    // Lookups
    _lookup = new ItemLookup(_main, this);
    _lookup->inventoriedOnly = true;

    // Add columns
    new LookupColumn(_items, tr("Item Number"), 18, _lookup);
    new TextColumn(_items, tr("Description"), 20);
    new TextColumn(_items, tr("Size"), 10);
    new NumberColumn(_items, tr("On Hand"), 6);
    new NumberColumn(_items, tr("Quantity"), 6);
    new MoneyColumn(_items, tr("Amount"));
    if (_company.depositAccount() != INVALID_ID) {
	new MoneyColumn(_items, tr("Deposit"), 4);
	new MoneyColumn(_items, tr("Total"));
    }

    // Add editors
    _size = new QComboBox(_items);
    new LookupEditor(_items, 0, new ItemEdit(_lookup, _items));
    new ComboEditor(_items, 2, _size);
    new NumberEditor(_items, 4, new DoubleEdit(_items));
    new NumberEditor(_items, 5, new MoneyEdit(_items));
    if (_company.depositAccount() != INVALID_ID) {
	new NumberEditor(_items, 6, new MoneyEdit(_items));
	new NumberEditor(_items, 7, new MoneyEdit(_items));
    }

    QLabel* reasonLabel = new QLabel(tr("Reason:"), _frame);
    AdjustReasonLookup* lookup = new AdjustReasonLookup(main, this);
    _reason = new LookupEdit(lookup, _frame);
    _reason->setLength(30);
    reasonLabel->setBuddy(_reason);
    connect(_reason, SIGNAL(validData()), SLOT(slotReasonChanged()));

    QLabel* accountLabel = new QLabel(tr("Expense Account:"), _frame);
    AccountLookup* acctLookup = new AccountLookup(main, this);
    _account = new LookupEdit(acctLookup, _frame);
    _account->setLength(30);
    accountLabel->setBuddy(_account);

    QLabel* totalLabel = new QLabel(tr("Expense Amount:"), _frame);
    _total = new MoneyEdit(_frame);
    _total->setLength(14);
    _total->setFocusPolicy(NoFocus);
    totalLabel->setBuddy(_total);

    _inactive->setText(tr("Voided?"));

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(2, 1);
    grid->setRowStretch(1, 1);
    grid->addMultiCellWidget(_gltxFrame, 0, 0, 0, 2);
    grid->addMultiCellWidget(_items, 1, 1, 0, 2);
    grid->addWidget(reasonLabel, 2, 0);
    grid->addWidget(_reason, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(accountLabel, 3, 0);
    grid->addWidget(_account, 3, 1, AlignLeft | AlignVCenter);
    grid->addWidget(totalLabel, 4, 0);
    grid->addWidget(_total, 4, 1, AlignLeft | AlignVCenter);

    setCaption(tr("Item Adjustment"));
    finalize();
}

ItemAdjustment::~ItemAdjustment()
{
}

void
ItemAdjustment::oldItem()
{
    _quasar->db()->lookup(_id, _orig);
    _curr = _orig;
    _firstField = _gltxFrame->firstField();
}

void
ItemAdjustment::newItem()
{
    ItemAdjust blank;
    _orig = blank;
    _gltxFrame->defaultData(_orig);

    _curr = _orig;
    _firstField = _gltxFrame->firstField();
}

void
ItemAdjustment::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _gltxFrame->cloneData(_curr);
    dataToWidget();
}

bool
ItemAdjustment::fileItem()
{
    if (_curr.number() != "#")
	if (checkGltxUsed(_curr.number(), DataObject::ITEM_ADJUST, _curr.id()))
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
ItemAdjustment::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
ItemAdjustment::restoreItem()
{
    _curr = _orig;
}

void
ItemAdjustment::cloneItem()
{
    ItemAdjustment* clone = new ItemAdjustment(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
ItemAdjustment::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
ItemAdjustment::dataToWidget()
{
    _gltxFrame->setData(_curr);
    _reason->setId(_curr.reasonId());
    _account->setId(_curr.accountId());
    _inactive->setChecked(!_curr.isActive());

    // Load the items
    _items->setUpdatesEnabled(false);
    _items->clear();
    _lines.clear();
    _lookup->store_id = _curr.storeId();
    for (unsigned int i = 0; i < _curr.items().size(); ++i) {
	const ItemLine& line = _curr.items()[i];
	if (line.voided) continue;

	Item item;
	findItem(line.item_id, item);

	fixed on_hand, total_cost, on_order;
	_quasar->db()->itemGeneral(item.id(), "", _curr.storeId(),
				   QDate::currentDate(), on_hand, total_cost,
				   on_order);

	VectorRow* row = new VectorRow(_items->columns());
	row->setValue(0, Plu(line.item_id, line.number));
	row->setValue(1, item.description());
	row->setValue(2, line.size);
	row->setValue(3, on_hand / item.sizeQty(line.size));
	row->setValue(4, line.quantity);
	row->setValue(5, line.inv_cost);
	if (_company.depositAccount() != INVALID_ID) {
	    row->setValue(6, line.ext_deposit);
	    row->setValue(7, line.inv_cost + line.ext_deposit);
	}
	_items->appendRow(row);

	AdjItem& iline = _lines[_items->rows() - 1];
	iline.item = item;
	iline.number = line.number;
	iline.size = line.size;
	iline.size_qty = line.size_qty;
	iline.quantity = line.quantity;
	iline.inv_cost = line.inv_cost;
	iline.ext_deposit = line.ext_deposit;

	if (i == 0) {
	    for (unsigned int i = 0; i < iline.item.sizes().size(); ++i) {
		_size->insertItem(item.sizes()[i].name);
		if (item.sizes()[i].name == line.size)
		    _size->setCurrentItem(_size->count() - 1);
	    }
	}
    }
    _items->appendRow(new VectorRow(_items->columns()));
    _items->setUpdatesEnabled(true);

    recalculate();
}

void
ItemAdjustment::printItem(bool ask)
{
    if (ask) {
	QString message = tr("Do you wish to print adjustment #%1?")
	    .arg(_curr.number());
	int ch = QMessageBox::information(this, tr("Question"), message,
					  QMessageBox::No, QMessageBox::Yes);
	if (ch != QMessageBox::Yes) return;
    }


    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    QString fileName = "item_adjust_print.xml";
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
    params["adjust_id"] = _curr.id().toString();
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
ItemAdjustment::widgetToData()
{
    unsigned int i;

    _gltxFrame->getData(_curr);
    _curr.setReasonId(_reason->getId());
    _curr.setAccountId(_account->getId());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    vector<InfoPair> asset_info;
    vector<InfoPair> expense_info;
    fixed total_amt = 0.0;

    // Add the items
    vector<ItemLine>& items = _curr.items();
    items.clear();
    for (i = 0; i < _lines.size(); ++i) {
	const AdjItem& line = _lines[i];
	if (line.item.id() == INVALID_ID) continue;

	ItemLine item;
	item.item_id = line.item.id();
	item.number = line.number;
	item.size = line.size;
	item.size_qty = line.size_qty;
	item.quantity = line.quantity;
	item.inv_cost = line.inv_cost;
	item.ext_deposit = line.ext_deposit;
	items.push_back(item);

	addInfo(asset_info, line.item.assetAccount(), line.inv_cost);
	if (_company.depositAccount() != INVALID_ID)
	    addInfo(expense_info, _company.depositAccount(), line.ext_deposit);

	total_amt += line.inv_cost + line.ext_deposit;
    }

    // Add the expense account
    Id account_id = _account->getId();
    if (account_id != INVALID_ID && total_amt != 0.0)
	addInfo(expense_info, account_id, -total_amt);

    // Post to the accounts
    vector<AccountLine>& accounts = _curr.accounts();
    accounts.clear();
    for (i = 0; i < asset_info.size(); ++i) {
	Id account_id = asset_info[i].first;
	fixed amount = asset_info[i].second;
	if (amount == 0.0) continue;
	accounts.push_back(AccountLine(account_id, amount));
    }
    for (i = 0; i < expense_info.size(); ++i) {
	Id account_id = expense_info[i].first;
	fixed amount = expense_info[i].second;
	if (amount == 0.0) continue;
	accounts.push_back(AccountLine(account_id, amount));
    }
}

void
ItemAdjustment::slotStoreChanged()
{
    for (int row = 0; row < _items->rows(); ++row) {
	AdjItem& line = _lines[row];
	if (line.item.id() == INVALID_ID) continue;

	// Check that item is stocked in new store
	if (!line.item.stocked(_gltxFrame->store->getId())) {
	    QString message = tr("Item #%1 is not stocked in the new\n"
				 "store and will be removed from the "
				 "adjustment.").arg(line.number);
	    QMessageBox::warning(this, tr("Warning"), message);
	    _items->deleteRow(row);
	    --row;
	    continue;
	}
    }

    recalculate();
    _lookup->store_id = _gltxFrame->store->getId();
}

void
ItemAdjustment::slotReasonChanged()
{
    AdjustReason reason;
    if (_db->lookup(_reason->getId(), reason))
	_account->setId(reason.accountId());
}

void
ItemAdjustment::slotSearch()
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
ItemAdjustment::cellMoved(int row, int)
{
    if (row == _items->currentRow() && _size->count() > 0)
	return;

    _size->clear();
    if (row == -1) return;

    const AdjItem& line = _lines[row];
    for (unsigned int i = 0; i < line.item.sizes().size(); ++i) {
	_size->insertItem(line.item.sizes()[i].name);
	if (line.item.sizes()[i].name == line.size)
	    _size->setCurrentItem(_size->count() - 1);
    }
}

void
ItemAdjustment::cellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _items->rows() - 1 && col == 0) {
	Id id = _items->cellValue(row, col).toId();
	if (id != INVALID_ID)
	    _items->appendRow(new VectorRow(_items->columns()));
    }

    // Check changes
    AdjItem& line = _lines[row];
    Plu plu;
    switch (col) {
    case 0: // item_id
	plu = _items->cellValue(row, 0).toPlu();
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

	    Id store_id = _gltxFrame->store->getId();
	    QDate date = QDate::currentDate();
	    fixed on_hand, total_cost, on_order;
	    _quasar->db()->itemGeneral(line.item.id(), "", store_id, date,
				       on_hand, total_cost, on_order);

	    line.number = number;
	    line.size = size;
	    line.size_qty = line.item.sizeQty(size);
	    line.quantity = 0.0;
	    line.inv_cost = 0.0;
	    line.ext_deposit = 0.0;

	    _items->setCellValue(row, 1, line.item.description());
	    _items->setCellValue(row, 2, line.size);
	    _items->setCellValue(row, 3, on_hand / line.size_qty);
	    _items->setCellValue(row, 4, "");
	    _items->setCellValue(row, 5, "");
	    if (_company.depositAccount() != INVALID_ID) {
		_items->setCellValue(row, 6, "");
		_items->setCellValue(row, 7, "");
	    }
	} else {
	    if (row != _items->rows() - 1) {
		_items->setCellValue(row, 1, tr("**Unknown Item**"));
	    }
	}
	break;
    case 2: // size
	if (line.item.id() != INVALID_ID) {
	    line.size = _items->cellValue(row, 2).toString();
	    line.size_qty = line.item.sizeQty(line.size);

	    Id store_id = _gltxFrame->store->getId();
	    QDate date = QDate::currentDate();
	    fixed on_hand, total_cost, on_order;
	    _quasar->db()->itemGeneral(line.item.id(), "", store_id, date,
				       on_hand, total_cost, on_order);

	    line.quantity = 0.0;
	    line.inv_cost = 0.0;
	    line.ext_deposit = 0.0;

	    _items->setCellValue(row, 3, on_hand / line.size_qty);
	    _items->setCellValue(row, 4, "");
	    _items->setCellValue(row, 5, "");
	    if (_company.depositAccount() != INVALID_ID) {
		_items->setCellValue(row, 6, "");
		_items->setCellValue(row, 7, "");
	    }
	}
	break;
    case 4: // quantity
	if (line.item.id() != INVALID_ID) {
	    line.quantity = _items->cellValue(row, 4).toFixed();

	    Id store_id = _gltxFrame->store->getId();
	    _quasar->db()->itemSellingCost(line.item, line.size, store_id,
					   line.quantity, 0, line.inv_cost);

	    _items->setCellValue(row, 5, line.inv_cost);
	    if (_company.depositAccount() != INVALID_ID) {
		line.ext_deposit = line.item.deposit() * line.size_qty *
		    line.quantity;
		_items->setCellValue(row, 6, line.ext_deposit);
		_items->setCellValue(row, 7, line.inv_cost + line.ext_deposit);
	    }
	}
	break;
    case 5: // amount
	line.inv_cost = _items->cellValue(row, 5).toFixed();
	if (_company.depositAccount() != INVALID_ID) {
	    _items->setCellValue(row, 7, line.inv_cost + line.ext_deposit);
	}
	break;
    case 6: // ext_deposit
	line.ext_deposit = _items->cellValue(row, 6).toFixed();
	_items->setCellValue(row, 7, line.inv_cost + line.ext_deposit);
	break;
    case 7: // total
	line.inv_cost = _items->cellValue(row, 7).toFixed() - line.ext_deposit;
	_items->setCellValue(row, 5, line.inv_cost);
	break;
    }

    recalculate();
}

void
ItemAdjustment::focusNext(bool& leave, int&, int& newCol, int type)
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
	    newCol = 4;
	}
    } else if (type == Table::MovePrev && col == 2) {
	newCol = 0;
    }
}

void
ItemAdjustment::rowInserted(int row)
{
    AdjItem line;
    _lines.insert(_lines.begin() + row, line);
}

void
ItemAdjustment::rowDeleted(int row)
{
    _lines.erase(_lines.begin() + row);
    recalculate();
}

void
ItemAdjustment::recalculate()
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _lines.size(); ++i) {
	AdjItem& line = _lines[i];
	if (line.item.id() == INVALID_ID) continue;

	total += line.inv_cost + line.ext_deposit;
    }

    if (total == 0.0)
	_total->clear();
    else
	_total->setFixed(-total);
}

bool
ItemAdjustment::findItem(Id item_id, Item& item)
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
