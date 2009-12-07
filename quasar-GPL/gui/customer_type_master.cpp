// $Id: customer_type_master.cpp,v 1.8 2004/01/31 01:50:30 arandell Exp $
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

#include "customer_type_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "customer_type_select.h"
#include "lookup_edit.h"
#include "account_lookup.h"
#include "term_lookup.h"
#include "tax_lookup.h"
#include "money_edit.h"
#include "percent_edit.h"
#include "account.h"

#include <qapplication.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>

CustomerTypeMaster::CustomerTypeMaster(MainWindow* main, Id type_id)
    : DataWindow(main, "CustomerTypeMaster", type_id)
{
    _helpSource = "customer_type_master.html";

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("&Name:"), _frame);
    _name = new LineEdit(20, _frame);
    nameLabel->setBuddy(_name);

    QLabel* accountLabel = new QLabel(tr("AR Account:"), _frame);
    _account = new LookupEdit(new AccountLookup(main, this, Account::AR),
			      _frame);
    _account->setLength(30);
    accountLabel->setBuddy(_account);

    QLabel* termsLabel = new QLabel(tr("Terms:"), _frame);
    _terms = new LookupEdit(new TermLookup(main, this), _frame);
    _terms->setLength(30);
    termsLabel->setBuddy(_terms);

    QLabel* taxExemptLabel = new QLabel(tr("Tax Exempt:"), _frame);
    _tax_exempt = new LookupEdit(new TaxLookup(main, this), _frame);
    _tax_exempt->setLength(30);
    taxExemptLabel->setBuddy(_tax_exempt);

    QLabel* creditLimitLabel = new QLabel(tr("Credit Limit:"), _frame);
    _credit_limit = new MoneyEdit(_frame);
    _credit_limit->setLength(14);
    creditLimitLabel->setBuddy(_credit_limit);

    QLabel* rateLabel = new QLabel(tr("Service Charge:"), _frame);
    _sc_rate = new PercentEdit(_frame);
    _sc_rate->setLength(8);
    rateLabel->setBuddy(_sc_rate);

    _print_stmts = new QCheckBox(tr("Statements?"), _frame);

    QGroupBox* flags = new QGroupBox(tr("Transactions"), _frame);
    QGridLayout* flagGrid = new QGridLayout(flags, 2, 2,
					    flags->frameWidth() * 2);
    flagGrid->addRowSpacing(0, flags->fontMetrics().height());

    _charge = new QCheckBox(tr("Can Charge?"), flags);
    _withdraw = new QCheckBox(tr("Can Withdraw?"), flags);
    _payment = new QCheckBox(tr("Can Make Payment?"), flags);
    _check_bal = new QCheckBox(tr("Check Withdraw Balance?"), flags);
    _second_rcpt = new QCheckBox(tr("Second Receipt?"), flags);

    flagGrid->setColStretch(0, 1);
    flagGrid->addWidget(_charge, 1, 0, AlignLeft | AlignVCenter);
    flagGrid->addWidget(_withdraw, 2, 0, AlignLeft | AlignVCenter);
    flagGrid->addWidget(_payment, 3, 0, AlignLeft | AlignVCenter);
    flagGrid->addWidget(_check_bal, 4, 0, AlignLeft | AlignVCenter);
    flagGrid->addWidget(_second_rcpt, 5, 0, AlignLeft | AlignVCenter);

    QGridLayout* grid = new QGridLayout(_frame, 4, 2, 3);
    grid->setColStretch(1, 1);
    grid->addWidget(nameLabel, 0, 0);
    grid->addMultiCellWidget(_name, 0, 0, 1, 2, AlignLeft | AlignVCenter);
    grid->addWidget(accountLabel, 1, 0);
    grid->addMultiCellWidget(_account, 1, 1, 1, 2, AlignLeft | AlignVCenter);
    grid->addWidget(termsLabel, 2, 0);
    grid->addMultiCellWidget(_terms, 2, 2, 1, 2, AlignLeft | AlignVCenter);
    grid->addWidget(taxExemptLabel, 3, 0);
    grid->addMultiCellWidget(_tax_exempt, 3, 3, 1, 2, AlignLeft|AlignVCenter);
    grid->addWidget(creditLimitLabel, 4, 0);
    grid->addWidget(_credit_limit, 4, 1, AlignLeft | AlignVCenter);
    grid->addWidget(rateLabel, 5, 0);
    grid->addWidget(_sc_rate, 5, 1, AlignLeft | AlignVCenter);
    grid->addWidget(_print_stmts, 5, 2, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(flags, 6, 6, 0, 2);

    setCaption(tr("Customer Type Master"));
    finalize();
}

CustomerTypeMaster::~CustomerTypeMaster()
{
}

void
CustomerTypeMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _name;
}

void
CustomerTypeMaster::newItem()
{
    CustomerType blank;
    _orig = blank;

    _curr = _orig;
    _firstField = _name;
}

void
CustomerTypeMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _name->selectAll();
}

bool
CustomerTypeMaster::fileItem()
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
CustomerTypeMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
CustomerTypeMaster::restoreItem()
{
    _curr = _orig;
}

void
CustomerTypeMaster::cloneItem()
{
    CustomerTypeMaster* clone = new CustomerTypeMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
CustomerTypeMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
CustomerTypeMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _account->setId(_curr.accountId());
    _terms->setId(_curr.termsId());
    _tax_exempt->setId(_curr.taxExemptId());
    _credit_limit->setFixed(_curr.creditLimit());
    _sc_rate->setFixed(_curr.serviceCharge());
    _print_stmts->setChecked(_curr.printStatements());
    _charge->setChecked(_curr.canCharge());
    _withdraw->setChecked(_curr.canWithdraw());
    _payment->setChecked(_curr.canPayment());
    _check_bal->setChecked(_curr.checkWithdrawBalance());
    _second_rcpt->setChecked(_curr.secondReceipt());
    _inactive->setChecked(!_curr.isActive());
}

// Set the data object from the widgets.
void
CustomerTypeMaster::widgetToData()
{
    _curr.setName(_name->text());
    _curr.setAccountId(_account->getId());
    _curr.setTermsId(_terms->getId());
    _curr.setTaxExemptId(_tax_exempt->getId());
    _curr.setCreditLimit(_credit_limit->getFixed());
    _curr.setServiceCharge(_sc_rate->getFixed());
    _curr.setPrintStatements(_print_stmts->isChecked());
    _curr.setCanCharge(_charge->isChecked());
    _curr.setCanWithdraw(_withdraw->isChecked());
    _curr.setCanPayment(_payment->isChecked());
    _curr.setCheckWithdrawBalance(_check_bal->isChecked());
    _curr.setSecondReceipt(_second_rcpt->isChecked());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());
}
