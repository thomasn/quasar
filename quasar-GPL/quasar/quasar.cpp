// $Id: quasar.cpp,v 1.59 2005/02/24 09:38:41 bpepers Exp $
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

#include "open_company.h"
#include "user_config.h"
#include "welcome_dialog.h"
#include "quasar_client.h"

#include <qapplication.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <unicode/locid.h>

int
main(int argc, char** argv)
{
    // Setup for Qt
    QApplication app(argc, argv);

    // Parse command line
    for (int i = 1; i < app.argc(); ++i) {
	QString arg = app.argv()[i];
	if (arg == "-configDir") {
	    if (i + 1 == app.argc())
		qFatal("Error: missing -configDir argument");
	    ConfigFile::setDefaultConfigDir(app.argv()[++i]);
	} else if (arg == "-help") {
	    qFatal("Usage: quasar [-configDir dir]");
	} else {
	    qFatal("Error: unknown command line arg: " + arg);
	}
    }

    // Save defaults
    UserConfig::defaultPalette = app.palette();
    UserConfig::defaultFont = app.font();

    // Create or load user configuration
    UserConfig config;
    if (!config.load(false)) {
	// Create config directory if it doesn't exist
	if (!QDir(config.configDir()).exists()) {
	    if (!QDir().mkdir(config.configDir())) {
		QString msg = "Couldn't create \"%1\" directory";
		QMessageBox::critical(0, "Quasar Accounting",
				      msg.arg(config.configDir()));
		exit(1);
	    }
	}

	// Welcome dialog
	WelcomeDialog* dialog = new WelcomeDialog();
	int result = dialog->exec();
	config.locale = dialog->locale();
	config.defaultServer = dialog->serverName();
	delete dialog;
	if (result != QDialog::Accepted)
	    exit(1);

	// Save initial config
	if (!config.save()) {
	    QString msg = "Couldn't create \"%1\" file";
	    QMessageBox::critical(0, "Quasar Accounting",
				  msg.arg(config.configDir()));
	    exit(1);
	}
    }

    // Install translator and set locale
    if (!config.locale.isEmpty())
	QuasarClient::setLocale(config.locale);

    // Set display configuration
    if (config.changeStyle && !config.style.isEmpty())
	app.setStyle(config.style);
    if (config.changeColor && !config.color.isEmpty())
	app.setPalette(QColor(config.color), true);
    if (config.changeFont && !config.font.isEmpty()) {
	QFont font;
	font.fromString(config.font);
	app.setFont(font, true);
    }

    // Start with open company window
    OpenCompany* screen = new OpenCompany();
    screen->setInfo(config.defaultServer, config.defaultCompany,
		    config.defaultUserName, config.defaultPassword);
    screen->show();

    app.setMainWidget(screen);
    return app.exec();
}
