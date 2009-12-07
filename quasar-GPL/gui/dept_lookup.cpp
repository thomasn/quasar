// $Id: dept_lookup.cpp,v 1.19 2004/11/10 10:36:52 bpepers Exp $
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

#include "dept_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "dept_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "dept_master.h"

#include <qlabel.h>
#include <qlayout.h>

DeptLookup::DeptLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "DeptLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(30);
    _list->addTextColumn(tr("Name"), 30);
    _list->addTextColumn(tr("Number"), 10);
    setCaption(tr("Department Lookup"));
}

DeptLookup::DeptLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "DeptLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(30);
    _list->addTextColumn(tr("Name"), 30);
    _list->addTextColumn(tr("Number"), 10);
    setCaption(tr("Department Lookup"));
}

DeptLookup::~DeptLookup()
{
}

QString
DeptLookup::lookupById(Id dept_id)
{
    Dept dept;
    if (dept_id != INVALID_ID && _quasar->db()->lookup(dept_id, dept))
	return dept.name();
    return "";
}

vector<DataPair>
DeptLookup::lookupByText(const QString& text)
{
    DeptSelect conditions;
    if (!text.isEmpty())
	conditions.name = text + "%";
    conditions.activeOnly = activeOnly;

    vector<Dept> depts;
    _quasar->db()->select(depts, conditions);
    if (depts.size() == 0 && !text.isEmpty()) {
	conditions.name = "";
	conditions.number = text;
	_quasar->db()->select(depts, conditions);
    }

    vector<DataPair> data;
    for (unsigned int i = 0; i < depts.size(); ++i) {
	data.push_back(DataPair(depts[i].id(), depts[i].name()));
    }

    return data;
}

void
DeptLookup::refresh()
{
    DeptSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.activeOnly = activeOnly;

    vector<Dept> depts;
    _quasar->db()->select(depts, conditions);

    _list->clear();
    for (unsigned int i = 0; i < depts.size(); ++i) {
	Dept& dept = depts[i];

	ListViewItem* item = new ListViewItem(_list, dept.id());
	item->setValue(0, dept.name());
	item->setValue(1, dept.number());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
DeptLookup::slotNew()
{
    DeptMaster* window = new DeptMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
DeptLookup::slotEdit(Id id)
{
    DeptMaster* window = new DeptMaster(_main, id);
    window->show();
    reject();
    return window;
}
