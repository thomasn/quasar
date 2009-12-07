// $Id: discount_list.cpp,v 1.12 2004/01/31 01:50:30 arandell Exp $
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

#include "discount_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "discount_select.h"
#include "discount_master.h"
#include "percent_valcon.h"
#include "money_valcon.h"
#include "grid.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

DiscountList::DiscountList(MainWindow* main)
    : ActiveList(main, "DiscountList")
{
    _helpSource = "discount_list.html";

    _list->addTextColumn(tr("Name"), 20);
    _list->addCheckColumn(tr("Line?"));
    _list->addCheckColumn(tr("Tx?"));
    _list->addTextColumn(tr("Method"), 10);
    _list->addTextColumn(tr("Amount"), 10);
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Discount List"));
    finalize();
}

DiscountList::~DiscountList()
{
}

bool
DiscountList::isActive(Id discount_id)
{
    Discount discount;
    _quasar->db()->lookup(discount_id, discount);
    return discount.isActive();
}

void
DiscountList::setActive(Id discount_id, bool active)
{
    if (discount_id == INVALID_ID) return;

    Discount discount;
    _quasar->db()->lookup(discount_id, discount);

    Discount orig = discount;
    _quasar->db()->setActive(discount, active);
    _quasar->db()->update(orig, discount);
}

void
DiscountList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::DISCOUNT)
	slotRefresh();
}

void
DiscountList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id discount_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    DiscountSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<Discount> discounts;
    _quasar->db()->select(discounts, conditions);

    for (unsigned int i = 0; i < discounts.size(); ++i) {
	Discount& discount = discounts[i];

	QString amount = "";
	if (discount.method() == Discount::PERCENT)
	    amount = PercentValcon().format(discount.amount());
	else
	    amount = MoneyValcon().format(discount.amount());

	ListViewItem* lvi = new ListViewItem(_list, discount.id());
	lvi->setValue(0, discount.name());
	lvi->setValue(1, discount.lineDiscount());
	lvi->setValue(2, discount.txDiscount());
	lvi->setValue(3, discount.methodName());
	lvi->setValue(4, amount);
	if (showInactive) lvi->setValue(5, !discount.isActive());
	if (discount.id() == discount_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
DiscountList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Discounts"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
DiscountList::performNew()
{
    DiscountMaster* master = new DiscountMaster(_main, INVALID_ID);
    master->show();
}

void
DiscountList::performEdit()
{
    Id discount_id = currentId();
    DiscountMaster* master = new DiscountMaster(_main, discount_id);
    master->show();
}
