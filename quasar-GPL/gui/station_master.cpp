// $Id: station_master.cpp,v 1.8 2004/01/31 01:50:31 arandell Exp $
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

#include "station_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "station_select.h"
#include "line_edit.h"
#include "integer_edit.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qlayout.h>
#include <qmessagebox.h>

StationMaster::StationMaster(MainWindow* main, Id station_id)
    : DataWindow(main, "StationMaster", station_id)
{
    _helpSource = "station_master.html";

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("&Name:"), _frame);
    _name = new LineEdit(30, _frame);
    nameLabel->setBuddy(_name);

    QLabel* numberLabel = new QLabel(tr("Number:"), _frame);
    _number = new LineEdit(5, _frame);
    numberLabel->setBuddy(_number);

    QLabel* offlineLabel = new QLabel(tr("Offline #:"), _frame);
    _offline = new IntegerEdit(5, _frame);
    _offline->setFocusPolicy(NoFocus);
    offlineLabel->setBuddy(_offline);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(2, 1);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(numberLabel, 1, 0);
    grid->addWidget(_number, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(offlineLabel, 2, 0);
    grid->addWidget(_offline, 2, 1, AlignLeft | AlignVCenter);

    setCaption(tr("Station Master"));
    finalize();
}

StationMaster::~StationMaster()
{
}

void
StationMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _name;
}

void
StationMaster::newItem()
{
    Station blank;
    _orig = blank;
    _orig.setNumber("#");

    _curr = _orig;
    _firstField = _name;
}

void
StationMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _curr.setNumber("#");
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _name->selectAll();
}

bool
StationMaster::fileItem()
{
    QString number = _curr.number();
    if (number != "" && _orig.number() != number) {
	vector<Station> stations;
	StationSelect conditions;
	conditions.number = number;
	_quasar->db()->select(stations, conditions);
	if (stations.size() != 0) {
	    QString message = tr("This station number is already used\n"
		"for another station. Are you sure\n"
		"you want to file this station?");
	    int ch = QMessageBox::warning(this, tr("Warning"), message,
					  QMessageBox::No, QMessageBox::Yes);
	    if (ch != QMessageBox::Yes)
		return false;
	}
    }

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
StationMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
StationMaster::restoreItem()
{
    _curr = _orig;
}

void
StationMaster::cloneItem()
{
    StationMaster* clone = new StationMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
StationMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
StationMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _number->setText(_curr.number());
    _offline->setFixed(_curr.offlineNumber());
    _inactive->setChecked(!_curr.isActive());
}

// Set the data object from the widgets.
void
StationMaster::widgetToData()
{
    _curr.setName(_name->text());
    _curr.setNumber(_number->text());
    _curr.setOfflineNumber(_offline->getFixed());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());
}
