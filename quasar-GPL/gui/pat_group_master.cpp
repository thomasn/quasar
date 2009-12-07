// $Id: pat_group_master.cpp,v 1.7 2004/01/31 01:50:30 arandell Exp $
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

#include "pat_group_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "pat_group_select.h"
#include "card.h"
#include "customer_lookup.h"
#include "line_edit.h"
#include "lookup_edit.h"
#include "table.h"
#include "combo_box.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qlayout.h>
#include <qmessagebox.h>

PatGroupMaster::PatGroupMaster(MainWindow* main, Id group_id)
    : DataWindow(main, "PatGroupMaster", group_id)
{
    _helpSource = "pat_group_master.html";

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("Name:"), _frame);
    _name = new LineEdit(_frame);
    _name->setLength(20);
    nameLabel->setBuddy(_name);

    QLabel* numberLabel = new QLabel(tr("Number:"), _frame);
    _number = new LineEdit(_frame);
    _number->setLength(12);
    numberLabel->setBuddy(_number);

    QLabel* cardLabel = new QLabel(tr("Members:"), _frame);
    _cards = new Table(_frame);
    _cards->setVScrollBarMode(QScrollView::AlwaysOn);
    _cards->setDisplayRows(6);
    _cards->setLeftMargin(fontMetrics().width("999"));
    cardLabel->setBuddy(_cards);
    connect(_cards, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(cardCellChanged(int,int,Variant)));
    connect(_cards, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(cardFocusNext(bool&,int&,int&,int)));

    CardLookup* cardLookup = new CardLookup(_main, this);
    cardLookup->type->setCurrentItem(tr("Customer and Vendor"));
    new LookupColumn(_cards, tr("Customer or Vendor"), 20, cardLookup);
    new TextColumn(_cards, tr("Number"), 10, AlignRight);
    new TextColumn(_cards, tr("Type"), 12, AlignRight);

    new LookupEditor(_cards, 0, new LookupEdit(cardLookup, _cards));

    QLabel* equityLabel = new QLabel(tr("Equity Member:"), _frame);
    _equity = new LookupEdit(new CustomerLookup(_main, this), _frame);
    _equity->setLength(30);
    equityLabel->setBuddy(_equity);

    QLabel* creditLabel = new QLabel(tr("Credit Member:"), _frame);
    _credit = new LookupEdit(new CustomerLookup(_main, this), _frame);
    _credit->setLength(30);
    creditLabel->setBuddy(_credit);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(3, 1);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(numberLabel, 1, 0);
    grid->addWidget(_number, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(cardLabel, 2, 0);
    grid->addMultiCellWidget(_cards, 2, 3, 1, 1);
    grid->addWidget(equityLabel, 4, 0);
    grid->addWidget(_equity, 4, 1, AlignLeft | AlignVCenter);
    grid->addWidget(creditLabel, 5, 0);
    grid->addWidget(_credit, 5, 1, AlignLeft | AlignVCenter);

    setCaption(tr("Patronage Group Master"));
    finalize();
}

PatGroupMaster::~PatGroupMaster()
{
}

void
PatGroupMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _name;
}

void
PatGroupMaster::newItem()
{
    PatGroup blank;
    _orig = blank;

    _curr = _orig;
    _firstField = _name;
}

void
PatGroupMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _name->selectAll();
}

bool
PatGroupMaster::fileItem()
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
PatGroupMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
PatGroupMaster::restoreItem()
{
    _curr = _orig;
}

void
PatGroupMaster::cloneItem()
{
    PatGroupMaster* clone = new PatGroupMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
PatGroupMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
PatGroupMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _number->setText(_curr.number());
    _equity->setId(_curr.equityId());
    _credit->setId(_curr.creditId());
    _inactive->setChecked(!_curr.isActive());

    // Set cards
    _cards->clear();
    for (unsigned int i = 0; i < _curr.card_ids().size(); ++i) {
	Id card_id = _curr.card_ids()[i];
	Card card;
	_db->lookup(card_id, card);

	VectorRow* row = new VectorRow(_cards->columns());
	row->setValue(0, card_id);
	row->setValue(1, card.number());
	row->setValue(2, card.dataTypeName());
	_cards->appendRow(row);
    }
    _cards->appendRow(new VectorRow(_cards->columns()));

}

// Set the data object from the widgets.
void
PatGroupMaster::widgetToData()
{
    _curr.setName(_name->text());
    _curr.setNumber(_number->text());
    _curr.setEquityId(_equity->getId());
    _curr.setCreditId(_credit->getId());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    _curr.card_ids().clear();
    for (int row = 0; row < _cards->rows(); ++row) {
	Id card_id = _cards->cellValue(row, 0).toId();
	if (card_id != INVALID_ID)
	    _curr.card_ids().push_back(card_id);
    }
}

void
PatGroupMaster::cardCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _cards->rows() - 1 && col == 0) {
	Id card_id = _cards->cellValue(row, col).toId();
	if (card_id != INVALID_ID)
	    _cards->appendRow(new VectorRow(_cards->columns()));
    }

    // Check for other changes
    if (col == 0) {
	Id id = _cards->cellValue(row, 0).toId();
	Card card;
	if (_quasar->db()->lookup(id, card)) {
	    _cards->setCellValue(row, 1, card.number());
	    _cards->setCellValue(row, 2, card.dataTypeName());
	} else {
	    _cards->setCellValue(row, 1, "");
	    _cards->setCellValue(row, 2, "");
	}
    }
}

void
PatGroupMaster::cardFocusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _cards->currentRow();
    int col = _cards->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id id = _cards->cellValue(row, col).toId();
	if (id == INVALID_ID && row == _cards->rows() - 1) {
	    leave = true;
	} else {
	    newRow = row + 1;
	    newCol = 0;
	}
    } else if (type == Table::MovePrev && col == 0) {
	if (row == 0) {
	    leave = true;
	} else {
	    newRow = newRow - 1;
	    newCol = 0;
	}
    }
}
