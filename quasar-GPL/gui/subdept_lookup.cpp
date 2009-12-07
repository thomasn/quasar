// $Id: subdept_lookup.cpp,v 1.22 2004/11/10 10:36:52 bpepers Exp $
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

#include "subdept_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "subdept_select.h"
#include "list_view_item.h"
#include "lookup_edit.h"
#include "dept_lookup.h"
#include "subdept_master.h"

#include <qlabel.h>
#include <qlayout.h>

SubdeptLookup::SubdeptLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "SubdeptLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(30);
    _list->addTextColumn(tr("Name"), 30);
    _list->addTextColumn(tr("Number"), 10);
    _list->addTextColumn(tr("Department"), 30);
    setCaption(tr("Subdepartment Lookup"));

    QLabel* deptLabel = new QLabel(tr("Department:"), _search);
    dept = new LookupEdit(new DeptLookup(main, this), _search);
    dept->setLength(30);
    deptLabel->setBuddy(dept);

    setTabOrder(_text, dept);
    _searchGrid->addWidget(deptLabel, 1, 0);
    _searchGrid->addWidget(dept, 1, 1, AlignLeft | AlignVCenter);
}

SubdeptLookup::SubdeptLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "SubdeptLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(30);
    _list->addTextColumn(tr("Name"), 30);
    _list->addTextColumn(tr("Number"), 10);
    _list->addTextColumn(tr("Department"), 30);
    setCaption(tr("Subdepartment Lookup"));

    QLabel* deptLabel = new QLabel(tr("Department:"), _search);
    dept = new LookupEdit(new DeptLookup(quasar, this), _search);
    dept->setLength(30);
    deptLabel->setBuddy(dept);

    setTabOrder(_text, dept);
    _searchGrid->addWidget(deptLabel, 1, 0);
    _searchGrid->addWidget(dept, 1, 1, AlignLeft | AlignVCenter);
}

SubdeptLookup::~SubdeptLookup()
{
}

QString
SubdeptLookup::lookupById(Id subdept_id)
{
    Subdept subdept;
    if (subdept_id != INVALID_ID && _quasar->db()->lookup(subdept_id, subdept))
	return subdept.name();
    return "";
}

vector<DataPair>
SubdeptLookup::lookupByText(const QString& text)
{
    SubdeptSelect conditions;
    if (!text.isEmpty())
	conditions.name = text + "%";
    conditions.dept_id = dept->getId();
    conditions.activeOnly = activeOnly;

    vector<Subdept> subdepts;
    _quasar->db()->select(subdepts, conditions);

    if (subdepts.size() == 0 && !text.isEmpty()) {
	conditions.name = "";
	conditions.number = text;
	_quasar->db()->select(subdepts, conditions);
    }

    vector<DataPair> data;
    for (unsigned int i = 0; i < subdepts.size(); ++i) {
	data.push_back(DataPair(subdepts[i].id(), subdepts[i].name()));
    }

    return data;
}

void
SubdeptLookup::refresh()
{
    SubdeptSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.dept_id = dept->getId();
    conditions.activeOnly = activeOnly;

    vector<Subdept> subdepts;
    _quasar->db()->select(subdepts, conditions);

    _list->clear();
    for (unsigned int i = 0; i < subdepts.size(); ++i) {
	Subdept& subdept = subdepts[i];

	Dept dept;
	findDept(subdept.deptId(), dept);

	ListViewItem* item = new ListViewItem(_list, subdept.id());
	item->setValue(0, subdept.name());
	item->setValue(1, subdept.number());
	item->setValue(2, dept.name());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

QWidget*
SubdeptLookup::slotNew()
{
    SubdeptMaster* window = new SubdeptMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
SubdeptLookup::slotEdit(Id id)
{
    SubdeptMaster* window = new SubdeptMaster(_main, id);
    window->show();
    reject();
    return window;
}

bool
SubdeptLookup::findDept(Id dept_id, Dept& dept)
{
    for (unsigned int i = 0; i < _depts.size(); ++i) {
	if (_depts[i].id() == dept_id) {
	    dept = _depts[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(dept_id, dept))
	return false;

    _depts.push_back(dept);
    return true;
}
