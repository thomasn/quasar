// $Id: price_batch_calc.cpp,v 1.1 2004/03/18 22:41:16 bpepers Exp $
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

#include "price_batch_calc.h"
#include "percent_edit.h"

#include <qlabel.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qlayout.h>

PriceBatchCalc::PriceBatchCalc(QWidget* parent)
    : QDialog(parent, "PriceBatchCalc", true)
{
    setCaption(tr("Calculate Prices"));

    _useMarkup = new QRadioButton(tr("Use Markup?"), this);
    _useMargin = new QRadioButton(tr("Use Margin?"), this);
    _useChange = new QRadioButton(tr("% Change?"), this);
    _useTarget = new QRadioButton(tr("Use Target?"), this);

    QButtonGroup* buttons = new QButtonGroup(this);
    buttons->hide();
    buttons->insert(_useMarkup);
    buttons->insert(_useMargin);
    buttons->insert(_useChange);
    buttons->insert(_useTarget);

    _markup = new PercentEdit(this);
    _margin = new PercentEdit(this);
    _change = new PercentEdit(this);

    connect(buttons, SIGNAL(clicked(int)), SLOT(slotModeChanged(int)));
    connect(_markup, SIGNAL(validData()), SLOT(slotMarkupChanged()));
    connect(_margin, SIGNAL(validData()), SLOT(slotMarginChanged()));
    connect(_change, SIGNAL(validData()), SLOT(slotChangeChanged()));

    QFrame* box = new QFrame(this);
    _ok = new QPushButton(tr("OK"), box);
    QPushButton* cancel = new QPushButton(tr("Cancel"), box);

    _ok->setMinimumSize(cancel->sizeHint());
    cancel->setMinimumSize(cancel->sizeHint());
    connect(_ok, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    _ok->setDefault(true);

    QGridLayout* grid1 = new QGridLayout(box);
    grid1->setSpacing(3);
    grid1->setMargin(3);
    grid1->setColStretch(0, 1);
    grid1->addWidget(cancel, 0, 0, AlignRight | AlignVCenter);
    grid1->addWidget(_ok, 0, 1, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(this);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->addWidget(_useMarkup, 0, 0);
    grid->addWidget(_markup, 0, 1);
    grid->addWidget(_useMargin, 1, 0);
    grid->addWidget(_margin, 1, 1);
    grid->addWidget(_useChange, 2, 0);
    grid->addWidget(_change, 2, 1);
    grid->addWidget(_useTarget, 3, 0);
    grid->addMultiCellWidget(box, 4, 4, 0, 1);

    _useMarkup->setChecked(true);
    _markup->setFocus();
    setMinimumSize(sizeHint());
}

PriceBatchCalc::~PriceBatchCalc()
{
}

int
PriceBatchCalc::getType()
{
    if (_useMarkup->isChecked()) return Markup;
    if (_useMargin->isChecked()) return Margin;
    if (_useChange->isChecked()) return Change;
    return Target;
}

fixed
PriceBatchCalc::getPercent()
{
    if (_useMarkup->isChecked()) return _markup->getFixed();
    if (_useMargin->isChecked()) return _margin->getFixed();
    if (_useChange->isChecked()) return _change->getFixed();
    return 0.0;
}

void
PriceBatchCalc::slotMarkupChanged()
{
    if (_markup->text().isEmpty()) return;

    _margin->setText("");
    _change->setText("");
    if (!_useMarkup->isChecked())
	_useMarkup->setChecked(true);
}

void
PriceBatchCalc::slotMarginChanged()
{
    if (_margin->text().isEmpty()) return;

    _markup->setText("");
    _change->setText("");
    if (!_useMargin->isChecked())
	_useMargin->setChecked(true);
}

void
PriceBatchCalc::slotChangeChanged()
{
    if (_change->text().isEmpty()) return;

    _markup->setText("");
    _margin->setText("");
    if (!_useChange->isChecked())
	_useChange->setChecked(true);
}

void
PriceBatchCalc::slotModeChanged(int index)
{
    switch (index) {
    case 0:
	_margin->setText("");
	_change->setText("");
	_markup->setFocus();
	break;
    case 1:
	_markup->setText("");
	_change->setText("");
	_margin->setFocus();
	break;
    case 2:
	_markup->setText("");
	_margin->setText("");
	_change->setFocus();
	break;
    case 3:
	_markup->setText("");
	_margin->setText("");
	_change->setText("");
	_ok->setFocus();
	break;
    }
}

void
PriceBatchCalc::accept()
{
    if (!_markup->valid()) return;
    if (!_margin->valid()) return;
    if (!_change->valid()) return;

    fixed margin = _margin->getFixed();
    if (margin < 0.0 || margin > 99.0) return;

    QDialog::accept();
}
