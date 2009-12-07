// $Id: security_rule_dialog.cpp,v 1.9 2005/01/08 10:52:41 bpepers Exp $
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

#include "security_rule_dialog.h"
#include "combo_box.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>

SecurityRuleDialog::SecurityRuleDialog(QWidget* parent)
    : QDialog(parent, "SecurityRuleDialog", true)
{
    setCaption(tr("Create Rule"));

    QLabel* screenLabel = new QLabel(tr("Screen:"), this);
    _screen = new ComboBox(this);

    _allowView = new QCheckBox(tr("View?"), this);
    _allowCreate = new QCheckBox(tr("Create?"), this);
    _allowUpdate = new QCheckBox(tr("Update?"), this);
    _allowDelete = new QCheckBox(tr("Delete?"), this);

    QFrame* buttons = new QFrame(this);
    QPushButton* ok = new QPushButton(tr("OK"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);

    connect(ok, SIGNAL(clicked()), SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), SLOT(reject()));
    ok->setDefault(true);

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(ok, 0, 1);
    buttonGrid->addWidget(cancel, 0, 2);

    QGridLayout* grid = new QGridLayout(this);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setColStretch(1, 1);
    grid->addWidget(screenLabel, 0, 0);
    grid->addWidget(_screen, 0, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_allowView, 1, 1, 0, 1);
    grid->addMultiCellWidget(_allowCreate, 2, 2, 0, 1);
    grid->addMultiCellWidget(_allowUpdate, 3, 3, 0, 1);
    grid->addMultiCellWidget(_allowDelete, 4, 4, 0, 1);
    grid->addMultiCellWidget(buttons, 5, 5, 0, 1);

    _screen->insertItem("All");
    _screen->insertItem("AccountMaster");
    _screen->insertItem("AdjustReasonMaster");
    _screen->insertItem("CardAdjustment");
    _screen->insertItem("CardTransfer");
    _screen->insertItem("ChargeMaster");
    _screen->insertItem("ChequeMaster");
    _screen->insertItem("CountMaster");
    _screen->insertItem("CountTender");
    _screen->insertItem("CustomerCheque");
    _screen->insertItem("CustomerInvoice");
    _screen->insertItem("CustomerMaster");
    _screen->insertItem("CustomerPayment");
    _screen->insertItem("CustomerTypeMaster");
    _screen->insertItem("CustomerQuote");
    _screen->insertItem("CustomerWithdraw");
    _screen->insertItem("DeptMaster");
    _screen->insertItem("DiscountMaster");
    _screen->insertItem("EmployeeMaster");
    _screen->insertItem("ExpenseMaster");
    _screen->insertItem("GeneralMaster");
    _screen->insertItem("GroupMaster");
    _screen->insertItem("ItemAdjustment");
    _screen->insertItem("ItemMaster");
    _screen->insertItem("ItemTransfer");
    _screen->insertItem("LabelBatchMaster");
    _screen->insertItem("LedgerTransfer");
    _screen->insertItem("LocationMaster");
    _screen->insertItem("OrderMaster");
    _screen->insertItem("OrderTemplateMaster");
    _screen->insertItem("PatGroupMaster");
    _screen->insertItem("PatWorksheetMaster");
    _screen->insertItem("PayBills");
    _screen->insertItem("PayoutMaster");
    _screen->insertItem("PersonalMaster");
    _screen->insertItem("PriceBatchMaster");
    _screen->insertItem("PriceMaster");
    _screen->insertItem("PromoBatchMaster");
    _screen->insertItem("ReconcileMaster");
    _screen->insertItem("RecurringMaster");
    _screen->insertItem("SecurityTypeMaster");
    _screen->insertItem("SequenceNumber");
    _screen->insertItem("SlipMaster");
    _screen->insertItem("StationMaster");
    _screen->insertItem("StoreMaster");
    _screen->insertItem("SubdeptMaster");
    _screen->insertItem("TaxMaster");
    _screen->insertItem("TenderAdjustment");
    _screen->insertItem("TenderMaster");
    _screen->insertItem("TenderTransfer");
    _screen->insertItem("TermMaster");
    _screen->insertItem("TodoMaster");
    _screen->insertItem("UserMaster");
    _screen->insertItem("VendorInvoice");
    _screen->insertItem("VendorMaster");
    _screen->insertItem("VendorPayment");

    _screen->setFocus();
}

SecurityRuleDialog::~SecurityRuleDialog()
{
}

void
SecurityRuleDialog::setRule(const SecurityRule& rule)
{
    setCaption(tr("Edit Rule"));
    _screen->setCurrentItem(rule.screen);
    _allowView->setChecked(rule.allowView);
    _allowCreate->setChecked(rule.allowCreate);
    _allowUpdate->setChecked(rule.allowUpdate);
    _allowDelete->setChecked(rule.allowDelete);
}

SecurityRule
SecurityRuleDialog::getRule()
{
    SecurityRule rule;
    rule.screen = _screen->currentText();
    rule.allowView = _allowView->isChecked();
    rule.allowCreate = _allowCreate->isChecked();
    rule.allowUpdate = _allowUpdate->isChecked();
    rule.allowDelete = _allowDelete->isChecked();
    return rule;
}
