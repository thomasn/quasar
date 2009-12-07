// $Id: company_lookup.cpp,v 1.1 2005/03/13 22:09:17 bpepers Exp $
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

#include "company_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "company_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "company_master.h"

#include <qlabel.h>
#include <qlayout.h>

CompanyLookup::CompanyLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "CompanyLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(30);
    _list->addTextColumn(tr("Name"), 30);
    _list->addTextColumn(tr("Number"), 10);
    setCaption(tr("Company Lookup"));
}

CompanyLookup::CompanyLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "CompanyLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(30);
    _list->addTextColumn(tr("Name"), 30);
    _list->addTextColumn(tr("Number"), 10);
    setCaption(tr("Company Lookup"));
}

CompanyLookup::~CompanyLookup()
{
}

QString
CompanyLookup::lookupById(Id company_id)
{
    Company loc;
    if (company_id != INVALID_ID && _quasar->db()->lookup(company_id, loc))
	return loc.name();
    return "";
}

vector<DataPair>
CompanyLookup::lookupByText(const QString& text)
{
    CompanySelect conditions;
    if (!text.isEmpty())
	conditions.name = text + "%";
    conditions.activeOnly = activeOnly;
    vector<Company> companies;

    _quasar->db()->select(companies, conditions);
    if (companies.size() == 0 && !text.isEmpty()) {
	conditions.name = "";
	conditions.number = text;
	_quasar->db()->select(companies, conditions);
    }

    vector<DataPair> data;
    for (unsigned int i = 0; i < companies.size(); ++i) {
	data.push_back(DataPair(companies[i].id(), companies[i].name()));
    }

    return data;
}

void
CompanyLookup::refresh()
{
    CompanySelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.activeOnly = activeOnly;

    vector<Company> companies;
    _quasar->db()->select(companies, conditions);

    _list->clear();
    for (unsigned int i = 0; i < companies.size(); ++i) {
	Company& company = companies[i];

	ListViewItem* item = new ListViewItem(_list, company.id());
	item->setText(0, company.name());
	item->setText(1, company.number());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
CompanyLookup::slotNew()
{
    CompanyMaster* window = new CompanyMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
CompanyLookup::slotEdit(Id id)
{
    CompanyMaster* window = new CompanyMaster(_main, id);
    window->show();
    reject();
    return window;
}
