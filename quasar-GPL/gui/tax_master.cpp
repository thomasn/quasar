// $Id: tax_master.cpp,v 1.34 2004/01/31 01:50:31 arandell Exp $
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

#include "tax_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "tax_select.h"
#include "table.h"
#include "percent_edit.h"
#include "lookup_edit.h"
#include "account_lookup.h"
#include "tax_lookup.h"
#include "account.h"

#include <qapplication.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qlayout.h>
#include <qwhatsthis.h>

TaxMaster::TaxMaster(MainWindow* main, Id tax_id)
    : DataWindow(main, "TaxMaster", tax_id)
{
    _helpSource = "tax_master.html";

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("Name:"), _frame);
    _name = new LineEdit(_frame);
    _name->setMaxLength(4);
    _name->setMaximumWidth(7 * fontMetrics().width('x'));
    nameLabel->setBuddy(_name);

    QLabel* descLabel = new QLabel(tr("Description:"), _frame);
    _desc = new LineEdit(_frame);
    _desc->setMaxLength(20);
    descLabel->setBuddy(_desc);

    _group = new QCheckBox(tr("Tax Group?"), _frame);
    connect(_group, SIGNAL(clicked()), SLOT(groupChanged()));

    _tabs = new QTabWidget(_frame);
    _controlFrame = new QFrame(_tabs);
    _totFrame = new QFrame(_tabs);
    _groupFrame = new QFrame(_tabs);
    _tabs->addTab(_controlFrame, tr("Control"));
    _tabs->addTab(_totFrame, tr("Tax on Tax"));
    _tabs->addTab(_groupFrame, tr("Groups"));

    QLabel* numLabel = new QLabel(tr("Number:"), _controlFrame);
    _number = new LineEdit(_controlFrame);
    _number->setMaxLength(20);
    numLabel->setBuddy(_number);

    QLabel* rateLabel = new QLabel(tr("&Rate:"), _controlFrame);
    _rate = new PercentEdit(_controlFrame);
    _rate->setLength(8);
    rateLabel->setBuddy(_rate);

    QLabel* collectedLabel = new QLabel(tr("Tax Collected:"), _controlFrame);
    AccountLookup* lk1 = new AccountLookup(main, this,
					   Account::OtherCurLiability);
    _collected = new LookupEdit(lk1, _controlFrame);
    _collected->setLength(30);
    collectedLabel->setBuddy(_collected);

    QLabel* paidLabel = new QLabel(tr("Tax Paid:"), _controlFrame);
    AccountLookup* lk2 = new AccountLookup(main, this,
					   Account::OtherCurLiability);
    _paid = new LookupEdit(lk2, _controlFrame);
    _paid->setLength(30);
    paidLabel->setBuddy(_paid);

    QGridLayout* controlGrid = new QGridLayout(_controlFrame);
    controlGrid->setSpacing(3);
    controlGrid->setMargin(3);
    controlGrid->setRowStretch(4, 1);
    controlGrid->addWidget(numLabel, 0, 0);
    controlGrid->addWidget(_number, 0, 1);
    controlGrid->addWidget(rateLabel, 1, 0);
    controlGrid->addWidget(_rate, 1, 1, AlignLeft | AlignVCenter);
    controlGrid->addWidget(collectedLabel, 2, 0);
    controlGrid->addWidget(_collected, 2, 1, AlignLeft | AlignVCenter);
    controlGrid->addWidget(paidLabel, 3, 0);
    controlGrid->addWidget(_paid, 3, 1, AlignLeft | AlignVCenter);

    _tots = new Table(_totFrame);
    _tots->setVScrollBarMode(QScrollView::AlwaysOn);
    _tots->setDisplayRows(4);
    connect(_tots, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(totCellChanged(int,int,Variant)));
    connect(_tots, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(totFocusNext(bool&,int&,int&,int)));

    // Add columns
    TaxLookup* totTaxLookup = new TaxLookup(_main, this);
    new LookupColumn(_tots, tr("Name"), 6, totTaxLookup);
    new TextColumn(_tots, tr("Description"), 20);

    // Add editors
    new LookupEditor(_tots, 0, new LookupEdit(totTaxLookup, _tots));

    QGridLayout* totGrid = new QGridLayout(_totFrame);
    totGrid->setSpacing(3);
    totGrid->setMargin(3);
    totGrid->addWidget(_tots, 0, 0);

    _groups = new Table(_groupFrame);
    _groups->setVScrollBarMode(QScrollView::AlwaysOn);
    _groups->setDisplayRows(4);
    connect(_groups, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(groupCellChanged(int,int,Variant)));
    connect(_groups, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(groupFocusNext(bool&,int&,int&,int)));

    // Add columns
    TaxLookup* taxLookup = new TaxLookup(_main, this);
    new LookupColumn(_groups, tr("Name"), 6, taxLookup);
    new TextColumn(_groups, tr("Description"), 20);

    // Add editors
    new LookupEditor(_groups, 0, new LookupEdit(taxLookup, _groups));

    QGridLayout* groupGrid = new QGridLayout(_groupFrame);
    groupGrid->setSpacing(3);
    groupGrid->setMargin(3);
    groupGrid->addWidget(_groups, 0, 0);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(descLabel, 1, 0);
    grid->addWidget(_desc, 1, 1);
    grid->addMultiCellWidget(_group, 2, 2, 0, 1);
    grid->addMultiCellWidget(_tabs, 3, 3, 0, 1);

    setCaption(tr("Tax Master"));
    finalize();
}

TaxMaster::~TaxMaster()
{
}

void
TaxMaster::setName(const QString& name)
{
    _name->setText(name);
}

void
TaxMaster::groupChanged()
{
    if (_group->isChecked()) {
	_tabs->setTabEnabled(_groupFrame, true);
	_tabs->setTabEnabled(_controlFrame, false);
	_tabs->setTabEnabled(_totFrame, false);
    } else {
	_tabs->setTabEnabled(_controlFrame, true);
	_tabs->setTabEnabled(_totFrame, true);
	_tabs->setTabEnabled(_groupFrame, false);
    }
}

void
TaxMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _name;
}

void
TaxMaster::newItem()
{
    Tax blank;
    _orig = blank;

    _curr = _orig;
    _firstField = _name;
}

void
TaxMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _name->selectAll();
}

bool
TaxMaster::fileItem()
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
TaxMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
TaxMaster::restoreItem()
{
    _curr = _orig;
}

void
TaxMaster::cloneItem()
{
    TaxMaster* clone = new TaxMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
TaxMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
TaxMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _desc->setText(_curr.description());
    _group->setChecked(_curr.isGroup());
    _number->setText(_curr.number());
    _rate->setFixed(_curr.rate());
    _collected->setId(_curr.collectedAccount());
    _paid->setId(_curr.paidAccount());
    _inactive->setChecked(!_curr.isActive());

    // Load the tax on tax
    _tots->clear();
    unsigned int i;
    for (i = 0; i < _curr.tot_ids().size(); ++i) {
	Id tax_id = _curr.tot_ids()[i];

	Tax tax;
	_quasar->db()->lookup(tax_id, tax);

	VectorRow* row = new VectorRow(_tots->columns());
	row->setValue(0, tax_id);
	row->setValue(1, tax.description());
	_tots->appendRow(row);
    }
    _tots->appendRow(new VectorRow(_tots->columns()));

    // Load the groups
    _groups->clear();
    for (i = 0; i < _curr.group_ids().size(); ++i) {
	Id tax_id = _curr.group_ids()[i];

	Tax tax;
	_quasar->db()->lookup(tax_id, tax);

	VectorRow* row = new VectorRow(_groups->columns());
	row->setValue(0, tax_id);
	row->setValue(1, tax.description());
	_groups->appendRow(row);
    }
    _groups->appendRow(new VectorRow(_groups->columns()));

    groupChanged();
}

// Set the data object from the widgets.
void
TaxMaster::widgetToData()
{
    _curr.setName(_name->text());
    _curr.setDescription(_desc->text());
    _curr.setGroup(_group->isChecked());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());
    _curr.group_ids().clear();
    _curr.tot_ids().clear();

    if (_curr.isGroup()) {
	_curr.setNumber("");
	_curr.setRate(0.0);
	_curr.setCollectedAccount(INVALID_ID);
	_curr.setPaidAccount(INVALID_ID);

	for (int row = 0; row < _groups->rows(); ++row) {
	    Id tax_id = _groups->cellValue(row, 0).toId();
	    if (tax_id == INVALID_ID) continue;
	    _curr.group_ids().push_back(tax_id);
	}
    } else {
	_curr.setNumber(_number->text());
	_curr.setRate(_rate->getFixed());
	_curr.setCollectedAccount(_collected->getId());
	_curr.setPaidAccount(_paid->getId());

	for (int row = 0; row < _tots->rows(); ++row) {
	    Id tax_id = _tots->cellValue(row, 0).toId();
	    if (tax_id == INVALID_ID) continue;
	    _curr.tot_ids().push_back(tax_id);
	}
    }
}

void
TaxMaster::totCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _tots->rows() - 1 && col == 0) {
	Id id = _tots->cellValue(row, col).toId();
	if (id != INVALID_ID)
	    _tots->appendRow(new VectorRow(_tots->columns()));
    }

    // Check for other changes
    Id tax_id;
    Tax tax;
    switch (col) {
    case 0:
	tax_id = _tots->cellValue(row, 0).toId();
	if (_quasar->db()->lookup(tax_id, tax)) {
	    _tots->setCellValue(row, 1, tax.description());
	} else {
	    if (row != _tots->rows() - 1)
		_tots->setCellValue(row, 1, tr("**Unknown Tax**"));
	}
	break;
    }
}

void
TaxMaster::totFocusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _tots->currentRow();
    int col = _tots->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id id = _tots->cellValue(row, col).toId();
	if (id == INVALID_ID && row == _tots->rows() - 1) {
	    leave = true;
	} else {
	    newCol = 0;
	    newRow = row + 1;
	}
    }
    if (type == Table::MovePrev && col == 0) {
	if (row > 0) {
	    newCol = 0;
	    newRow = row - 1;
	}
    }
}

void
TaxMaster::groupCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _groups->rows() - 1 && col == 0) {
	Id id = _groups->cellValue(row, col).toId();
	if (id != INVALID_ID)
	    _groups->appendRow(new VectorRow(_groups->columns()));
    }

    // Check for other changes
    Id tax_id;
    Tax tax;
    switch (col) {
    case 0:
	tax_id = _groups->cellValue(row, 0).toId();
	if (_quasar->db()->lookup(tax_id, tax)) {
	    _groups->setCellValue(row, 1, tax.description());
	} else {
	    if (row != _groups->rows() - 1)
		_groups->setCellValue(row, 1, tr("**Unknown Tax**"));
	}
	break;
    }
}

void
TaxMaster::groupFocusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _groups->currentRow();
    int col = _groups->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id id = _groups->cellValue(row, col).toId();
	if (id == INVALID_ID && row == _groups->rows() - 1) {
	    leave = true;
	} else {
	    newCol = 0;
	    newRow = row + 1;
	}
    }
    if (type == Table::MovePrev && col == 0) {
	if (row > 0) {
	    newCol = 0;
	    newRow = row - 1;
	}
    }
}
