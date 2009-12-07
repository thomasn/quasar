// $Id: item_lookup.cpp,v 1.26 2004/11/10 10:36:52 bpepers Exp $
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

#include "item_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "group.h"
#include "item_select.h"
#include "list_view_item.h"
#include "lookup_edit.h"
#include "dept_lookup.h"
#include "subdept_lookup.h"
#include "group_lookup.h"
#include "item_master.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qlayout.h>

ItemLookup::ItemLookup(MainWindow* main, QWidget* parent)
    : QuasarLookup(main, parent, "ItemLookup")
{
    _label->setText(tr("Item Number:"));
    _list->addTextColumn(tr("Item Number"), 18, AlignRight);
    _list->addTextColumn(tr("Description"), 30);
    _list->addTextColumn(tr("Size"), 10);
    setCaption(tr("Item Lookup"));

    QLabel* descLabel = new QLabel(tr("&Description:"), _search);
    desc = new LineEdit(_search);
    desc->setLength(40);
    descLabel->setBuddy(desc);

    QLabel* deptLabel = new QLabel(tr("De&partment:"), _search);
    dept = new LookupEdit(new DeptLookup(main, this), _search);
    dept->setLength(30);
    deptLabel->setBuddy(dept);

    QLabel* subdeptLabel = new QLabel(tr("S&ubdepartment:"), _search);
    subdept = new LookupEdit(new SubdeptLookup(main, this), _search);
    subdept->setLength(30);
    subdeptLabel->setBuddy(subdept);

    QLabel* groupLabel = new QLabel(tr("&Group:"), _search);
    group = new LookupEdit(new GroupLookup(main, this, Group::ITEM), _search);
    group->setLength(15);
    groupLabel->setBuddy(group);

    setTabOrder(_text, desc);
    setTabOrder(desc, dept);
    setTabOrder(dept, subdept);
    setTabOrder(subdept, group);
    _searchGrid->addWidget(descLabel, 1, 0);
    _searchGrid->addWidget(desc, 1, 1, AlignLeft | AlignVCenter);
    _searchGrid->addWidget(deptLabel, 2, 0);
    _searchGrid->addWidget(dept, 2, 1, AlignLeft | AlignVCenter);
    _searchGrid->addWidget(subdeptLabel, 3, 0);
    _searchGrid->addWidget(subdept, 3, 1, AlignLeft | AlignVCenter);
    _searchGrid->addWidget(groupLabel, 4, 0);
    _searchGrid->addWidget(group, 4, 1, AlignLeft | AlignVCenter);

    store_id = _quasar->defaultStore();
    purchasedOnly = false;
    soldOnly = false;
    inventoriedOnly = false;
    stockedOnly = true;
    activeOnly = true;
    checkOrderNum = true;
}

ItemLookup::ItemLookup(QuasarClient* quasar, QWidget* parent)
    : QuasarLookup(quasar, parent, "ItemLookup")
{
    _label->setText(tr("Item Number:"));
    _list->addTextColumn(tr("Item Number"), 18, AlignRight);
    _list->addTextColumn(tr("Description"), 30);
    _list->addTextColumn(tr("Size"), 10);
    setCaption(tr("Item Lookup"));

    QLabel* descLabel = new QLabel(tr("&Description:"), _search);
    desc = new LineEdit(_search);
    desc->setLength(40);
    descLabel->setBuddy(desc);

    QLabel* deptLabel = new QLabel(tr("De&partment:"), _search);
    dept = new LookupEdit(new DeptLookup(quasar, this), _search);
    dept->setLength(30);
    deptLabel->setBuddy(dept);

    QLabel* subdeptLabel = new QLabel(tr("S&ubdepartment:"), _search);
    subdept = new LookupEdit(new SubdeptLookup(quasar, this), _search);
    subdept->setLength(30);
    subdeptLabel->setBuddy(subdept);

    QLabel* groupLabel = new QLabel(tr("&Group:"), _search);
    group = new LookupEdit(new GroupLookup(quasar, this, Group::ITEM),_search);
    group->setLength(15);
    groupLabel->setBuddy(group);

    setTabOrder(_text, desc);
    setTabOrder(desc, dept);
    setTabOrder(dept, subdept);
    setTabOrder(subdept, group);
    _searchGrid->addWidget(descLabel, 1, 0);
    _searchGrid->addWidget(desc, 1, 1, AlignLeft | AlignVCenter);
    _searchGrid->addWidget(deptLabel, 2, 0);
    _searchGrid->addWidget(dept, 2, 1, AlignLeft | AlignVCenter);
    _searchGrid->addWidget(subdeptLabel, 3, 0);
    _searchGrid->addWidget(subdept, 3, 1, AlignLeft | AlignVCenter);
    _searchGrid->addWidget(groupLabel, 4, 0);
    _searchGrid->addWidget(group, 4, 1, AlignLeft | AlignVCenter);

    store_id = _quasar->defaultStore();
    purchasedOnly = false;
    soldOnly = false;
    inventoriedOnly = false;
    stockedOnly = true;
    activeOnly = true;
    checkOrderNum = true;
}

ItemLookup::~ItemLookup()
{
}

QString
ItemLookup::lookupById(Id item_id)
{
    Item item;
    if (item_id != INVALID_ID && _quasar->db()->lookup(item_id, item))
	return item.number();
    return "";
}

vector<DataPair>
ItemLookup::lookupByText(const QString& text)
{
    ItemSelect conditions;
    conditions.store_id = store_id;
    conditions.purchasedOnly = purchasedOnly;
    conditions.soldOnly = soldOnly;
    conditions.inventoriedOnly = inventoriedOnly;
    conditions.stockedOnly = stockedOnly;
    conditions.activeOnly = activeOnly;
    conditions.checkOrderNum = checkOrderNum;

    vector<Id> item_ids;
    _db->lookup(conditions, text, item_ids);
    if (item_ids.size() != 0 || text.isEmpty()) {
	vector<DataPair> data;
	for (unsigned int i = 0; i < item_ids.size(); ++i)
	    data.push_back(DataPair(item_ids[i], conditions.number));
	return data;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    conditions.number = text + "%";
    vector<Item> items;
    _quasar->db()->select(items, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < items.size(); ++i) {
	const Item& item = items[i];

	for (unsigned int j = 0; j < item.numbers().size(); ++j) {
	    QString number = item.numbers()[j].number;
	    if (number.left(text.length()) == text)
		data.push_back(DataPair(item.id(), number));
	}

	if (checkOrderNum) {
	    for (unsigned int j = 0; j < item.vendors().size(); ++j) {
		QString number = item.vendors()[j].number;
		if (number.isEmpty()) continue;
		if (number.left(text.length()) == text)
		    data.push_back(DataPair(item.id(), number));
	    }
	}
    }

    QApplication::restoreOverrideCursor();
    return data;
}

int
ItemLookup::matchCount(const QString& text)
{
    ItemSelect conditions;
    conditions.store_id = store_id;
    conditions.purchasedOnly = purchasedOnly;
    conditions.soldOnly = soldOnly;
    conditions.inventoriedOnly = inventoriedOnly;
    conditions.stockedOnly = stockedOnly;
    conditions.activeOnly = activeOnly;
    conditions.checkOrderNum = checkOrderNum;

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    vector<Id> item_ids;
    _db->lookup(conditions, text, item_ids);
    if (item_ids.size() != 0 || text.isEmpty()) {
	QApplication::restoreOverrideCursor();
	return item_ids.size();
    }

    conditions.number = text + "%";
    int count;
    _quasar->db()->count(count, conditions);

    QApplication::restoreOverrideCursor();
    return count;
}

void
ItemLookup::refresh()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    _list->clear();
    ItemSelect conditions;
    if (!text().isEmpty())
	conditions.number = text() + "%";
    if (!desc->text().isEmpty())
	conditions.description = "%" + desc->text() + "%";
    conditions.group_id = group->getId();
    conditions.dept_id = dept->getId();
    conditions.subdept_id = subdept->getId();
    conditions.store_id = store_id;
    conditions.purchasedOnly = purchasedOnly;
    conditions.soldOnly = soldOnly;
    conditions.inventoriedOnly = inventoriedOnly;
    conditions.stockedOnly = stockedOnly;
    conditions.activeOnly = activeOnly;
    conditions.checkOrderNum = checkOrderNum;

    int count;
    _quasar->db()->count(count, conditions);
    if (count > 100) {
	QApplication::restoreOverrideCursor();

	QWidget* parent = this;
	if (isHidden() && parentWidget() != NULL)
	    parent = parentWidget();

	QString message = tr("This will select %1 items\n"
			     "which may be slow.  Continue?").arg(count);
	int ch = QMessageBox::warning(parent, tr("Are You Sure?"), message,
				      QMessageBox::Yes, QMessageBox::No);
	if (ch != QMessageBox::Yes) return;

	QApplication::setOverrideCursor(waitCursor);
	qApp->processEvents();
    }

    vector<Item> items;
    _quasar->db()->select(items, conditions);

    for (unsigned int i = 0; i < items.size(); ++i) {
	Item& item = items[i];

	ListViewItem* lvi = new ListViewItem(_list, item.id());
	lvi->setValue(0, item.number());
	lvi->setValue(1, item.description());
	lvi->setValue(2, item.sellSize());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);

    QApplication::restoreOverrideCursor();
}

QWidget*
ItemLookup::slotNew()
{
    ItemMaster* window = new ItemMaster(_main);
    window->show();
    reject();
    return window;
}

QWidget*
ItemLookup::slotEdit(Id id)
{
    ItemMaster* window = new ItemMaster(_main, id);
    window->show();
    reject();
    return window;
}
