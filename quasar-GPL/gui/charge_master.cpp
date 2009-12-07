// $Id: charge_master.cpp,v 1.17 2004/11/12 18:31:57 bpepers Exp $
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

#include "charge_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "charge_select.h"
#include "percent_edit.h"
#include "money_edit.h"
#include "lookup_edit.h"
#include "account_lookup.h"
#include "tax_lookup.h"
#include "account.h"

#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlayout.h>

ChargeMaster::ChargeMaster(MainWindow* main, Id charge_id)
    : DataWindow(main, "ChargeMaster", charge_id)
{
    _helpSource = "charge_master.html";

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("&Name:"), _frame);
    _name = new LineEdit(_frame);
    _name->setLength(20);
    nameLabel->setBuddy(_name);

    QLabel* accountLabel = new QLabel(tr("GL Account:"), _frame);
    _account = new LookupEdit(new AccountLookup(main, this, Account::Expense),
			      _frame);
    _account->setLength(30);
    accountLabel->setBuddy(_account);

    QLabel* taxLabel = new QLabel(tr("Tax:"), _frame);
    _tax = new LookupEdit(new TaxLookup(main, this), _frame);
    _tax->setLength(6);
    taxLabel->setBuddy(_tax);

    _incTax = new QCheckBox(tr("Amount includes tax?"), _frame);

    QGroupBox* calc = new QGroupBox(tr("Calculation Method"), _frame);
    QGridLayout* calcGrid = new QGridLayout(calc, 4, 2, calc->frameWidth()*2);
    calcGrid->addRowSpacing(0, calc->fontMetrics().height());
    calcGrid->setColStretch(1, 1);

    _calcManual = new QRadioButton(tr("Manual"), calc);

    _calcCost = new QRadioButton(tr("Cost"), calc);
    _percent = new PercentEdit(calc);
    _percent->setLength(10);

    _calcWeight = new QRadioButton(tr("Weight"), calc);
    _dollar = new MoneyEdit(calc);
    _dollar->setLength(10);

    QButtonGroup* calcButtons = new QButtonGroup(this);
    calcButtons->hide();
    calcButtons->insert(_calcManual);
    calcButtons->insert(_calcCost);
    calcButtons->insert(_calcWeight);
    connect(calcButtons, SIGNAL(clicked(int)), SLOT(slotCalcChanged()));

    calcGrid->addWidget(_calcManual, 1, 0);
    calcGrid->addWidget(_calcCost, 2, 0);
    calcGrid->addWidget(_percent, 2, 1, AlignLeft | AlignVCenter);
    calcGrid->addWidget(_calcWeight, 3, 0);
    calcGrid->addWidget(_dollar, 3, 1, AlignLeft | AlignVCenter);

    QLabel* allocMethodLabel = new QLabel(tr("Allocation Method:"), _frame);
    _alloc_method = new QComboBox(false, _frame);
    _alloc_method->insertItem(tr("None"));
    _alloc_method->insertItem(tr("Cost"));
    _alloc_method->insertItem(tr("Weight"));
    allocMethodLabel->setBuddy(_alloc_method);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(accountLabel, 1, 0);
    grid->addWidget(_account, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(taxLabel, 2, 0);
    grid->addWidget(_tax, 2, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_incTax, 3, 3, 0, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(calc, 4, 4, 0, 1);
    grid->addWidget(allocMethodLabel, 5, 0);
    grid->addWidget(_alloc_method, 5, 1, AlignLeft | AlignVCenter);

    _calcManual->setChecked(true);

    setCaption(tr("Charge Master"));
    finalize();
}

ChargeMaster::~ChargeMaster()
{
}

void
ChargeMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _name;
}

void
ChargeMaster::newItem()
{
    Charge blank;
    _orig = blank;

    _curr = _orig;
    _firstField = _name;
}

void
ChargeMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _name->selectAll();
}

bool
ChargeMaster::fileItem()
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
ChargeMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
ChargeMaster::restoreItem()
{
    _curr = _orig;
}

void
ChargeMaster::cloneItem()
{
    ChargeMaster* clone = new ChargeMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
ChargeMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
ChargeMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _account->setId(_curr.accountId());
    _tax->setId(_curr.taxId());
    _incTax->setChecked(_curr.includeTax());
    _alloc_method->setCurrentItem(_curr.allocateMethod());
    _inactive->setChecked(!_curr.isActive());

    switch (_curr.calculateMethod()) {
    case Charge::MANUAL:
	_calcManual->setChecked(true);
	_percent->setEnabled(false);
	_dollar->setEnabled(false);
	break;
    case Charge::COST:
	_calcCost->setChecked(true);
	_percent->setEnabled(true);
	_percent->setFixed(_curr.amount());
	_dollar->setEnabled(false);
	break;
    case Charge::WEIGHT:
	_calcWeight->setChecked(true);
	_percent->setEnabled(false);
	_dollar->setEnabled(true);
	_dollar->setFixed(_curr.amount());
	break;
    }
}

// Set the data object from the widgets.
void
ChargeMaster::widgetToData()
{
    _curr.setName(_name->text());
    _curr.setAccountId(_account->getId());
    _curr.setTaxId(_tax->getId());
    _curr.setIncludeTax(_incTax->isChecked());
    _curr.setAllocateMethod(Charge::Method(_alloc_method->currentItem()));
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    if (_calcManual->isChecked()) {
	_curr.setCalculateMethod(Charge::MANUAL);
    } else if (_calcCost->isChecked()) {
	_curr.setCalculateMethod(Charge::COST);
	_curr.setAmount(_percent->getFixed());
    } else {
	_curr.setCalculateMethod(Charge::WEIGHT);
	_curr.setAmount(_dollar->getFixed());
    }
}

void
ChargeMaster::slotCalcChanged()
{
    _percent->setEnabled(_calcCost->isChecked());
    _dollar->setEnabled(_calcWeight->isChecked());
}
