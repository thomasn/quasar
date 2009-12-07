// $Id: expense_master.cpp,v 1.15 2004/01/31 01:50:30 arandell Exp $
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

#include "expense_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "expense_select.h"
#include "line_edit.h"
#include "lookup_edit.h"
#include "tax_lookup.h"
#include "account_lookup.h"
#include "account.h"

#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>

ExpenseMaster::ExpenseMaster(MainWindow* main, Id expense_id)
    : DataWindow(main, "ExpenseMaster", expense_id)
{
    _helpSource = "expense_master.html";

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("&Name:"), _frame);
    _name = new LineEdit(_frame);
    _name->setLength(20);
    nameLabel->setBuddy(_name);

    QLabel* numberLabel = new QLabel(tr("Number:"), _frame);
    _number = new LineEdit(5, _frame);
    numberLabel->setBuddy(_number);

    QLabel* taxLabel = new QLabel(tr("Tax:"), _frame);
    _tax = new LookupEdit(new TaxLookup(main, this), _frame);
    _tax->setLength(6);
    taxLabel->setBuddy(_tax);

    QLabel* accountLabel = new QLabel(tr("GL Account:"), _frame);
    _account = new LookupEdit(new AccountLookup(main, this, Account::Expense),
			      _frame);
    _account->setLength(30);
    accountLabel->setBuddy(_account);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(4, 1);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(numberLabel, 1, 0);
    grid->addWidget(_number, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(taxLabel, 2, 0);
    grid->addWidget(_tax, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(accountLabel, 3, 0);
    grid->addWidget(_account, 3, 1, AlignLeft | AlignVCenter);

    setCaption(tr("Expense Master"));
    finalize();
}

ExpenseMaster::~ExpenseMaster()
{
}

void
ExpenseMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _name;
}

void
ExpenseMaster::newItem()
{
    Expense blank;
    _orig = blank;

    _curr = _orig;
    _firstField = _name;
}

void
ExpenseMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _name->selectAll();
}

bool
ExpenseMaster::fileItem()
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
ExpenseMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
ExpenseMaster::restoreItem()
{
    _curr = _orig;
}

void
ExpenseMaster::cloneItem()
{
    ExpenseMaster* clone = new ExpenseMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
ExpenseMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
ExpenseMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _number->setText(_curr.number());
    _tax->setId(_curr.taxId());
    _account->setId(_curr.accountId());
    _inactive->setChecked(!_curr.isActive());
}

// Set the data object from the widgets.
void
ExpenseMaster::widgetToData()
{
    _curr.setName(_name->text());
    _curr.setNumber(_number->text());
    _curr.setTaxId(_tax->getId());
    _curr.setAccountId(_account->getId());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());
}
