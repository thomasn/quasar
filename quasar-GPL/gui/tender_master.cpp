// $Id: tender_master.cpp,v 1.46 2004/01/31 01:50:31 arandell Exp $
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

#include "tender_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "tender_select.h"
#include "money_edit.h"
#include "double_edit.h"
#include "id_edit.h"
#include "lookup_edit.h"
#include "combo_box.h"
#include "table.h"
#include "account_lookup.h"
#include "account.h"

#include <qapplication.h>
#include <qtabwidget.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>

TenderMaster::TenderMaster(MainWindow* main, Id tender_id)
    : DataWindow(main, "TenderMaster", tender_id)
{
    _helpSource = "tender_master.html";

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("&Name:"), _frame);
    _name = new LineEdit(_frame);
    _name->setLength(30);
    nameLabel->setBuddy(_name);

    QLabel* typeLabel = new QLabel(tr("Type:"), _frame);
    _type = new ComboBox(_frame);
    typeLabel->setBuddy(_type);

    _type->insertItem(Tender::typeName(Tender::CASH));
    _type->insertItem(Tender::typeName(Tender::CHEQUE));
    _type->insertItem(Tender::typeName(Tender::CARD));
    connect(_type, SIGNAL(activated(int)), SLOT(slotTypeChanged()));

    QLabel* limitLabel = new QLabel(tr("&Limit:"), _frame);
    _limit = new MoneyEdit(_frame);
    _limit->setLength(14);
    limitLabel->setBuddy(_limit);

    QLabel* convertLabel = new QLabel(tr("&Conversion Rate:"), _frame);
    _convertRate = new DoubleEdit(_frame);
    _convertRate->setLength(6);
    convertLabel->setBuddy(_convertRate);

    QLabel* menuNumLabel = new QLabel(tr("Menu Number:"), _frame);
    _menuNum = new IdEdit(_frame);
    _menuNum->setLength(4);
    menuNumLabel->setBuddy(_menuNum);

    _tabs = new QTabWidget(_frame);
    QFrame* flags = new QFrame(_tabs);
    QFrame* accounts = new QFrame(_tabs);
    _denomTab = new QFrame(_tabs);
    _tabs->addTab(flags, tr("Flags"));
    _tabs->addTab(accounts, tr("Accounts"));
    _tabs->addTab(_denomTab, tr("Denominations"));

    _overTender = new QCheckBox(tr("Over Tender?"), flags);
    _openDrawer = new QCheckBox(tr("Open Drawer?"), flags);
    _forceAmount = new QCheckBox(tr("Force Amount?"), flags);
    _secondRcpt = new QCheckBox(tr("Second Receipt?"), flags);

    QGridLayout* flagGrid = new QGridLayout(flags);
    flagGrid->setSpacing(3);
    flagGrid->setMargin(3);
    flagGrid->setRowStretch(2, 1);
    flagGrid->setColStretch(1, 1);
    flagGrid->addWidget(_overTender, 0, 0);
    flagGrid->addWidget(_forceAmount, 0, 2);
    flagGrid->addWidget(_openDrawer, 1, 0);
    flagGrid->addWidget(_secondRcpt, 1, 2);

    QLabel* accountLabel = new QLabel(tr("Transaction Account:"), accounts);
    AccountLookup* acctLookup = new AccountLookup(main, this, Account::Bank);
    _account = new LookupEdit(acctLookup, accounts);
    _account->setLength(30);
    accountLabel->setBuddy(_account);

    QLabel* safeLabel = new QLabel(tr("Safe Account:"), accounts);
    AccountLookup* safeLookup = new AccountLookup(main, this, Account::Bank);
    _safe = new LookupEdit(safeLookup, accounts);
    _safe->setLength(30);
    safeLabel->setBuddy(_safe);

    QLabel* bankLabel = new QLabel(tr("Bank Account:"), accounts);
    AccountLookup* bankLookup = new AccountLookup(main, this, Account::Bank);
    _bank = new LookupEdit(bankLookup, accounts);
    _bank->setLength(30);
    bankLabel->setBuddy(_bank);

    QGridLayout* acctGrid = new QGridLayout(accounts);
    acctGrid->setSpacing(3);
    acctGrid->setMargin(3);
    acctGrid->setRowStretch(4, 1);
    acctGrid->addWidget(accountLabel, 0, 0);
    acctGrid->addWidget(_account, 0, 1, AlignLeft | AlignVCenter);
    acctGrid->addWidget(safeLabel, 1, 0);
    acctGrid->addWidget(_safe, 1, 1, AlignLeft | AlignVCenter);
    acctGrid->addWidget(bankLabel, 2, 0);
    acctGrid->addWidget(_bank, 2, 1, AlignLeft | AlignVCenter);

    _denoms = new Table(_denomTab);
    _denoms->setVScrollBarMode(QScrollView::AlwaysOn);
    _denoms->setDisplayRows(3);
    connect(_denoms, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(denomCellChanged(int,int,Variant)));

    new TextColumn(_denoms, tr("Name"), 14);
    new NumberColumn(_denoms, tr("Multiplier"));
    new LineEditor(_denoms, 0, new LineEdit(_denoms));
    new NumberEditor(_denoms, 1, new DoubleEdit(_denoms));

    QGridLayout* denomGrid = new QGridLayout(_denomTab);
    denomGrid->setSpacing(3);
    denomGrid->setMargin(3);
    denomGrid->setRowStretch(0, 1);
    denomGrid->setColStretch(0, 1);
    denomGrid->addWidget(_denoms, 0, 0);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(5, 1);
    grid->setColStretch(1, 1);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(typeLabel, 1, 0);
    grid->addWidget(_type, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(limitLabel, 2, 0);
    grid->addWidget(_limit, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(convertLabel, 3, 0);
    grid->addWidget(_convertRate, 3, 1, AlignLeft | AlignVCenter);
    grid->addWidget(menuNumLabel, 4, 0);
    grid->addWidget(_menuNum, 4, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_tabs, 5, 5, 0, 1);

    setCaption(tr("Tender Master"));
    finalize();
}

TenderMaster::~TenderMaster()
{
}

void
TenderMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _name;
}

void
TenderMaster::newItem()
{
    Tender blank;
    _orig = blank;

    _curr = _orig;
    _firstField = _name;
}

void
TenderMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _curr.setMenuNumber(-1);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _name->selectAll();
}

bool
TenderMaster::fileItem()
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
TenderMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
TenderMaster::restoreItem()
{
    _curr = _orig;
}

void
TenderMaster::cloneItem()
{
    TenderMaster* clone = new TenderMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
TenderMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
TenderMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _type->setCurrentItem(_curr.type());
    _limit->setFixed(_curr.limit());
    _convertRate->setFixed(_curr.convertRate());
    _overTender->setChecked(_curr.overTender());
    _openDrawer->setChecked(_curr.openDrawer());
    _forceAmount->setChecked(_curr.forceAmount());
    _secondRcpt->setChecked(_curr.secondReceipt());
    _account->setId(_curr.accountId());
    _safe->setId(_curr.safeId());
    _bank->setId(_curr.bankId());
    _menuNum->setFixed(_curr.menuNumber());
    _inactive->setChecked(!_curr.isActive());

    // Load the denominations
    _denoms->clear();
    for (unsigned int i = 0; i < _curr.denominations().size(); ++i) {
	const TenderDenom& line = _curr.denominations()[i];

	VectorRow* row = new VectorRow(_denoms->columns());
	row->setValue(0, line.name);
	row->setValue(1, line.multiplier);
	_denoms->appendRow(row);
    }
    _denoms->appendRow(new VectorRow(_denoms->columns()));

    slotTypeChanged();
}

// Set the data object from the widgets.
void
TenderMaster::widgetToData()
{
    _curr.setName(_name->text());
    _curr.setType(Tender::Type(_type->currentItem()));
    _curr.setLimit(_limit->getFixed());
    _curr.setConvertRate(_convertRate->getFixed());
    _curr.setOverTender(_overTender->isChecked());
    _curr.setOpenDrawer(_openDrawer->isChecked());
    _curr.setForceAmount(_forceAmount->isChecked());
    _curr.setSecondReceipt(_secondRcpt->isChecked());
    _curr.setAccountId(_account->getId());
    _curr.setSafeId(_safe->getId());
    _curr.setBankId(_bank->getId());
    _curr.setMenuNumber(_menuNum->getInt());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    // Add the denominiations
    _curr.denominations().clear();
    if (_curr.type() == Tender::CASH) {
	for (int row = 0; row < _denoms->rows(); ++row) {
	    TenderDenom line;
	    line.name = _denoms->cellValue(row, 0).toString();
	    line.multiplier = _denoms->cellValue(row, 1).toFixed();
	    if (line.name.isEmpty()) continue;

	    _curr.denominations().push_back(line);
	}
    }
}

void
TenderMaster::slotTypeChanged()
{
    _tabs->setTabEnabled(_denomTab, _type->currentItem() == 0);
}

void
TenderMaster::denomCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _denoms->rows() - 1 && col == 0) {
	QString name = _denoms->cellValue(row, col).toString();
	if (!name.isEmpty())
	    _denoms->appendRow(new VectorRow(_denoms->columns()));
    }
}
