// $Id: account_master.cpp,v 1.47 2004/01/31 01:50:30 arandell Exp $
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

#include "account_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "account_select.h"
#include "company.h"
#include "group.h"
#include "general.h"
#include "general_select.h"
#include "lookup_edit.h"
#include "account_lookup.h"
#include "group_lookup.h"
#include "date_edit.h"
#include "integer_edit.h"
#include "table.h"

#include <qapplication.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlayout.h>

AccountMaster::AccountMaster(MainWindow* main, Id account_id)
    : DataWindow(main, "AccountMaster", account_id)
{
    _helpSource = "account_master.html";

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("&Name:"), _frame);
    _name = new LineEdit(_frame);
    _name->setLength(30);
    nameLabel->setBuddy(_name);

    QLabel* numberLabel = new QLabel(tr("N&umber:"), _frame);
    _number = new LineEdit(_frame);
    _number->setLength(12);
    numberLabel->setBuddy(_number);

    QLabel* typeLabel = new QLabel(tr("&Type:"), _frame);
    _type = new QComboBox(false, _frame);
    typeLabel->setBuddy(_type);
    connect(_type, SIGNAL(activated(int)), SLOT(typeChanged(int)));

    for (int type = Account::Bank; type <= Account::OtherExpense; ++type) {
	_type->insertItem(Account::typeName(Account::Type(type)));
    }
    _type->setMinimumSize(_type->sizeHint());
    _type->setMaximumSize(_type->sizeHint());

    QLabel* parentLabel = new QLabel(tr("&Parent:"), _frame);
    _parentLookup = new AccountLookup(main, this, -1, true, false);
    _parent = new LookupEdit(_parentLookup, _frame);
    parentLabel->setBuddy(_parent);

    QFrame* boxes = new QFrame(_frame);

    _header = new QCheckBox(tr("Header Account?"), boxes);

    QLabel* nextNumLabel = new QLabel(tr("Next Cheque #:"), boxes);
    _nextNum = new IntegerEdit(boxes);
    _nextNum->setLength(6);

    QLabel* lastReconLabel = new QLabel(tr("Last Reconciled:"), boxes);
    _lastRecon = new DateEdit(boxes);
    _lastRecon->setEnabled(false);

    QGroupBox* groups = new QGroupBox(tr("Groups"), boxes);
    QGridLayout* g_grid = new QGridLayout(groups, 2, 1,groups->frameWidth()*2);
    g_grid->addRowSpacing(0, groups->fontMetrics().height());

    _groups = new Table(groups);
    _groups->setVScrollBarMode(QScrollView::AlwaysOn);
    _groups->setDisplayRows(2);
    _groups->setLeftMargin(fontMetrics().width("999"));
    connect(_groups, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(groupCellChanged(int,int,Variant)));

    GroupLookup* groupLookup = new GroupLookup(_main, this, Group::ACCOUNT);
    new LookupColumn(_groups, tr("Group Name"), 15, groupLookup);
    new LookupEditor(_groups, 0, new LookupEdit(groupLookup, _groups));

    g_grid->addWidget(_groups, 1, 0);

    QGridLayout* grid1 = new QGridLayout(boxes);
    grid1->setSpacing(3);
    grid1->setMargin(3);
    grid1->setRowStretch(2, 1);
    grid1->setColStretch(1, 1);
    grid1->addWidget(_header, 0, 0);
    grid1->addWidget(nextNumLabel, 1, 0, AlignLeft | AlignVCenter);
    grid1->addWidget(_nextNum, 1, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(lastReconLabel, 3, 0, AlignLeft | AlignVCenter);
    grid1->addWidget(_lastRecon, 3, 1, AlignLeft | AlignVCenter);
    grid1->addMultiCellWidget(groups, 0, 3, 2, 2);

    QGridLayout *grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1);
    grid->addWidget(numberLabel, 0, 2);
    grid->addWidget(_number, 0, 3, AlignLeft | AlignVCenter);
    grid->addWidget(typeLabel, 1, 0);
    grid->addWidget(_type, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(parentLabel, 1, 2);
    grid->addWidget(_parent, 1, 3);
    grid->addMultiCellWidget(boxes, 2, 2, 0, 3);

    setCaption(tr("Account Master"));
    finalize();
}

AccountMaster::~AccountMaster()
{
}

void
AccountMaster::setName(const QString& name)
{
    _name->setText(name);
}

void
AccountMaster::setType(int type)
{
    _orig.setType(Account::Type(type));
    _type->setCurrentItem(type);
    typeChanged(type);
}

void
AccountMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);
    _type->setEnabled(false);
    _header->setEnabled(false);

    _curr = _orig;
    _firstField = _name;
}

void
AccountMaster::newItem()
{
    Account blank;
    _orig = blank;
    _type->setEnabled(true);
    _header->setEnabled(true);

    _firstField = _name;
    _curr = _orig;
}

void
AccountMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _name->selectAll();
}

bool
AccountMaster::fileItem()
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
AccountMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
AccountMaster::restoreItem()
{
    _curr = _orig;
}

void
AccountMaster::cloneItem()
{
    AccountMaster* clone = new AccountMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
AccountMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
AccountMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _number->setText(_curr.number());
    _type->setCurrentItem(_curr.type());
    _parent->setId(_curr.parentId());
    _parentLookup->type->setCurrentText(_curr.typeName());
    _parentLookup->skipId = _curr.id();
    _header->setChecked(_curr.isHeader());
    _lastRecon->setDate(_curr.lastReconciled());
    _nextNum->setFixed(_curr.nextNumber());
    _inactive->setChecked(!_curr.isActive());

    // Set groups
    _groups->clear();
    for (unsigned int i = 0; i < _curr.groups().size(); ++i) {
	VectorRow* row = new VectorRow(_groups->columns());
	row->setValue(0, _curr.groups()[i]);
	_groups->appendRow(row);
    }
    _groups->appendRow(new VectorRow(_groups->columns()));

    typeChanged(_curr.type());
}

// Set the data object from the widgets.  Return true if no errors.
void
AccountMaster::widgetToData()
{
    _curr.setName(_name->text());
    _curr.setNumber(_number->text());
    _curr.setType(Account::Type(_type->currentItem()));
    _curr.setHeader(_header->isChecked());
    _curr.setParentId(_parent->getId());
    _curr.setNextNumber(_nextNum->getFixed().toInt());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    _curr.groups().clear();
    if (_curr.type() > Account::Equity) {
	for (int row = 0; row < _groups->rows(); ++row) {
	    Id group_id = _groups->cellValue(row, 0).toId();
	    if (group_id != INVALID_ID)
		_curr.groups().push_back(group_id);
	}
    }
}

void
AccountMaster::typeChanged(int index)
{
    _curr.setType(Account::Type(index));
    _parentLookup->type->setCurrentText(_curr.typeName());
    _nextNum->setEnabled(index == Account::Bank);
    _groups->setEnabled(index > Account::Equity);

    if (index != _curr.type())
	_parent->setId(INVALID_ID);
}

void
AccountMaster::groupCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _groups->rows() - 1 && col == 0) {
	Id group_id = _groups->cellValue(row, col).toId();
	if (group_id != INVALID_ID)
	    _groups->appendRow(new VectorRow(_groups->columns()));
    }
}
