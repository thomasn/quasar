// $Id: server_config_tab.cpp,v 1.4 2005/03/03 21:35:33 bpepers Exp $
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

#include "server_config_tab.h"
#include "integer_edit.h"

#include <qlabel.h>
#include <qlayout.h>

ServerConfigTab::ServerConfigTab(QWidget* parent)
    : ConfigTab(parent, "ServerConfig")
{
    QLabel* portLabel = new QLabel(tr("Port:"), this);
    _port = new IntegerEdit(6, this);
    portLabel->setBuddy(_port);

    QLabel* systemLabel = new QLabel(tr("System Id:"), this);
    _systemId = new IntegerEdit(4, this);
    systemLabel->setBuddy(_systemId);

    QLabel* dataLabel = new QLabel(tr("Data Dir:"), this);
    _dataDir = new LineEdit(40, this);
    dataLabel->setBuddy(_dataDir);

    QLabel* driverLabel = new QLabel(tr("Driver Dir:"), this);
    _driverDir = new LineEdit(40, this);
    driverLabel->setBuddy(_driverDir);

    QLabel* backupLabel = new QLabel(tr("Backup Dir:"), this);
    _backupDir = new LineEdit(40, this);
    backupLabel->setBuddy(_backupDir);

    QLabel* importLabel = new QLabel(tr("Import Dir:"), this);
    _importDir = new LineEdit(40, this);
    importLabel->setBuddy(_importDir);

    QLabel* adminLabel = new QLabel(tr("Admin Password:"), this);
    _adminPassword = new LineEdit(20, this);
    adminLabel->setBuddy(_adminPassword);

    QGridLayout* grid = new QGridLayout(this);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(7, 1);
    grid->setColStretch(2, 1);
    grid->addWidget(portLabel, 0, 0);
    grid->addWidget(_port, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(systemLabel, 1, 0);
    grid->addWidget(_systemId, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(dataLabel, 2, 0);
    grid->addWidget(_dataDir, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(driverLabel, 3, 0);
    grid->addWidget(_driverDir, 3, 1, AlignLeft | AlignVCenter);
    grid->addWidget(backupLabel, 4, 0);
    grid->addWidget(_backupDir, 4, 1, AlignLeft | AlignVCenter);
    grid->addWidget(importLabel, 5, 0);
    grid->addWidget(_importDir, 5, 1, AlignLeft | AlignVCenter);
    grid->addWidget(adminLabel, 6, 0);
    grid->addWidget(_adminPassword, 6, 1, AlignLeft | AlignVCenter);
}

ServerConfigTab::~ServerConfigTab()
{
}

void
ServerConfigTab::loadData()
{
    ServerConfig config;
    config.load();

    if (config.port == 0)
	_port->setText("");
    else
	_port->setInt(config.port);
    _systemId->setInt(config.systemId);
    _dataDir->setText(config.dataDir);
    _driverDir->setText(config.driverDir);
    _backupDir->setText(config.backupDir);
    _importDir->setText(config.importDir);
    _adminPassword->setText(config.adminPassword);

    _port->setEnabled(config.canSave());
    _systemId->setEnabled(config.canSave());
    _dataDir->setEnabled(config.canSave());
    _driverDir->setEnabled(config.canSave());
    _backupDir->setEnabled(config.canSave());
    _importDir->setEnabled(config.canSave());
    _adminPassword->setEnabled(config.canSave());
}

bool
ServerConfigTab::saveChanges()
{
    ServerConfig config;
    if (!config.canSave()) return true;

    config.port = _port->getInt();
    config.systemId = _systemId->getInt();
    config.dataDir = _dataDir->text();
    config.driverDir = _driverDir->text();
    config.backupDir = _backupDir->text();
    config.importDir = _importDir->text();
    config.adminPassword = _adminPassword->text();
    return config.save(true);
}
