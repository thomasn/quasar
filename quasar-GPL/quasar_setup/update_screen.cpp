// $Id: update_screen.cpp,v 1.5 2005/03/01 19:53:02 bpepers Exp $
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

#include "update_screen.h"
#include "quasar_version.h"
#include "server_config.h"
#include "data_model.h"
#include "line_edit.h"
#include "combo_box.h"
#include "stmt.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qdir.h>

UpdateScreen::UpdateScreen(const CompanyDefn& company)
    : QMainWindow(0, "UpdateScreen", WType_TopLevel | WDestructiveClose),
      _connection(NULL)
{
    QFrame* frame = new QFrame(this);

    QLabel* fromLabel = new QLabel(tr("From Version:"), frame);
    _fromVersion = new LineEdit(14, frame);
    _fromVersion->setFocusPolicy(NoFocus);

    QLabel* toLabel = new QLabel(tr("To Version:"), frame);
    _toVersion = new ComboBox(frame);
    toLabel->setBuddy(_toVersion);

    QFrame* buttons = new QFrame(frame);
    _update = new QPushButton(tr("&Update"), buttons);
    _close = new QPushButton(tr("&Close"), buttons);

    connect(_update, SIGNAL(clicked()), SLOT(slotUpdate()));
    connect(_close, SIGNAL(clicked()), SLOT(slotClose()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(_update, 0, 1);
    buttonGrid->addWidget(_close, 0, 2);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(2, 1);
    grid->addColSpacing(2, 20);
    grid->addWidget(fromLabel, 0, 0);
    grid->addWidget(_fromVersion, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(toLabel, 0, 3);
    grid->addWidget(_toVersion, 0, 4, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(buttons, 1, 1, 0, 4);

    ServerConfig config;
    config.load();

    _fromVersion->setText(company.version());

    QDir dir(parseDir(config.dataDir), "*.xml");
    dir.cd("models");
    QStringList entries = dir.entryList();
    for (unsigned int i = 0; i < entries.size(); ++i) {
	QString version = QFileInfo(entries[i]).baseName();
	if (version < company.version()) continue;
	_toVersion->insertItem(version);
    }
    _toVersion->setCurrentItem(company.version());

    _company = company;

    setCentralWidget(frame);
    setCaption(tr("Version Update: %1").arg(company.name()));
}

UpdateScreen::~UpdateScreen()
{
    delete _connection;
}

void
UpdateScreen::slotUpdate()
{
    QString from = _fromVersion->text();
    QString to = _toVersion->currentText();

    if (from == to) {
	qApp->beep();
	QString message = tr("Versions must be different");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    // Connect to company database
    Driver* driver = Driver::getDriver(_company.dbType());
    if (driver == NULL) {
	qApp->beep();
	QString message = tr("Get driver failed: %1").arg(driver->lastError());
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    _connection = driver->allocConnection();
    if (!_connection->dbaConnect(_company.database())) {
	qApp->beep();
	QString message = tr("Open company failed: %1")
	    .arg(_connection->lastError());
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    ServerConfig config;
    config.load();

    QString dataDir = parseDir(config.dataDir);
    QString version = to;

    QValueVector<DataModel> models;
    while (true) {
	QString filePath = dataDir + "/models/" + version + ".xml";
	DataModel model;
	if (!model.load(filePath)) {
	    qApp->beep();
	    QString message = tr("Failed loading model: %1").arg(filePath);
	    QMessageBox::critical(this, tr("Error"), message);
	    return;
	}

	if (model.fromVersion.isEmpty()) {
	    qApp->beep();
	    QString message = tr("Didn't find version: " + from);
	    QMessageBox::critical(this, tr("Error"), message);
	    return;
	}

	models.push_back(model);
	version = model.fromVersion;
	if (version == from) break;
    }

    if (models.size() == 0) {
	qApp->beep();
	QString message = tr("No models found for update use");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QString dbType = _company.dbType();
    for (int i = models.size() - 1; i >= 0; --i) {
	DataModel& model = models[i];
	bool good = true;

	QValueVector<UpdateDefn> completed;
	for (unsigned int j = 0; j < model.updates.size(); ++j) {
	    UpdateDefn& update = model.updates[j];

	    // Only run update if its for All or the right type
	    if (!update.databases.contains("All"))
		if (!update.databases.contains(dbType))
		    continue;

	    // Run update commands catching errors
	    for (unsigned int k = 0; k < update.updateCmds.size(); ++k) {
		QString cmd = update.updateCmds[k];
		Stmt stmt(_connection, cmd);
		if (!stmt.execute()) {
		    qWarning(stmt.lastError());
		    good = false;
		    break;
		}
	    }

	    completed.push_back(update);
	}

	// If good then try to run cleanup commands
	if (good) {
	    for (unsigned int j = 0; j < completed.size(); ++j) {
		UpdateDefn& update = completed[j];

		for (unsigned int k = 0; k < update.cleanupCmds.size(); ++k) {
		    QString cmd = update.cleanupCmds[k];
		    Stmt stmt(_connection, cmd);
		    if (!stmt.execute()) {
			qWarning(stmt.lastError());
			good = false;
			break;
		    }
		}
	    }
	}

	// If still good then update version in database
	if (good) {
	    QString cmd = "update db_config set config_value='" +
		model.version + "' where config_key='version'";
	    Stmt stmt(_connection, cmd);
	    if (!stmt.execute()) {
		qWarning(stmt.lastError());
		good = false;
	    }
	    _connection->commit();
	}

	// If failed then run restore commands and inform user
	if (!good) {
	    for (unsigned int j = 0; j < completed.size(); ++j) {
		UpdateDefn& update = completed[j];

		for (unsigned int k = 0; k < update.restoreCmds.size(); ++k) {
		    QString cmd = update.restoreCmds[k];
		    Stmt stmt(_connection, cmd);
		    if (!stmt.execute()) {
			qWarning(stmt.lastError());
		    }
		}
	    }

	    qApp->beep();
	    QString message = tr("Update failed on version: " + model.version);
	    QMessageBox::critical(this, tr("Error"), message);
	    close();
	    return;
	}

	// Set new version in CompanyDefn
	CompanyDefn company = _company;
	company.setVersion(model.version);
	company.save(company.filePath(), true);
	_company = company;
    }

    QMessageBox::information(this, tr("Finished"), tr("Update successful"));
    close();
}

void
UpdateScreen::slotClose()
{
    close();
}
