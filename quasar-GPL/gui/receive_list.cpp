// $Id: receive_list.cpp,v 1.27 2004/08/04 09:15:43 bpepers Exp $
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

#include "receive_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "receive_select.h"
#include "receive_master.h"
#include "vendor.h"
#include "lookup_edit.h"
#include "vendor_lookup.h"
#include "store_lookup.h"
#include "money_valcon.h"
#include "date_valcon.h"
#include "grid.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>

ReceiveList::ReceiveList(MainWindow* main)
    : ActiveList(main, "ReceiveList")
{
    _helpSource = "receive_list.html";

    _list->addTextColumn(tr("Invoice"), 10, AlignRight);
    _list->addDateColumn(tr("Date"));
    _list->addTextColumn(tr("Vendor"), 20);
    _list->addMoneyColumn(tr("Amount"));
    _list->setSorting(0);

    QLabel* vendorLabel = new QLabel(tr("&Vendor:"), _search);
    _vendor = new LookupEdit(new VendorLookup(_main, this), _search);
    _vendor->setLength(30);
    vendorLabel->setBuddy(_vendor);

    QLabel* storeLabel = new QLabel(tr("&Store:"), _search);
    _store = new LookupEdit(new StoreLookup(_main, this), _search);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    QGridLayout* grid = new QGridLayout(_search);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(2, 1);
    grid->addWidget(vendorLabel, 0, 0);
    grid->addWidget(_vendor, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(storeLabel, 1, 0);
    grid->addWidget(_store, 1, 1, AlignLeft | AlignVCenter);

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    _show_paid = new QCheckBox(tr("Show Paid?"), _extra);
    connect(_show_paid, SIGNAL(toggled(bool)), SLOT(slotPaidChanged()));

    _inactive->setText(tr("Show Voided?"));
    _grid->addWidget(_show_paid, 0, 1, AlignLeft | AlignVCenter);

    setCaption(tr("Vendor Invoices"));
    finalize(false);
    _vendor->setFocus();
}

ReceiveList::~ReceiveList()
{
}

void
ReceiveList::slotPaidChanged()
{
    clearLists();

    int columns = _list->columns();
    if (_show_paid->isChecked()) {
	_list->addCheckColumn(tr("Paid?"));
	resize(width() + _list->columnWidth(columns), height());
    } else {
	int column = _list->findColumn(tr("Paid?"));
	if (column != -1) {
	    resize(width() - _list->columnWidth(column), height());
	    _list->removeColumn(column);
	}
    }

    slotRefresh();
}

bool
ReceiveList::isActive(Id receive_id)
{
    Gltx receive;
    _quasar->db()->lookup(receive_id, receive);
    return receive.isActive();
}

void
ReceiveList::setActive(Id receive_id, bool active)
{
    if (receive_id == INVALID_ID) return;

    Receive receive;
    _quasar->db()->lookup(receive_id, receive);

    Receive orig = receive;
    _quasar->db()->setActive(receive, active);
    _quasar->db()->update(orig, receive);
}

void
ReceiveList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    bool showPaid = _show_paid->isChecked();
    int inactiveColumn = _list->findColumn(tr("Inactive?"));
    int paidColumn = _list->findColumn(tr("Paid?"));
    Id receive_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    GltxSelect conditions;
    conditions.type = DataObject::RECEIVE;
    conditions.activeOnly = !showInactive;
    conditions.unpaid = !showPaid;
    conditions.card_id = _vendor->getId();
    conditions.store_id = _store->getId();
    vector<Gltx> receives;
    _quasar->db()->select(receives, conditions);

    conditions.type = DataObject::CLAIM;
    vector<Gltx> claims;
    _quasar->db()->select(claims, conditions);
    unsigned int i;
    for (i = 0; i < claims.size(); ++i)
	receives.push_back(claims[i]);

    for (i = 0; i < receives.size(); i++) {
	Gltx& receive = receives[i];

	Vendor vendor;
	_quasar->db()->lookup(receive.cardId(), vendor);

	ListViewItem* lvi = new ListViewItem(_list, receive.id());
	lvi->setValue(0, receive.number());
	lvi->setValue(1, receive.postDate());
	lvi->setValue(2, vendor.name());
	lvi->setValue(3, receive.amount());
	if (showInactive) lvi->setValue(inactiveColumn, !receive.isActive());
	if (showPaid) lvi->setValue(paidColumn, receive.paid());
	if (receive.id() == receive_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
ReceiveList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Vendor Invoices"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
ReceiveList::performNew()
{
    ReceiveMaster* master = new ReceiveMaster(_main, INVALID_ID);
    master->show();
}

void
ReceiveList::performEdit()
{
    Id receive_id = currentId();
    ReceiveMaster* master = new ReceiveMaster(_main, receive_id);
    master->show();
}
