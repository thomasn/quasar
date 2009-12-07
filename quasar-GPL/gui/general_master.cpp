// $Id: general_master.cpp,v 1.35 2005/01/30 04:25:31 bpepers Exp $
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

#include "general_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "general_select.h"
#include "gltx_frame.h"
#include "table.h"
#include "money_edit.h"
#include "lookup_edit.h"
#include "account_lookup.h"
#include "store_lookup.h"
#include "recurring_master.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpopupmenu.h>
#include <qvbox.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qlayout.h>
#include <qmessagebox.h>

GeneralMaster::GeneralMaster(MainWindow* main, Id general_id)
    : DataWindow(main, "GeneralMaster", general_id)
{
    _helpSource = "general_master.html";

    _file->insertItem(tr("Recurring"), this, SLOT(slotRecurring()));

    // Create widgets
    _gltxFrame = new GltxFrame(main, tr("Id No."), _frame);

    _table = new Table(_frame);
    _table->setVScrollBarMode(QScrollView::AlwaysOn);
    connect(_table, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(cellChanged(int,int,Variant)));
    connect(_table, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(focusNext(bool&,int&,int&,int)));

    // Lookups
    LookupWindow* accountLookup = new AccountLookup(_main, this);

    // Add columns
    new LookupColumn(_table, tr("Account"), 30, accountLookup);
    new MoneyColumn(_table, tr("Debit"));
    new MoneyColumn(_table, tr("Credit"));

    // Add editors
    new LookupEditor(_table, 0, new LookupEdit(accountLookup, _table));
    new NumberEditor(_table, 1, new MoneyEdit(_table));
    new NumberEditor(_table, 2, new MoneyEdit(_table));

    QFrame* totals = new QFrame(_frame);

    QLabel* dbTotalLabel = new QLabel(tr("Debit Total:"), totals);
    _debit_total = new MoneyEdit(totals);
    _debit_total->setFocusPolicy(NoFocus);

    QLabel* crTotalLabel = new QLabel(tr("Credit Total:"), totals);
    _credit_total = new MoneyEdit(totals);
    _credit_total->setFocusPolicy(NoFocus);

    QLabel* differenceLabel = new QLabel(tr("Difference:"), totals);
    _difference = new MoneyEdit(totals);
    _difference->setFocusPolicy(NoFocus);

    QPalette palette = _difference->palette();
    palette.setColor(QPalette::Active, QColorGroup::Text, QColor(255, 10, 10));
    _difference->setPalette(palette);

    QGridLayout* totalGrid = new QGridLayout(totals);
    totalGrid->setSpacing(3);
    totalGrid->setMargin(3);
    totalGrid->setColStretch(0, 1);
    totalGrid->addWidget(dbTotalLabel, 0, 1);
    totalGrid->addWidget(_debit_total, 0, 2);
    totalGrid->addWidget(crTotalLabel, 1, 1);
    totalGrid->addWidget(_credit_total, 1, 2);
    totalGrid->addWidget(differenceLabel, 2, 1);
    totalGrid->addWidget(_difference, 2, 2);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(1, 1);
    grid->addWidget(_gltxFrame, 0, 0);
    grid->addWidget(_table, 1, 0);
    grid->addWidget(totals, 2, 0);

    _inactive->setText(tr("Voided?"));
    _reverse = new QCheckBox(tr("Reverse?"), _buttons);
    _reverse->setFocusPolicy(ClickFocus);

    setCaption(tr("Journal Entry"));
    finalize();
}

GeneralMaster::~GeneralMaster()
{
}

void
GeneralMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);
    _curr = _orig;
    _firstField = _gltxFrame->firstField();
    _reverse->setEnabled(false);
}

void
GeneralMaster::newItem()
{
    General blank;
    _orig = blank;
    _gltxFrame->defaultData(_orig);

    _curr = _orig;
    _firstField = _gltxFrame->firstField();
    _reverse->setEnabled(true);
}

void
GeneralMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _gltxFrame->cloneData(_curr);
    dataToWidget();
}

bool
GeneralMaster::fileItem()
{
    if (_curr.number() != "#")
	if (checkGltxUsed(_curr.number(), DataObject::GENERAL, _curr.id()))
	    return false;

    if (_orig.id() == INVALID_ID) {
	if (!_quasar->db()->create(_curr)) return false;

	if (_reverse->isChecked()) {
	    QDate date = _curr.postDate();
	    General reverse = _curr;
	    reverse.setNumber("#");
	    reverse.setPostDate(QDate(date.year(), date.month() + 1, 1));
	    reverse.setMemo(tr("Reversing Journal #%1")
			    .arg(_curr.number()));
	    for (unsigned int i = 0; i < reverse.accounts().size(); ++i)
		reverse.accounts()[i].amount = -reverse.accounts()[i].amount;
	    _quasar->db()->create(reverse);
	}
    } else {
	if (!_quasar->db()->update(_orig, _curr)) return false;
    }

    _orig = _curr;
    _id = _curr.id();

    return true;
}

bool
GeneralMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
GeneralMaster::restoreItem()
{
    _curr = _orig;
}

void
GeneralMaster::cloneItem()
{
    GeneralMaster* clone = new GeneralMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
GeneralMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
GeneralMaster::dataToWidget()
{
    _gltxFrame->setData(_curr);
    _inactive->setChecked(!_curr.isActive());
    _reverse->setChecked(false);

    // Load the table model
    const vector<AccountLine>& lines = _curr.accounts();
    _table->clear();
    for (unsigned int line = 0; line < lines.size(); ++line) {
	Id account_id = lines[line].account_id;
	fixed amount = lines[line].amount;

	VectorRow* row = new VectorRow(_table->columns());
	row->setValue(0, account_id);
	if (amount > 0.0)
	    row->setValue(1, amount);
	else
	    row->setValue(2, -amount);
	_table->appendRow(row);
    }
    _table->appendRow(new VectorRow(_table->columns()));

    recalculate();
}

// Set the data object from the widgets.
void
GeneralMaster::widgetToData()
{
    _gltxFrame->getData(_curr);
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    Id store_id = _curr.storeId();

    // Load the lines from the table
    vector<AccountLine>& lines = _curr.accounts();
    lines.clear();
    for (int row = 0; row < _table->rows(); ++row) {
	Id account_id = _table->cellValue(row, 0).toId();
	if (account_id == INVALID_ID) continue;

	fixed debit = _table->cellValue(row, 1).toFixed();
	fixed credit = _table->cellValue(row, 2).toFixed();

	if (debit != 0.0)
	    lines.push_back(AccountLine(account_id, debit));
	else
	    lines.push_back(AccountLine(account_id, -credit));
    }
}

void
GeneralMaster::cellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _table->rows() - 1 && col == 0) {
	Id id = _table->cellValue(row, col).toId();
	if (id != INVALID_ID)
	    _table->appendRow(new VectorRow(_table->columns()));
    }

    recalculate();
}

void
GeneralMaster::focusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _table->currentRow();
    int col = _table->currentColumn();

    fixed remain = 0.0;
    for (int r = 0; r < _table->rows(); ++r) {
	remain += _table->cellValue(r, 1, false).toFixed();
	remain -= _table->cellValue(r, 2, false).toFixed();
    }

    if (type == Table::MoveNext && col == 0) {
	Id id = _table->cellValue(row, col).toId();
	fixed debit = _table->cellValue(row, 1).toFixed();
	fixed credit = _table->cellValue(row, 2).toFixed();
	if (id == INVALID_ID && row == _table->rows() - 1) {
	    leave = true;
	} else if (debit == 0.0 && credit == 0.0 && remain < 0.0) {
	    _table->setCellValue(row, 1, -remain);
	    recalculate();
	}
    } else if (type == Table::MoveNext && col == 1) {
	fixed debit = _table->cellValue(row, 1).toFixed();
	fixed credit = _table->cellValue(row, 2).toFixed();
	if (debit != 0.0) {
	    newRow = QMIN(row + 1, _table->rows() - 1);
	    newCol = 0;
	} else if (credit == 0.0 && remain > 0.0) {
	    _table->setCellValue(row, 2, remain);
	    recalculate();
	}
    } else if (type == Table::MovePrev && col == 0 && row > 0) {
	fixed debit = _table->cellValue(row - 1, 1).toFixed();
	newRow = row - 1;
	if (debit != 0.0)
	    newCol = 1;
	else
	    newCol = 2;
    } else if (type == Table::MovePrev && col == 2) {
	fixed debit = _table->cellValue(row, 1).toFixed();
	newRow = row;
	if (debit != 0.0)
	    newCol = 1;
	else
	    newCol = 0;
    }
}

void
GeneralMaster::recalculate()
{
    fixed debitTotal = 0.0;
    fixed creditTotal = 0.0;
    for (int i = 0; i < _table->rows(); ++i) {
	debitTotal += _table->cellValue(i, 1).toFixed();
	creditTotal += _table->cellValue(i, 2).toFixed();
    }
    _debit_total->setFixed(debitTotal);
    _credit_total->setFixed(creditTotal);
    if (debitTotal == creditTotal)
	_difference->clear();
    else
	_difference->setFixed(debitTotal - creditTotal);
}

bool
GeneralMaster::checkReconciled(QWidget* parent, const Gltx& orig, Gltx& curr)
{
    if (orig.id() == INVALID_ID) return true;

    bool hasReconciled = false;
    bool sameInfo = true;
    for (unsigned int i = 0; i < orig.accounts().size(); ++i) {
	if (i >= curr.accounts().size()) {
	    sameInfo = false;
	    continue;
	}

	const AccountLine& line1 = orig.accounts()[i];
	const AccountLine& line2 = curr.accounts()[i];

	if (!line1.cleared.isNull()) hasReconciled = true;
	if (line1.account_id != line2.account_id) sameInfo = false;
	if (line1.amount != line2.amount) sameInfo = false;
    }

    if (sameInfo) {
	curr.accounts() = orig.accounts();
	return true;
    }

    if (hasReconciled) {
	QString message = tr("The transaction you are changing was\n"
	    "reconciled.  Saving these changes would\n"
	    "change a completed reconciliation and is\n"
	    "not allowed.");
	QMessageBox::critical(parent, tr("Error"), message);
	return false;
    }

    return true;
}

void
GeneralMaster::slotRecurring()
{
    if (!saveItem(true)) return;

    RecurringMaster* screen = new RecurringMaster(_main);
    screen->setGltx(_curr.id());
    screen->show();
}
