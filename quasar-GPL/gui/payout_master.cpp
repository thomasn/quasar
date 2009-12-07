// $Id: payout_master.cpp,v 1.14 2005/05/13 22:21:37 bpepers Exp $
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

#include "payout_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "object_cache.h"
#include "payout_select.h"
#include "expense.h"
#include "tender.h"
#include "tax.h"
#include "gltx_frame.h"
#include "table.h"
#include "money_edit.h"
#include "lookup_edit.h"
#include "expense_lookup.h"
#include "tax_lookup.h"
#include "tender_lookup.h"
#include "store_lookup.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpopupmenu.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qlayout.h>
#include <qmessagebox.h>

PayoutMaster::PayoutMaster(MainWindow* main, Id payout_id)
    : DataWindow(main, "PayoutMaster", payout_id)
{
    _helpSource = "payout_master.html";

    // Create widgets
    _gltxFrame = new GltxFrame(main, tr("Payout No."), _frame);

    _expenses = new Table(_frame);
    _expenses->setVScrollBarMode(QScrollView::AlwaysOn);
    connect(_expenses, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(expenseCellChanged(int,int,Variant)));
    connect(_expenses, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(expenseFocusNext(bool&,int&,int&,int)));

    // Lookups
    LookupWindow* expenseLookup = new ExpenseLookup(_main, this);
    LookupWindow* taxLookup = new TaxLookup(_main, this);

    // Add columns
    new LookupColumn(_expenses, tr("Expense"), 30, expenseLookup);
    new MoneyColumn(_expenses, tr("Amount"));
    new LookupColumn(_expenses, tr("Tax"), 8, taxLookup);

    // Add editors
    new LookupEditor(_expenses, 0, new LookupEdit(expenseLookup, _expenses));
    new NumberEditor(_expenses, 1, new MoneyEdit(_expenses));
    new LookupEditor(_expenses, 2, new LookupEdit(taxLookup, _expenses));

    QTabWidget* tabs = new QTabWidget(_frame);
    QFrame* totals = new QFrame(tabs);
    QFrame* taxes = new QFrame(tabs);
    QFrame* tenders = new QFrame(tabs);
    tabs->addTab(totals, tr("Totals"));
    tabs->addTab(taxes, tr("Taxes"));
    tabs->addTab(tenders, tr("Tenders"));

    QLabel* expenseTotalLabel = new QLabel(tr("Expense Total:"), totals);
    _expenseTotal = new MoneyEdit(totals);
    _expenseTotal->setFocusPolicy(NoFocus);

    QLabel* taxTotalLabel = new QLabel(tr("Tax Total:"), totals);
    _taxTotal = new MoneyEdit(totals);
    _taxTotal->setFocusPolicy(NoFocus);

    QLabel* totalLabel = new QLabel(tr("Total:"), totals);
    _total = new MoneyEdit(totals);
    _total->setFocusPolicy(NoFocus);

    QLabel* tenderTotalLabel = new QLabel(tr("Tendered:"), totals);
    _tenderTotal = new MoneyEdit(totals);
    _tenderTotal->setFocusPolicy(NoFocus);

    QLabel* diffLabel = new QLabel(tr("Difference:"), totals);
    _diff = new MoneyEdit(totals);
    _diff->setFocusPolicy(NoFocus);

    QPalette palette = _diff->palette();
    palette.setColor(QPalette::Active, QColorGroup::Text, QColor(255, 10, 10));
    _diff->setPalette(palette);

    QGridLayout* totalGrid = new QGridLayout(totals);
    totalGrid->setSpacing(3);
    totalGrid->setMargin(3);
    totalGrid->setColStretch(2, 1);
    totalGrid->addWidget(expenseTotalLabel, 0, 0);
    totalGrid->addWidget(_expenseTotal, 0, 1, AlignLeft | AlignVCenter);
    totalGrid->addWidget(taxTotalLabel, 1, 0);
    totalGrid->addWidget(_taxTotal, 1, 1, AlignLeft | AlignVCenter);
    totalGrid->addWidget(totalLabel, 0, 3);
    totalGrid->addWidget(_total, 0, 4, AlignLeft | AlignVCenter);
    totalGrid->addWidget(tenderTotalLabel, 1, 3);
    totalGrid->addWidget(_tenderTotal, 1, 4, AlignLeft | AlignVCenter);
    totalGrid->addWidget(diffLabel, 2, 3);
    totalGrid->addWidget(_diff, 2, 4, AlignLeft | AlignVCenter);

    _taxes = new Table(taxes);
    _taxes->setVScrollBarMode(QScrollView::AlwaysOn);
    _taxes->setDisplayRows(3);

    new TextColumn(_taxes, tr("Tax"), 6);
    new TextColumn(_taxes, tr("Name"), 20);
    new MoneyColumn(_taxes, tr("Taxable"), 6);
    new MoneyColumn(_taxes, tr("Tax Amt"), 6);

    QGridLayout* taxGrid = new QGridLayout(taxes);
    taxGrid->setSpacing(3);
    taxGrid->setMargin(3);
    taxGrid->setRowStretch(0, 1);
    taxGrid->setColStretch(0, 1);
    taxGrid->addWidget(_taxes, 0, 0);

    _tenders = new Table(tenders);
    _tenders->setVScrollBarMode(QScrollView::AlwaysOn);
    _tenders->setDisplayRows(3);
    connect(_tenders, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(tenderCellChanged(int,int,Variant)));
    connect(_tenders, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(tenderFocusNext(bool&,int&,int&,int)));

    TenderLookup* tenderLookup = new TenderLookup(_main, this);

    new LookupColumn(_tenders, tr("Tender"), 20, tenderLookup);
    new MoneyColumn(_tenders, tr("Amount"));
    new NumberColumn(_tenders, tr("Rate"), 3);
    new MoneyColumn(_tenders, tr("Total"));

    new LookupEditor(_tenders, 0, new LookupEdit(tenderLookup, _tenders));
    new NumberEditor(_tenders, 1, new MoneyEdit(_tenders));

    QGridLayout* tenderGrid = new QGridLayout(tenders);
    tenderGrid->setSpacing(3);
    tenderGrid->setMargin(3);
    tenderGrid->setRowStretch(0, 1);
    tenderGrid->setColStretch(0, 1);
    tenderGrid->addWidget(_tenders, 0, 0);

    _inactive->setText(tr("Voided?"));

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(1, 1);
    grid->addWidget(_gltxFrame, 0, 0);
    grid->addWidget(_expenses, 1, 0);
    grid->addWidget(tabs, 2, 0);

    setCaption(tr("Payout"));
    finalize();
}

PayoutMaster::~PayoutMaster()
{
}

void
PayoutMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);
    _curr = _orig;
    _firstField = _gltxFrame->firstField();
}

void
PayoutMaster::newItem()
{
    Payout blank;
    _orig = blank;
    _gltxFrame->defaultData(_orig);

    _curr = _orig;
    _firstField = _gltxFrame->firstField();
}

void
PayoutMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _gltxFrame->cloneData(_curr);
    dataToWidget();
}

bool
PayoutMaster::fileItem()
{
    if (_curr.number() != "#")
	if (checkGltxUsed(_curr.number(), DataObject::PAYOUT, _curr.id()))
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
PayoutMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
PayoutMaster::restoreItem()
{
    _curr = _orig;
}

void
PayoutMaster::cloneItem()
{
    PayoutMaster* clone = new PayoutMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
PayoutMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
PayoutMaster::dataToWidget()
{
    _gltxFrame->setData(_curr);
    _inactive->setChecked(!_curr.isActive());

    // Load the expenses
    _expenses->clear();
    unsigned int i;
    for (i = 0; i < _curr.expenses().size(); ++i) {
	const ExpenseLine& line = _curr.expenses()[i];
	if (line.voided) continue;

	VectorRow* row = new VectorRow(_expenses->columns());
	row->setValue(0, line.expense_id);
	row->setValue(1, line.amount);
	row->setValue(2, line.tax_id);
	_expenses->appendRow(row);
    }
    _expenses->appendRow(new VectorRow(_expenses->columns()));

    // Load the tenders
    _tenders->clear();
    for (i = 0; i < _curr.tenders().size(); ++i) {
	const TenderLine& line = _curr.tenders()[i];
	if (line.voided) continue;

	VectorRow* row = new VectorRow(_tenders->columns());
	row->setValue(0, line.tender_id);
	row->setValue(1, -line.conv_amt);
	row->setValue(2, line.conv_rate);
	row->setValue(3, -line.amount);
	_tenders->appendRow(row);
    }
    _tenders->appendRow(new VectorRow(_tenders->columns()));

    recalculate();
}

// Set the data object from the widgets.
void
PayoutMaster::widgetToData()
{
    _gltxFrame->getData(_curr);
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    Id store_id = _curr.storeId();
    vector<AccountLine>& accounts = _curr.accounts();
    accounts.clear();

    // Add the expenses
    _curr.expenses().clear();
    int row;
    for (row = 0; row < _expenses->rows(); ++row) {
	Id expense_id = _expenses->cellValue(row, 0).toId();
	fixed amount = _expenses->cellValue(row, 1).toFixed();
	Id tax_id = _expenses->cellValue(row, 2).toId();
	if (expense_id == INVALID_ID) continue;

	Expense expense;
	_quasar->db()->lookup(expense_id, expense);

	ExpenseLine line;
	line.expense_id = expense_id;
	line.amount = amount;
	line.itax_id = expense.taxId();
	line.tax_id = tax_id;
	_curr.expenses().push_back(line);

	accounts.push_back(AccountLine(expense.accountId(), amount));
    }

    // Process all the taxes
    _curr.taxes().clear();
    for (unsigned int i = 0; i < _tax_info.size(); ++i) {
	Id tax_id = _tax_info[i].tax_id;
	fixed taxable = _tax_info[i].taxable;
	fixed amount = _tax_info[i].amount;

	Tax tax;
	_quasar->db()->lookup(tax_id, tax);

	TaxLine line;
	line.tax_id = tax_id;
	line.taxable = taxable;
	line.amount = amount;
	_curr.taxes().push_back(line);

	accounts.push_back(AccountLine(tax.paidAccount(), amount));
    }

    // Add the tenders
    _curr.tenders().clear();
    for (row = 0; row < _tenders->rows(); ++row) {
	Id tender_id = _tenders->cellValue(row, 0).toId();
	if (tender_id == INVALID_ID) continue;
	fixed conv_amt = _tenders->cellValue(row, 1).toFixed();
	fixed conv_rate = _tenders->cellValue(row, 2).toFixed();
	fixed amount = _tenders->cellValue(row, 3).toFixed();

	Tender tender;
	_quasar->db()->lookup(tender_id, tender);

	TenderLine line;
	line.tender_id = tender_id;
	line.amount = -amount;
	line.conv_rate = conv_rate;
	line.conv_amt = -conv_amt;
	_curr.tenders().push_back(line);

	accounts.push_back(AccountLine(tender.accountId(), -amount));
    }
}

void
PayoutMaster::expenseCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _expenses->rows() - 1 && col == 0) {
	Id id = _expenses->cellValue(row, col).toId();
	if (id != INVALID_ID)
	    _expenses->appendRow(new VectorRow(_expenses->columns()));
    }

    // Check for other changes
    Id expense_id;
    switch (col) {
    case 0: // expense_id
	expense_id = _expenses->cellValue(row, 0).toId();
	if (expense_id != INVALID_ID) {
	    Expense expense;
	    _quasar->db()->lookup(expense_id, expense);

	    _expenses->setCellValue(row, 2, expense.taxId());
	} else {
	    _expenses->setCellValue(row, 1, "");
	    _expenses->setCellValue(row, 2, INVALID_ID);
	}
	break;
    }

    recalculate();
}

void
PayoutMaster::expenseFocusNext(bool& leave, int&, int&, int type)
{
    int row = _expenses->currentRow();
    int col = _expenses->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id id = _expenses->cellValue(row, col).toId();
	if (id == INVALID_ID && row == _expenses->rows() - 1) {
	    leave = true;
	}
    }
}

void
PayoutMaster::tenderCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _tenders->rows() - 1 && col == 0) {
	Id id = _tenders->cellValue(row, col).toId();
	if (id != INVALID_ID)
	    _tenders->appendRow(new VectorRow(_tenders->columns()));
    }

    // Check for other changes
    Id tender_id;
    switch (col) {
    case 0: // tender_id
	tender_id = _tenders->cellValue(row, 0).toId();
	if (tender_id != INVALID_ID) {
	    Tender tender;
	    _quasar->db()->lookup(tender_id, tender);

	    fixed remain = _total->getFixed();
	    for (int i = 0; i < _tenders->rows(); ++i) {
		if (i == row) continue;
		if (_tenders->cellValue(i, 0).toId() == INVALID_ID) continue;
		remain -= _tenders->cellValue(i, 3).toFixed();
	    }

	    fixed convertRate = tender.convertRate();
	    fixed amount = remain / convertRate;
	    amount.moneyRound();

	    _tenders->setCellValue(row, 1, amount);
	    _tenders->setCellValue(row, 2, convertRate);
	    _tenders->setCellValue(row, 3, remain);
	} else {
	    _tenders->setCellValue(row, 1, "");
	    _tenders->setCellValue(row, 2, "");
	    _tenders->setCellValue(row, 3, "");
	}
	break;
    case 1: // amount
	if (!_tenders->cellValue(row, 2).toString().isEmpty()) {
	    fixed amount = _tenders->cellValue(row, 1).toFixed();
	    fixed rate = _tenders->cellValue(row, 2).toFixed();
	    _tenders->setCellValue(row, 3, amount * rate);
	}
	break;
    }

    recalculate();
}

void
PayoutMaster::tenderFocusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _tenders->currentRow();
    int col = _tenders->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id id = _tenders->cellValue(row, col).toId();
	if (id == INVALID_ID && row == _tenders->rows() - 1) {
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
PayoutMaster::recalculate()
{
    int row;
    _tax_info.clear();

    ObjectCache cache(_db);

    // Total up expenses and calculate taxes
    fixed expenseTotal = 0.0;
    for (row = 0; row < _expenses->rows(); ++row) {
	Id expense_id = _expenses->cellValue(row, 0).toId();
	if (expense_id == INVALID_ID) continue;

	fixed amount = _expenses->cellValue(row, 1).toFixed();
	expenseTotal += amount;

	Id tax_id = _expenses->cellValue(row, 2).toId();
	if (tax_id != INVALID_ID) {
	    Tax tax;
	    _quasar->db()->lookup(tax_id, tax);

	    vector<Id> tax_ids;
	    vector<fixed> tax_amts;
	    _quasar->db()->calculateTaxOn(cache, tax, amount, Tax(), tax_ids,
					  tax_amts);
	    addTax(amount, tax_ids, tax_amts);
	}
    }

    // Round taxes to two decimal places and total up
    _taxes->clear();
    fixed taxTotal = 0.0;
    for (unsigned int i = 0; i < _tax_info.size(); ++i) {
	Id tax_id = _tax_info[i].tax_id;

	// NOTE: don't need to round inc_amount since its always returned
	// rounded from the tax calculation.
	_tax_info[i].amount.moneyRound();
	taxTotal += _tax_info[i].amount;

	Tax tax;
	_quasar->db()->lookup(tax_id, tax);

	VectorRow* row = new VectorRow(_taxes->columns());
	row->setValue(0, tax.name());
	row->setValue(1, tax.description());
	row->setValue(2, _tax_info[i].taxable);
	row->setValue(3, _tax_info[i].amount);
	_taxes->appendRow(row);
    }

    // Total up tenders
    fixed tenderTotal = 0.0;
    for (row = 0; row < _tenders->rows(); ++row) {
	Id tender_id = _tenders->cellValue(row, 0).toId();
	if (tender_id == INVALID_ID) continue;

	fixed amount = _tenders->cellValue(row, 3).toFixed();
	tenderTotal += amount;
    }

    fixed total = expenseTotal + taxTotal;
    fixed diff = total - tenderTotal;

    _expenseTotal->setFixed(expenseTotal);
    _taxTotal->setFixed(taxTotal);
    _total->setFixed(total);
    _tenderTotal->setFixed(tenderTotal);
    if (diff == 0.0)
	_diff->clear();
    else
	_diff->setFixed(diff);
}

void
PayoutMaster::addTax(fixed taxable, vector<Id>& ids, vector<fixed>& amts)
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
