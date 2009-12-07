// $Id: term_master.cpp,v 1.13 2004/01/31 01:50:31 arandell Exp $
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

#include "term_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "term_select.h"
#include "integer_edit.h"
#include "percent_edit.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qwidgetstack.h>
#include <qframe.h>
#include <qlayout.h>
#include <qwhatsthis.h>

TermMaster::TermMaster(MainWindow* main, Id term_id)
    : DataWindow(main, "TermMaster", term_id)
{
    _helpSource = "term_master.html";

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("Name:"), _frame);
    _name = new LineEdit(_frame);
    _name->setFocusPolicy(NoFocus);
    _name->setLength(20);
    nameLabel->setBuddy(_name);

    _cod = new QCheckBox(tr("Cash On Delivery?"), _frame);

    QLabel* dueDaysLabel = new QLabel(tr("Due Days:"), _frame);
    _due_days = new IntegerEdit(_frame);
    _due_days->setLength(8);
    dueDaysLabel->setBuddy(_due_days);

    QLabel* discDaysLabel = new QLabel(tr("Discount Days:"), _frame);
    _disc_days = new IntegerEdit(_frame);
    _disc_days->setLength(8);
    discDaysLabel->setBuddy(_disc_days);

    QLabel* discountLabel = new QLabel(tr("&Discount:"), _frame);
    _discount = new PercentEdit(_frame);
    _discount->setLength(8);
    discountLabel->setBuddy(_discount);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->addWidget(nameLabel, 0, 0);
    grid->addMultiCellWidget(_name, 0, 0, 1, 3, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_cod, 1, 1, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(dueDaysLabel, 2, 0);
    grid->addWidget(_due_days, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(discDaysLabel, 3, 0);
    grid->addWidget(_disc_days, 3, 1, AlignLeft | AlignVCenter);
    grid->addWidget(discountLabel, 2, 2);
    grid->addWidget(_discount, 2, 3, AlignLeft | AlignVCenter);

    connect(_cod, SIGNAL(clicked()), SLOT(slotNameChange()));
    connect(_due_days, SIGNAL(validData()), SLOT(slotNameChange()));
    connect(_disc_days, SIGNAL(validData()), SLOT(slotNameChange()));
    connect(_discount, SIGNAL(validData()), SLOT(slotNameChange()));
    slotNameChange();

    setCaption(tr("Terms Master"));
    finalize();
}

TermMaster::~TermMaster()
{
}

void
TermMaster::slotNameChange()
{
    Term term;
    term.setCOD(_cod->isChecked());
    term.setDueDays(_due_days->getInt());
    term.setDiscountDays(_disc_days->getInt());
    term.setDiscount(_discount->getFixed());
    _name->setText(term.name());

    _due_days->setEnabled(!_cod->isChecked());
    _disc_days->setEnabled(!_cod->isChecked());
    _discount->setEnabled(!_cod->isChecked());
}

void
TermMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _cod;
}

void
TermMaster::newItem()
{
    Term blank;
    _orig = blank;

    _curr = _orig;
    _firstField = _cod;
}

void
TermMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
}

bool
TermMaster::fileItem()
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
TermMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
TermMaster::restoreItem()
{
    _curr = _orig;
}

void
TermMaster::cloneItem()
{
    TermMaster* clone = new TermMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
TermMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
TermMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _cod->setChecked(_curr.isCOD());
    _due_days->setInt(_curr.dueDays());
    _disc_days->setInt(_curr.discountDays());
    _discount->setFixed(_curr.discount());
    _inactive->setChecked(!_curr.isActive());

    slotNameChange();
}

// Set the data object from the widgets.
void
TermMaster::widgetToData()
{
    _curr.setCOD(_cod->isChecked());
    _curr.setDueDays(_due_days->getInt());
    _curr.setDiscountDays(_disc_days->getInt());
    _curr.setDiscount(_discount->getFixed());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());
}
