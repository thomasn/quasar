// $Id: customer_type_list.cpp,v 1.7 2004/01/31 01:50:30 arandell Exp $
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

#include "customer_type_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "customer_type_select.h"
#include "customer_type_master.h"
#include "account.h"
#include "grid.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

CustomerTypeList::CustomerTypeList(MainWindow* main)
    : ActiveList(main, "CustomerTypeList")
{
    _helpSource = "customer_type_list.html";

    _list->addTextColumn(tr("Name"), 20);
    _list->addTextColumn(tr("Account"), 30);
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Customer Type List"));
    finalize();
}

CustomerTypeList::~CustomerTypeList()
{
}

bool
CustomerTypeList::isActive(Id type_id)
{
    CustomerType type;
    _quasar->db()->lookup(type_id, type);
    return type.isActive();
}

void
CustomerTypeList::setActive(Id type_id, bool active)
{
    if (type_id == INVALID_ID) return;

    CustomerType type;
    _quasar->db()->lookup(type_id, type);

    CustomerType orig = type;
    _quasar->db()->setActive(type, active);
    _quasar->db()->update(orig, type);
}

void
CustomerTypeList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::CUST_TYPE)
	slotRefresh();
}

void
CustomerTypeList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id type_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    CustomerTypeSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<CustomerType> types;
    _quasar->db()->select(types, conditions);

    for (unsigned int i = 0; i < types.size(); i++) {
	CustomerType& type = types[i];

	Account account;
	_quasar->db()->lookup(type.accountId(), account);

	ListViewItem* lvi = new ListViewItem(_list, type.id());
	lvi->setText(0, type.name());
	lvi->setText(1, account.name());
	if (showInactive) lvi->setValue(2, !type.isActive());
	if (type.id() == type_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
CustomerTypeList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Customer Types"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
CustomerTypeList::performNew()
{
    CustomerTypeMaster* master = new CustomerTypeMaster(_main);
    master->show();
}

void
CustomerTypeList::performEdit()
{
    Id type_id = currentId();
    CustomerTypeMaster* master = new CustomerTypeMaster(_main, type_id);
    master->show();
}
