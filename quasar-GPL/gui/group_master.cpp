// $Id: group_master.cpp,v 1.8 2004/01/31 01:50:30 arandell Exp $
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

#include "group_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "group_select.h"
#include "line_edit.h"
#include "combo_box.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qlayout.h>
#include <qmessagebox.h>

GroupMaster::GroupMaster(MainWindow* main, Id group_id)
    : DataWindow(main, "GroupMaster", group_id)
{
    _helpSource = "group_master.html";

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("&Name:"), _frame);
    _name = new LineEdit(_frame);
    _name->setLength(12);
    nameLabel->setBuddy(_name);

    QLabel* descLabel = new QLabel(tr("&Description:"), _frame);
    _desc = new LineEdit(_frame);
    _desc->setLength(40);
    descLabel->setBuddy(_desc);

    QLabel* typeLabel = new QLabel(tr("&Type:"), _frame);
    _type = new ComboBox(false, _frame);
    typeLabel->setBuddy(_type);

    _type->insertItem(Group::typeName(Group::ACCOUNT));
    _type->insertItem(Group::typeName(Group::CUSTOMER));
    _type->insertItem(Group::typeName(Group::EMPLOYEE));
    _type->insertItem(Group::typeName(Group::VENDOR));
    _type->insertItem(Group::typeName(Group::PERSONAL));
    _type->insertItem(Group::typeName(Group::ITEM));

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(descLabel, 1, 0);
    grid->addWidget(_desc, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(typeLabel, 2, 0);
    grid->addWidget(_type, 2, 1, AlignLeft | AlignVCenter);

    setCaption(tr("Group Master"));
    finalize();
}

GroupMaster::~GroupMaster()
{
}

void
GroupMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _name;
}

void
GroupMaster::newItem()
{
    Group blank;
    _orig = blank;

    _curr = _orig;
    _firstField = _name;
}

void
GroupMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _name->selectAll();
}

bool
GroupMaster::fileItem()
{
    if (_orig.id() == INVALID_ID) {
	if (!_quasar->db()->create(_curr)) return false;
    } else {
	if (!_quasar->db()->update(_orig, _curr)) return false;
    }

    _orig = _curr;
    _id = _curr.id();

    return true;
}

bool
GroupMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
GroupMaster::restoreItem()
{
    _curr = _orig;
}

void
GroupMaster::cloneItem()
{
    GroupMaster* clone = new GroupMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
GroupMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
GroupMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _desc->setText(_curr.description());
    _type->setCurrentItem(_curr.typeName());
    _inactive->setChecked(!_curr.isActive());
}

// Set the data object from the widgets.
void
GroupMaster::widgetToData()
{
    _curr.setName(_name->text());
    _curr.setDescription(_desc->text());
    _curr.setType(Group::type(_type->currentText()));
    _quasar->db()->setActive(_curr, !_inactive->isChecked());
}

void
GroupMaster::setType(int type)
{
    _type->setCurrentItem(Group::typeName(type));
}
