// $Id: discount_master.cpp,v 1.12 2004/01/31 01:50:30 arandell Exp $
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

#include "discount_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "discount_select.h"
#include "lookup_edit.h"
#include "account_lookup.h"
#include "line_edit.h"
#include "percent_edit.h"
#include "money_edit.h"
#include "account.h"

#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qlayout.h>

DiscountMaster::DiscountMaster(MainWindow* main, Id discount_id)
    : DataWindow(main, "DiscountMaster", discount_id)
{
    _helpSource = "discount_master.html";

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("&Name:"), _frame);
    _name = new LineEdit(_frame);
    _name->setLength(20);
    nameLabel->setBuddy(_name);

    _line = new QCheckBox(tr("Line Discounts?"), _frame);
    _tx = new QCheckBox(tr("Transaction Discounts?"), _frame);

    QLabel* accountLabel = new QLabel(tr("Account:"), _frame);
    AccountLookup* lookup = new AccountLookup(_main, this, Account::Expense);
    _account = new LookupEdit(lookup, _frame);
    _account->setLength(30);

    _percent = new QRadioButton(tr("Percent:"), _frame);
    _percentAmt = new PercentEdit(_frame);

    _dollar = new QRadioButton(tr("Dollar:"), _frame);
    _dollarAmt = new MoneyEdit(_frame);

    QButtonGroup* methods = new QButtonGroup(this);
    methods->hide();
    methods->insert(_percent);
    methods->insert(_dollar);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_line, 1, 1, 0, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_tx, 2, 2, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(accountLabel, 3, 0);
    grid->addWidget(_account, 3, 1, AlignLeft | AlignVCenter);
    grid->addWidget(_percent, 4, 0);
    grid->addWidget(_percentAmt, 4, 1, AlignLeft | AlignVCenter);
    grid->addWidget(_dollar, 5, 0);
    grid->addWidget(_dollarAmt, 5, 1, AlignLeft | AlignVCenter);

    setCaption(tr("Discount Master"));
    finalize();
}

DiscountMaster::~DiscountMaster()
{
}

void
DiscountMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _name;
}

void
DiscountMaster::newItem()
{
    Discount blank;
    _orig = blank;

    _curr = _orig;
    _firstField = _name;
}

void
DiscountMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _name->selectAll();
}

bool
DiscountMaster::fileItem()
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
DiscountMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
DiscountMaster::restoreItem()
{
    _curr = _orig;
}

void
DiscountMaster::cloneItem()
{
    DiscountMaster* clone = new DiscountMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
DiscountMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
DiscountMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _line->setChecked(_curr.lineDiscount());
    _tx->setChecked(_curr.txDiscount());
    _account->setId(_curr.accountId());
    if (_curr.method() == Discount::PERCENT) {
	_percent->setChecked(true);
	_percentAmt->setFixed(_curr.amount());
    } else {
	_dollar->setChecked(true);
	_dollarAmt->setFixed(_curr.amount());
    }
    _inactive->setChecked(!_curr.isActive());
}

// Set the data object from the widgets.
void
DiscountMaster::widgetToData()
{
    _curr.setName(_name->text());
    _curr.setLineDiscount(_line->isChecked());
    _curr.setTxDiscount(_tx->isChecked());
    _curr.setAccountId(_account->getId());
    if (_percent->isChecked()) {
	_curr.setMethod(Discount::PERCENT);
	_curr.setAmount(_percentAmt->getFixed());
    } else {
	_curr.setMethod(Discount::DOLLAR);
	_curr.setAmount(_dollarAmt->getFixed());
    }
    _quasar->db()->setActive(_curr, !_inactive->isChecked());
}
