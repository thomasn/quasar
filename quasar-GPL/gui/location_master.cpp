// $Id: location_master.cpp,v 1.15 2004/01/31 01:50:30 arandell Exp $
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

#include "location_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "location_select.h"
#include "line_edit.h"
#include "lookup_edit.h"
#include "store_lookup.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qlayout.h>
#include <qmessagebox.h>

LocationMaster::LocationMaster(MainWindow* main, Id location_id)
    : DataWindow(main, "LocationMaster", location_id)
{
    _helpSource = "location_master.html";

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("Name:"), _frame);
    _name = new LineEdit(_frame);
    _name->setLength(30);
    _name->setFocusPolicy(NoFocus);

    QLabel* storeLabel = new QLabel(tr("Store:"), _frame);
    _store = new LookupEdit(new StoreLookup(_main, this), _frame);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    QLabel* sectionLabel = new QLabel(tr("Section:"), _frame);
    _section = new LineEdit(_frame);
    _section->setLength(14);
    sectionLabel->setBuddy(_section);
    connect(_section, SIGNAL(validData()), SLOT(slotUpdateName()));

    QLabel* fixtureLabel = new QLabel(tr("Fixture:"), _frame);
    _fixture = new LineEdit(_frame);
    _fixture->setLength(10);
    fixtureLabel->setBuddy(_fixture);
    connect(_fixture, SIGNAL(validData()), SLOT(slotUpdateName()));

    QLabel* binLabel = new QLabel(tr("Bin:"), _frame);
    _bin = new LineEdit(_frame);
    _bin->setLength(4);
    binLabel->setBuddy(_bin);
    connect(_bin, SIGNAL(validData()), SLOT(slotUpdateName()));

    QGridLayout *grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(storeLabel, 1, 0);
    grid->addWidget(_store, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(sectionLabel, 2, 0);
    grid->addWidget(_section, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(fixtureLabel, 3, 0);
    grid->addWidget(_fixture, 3, 1, AlignLeft | AlignVCenter);
    grid->addWidget(binLabel, 4, 0);
    grid->addWidget(_bin, 4, 1, AlignLeft | AlignVCenter);

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    setCaption(tr("Location Master"));
    finalize();
}

LocationMaster::~LocationMaster()
{
}

void
LocationMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _section;
}

void
LocationMaster::newItem()
{
    Location blank;
    _orig = blank;
    _orig.setStoreId(_quasar->defaultStore());

    _curr = _orig;
    _firstField = _section;
}

void
LocationMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _section->selectAll();
}

bool
LocationMaster::fileItem()
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
LocationMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
LocationMaster::restoreItem()
{
    _curr = _orig;
}

void
LocationMaster::cloneItem()
{
    LocationMaster* clone = new LocationMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
LocationMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
LocationMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _store->setId(_curr.storeId());
    _section->setText(_curr.section());
    _fixture->setText(_curr.fixture());
    _bin->setText(_curr.bin());
    _inactive->setChecked(!_curr.isActive());
}

// Set the data object from the widgets.
void
LocationMaster::widgetToData()
{
    _curr.setStoreId(_store->getId());
    _curr.setSection(_section->text());
    _curr.setFixture(_fixture->text());
    _curr.setBin(_bin->text());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());
}

void
LocationMaster::slotUpdateName()
{
    QString section = _section->text();
    QString fixture = _fixture->text();
    QString bin = _bin->text();

    QString name = section;
    if (!fixture.isEmpty()) name += "," + fixture;
    if (!bin.isEmpty()) name += "," + bin;

    _name->setText(name);
}
