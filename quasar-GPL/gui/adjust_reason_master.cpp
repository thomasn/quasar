// $Id: adjust_reason_master.cpp,v 1.2 2005/01/09 10:02:53 bpepers Exp $
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

#include "adjust_reason_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "adjust_reason_select.h"
#include "double_edit.h"
#include "lookup_edit.h"
#include "account_lookup.h"
#include "account.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qlayout.h>
#include <qmessagebox.h>

AdjustReasonMaster::AdjustReasonMaster(MainWindow* main, Id reason_id)
    : DataWindow(main, "AdjustReasonMaster", reason_id)
{
    _helpSource = "adjust_reason_master.html";

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("&Name:"), _frame);
    _name = new LineEdit(14, _frame);
    nameLabel->setBuddy(_name);

    QLabel* numberLabel = new QLabel(tr("Number:"), _frame);
    _number = new LineEdit(5, _frame);
    numberLabel->setBuddy(_number);

    QLabel* accountLabel = new QLabel(tr("Account:"), _frame);
    AccountLookup* lookup = new AccountLookup(main, this, Account::Expense);
    _account = new LookupEdit(lookup, _frame);
    _account->setLength(30);
    accountLabel->setBuddy(_account);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(3, 1);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(numberLabel, 1, 0);
    grid->addWidget(_number, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(accountLabel, 2, 0);
    grid->addWidget(_account, 2, 1, AlignLeft | AlignVCenter);

    setCaption(tr("Adjustment Reason Master"));
    finalize();
}

AdjustReasonMaster::~AdjustReasonMaster()
{
}

void
AdjustReasonMaster::setName(const QString& name)
{
    _name->setText(name);
}

void
AdjustReasonMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _name;
}

void
AdjustReasonMaster::newItem()
{
    AdjustReason blank;
    _orig = blank;

    _curr = _orig;
    _firstField = _name;
}

void
AdjustReasonMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _name->selectAll();
}

bool
AdjustReasonMaster::fileItem()
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
AdjustReasonMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
AdjustReasonMaster::restoreItem()
{
    _curr = _orig;
}

void
AdjustReasonMaster::cloneItem()
{
    AdjustReasonMaster* clone = new AdjustReasonMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
AdjustReasonMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
AdjustReasonMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _number->setText(_curr.number());
    _account->setId(_curr.accountId());
    _inactive->setChecked(!_curr.isActive());
}

// Set the data object from the widgets.
void
AdjustReasonMaster::widgetToData()
{
    _curr.setName(_name->text());
    _curr.setNumber(_number->text());
    _curr.setAccountId(_account->getId());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());
}
