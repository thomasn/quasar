// $Id: order_template_list.cpp,v 1.8 2004/01/31 01:50:30 arandell Exp $
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

#include "order_template_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "order_template_select.h"
#include "order_template_master.h"
#include "vendor.h"
#include "grid.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

OrderTemplateList::OrderTemplateList(MainWindow* main)
    : ActiveList(main, "OrderTemplateList")
{
    _helpSource = "order_template_list.html";

    _list->addTextColumn(tr("Name"), 20);
    _list->addTextColumn(tr("Vendor"), 30);
    _list->addDateColumn(tr("Created"));
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Order Template List"));
    finalize();
}

OrderTemplateList::~OrderTemplateList()
{
}

bool
OrderTemplateList::isActive(Id template_id)
{
    OrderTemplate temp;
    _quasar->db()->lookup(template_id, temp);
    return temp.isActive();
}

void
OrderTemplateList::setActive(Id template_id, bool active)
{
    if (template_id == INVALID_ID) return;

    OrderTemplate temp;
    _quasar->db()->lookup(template_id, temp);

    OrderTemplate orig = temp;
    _quasar->db()->setActive(temp, active);
    _quasar->db()->update(orig, temp);
}

void
OrderTemplateList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::PO_TEMPLATE)
	slotRefresh();
}

void
OrderTemplateList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id template_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    OrderTemplateSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<OrderTemplate> templates;
    _quasar->db()->select(templates, conditions);

    for (unsigned int i = 0; i < templates.size(); i++) {
	OrderTemplate& temp = templates[i];

	Vendor vendor;
	_quasar->db()->lookup(temp.vendorId(), vendor);

	ListViewItem* lvi = new ListViewItem(_list, temp.id());
	lvi->setValue(0, temp.name());
	lvi->setValue(1, vendor.name());
	lvi->setValue(2, temp.createdOn());
	if (showInactive) lvi->setValue(3, !temp.isActive());
	if (temp.id() == template_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
OrderTemplateList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Order Templates"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
OrderTemplateList::performNew()
{
    OrderTemplateMaster* master = new OrderTemplateMaster(_main);
    master->show();
}

void
OrderTemplateList::performEdit()
{
    Id temp_id = currentId();
    OrderTemplateMaster* master = new OrderTemplateMaster(_main, temp_id);
    master->show();
}
