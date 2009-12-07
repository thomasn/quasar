// $Id: company_list.cpp,v 1.1 2005/03/13 22:09:17 bpepers Exp $
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

#include "company_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "company_select.h"
#include "company_master.h"
#include "grid.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>

CompanyList::CompanyList(MainWindow* main)
    : ActiveList(main, "CompanyList")
{
    _helpSource = "company_list.html";

    _list->addTextColumn(tr("Name"), 30);
    _list->addTextColumn(tr("Number"), 10);
    _list->setSorting(0);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Company List"));
    finalize();
}

CompanyList::~CompanyList()
{
}

bool
CompanyList::isActive(Id company_id)
{
    Company company;
    _quasar->db()->lookup(company_id, company);
    return company.isActive();
}

void
CompanyList::setActive(Id company_id, bool active)
{
    if (company_id == INVALID_ID) return;

    Company company;
    _quasar->db()->lookup(company_id, company);

    Company orig = company;
    _quasar->db()->setActive(company, active);
    _quasar->db()->update(orig, company);
}

void
CompanyList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::COMPANY)
	slotRefresh();
}

void
CompanyList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id company_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    CompanySelect conditions;
    conditions.activeOnly = !showInactive;
    vector<Company> companies;
    _quasar->db()->select(companies, conditions);

    for (unsigned int i = 0; i < companies.size(); ++i) {
	Company& company = companies[i];

	ListViewItem* lvi = new ListViewItem(_list, company.id());
	lvi->setValue(0, company.name());
	lvi->setValue(1, company.number());
	if (showInactive) lvi->setValue(2, !company.isActive());
	if (company.id() == company_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
CompanyList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Companies"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
CompanyList::performNew()
{
    CompanyMaster* master = new CompanyMaster(_main, INVALID_ID);
    master->show();
}

void
CompanyList::performEdit()
{
    Id company_id = currentId();
    CompanyMaster* master = new CompanyMaster(_main, company_id);
    master->show();
}
