// $Id: new_company.cpp,v 1.11 2005/03/15 15:36:36 bpepers Exp $
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

#include "new_company.h"
#include "quasar_version.h"
#include "quasar_misc.h"
#include "server_config.h"
#include "data_model.h"
#include "line_edit.h"
#include "combo_box.h"
#include "quasar_db.h"
#include "company.h"
#include "user.h"
#include "store.h"
#include "conn.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>

NewCompany::NewCompany()
    : QMainWindow(0, "NewCompany", WType_TopLevel | WDestructiveClose)
{
    QFrame* frame = new QFrame(this);

    QLabel* nameLabel = new QLabel(tr("Name:"), frame);
    _name = new LineEdit(60, frame);
    nameLabel->setBuddy(_name);

    QLabel* typeLabel = new QLabel(tr("&Database:"), frame);
    _type = new ComboBox(frame);
    typeLabel->setBuddy(_type);
    _type->insertStringList(Driver::types());

    QFrame* buttons = new QFrame(frame);
    QPushButton* create = new QPushButton(tr("Create"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);

    connect(create, SIGNAL(clicked()), SLOT(slotCreate()));
    connect(cancel, SIGNAL(clicked()), SLOT(close()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(create, 0, 1);
    buttonGrid->addWidget(cancel, 0, 2);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(1, 1);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1);
    grid->addWidget(typeLabel, 1, 0);
    grid->addWidget(_type, 1, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(buttons, 2, 2, 0, 1);

    setCentralWidget(frame);
    setCaption(tr("New Company"));
}

NewCompany::~NewCompany()
{
}

void
NewCompany::slotCreate()
{
    QString companyName = _name->text();
    QString databaseType = _type->currentText();

    ServerConfig config;
    if (!config.load()) {
	critical("Loading server.cfg failed");
	return;
    }

    // Get the data model for the current version
    QString dataDir = parseDir(config.dataDir);
    QString filePath = dataDir + "/models/" + QUASAR_DB_VERSION + ".xml";
    DataModel model;
    if (!model.load(filePath)) {
	critical("Loading data model failed");
	return;
    }

    // Database driver
    Driver* driver = Driver::getDriver(databaseType);
    if (driver == NULL) {
	critical("Failed loading driver");
	return;
    }

    // Change cursor
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Company definition setup
    CompanyDefn companyDefn;
    companyDefn.setName(companyName);
    companyDefn.setVersion(QUASAR_DB_VERSION);

    // Create the company database
    if (!driver->create(companyDefn)) {
	QApplication::restoreOverrideCursor();
	critical(driver->lastError());
	return;
    }

    // Connect to the company database
    Conn conn(driver);
    if (!conn.dbaConnect(companyDefn.database())) {
	qWarning("Error: " + conn.lastError());
	driver->remove(companyDefn);
	QApplication::restoreOverrideCursor();
	critical(conn.lastError());
	return;
    }

    // Create the tables
    QValueList<TableDefn> tables = model.tables();
    if (!conn.create(tables)) {
	qWarning("Error: " + conn.lastError());
	conn.disconnect();
	driver->remove(companyDefn);
	QApplication::restoreOverrideCursor();
	critical(conn.lastError());
	return;
    }

    // Create QuasarDB object for database operations
    QuasarDB db(conn);
    db.setUsername("system");
    db.setSystemId(config.systemId);

    // Set the version in the config
    if (!db.setConfig("version", QUASAR_DB_VERSION)) {
	conn.disconnect();
	driver->remove(companyDefn);
	QApplication::restoreOverrideCursor();
	critical("Error setting version in config user");
	return;
    }

    // Create the default admin user
    User admin;
    admin.setName("admin");
    admin.setPassword(sha1Crypt(config.adminPassword));
    if (!db.create(admin)) {
	conn.disconnect();
	driver->remove(companyDefn);
	QApplication::restoreOverrideCursor();
	critical("Error creating admin user");
	return;
    }

    // Create the default company
    Company company;
    company.setName(companyName);
    company.setNumber("1");
    if (!db.create(company)) {
	conn.disconnect();
	driver->remove(companyDefn);
	QApplication::restoreOverrideCursor();
	critical("Error creating default company");
	return;
    }

    // Create the default store
    Store store;
    store.setName("Default");
    store.setNumber("1");
    store.setCanSell(true);
    store.setCompanyId(company.id());
    if (!db.create(store)) {
	conn.disconnect();
	driver->remove(companyDefn);
	QApplication::restoreOverrideCursor();
	critical("Error creating default store");
	return;
    }

    // Set default store in company
    Company orig = company;
    company.setDefaultStore(store.id());
    if (!db.update(orig, company)) {
	conn.disconnect();
	driver->remove(companyDefn);
	QApplication::restoreOverrideCursor();
	critical("Error updating company");
	return;
    }

    // Save company definition
    filePath = dataDir + "/companies/" + companyName + ".xml";
    if (!companyDefn.save(filePath)) {
	conn.disconnect();
	driver->remove(companyDefn);
	QApplication::restoreOverrideCursor();
	critical(companyDefn.lastError());
	return;
    }

    QApplication::restoreOverrideCursor();
    close();
}

void
NewCompany::critical(const QString& message)
{
    qApp->beep();
    QMessageBox::critical(this, tr("Error"), message);
}
