// $Id: dept_master.cpp,v 1.27 2004/11/10 10:36:52 bpepers Exp $
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

#include "dept_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "dept_select.h"
#include "double_edit.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qlayout.h>
#include <qmessagebox.h>

DeptMaster::DeptMaster(MainWindow* main, Id dept_id)
    : DataWindow(main, "DeptMaster", dept_id)
{
    _helpSource = "dept_master.html";

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("&Name:"), _frame);
    _name = new LineEdit(30, _frame);
    nameLabel->setBuddy(_name);

    QLabel* numberLabel = new QLabel(tr("Number:"), _frame);
    _number = new LineEdit(5, _frame);
    numberLabel->setBuddy(_number);

    QLabel* pointsLabel = new QLabel(tr("Points:"), _frame);
    _points = new DoubleEdit(_frame);
    _points->setLength(5);
    pointsLabel->setBuddy(_points);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(3, 1);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(numberLabel, 1, 0);
    grid->addWidget(_number, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(pointsLabel, 2, 0);
    grid->addWidget(_points, 2, 1, AlignLeft | AlignVCenter);

    setCaption(tr("Department Master"));
    finalize();
}

DeptMaster::~DeptMaster()
{
}

void
DeptMaster::setName(const QString& name)
{
    _name->setText(name);
}

void
DeptMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _name;
}

void
DeptMaster::newItem()
{
    Dept blank;
    _orig = blank;

    _curr = _orig;
    _firstField = _name;
}

void
DeptMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _name->selectAll();
}

bool
DeptMaster::fileItem()
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
DeptMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
DeptMaster::restoreItem()
{
    _curr = _orig;
}

void
DeptMaster::cloneItem()
{
    DeptMaster* clone = new DeptMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
DeptMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
DeptMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _number->setText(_curr.number());
    _points->setFixed(_curr.patPoints());
    _inactive->setChecked(!_curr.isActive());
}

// Set the data object from the widgets.
void
DeptMaster::widgetToData()
{
    _curr.setName(_name->text());
    _curr.setNumber(_number->text());
    _curr.setPatPoints(_points->getFixed());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());
}
