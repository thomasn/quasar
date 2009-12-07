// $Id: invoice_info.cpp,v 1.10 2005/03/13 23:13:45 bpepers Exp $
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

#include "invoice_info.h"
#include "main_window.h"
#include "screen_decl.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "receive.h"
#include "money_edit.h"
#include "date_edit.h"

#include <qlabel.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlayout.h>

InvoiceInfo::InvoiceInfo(MainWindow* main, QWidget* parent)
    : QDialog(parent, "InvoiceInfo", false, WDestructiveClose), _main(main)
{
    QLabel* numberLabel = new QLabel(tr("Invoice:"), this);
    _number = new LineEdit(this);
    _number->setFocusPolicy(NoFocus);

    QLabel* totalLabel = new QLabel(tr("Total:"), this);
    _total = new MoneyEdit(this);
    _total->setFocusPolicy(NoFocus);

    QLabel* paidLabel = new QLabel(tr("Paid:"), this);
    _paid = new MoneyEdit(this);
    _paid->setFocusPolicy(NoFocus);

    QLabel* dueLabel = new QLabel(tr("Due:"), this);
    _due = new MoneyEdit(this);
    _due->setFocusPolicy(NoFocus);

    QLabel* termsLabel = new QLabel(tr("Terms:"), this);
    _terms = new LineEdit(this);
    _terms->setFocusPolicy(NoFocus);

    QLabel* dateLabel = new QLabel(tr("Invoice Date:"), this);
    _date = new DateEdit(this);
    _date->setFocusPolicy(NoFocus);

    QLabel* dueDateLabel = new QLabel(tr("Due Date:"), this);
    _dueDate = new DateEdit(this);
    _dueDate->setFocusPolicy(NoFocus);

    QLabel* discDateLabel = new QLabel(tr("Discount Date:"), this);
    _discDate = new DateEdit(this);
    _discDate->setFocusPolicy(NoFocus);

    QGroupBox* disc = new QGroupBox(tr("Discount"), this);
    QGridLayout* grid1 = new QGridLayout(disc, 4, 2,
					 disc->frameWidth() * 2);
    grid1->addRowSpacing(0, disc->fontMetrics().height());
    grid1->setColStretch(2, 1);

    QLabel* discountLabel = new QLabel(tr("Discount:"), disc);
    _discount = new MoneyEdit(disc);
    _discount->setFocusPolicy(NoFocus);

    QLabel* takenLabel = new QLabel(tr("Taken:"), disc);
    _taken = new MoneyEdit(disc);
    connect(_taken, SIGNAL(validData()), SLOT(slotDiscountChanged()));

    QPushButton* takeDisc = new QPushButton(tr("Take Discount"), disc);
    QPushButton* clearDisc = new QPushButton(tr("Clear Discount"), disc);

    connect(takeDisc, SIGNAL(clicked()), SLOT(slotTakeDiscount()));
    connect(clearDisc, SIGNAL(clicked()), SLOT(slotClearDiscount()));

    grid1->setColStretch(2, 1);
    grid1->addWidget(discountLabel, 1, 0);
    grid1->addWidget(_discount, 1, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(takeDisc, 1, 3);
    grid1->addWidget(takenLabel, 2, 0);
    grid1->addWidget(_taken, 2, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(clearDisc, 2, 3);

    QFrame* buttons = new QFrame(this);

    _view = new QPushButton(tr("View Invoice"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);
    QPushButton* ok = new QPushButton(tr("OK"), buttons);

    ok->setMinimumSize(cancel->sizeHint());
    ok->setDefault(true);
    connect(_view, SIGNAL(clicked()), SLOT(slotViewInvoice()));
    connect(cancel, SIGNAL(clicked()), SLOT(reject()));
    connect(ok, SIGNAL(clicked()), SLOT(accept()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(3);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(1, 1);
    buttonGrid->addWidget(_view, 0, 0);
    buttonGrid->addWidget(cancel, 0, 2);
    buttonGrid->addWidget(ok, 0, 3);

    QGridLayout* grid = new QGridLayout(this);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(2, 1);
    grid->addColSpacing(2, 40);
    grid->addWidget(numberLabel, 0, 0);
    grid->addWidget(_number, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(termsLabel, 0, 3);
    grid->addWidget(_terms, 0, 4, AlignLeft | AlignVCenter);
    grid->addWidget(totalLabel, 1, 0);
    grid->addWidget(_total, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(dateLabel, 1, 3);
    grid->addWidget(_date, 1, 4, AlignLeft | AlignVCenter);
    grid->addWidget(paidLabel, 2, 0);
    grid->addWidget(_paid, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(dueDateLabel, 2, 3);
    grid->addWidget(_dueDate, 2, 4, AlignLeft | AlignVCenter);
    grid->addWidget(dueLabel, 3, 0);
    grid->addWidget(_due, 3, 1, AlignLeft | AlignVCenter);
    grid->addWidget(discDateLabel, 3, 3);
    grid->addWidget(_discDate, 3, 4, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(disc, 4, 4, 0, 4);
    grid->addMultiCellWidget(buttons, 5, 5, 0, 4);

    _view->setEnabled(false);
    _taken->setFocus();
    setCaption(tr("Invoice Information"));
}

InvoiceInfo::~InvoiceInfo()
{
}

void
InvoiceInfo::setInvoice(const Gltx& gltx, const Term& term, fixed taken)
{
    _gltx = gltx;
    _term = term;
    _view->setEnabled(true);

    QDate invoiceDate = gltx.postDate();
    if (gltx.dataType() == DataObject::RECEIVE) {
	Receive receive;
	_main->quasar()->db()->lookup(gltx.id(), receive);
	invoiceDate = receive.invoiceDate();
    }

    _number->setText(gltx.number());
    _total->setFixed(gltx.cardTotal());
    _paid->setFixed(gltx.paymentTotal());
    _due->setFixed(gltx.cardTotal() - gltx.paymentTotal());
    _terms->setText(term.name());
    _date->setDate(invoiceDate);
    _dueDate->setDate(invoiceDate + term.dueDays());

    fixed discount = gltx.cardTotal() * term.discount() / 100.0;
    discount.moneyRound();
    _discount->setFixed(discount);

    if (term.discountDays() != 0)
	_discDate->setDate(invoiceDate + term.discountDays());
    else
	_discDate->setText("");

    _taken->setFixed(taken);
}

void
InvoiceInfo::slotViewInvoice()
{
    if (_gltx.id() == INVALID_ID) return;
    QWidget* view = editGltx(_gltx, _main);
    if (view != NULL) view->show();
}

void
InvoiceInfo::slotTakeDiscount()
{
    _taken->setFixed(_discount->getFixed());
    emit newDiscount(_taken->getFixed());
}

void
InvoiceInfo::slotClearDiscount()
{
    _taken->setText("");
    emit newDiscount(0.0);
}

void
InvoiceInfo::slotDiscountChanged()
{
    emit newDiscount(_taken->getFixed());
}
