// $Id: todo_master.cpp,v 1.20 2004/01/31 01:50:31 arandell Exp $
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

#include "todo_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "date_popup.h"
#include "multi_line_edit.h"

#include <qapplication.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qwhatsthis.h>

TodoMaster::TodoMaster(MainWindow* main, Id todo_id)
    : DataWindow(main, "TodoMaster", todo_id)
{
    _helpSource = "todo_master.html";

    // Create widgets
    QLabel* noteLabel = new QLabel(tr("&Note:"), _frame);
    _note = new MultiLineEdit(_frame);
    _note->setFixedVisibleLines(8);
    _note->setMinimumWidth(_note->fontMetrics().width('x') * 30);
    noteLabel->setBuddy(_note);

    QLabel* dateLabel = new QLabel(tr("&Remind On:"), _frame);
    _date = new DatePopup(_frame);
    dateLabel->setBuddy(_date);

    QGridLayout *grid = new QGridLayout(_frame, 3, 4);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->addWidget(noteLabel, 0, 0);
    grid->addMultiCellWidget(_note, 0, 2, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(dateLabel, 0, 2);
    grid->addWidget(_date, 0, 3, AlignLeft | AlignVCenter);
    grid->activate();

    _inactive->setText(tr("Done?"));

    setCaption(tr("Todo Master"));
    finalize();
}

TodoMaster::~TodoMaster()
{
}

void
TodoMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _note;
}

void
TodoMaster::newItem()
{
    Todo blank;
    _orig = blank;

    _curr = _orig;
    _firstField = _note;
}

void
TodoMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _note->selectAll();
}

bool
TodoMaster::fileItem()
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
TodoMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
TodoMaster::restoreItem()
{
    _curr = _orig;
}

void
TodoMaster::cloneItem()
{
    TodoMaster* clone = new TodoMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
TodoMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
TodoMaster::dataToWidget()
{
    _note->setText(_curr.note());
    _date->setDate(_curr.remindOn());
    _inactive->setChecked(!_curr.isActive());
}

// Set the data object from the widgets.
void
TodoMaster::widgetToData()
{
    _curr.setNote(_note->text());
    _curr.setRemindOn(_date->getDate());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());
}
