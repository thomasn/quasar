// $Id: subdept_master.cpp,v 1.44 2005/01/11 06:51:54 bpepers Exp $
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

#include "subdept_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "subdept_select.h"
#include "lookup_edit.h"
#include "dept_lookup.h"
#include "tax_lookup.h"
#include "account_lookup.h"
#include "tax_lookup.h"
#include "percent_edit.h"
#include "table.h"
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

SubdeptMaster::SubdeptMaster(MainWindow* main, Id subdept_id)
    : DataWindow(main, "SubdeptMaster", subdept_id)
{
    _helpSource = "subdept_master.html";

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("&Name:"), _frame);
    _name = new LineEdit(30, _frame);
    nameLabel->setBuddy(_name);

    QLabel* numberLabel = new QLabel(tr("Number:"), _frame);
    _number = new LineEdit(5, _frame);
    numberLabel->setBuddy(_number);

    QLabel* deptLabel = new QLabel(tr("&Department:"), _frame);
    _dept = new LookupEdit(new DeptLookup(main, this), _frame);
    _dept->setLength(30);
    deptLabel->setBuddy(_dept);

    QGroupBox* flags = new QGroupBox(tr("Type"), _frame);
    QGridLayout* f_grid = new QGridLayout(flags, 2, 2,
					  flags->frameWidth() * 2);
    f_grid->addRowSpacing(0, flags->fontMetrics().height());

    _purchased = new QCheckBox(tr("Purchased?"), flags);
    _sold = new QCheckBox(tr("Sold?"), flags);
    _inventoried = new QCheckBox(tr("Inventoried?"), flags);

    _expenseLabel = new QLabel(tr("COGS Account:"), flags);
    _expenseLookup = new AccountLookup(main, this, Account::COGS);
    _expense_acct = new LookupEdit(_expenseLookup, flags);
    _expense_acct->setLength(30);
    _expenseLabel->setBuddy(_expense_acct);

    QLabel* incomeLabel = new QLabel(tr("Income Account:"), flags);
    AccountLookup* il = new AccountLookup(main, this, Account::Income);
    _income_acct = new LookupEdit(il, flags);
    _income_acct->setLength(30);
    incomeLabel->setBuddy(_income_acct);

    QLabel* assetLabel = new QLabel(tr("Asset Account:"), flags);
    AccountLookup* al = new AccountLookup(main, this, Account::Inventory);
    _asset_acct = new LookupEdit(al, flags);
    _asset_acct->setLength(30);
    assetLabel->setBuddy(_asset_acct);

    connect(_purchased, SIGNAL(toggled(bool)), SLOT(flagsChanged()));
    connect(_sold, SIGNAL(toggled(bool)), SLOT(flagsChanged()));
    connect(_inventoried, SIGNAL(toggled(bool)), SLOT(flagsChanged()));

    f_grid->setColStretch(1, 1);
    f_grid->addWidget(_purchased, 1, 0, AlignLeft | AlignVCenter);
    f_grid->addWidget(_sold, 2, 0, AlignLeft | AlignVCenter);
    f_grid->addWidget(_inventoried, 3, 0, AlignLeft | AlignVCenter);
    f_grid->addWidget(_expenseLabel, 1, 1, AlignRight | AlignVCenter);
    f_grid->addWidget(_expense_acct, 1, 2, AlignLeft | AlignVCenter);
    f_grid->addWidget(incomeLabel, 2, 1, AlignRight | AlignVCenter);
    f_grid->addWidget(_income_acct, 2, 2, AlignLeft | AlignVCenter);
    f_grid->addWidget(assetLabel, 3, 1, AlignRight | AlignVCenter);
    f_grid->addWidget(_asset_acct, 3, 2, AlignLeft | AlignVCenter);

    QHBox* box = new QHBox(_frame);
    box->setSpacing(3);

    QGroupBox* purchase = new QGroupBox(tr("Purchase Information"), box);
    QGridLayout* p_grid = new QGridLayout(purchase, 2, 2,
					  purchase->frameWidth() * 2);
    p_grid->addRowSpacing(0, purchase->fontMetrics().height());

    QLabel* purchLabel = new QLabel(tr("Purchase Tax:"), purchase);
    _purch_tax = new LookupEdit(new TaxLookup(main, this), purchase);
    _purch_tax->setLength(6);
    purchLabel->setBuddy(_purch_tax);

    p_grid->setColStretch(1, 1);
    p_grid->setRowStretch(3, 1);
    p_grid->addWidget(purchLabel, 1, 0, AlignLeft | AlignVCenter);
    p_grid->addWidget(_purch_tax, 1, 1, AlignLeft | AlignVCenter);

    QGroupBox* sales = new QGroupBox(tr("Sales Information"), box);
    QGridLayout* s_grid = new QGridLayout(sales, 2, 2,
					  sales->frameWidth() * 2);
    s_grid->addRowSpacing(0, sales->fontMetrics().height());

    QLabel* targetLabel = new QLabel(tr("Target GM:"), sales);
    _target_gm = new PercentEdit(sales);
    _target_gm->setLength(10);
    targetLabel->setBuddy(_target_gm);

    QLabel* allowLabel = new QLabel(tr("Variance:"), sales);
    _allowed_var = new PercentEdit(sales);
    _allowed_var->setLength(10);
    allowLabel->setBuddy(_allowed_var);

    QLabel* sellLabel = new QLabel(tr("Selling Tax:"), sales);
    _sell_tax = new LookupEdit(new TaxLookup(main, this), sales);
    _sell_tax->setLength(6);
    sellLabel->setBuddy(_sell_tax);

    _discountable = new QCheckBox(tr("Discountable?"), sales);

    s_grid->setRowStretch(3, 1);
    s_grid->setColStretch(1, 1);
    s_grid->addWidget(targetLabel, 1, 0, AlignLeft | AlignVCenter);
    s_grid->addWidget(_target_gm, 1, 1, AlignLeft | AlignVCenter);
    s_grid->addWidget(allowLabel, 1, 2, AlignLeft | AlignVCenter);
    s_grid->addWidget(_allowed_var, 1, 3, AlignLeft | AlignVCenter);
    s_grid->addWidget(sellLabel, 2, 0, AlignLeft | AlignVCenter);
    s_grid->addWidget(_sell_tax, 2, 1, AlignLeft | AlignVCenter);
    s_grid->addMultiCellWidget(_discountable, 2, 2, 2, 3,
			       AlignLeft | AlignVCenter);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setMargin(3);
    grid->setSpacing(3);
    grid->setColStretch(2, 1);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(numberLabel, 0, 3);
    grid->addWidget(_number, 0, 4, AlignLeft | AlignVCenter);
    grid->addWidget(deptLabel, 1, 0);
    grid->addWidget(_dept, 1, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(flags, 2, 2, 0, 4);
    grid->addMultiCellWidget(box, 3, 3, 0, 4);

    setCaption(tr("Subdepartment Master"));
    finalize();
}

SubdeptMaster::~SubdeptMaster()
{
}

void
SubdeptMaster::setName(const QString& name)
{
    _name->setText(name);
}

void
SubdeptMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _name;
}

void
SubdeptMaster::newItem()
{
    Subdept blank;
    _orig = blank;

    _curr = _orig;
    _firstField = _name;
}

void
SubdeptMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _name->selectAll();
}

bool
SubdeptMaster::fileItem()
{
    if (!_curr.isPurchased()) {
	_curr.setPurchaseTax(INVALID_ID);
    }
    if (!_curr.isSold()) {
	_curr.setSellTax(INVALID_ID);
    }

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
SubdeptMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
SubdeptMaster::restoreItem()
{
    _curr = _orig;
}

void
SubdeptMaster::cloneItem()
{
    SubdeptMaster* clone = new SubdeptMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
SubdeptMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
SubdeptMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _number->setText(_curr.number());
    _dept->setId(_curr.deptId());
    _purchased->setChecked(_curr.isPurchased());
    _sold->setChecked(_curr.isSold());
    _inventoried->setChecked(_curr.isInventoried());
    _sell_tax->setId(_curr.sellTax());
    _purch_tax->setId(_curr.purchaseTax());
    _target_gm->setFixed(_curr.targetGM());
    _allowed_var->setFixed(_curr.allowedVariance());
    _expense_acct->setId(_curr.expenseAccount());
    _income_acct->setId(_curr.incomeAccount());
    _asset_acct->setId(_curr.assetAccount());
    _discountable->setChecked(_curr.isDiscountable());
    _inactive->setChecked(!_curr.isActive());

    flagsChanged();
}

// Set the data object from the widgets.
void
SubdeptMaster::widgetToData()
{
    bool purchased = _purchased->isChecked();
    bool sold = _sold->isChecked();
    bool inventoried = _inventoried->isChecked();

    _curr.setName(_name->text());
    _curr.setNumber(_number->text());
    _curr.setDeptId(_dept->getId());
    _curr.setPurchased(purchased);
    _curr.setSold(sold);
    _curr.setInventoried(inventoried);
    _curr.setTargetGM(_target_gm->getFixed());
    _curr.setAllowedVariance(_allowed_var->getFixed());
    _curr.setDiscountable(_discountable->isChecked());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    // Taxes set based on flags
    if (purchased) {
	_curr.setPurchaseTax(_purch_tax->getId());
    } else {
	_curr.setPurchaseTax(INVALID_ID);
    }
    if (sold) {
	_curr.setSellTax(_sell_tax->getId());
    } else {
	_curr.setSellTax(INVALID_ID);
    }

    // Ledger accounts set based on flags
    if (sold) {
	_curr.setIncomeAccount(_income_acct->getId());
    } else {
	_curr.setIncomeAccount(INVALID_ID);
    }
    if (inventoried) {
	_curr.setAssetAccount(_asset_acct->getId());
    } else {
	_curr.setAssetAccount(INVALID_ID);
    }
    if ((inventoried && sold) || (purchased && !inventoried)) {
	_curr.setExpenseAccount(_expense_acct->getId());
    } else {
	_curr.setExpenseAccount(INVALID_ID);
    }
}

void
SubdeptMaster::flagsChanged()
{
    bool purchased = _purchased->isChecked();
    bool sold = _sold->isChecked();
    bool inventoried = _inventoried->isChecked();

    // Set accounts based on flags
    _asset_acct->setEnabled(inventoried);
    _income_acct->setEnabled(sold);
    _expense_acct->setEnabled((inventoried&&sold)||(purchased&&!inventoried));

    // Set purchasing info based on flags
    _purch_tax->setEnabled(purchased);

    // Set selling info based on flags
    _sell_tax->setEnabled(sold);
    _target_gm->setEnabled(sold && purchased);
    _allowed_var->setEnabled(sold && purchased);

    QString typeName;
    if (inventoried && sold) {
	_expenseLabel->setText(tr("COGS Account:"));
	typeName = Account::typeName(Account::COGS);
    } else {
	_expenseLabel->setText(tr("Expense Account:"));
	typeName = Account::typeName(Account::Expense);
    }
    _expenseLookup->type->setCurrentText(typeName);
}
