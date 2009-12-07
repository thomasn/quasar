// $Id: client_config_tab.cpp,v 1.2 2004/05/22 18:07:41 bpepers Exp $
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

#include "client_config_tab.h"
#include "integer_edit.h"

#include <qlabel.h>
#include <qlayout.h>

ClientConfigTab::ClientConfigTab(QWidget* parent)
    : ConfigTab(parent, "ClientConfig")
{
    QLabel* localeLabel = new QLabel(tr("Locale Dir:"), this);
    _localeDir = new LineEdit(40, this);
    localeLabel->setBuddy(_localeDir);

    QLabel* stationLabel = new QLabel(tr("Station:"), this);
    _station = new IntegerEdit(4, this);
    stationLabel->setBuddy(_station);

    QGridLayout* grid = new QGridLayout(this);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(2, 1);
    grid->setColStretch(2, 1);
    grid->addWidget(localeLabel, 0, 0);
    grid->addWidget(_localeDir, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(stationLabel, 1, 0);
    grid->addWidget(_station, 1, 1, AlignLeft | AlignVCenter);
}

ClientConfigTab::~ClientConfigTab()
{
}

void
ClientConfigTab::loadData()
{
    ClientConfig config;
    config.load();

    _localeDir->setText(config.localeDir);
    _station->setText(config.stationNumber);

    _localeDir->setEnabled(config.canSave());
    _station->setEnabled(config.canSave());
}

bool
ClientConfigTab::saveChanges()
{
    ClientConfig config;
    if (!config.canSave()) return true;

    config.localeDir = _localeDir->text();
    config.stationNumber = _station->text();
    return config.save(true);
}
