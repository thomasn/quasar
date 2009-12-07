// $Id: order_template_master.cpp,v 1.15 2005/04/01 22:14:13 bpepers Exp $
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

#include "order_template_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "order_template_select.h"
#include "item.h"
#include "dept.h"
#include "subdept.h"
#include "item_edit.h"
#include "money_edit.h"
#include "vendor_lookup.h"
#include "item_lookup.h"
#include "charge_lookup.h"
#include "tax_lookup.h"
#include "table.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <algorithm>

OrderTemplateMaster::OrderTemplateMaster(MainWindow* main, Id temp_id)
    : DataWindow(main, "OrderTemplateMaster", temp_id)
{
    _helpSource = "order_template_master.html";

    QPushButton* search = new QPushButton(tr("Search"), _buttons);
    connect(search, SIGNAL(clicked()), SLOT(slotSearch()));

    QPushButton* sort = new QPushButton(tr("Sort"), _buttons);
    connect(sort, SIGNAL(clicked()), SLOT(slotSort()));

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("&Name:"), _frame);
    _name = new LineEdit(30, _frame);
    nameLabel->setBuddy(_name);

    QLabel* vendorLabel = new QLabel(tr("Vendor:"), _frame);
    _vendor = new LookupEdit(new VendorLookup(main, this), _frame);
    _vendor->setLength(30);
    vendorLabel->setBuddy(_vendor);

    QTabWidget* tabs = new QTabWidget(_frame);
    QFrame* items = new QFrame(tabs);
    QFrame* charges = new QFrame(tabs);
    tabs->addTab(items, tr("Items"));
    tabs->addTab(charges, tr("Charges"));

    _items = new Table(items);
    _items->setVScrollBarMode(QScrollView::AlwaysOn);
    _items->setLeftMargin(fontMetrics().width("99999"));
    _items->setDisplayRows(10);
    connect(_items, SIGNAL(cellMoved(int,int)), SLOT(itemCellMoved(int,int)));
    connect(_items, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(itemCellChanged(int,int,Variant)));
    connect(_items, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(itemFocusNext(bool&,int&,int&,int)));

    // Lookups
    ItemLookup* lookup = new ItemLookup(_main, this);
    lookup->purchasedOnly = true;

    // Add columns
    new LookupColumn(_items, tr("Item Number"), 18, lookup);
    new TextColumn(_items, tr("Description"), 20);
    new TextColumn(_items, tr("Size"), 8);
    new TextColumn(_items, tr("Department"), 12);
    new TextColumn(_items, tr("Subdepartment"), 12);

    // Add editors
    _size = new QComboBox(false, _items);
    new LookupEditor(_items, 0, new ItemEdit(lookup, _items));
    new ComboEditor(_items, 2, _size);

    QGridLayout* itemGrid = new QGridLayout(items);
    itemGrid->setSpacing(3);
    itemGrid->setMargin(3);
    itemGrid->setRowStretch(0, 1);
    itemGrid->setColStretch(0, 1);
    itemGrid->addWidget(_items, 0, 0);

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

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setRowStretch(2, 1);
    grid->setColStretch(1, 1);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(vendorLabel, 1, 0);
    grid->addWidget(_vendor, 1, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(tabs, 2, 2, 0, 1);

    setCaption(tr("Order Template Master"));
    finalize();
}

OrderTemplateMaster::~OrderTemplateMaster()
{
}

void
OrderTemplateMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _name;
}

void
OrderTemplateMaster::newItem()
{
    OrderTemplate blank;
    _orig = blank;

    _curr = _orig;
    _firstField = _name;
}

void
OrderTemplateMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _name->selectAll();
}

bool
OrderTemplateMaster::fileItem()
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
OrderTemplateMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
OrderTemplateMaster::restoreItem()
{
    _curr = _orig;
}

void
OrderTemplateMaster::cloneItem()
{
    OrderTemplateMaster* clone = new OrderTemplateMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
OrderTemplateMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
OrderTemplateMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _vendor->setId(_curr.vendorId());
    _inactive->setChecked(!_curr.isActive());

    // Load the items
    _items->clear();
    unsigned int i;
    for (i = 0; i < _curr.items().size(); ++i) {
	const TemplateItem& line = _curr.items()[i];

	Item item;
	_quasar->db()->lookup(line.item_id, item);

	Dept dept;
	_quasar->db()->lookup(item.deptId(), dept);

	Subdept subdept;
	_quasar->db()->lookup(item.subdeptId(), subdept);

	VectorRow* row = new VectorRow(_items->columns());
	row->setValue(0, Plu(line.item_id, line.number));
	row->setValue(1, item.description());
	row->setValue(2, line.size);
	row->setValue(3, dept.name());
	row->setValue(4, subdept.name());
	_items->appendRow(row);
    }
    _items->appendRow(new VectorRow(_items->columns()));

    // Load the charges
    _icharges->clear();
    _icharge_lines.clear();
    _echarges->clear();
    _echarge_lines.clear();
    for (i = 0; i < _curr.charges().size(); ++i) {
	const TemplateCharge& line = _curr.charges()[i];

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
	    cline.tax_id = line.tax_id;
	} else {
	    VectorRow* row = new VectorRow(_echarges->columns());
	    row->setValue(0, line.charge_id);
	    row->setValue(1, line.amount);
	    _echarges->appendRow(row);

	    ChargeLine& cline = _echarge_lines[_echarges->rows() - 1];
	    cline.charge = charge;
	    cline.amount = line.amount;
	    cline.tax_id = INVALID_ID;
	}
    }
    _icharges->appendRow(new VectorRow(_icharges->columns()));
    _echarges->appendRow(new VectorRow(_echarges->columns()));
}

// Set the data object from the widgets.
void
OrderTemplateMaster::widgetToData()
{
    _curr.setName(_name->text());
    _curr.setVendorId(_vendor->getId());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    // Process all the items
    _curr.items().clear();
    for (int row = 0; row < _items->rows(); ++row) {
	Id item_id = _items->cellValue(row, 0).toId();
	QString number = _items->cellValue(row, 0).toPlu().number();
	QString size = _items->cellValue(row, 2).toString();
	if (item_id == INVALID_ID) continue;

	TemplateItem item;
	item.item_id = item_id;
	item.number = number;
	item.size = size;
	_curr.items().push_back(item);
    }

    // Process internal charges
    vector<TemplateCharge>& charges = _curr.charges();
    charges.clear();
    unsigned int i;
    for (i = 0; i < _icharge_lines.size(); ++i) {
	const Charge& charge = _icharge_lines[i].charge;
	if (charge.id() == INVALID_ID) continue;
	Id tax_id = _icharge_lines[i].tax_id;
	fixed amount = _icharge_lines[i].amount;;

	TemplateCharge line(charge.id(), tax_id, amount, true);
	charges.push_back(line);
    }

    // Process external charges
    for (i = 0; i < _echarge_lines.size(); ++i) {
	const Charge& charge = _echarge_lines[i].charge;
	if (charge.id() == INVALID_ID) continue;
	fixed amount = _echarge_lines[i].amount;;

	TemplateCharge line(charge.id(), INVALID_ID, amount, false);
	charges.push_back(line);
    }
}

void
OrderTemplateMaster::slotSearch()
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

struct SortInfo {
    enum { TYPE_DEPT=1, TYPE_SUBDEPT=2, TYPE_NUMBER=4, TYPE_DESC=8 };
    int type;
    QString dept;
    QString subdept;
    QString number;
    QString description;
    Id item_id;
    QString size;
};

bool operator<(const SortInfo& lhs, const SortInfo& rhs) {
    if (lhs.type & SortInfo::TYPE_DEPT) {
	if (lhs.dept < rhs.dept) return true;
	if (lhs.dept > rhs.dept) return false;
    }
    if (lhs.type & SortInfo::TYPE_SUBDEPT) {
	if (lhs.subdept < rhs.subdept) return true;
	if (lhs.subdept > rhs.subdept) return false;
    }
    if (lhs.type & SortInfo::TYPE_NUMBER) {
	QString number1 = ("000000000000000000" + lhs.number).right(18);
	QString number2 = ("000000000000000000" + rhs.number).right(18);
	if (number1 < number2) return true;
	if (number1 > number2) return false;
    }
    if (lhs.type & SortInfo::TYPE_DESC) {
	if (lhs.description < rhs.description) return true;
	if (lhs.description > rhs.description) return false;
    }
    return false;
}

void
OrderTemplateMaster::slotSort()
{
    QDialog* dialog = new QDialog(this, "AskType", true);
    dialog->setCaption(tr("Sort Selection"));

    QCheckBox* byDept = new QCheckBox(tr("By Department?"), dialog);
    QCheckBox* bySubdept = new QCheckBox(tr("By Subdepartment?"), dialog);
    QCheckBox* byNumber = new QCheckBox(tr("By Number?"), dialog);
    QCheckBox* byDesc = new QCheckBox(tr("By Description?"), dialog);

    QFrame* buttons = new QFrame(dialog);
    QPushButton* ok = new QPushButton(tr("&OK"), buttons);
    QPushButton* cancel = new QPushButton(tr("&Cancel"), buttons);
    ok->setDefault(true);

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setMargin(6);
    buttonGrid->setSpacing(10);
    buttonGrid->addWidget(ok, 0, 1);
    buttonGrid->addWidget(cancel, 0, 2);

    QGridLayout* grid = new QGridLayout(dialog);
    grid->setMargin(6);
    grid->setSpacing(10);
    grid->addWidget(byDept, 0, 0, AlignLeft | AlignVCenter);
    grid->addWidget(bySubdept, 1, 0, AlignLeft | AlignVCenter);
    grid->addWidget(byNumber, 2, 0, AlignLeft | AlignVCenter);
    grid->addWidget(byDesc, 3, 0, AlignLeft | AlignVCenter);
    grid->addWidget(buttons, 4, 0);

    connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
    int result = dialog->exec();

    int type = 0;
    if (byDept->isChecked()) type |= SortInfo::TYPE_DEPT;
    if (bySubdept->isChecked()) type |= SortInfo::TYPE_SUBDEPT;
    if (byNumber->isChecked()) type |= SortInfo::TYPE_NUMBER;
    if (byDesc->isChecked()) type |= SortInfo::TYPE_DESC;

    delete dialog;
    if (result != QDialog::Accepted || type == 0) return;

    vector<SortInfo> lines;
    for (int row = 0; row < _items->rows(); ++row) {
	Id item_id = _items->cellValue(row, 0).toId();
	QString number = _items->cellValue(row, 0).toPlu().number();
	QString description = _items->cellValue(row, 1).toString();
	QString size = _items->cellValue(row, 2).toString();
	QString dept = _items->cellValue(row, 3).toString();
	QString subdept = _items->cellValue(row, 4).toString();
	if (item_id == INVALID_ID) continue;

	SortInfo info;
	info.type = type;
	info.dept = dept;
	info.subdept = subdept;
	info.number = number;
	info.description = description;
	info.item_id = item_id;
	info.size = size;
	lines.push_back(info);
    }

    std::sort(lines.begin(), lines.end());

    _items->clear();
    for (unsigned int i = 0; i < lines.size(); ++i) {
	const SortInfo& info = lines[i];

	VectorRow* row = new VectorRow(_items->columns());
	row->setValue(0, Plu(info.item_id, info.number));
	row->setValue(1, info.description);
	row->setValue(2, info.size);
	row->setValue(3, info.dept);
	row->setValue(4, info.subdept);
	_items->appendRow(row);
    }
    _items->appendRow(new VectorRow(_items->columns()));
}

void
OrderTemplateMaster::itemCellMoved(int row, int)
{
    if (row == _items->currentRow() && _size->count() > 0)
	return;

    _size->clear();
    if (row == -1) return;

    Id item_id = _items->cellValue(row, 0).toId();
    QString size = _items->cellValue(row, 2).toString();

    Item item;
    _quasar->db()->lookup(item_id, item);

    for (unsigned int i = 0; i < item.sizes().size(); ++i) {
	_size->insertItem(item.sizes()[i].name);
	if (item.sizes()[i].name == size)
	    _size->setCurrentItem(_size->count() - 1);
    }
}

void
OrderTemplateMaster::itemCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _items->rows() - 1 && col == 0) {
	Id id = _items->cellValue(row, col).toId();
	if (id != INVALID_ID) {
	    _items->appendRow(new VectorRow(_items->columns()));
	    _items->setCellValue(_items->rows() - 1, 0, INVALID_ID);
	}
    }

    // Check for other changes
    Id item_id;
    Item item;
    switch (col) {
    case 0: // item_id
	item_id = _items->cellValue(row, 0).toId();
	if (_quasar->db()->lookup(item_id, item)) {
	    QString number = _items->cellValue(row, 0).toPlu().number();
	    QString size = item.numberSize(number);
	    if (size.isEmpty()) size = item.purchaseSize();

	    _size->clear();
	    for (unsigned int i = 0; i < item.sizes().size(); ++i) {
		_size->insertItem(item.sizes()[i].name);
		if (item.sizes()[i].name == size)
		    _size->setCurrentItem(_size->count() - 1);
	    }

	    Dept dept;
	    _quasar->db()->lookup(item.deptId(), dept);

	    Subdept subdept;
	    _quasar->db()->lookup(item.subdeptId(), subdept);

	    _items->setCellValue(row, 1, item.description());
	    _items->setCellValue(row, 2, size);
	    _items->setCellValue(row, 3, dept.name());
	    _items->setCellValue(row, 4, subdept.name());
	} else {
	    if (row != _items->rows() - 1) {
		_items->setCellValue(row, 1, tr("**Unknown Item**"));
	    }
	}
	break;
    }
}

void
OrderTemplateMaster::itemFocusNext(bool& leave, int& newRow, int& newCol,
				   int type)
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
	    newCol = 2;
	}
    } else if (type == Table::MovePrev && col == 0) {
	newCol = 2;
    } else if (type == Table::MovePrev && col == 2) {
	newCol = 0;
    } else if (type == Table::MoveNext && col == 2) {
	newRow = row + 1;
	newCol = 0;
    }
}

void
OrderTemplateMaster::ichargeCellChanged(int row, int col, Variant)
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

	    _icharges->setCellValue(row, 1, "");
	    _icharges->setCellValue(row, 2, line.tax_id);
	}
	break;
    case 1: // amount
	line.amount = _icharges->cellValue(row, 1).toFixed();
	break;
    case 2: // tax_id
	line.tax_id = _icharges->cellValue(row, 2).toId();
	break;
    }
}

void
OrderTemplateMaster::ichargeFocusNext(bool& leave, int& newRow, int& newCol, int type)
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
OrderTemplateMaster::ichargeRowInserted(int row)
{
    ChargeLine line;
    _icharge_lines.insert(_icharge_lines.begin() + row, line);
}

void
OrderTemplateMaster::ichargeRowDeleted(int row)
{
    _icharge_lines.erase(_icharge_lines.begin() + row);
}

void
OrderTemplateMaster::echargeCellChanged(int row, int col, Variant)
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

	    _echarges->setCellValue(row, 1, "");
	}
	break;
    case 1: // amount
	line.amount = _echarges->cellValue(row, 1).toFixed();
	break;
    }
}

void
OrderTemplateMaster::echargeFocusNext(bool& leave, int&, int&, int type)
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
OrderTemplateMaster::echargeRowInserted(int row)
{
    ChargeLine line;
    _echarge_lines.insert(_echarge_lines.begin() + row, line);
}

void
OrderTemplateMaster::echargeRowDeleted(int row)
{
    _echarge_lines.erase(_echarge_lines.begin() + row);
}

bool
OrderTemplateMaster::findCharge(Id charge_id, Charge& charge)
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

OrderTemplateMaster::ChargeLine::ChargeLine()
    : tax_id(INVALID_ID)
{
}
