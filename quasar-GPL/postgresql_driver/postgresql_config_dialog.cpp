// $Id: postgresql_config_dialog.cpp,v 1.8 2005/03/05 16:41:31 bpepers Exp $
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

#include "postgresql_config_dialog.h"
#include "postgresql_driver.h"
#include "integer_edit.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qtextcodec.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qsocketdevice.h>
#include <qlibrary.h>

#ifdef LINUX
#include <netdb.h>
#else
#include <winsock.h>
#endif

PostgresqlConfigDialog::PostgresqlConfigDialog(QWidget* parent)
    : QDialog(parent, "PostgresqlConfigDialog", true, WDestructiveClose)
{
    QFrame* main = new QFrame(this);

    QLabel* hostnameLabel = new QLabel(tr("Hostname:"), main);
    _hostname = new LineEdit(main);
    _hostname->setMinCharWidth(20);
    hostnameLabel->setBuddy(_hostname);

    QLabel* portLabel = new QLabel(tr("Port:"), main);
    _port = new IntegerEdit(7, main);
    portLabel->setBuddy(_port);

    QLabel* libraryLabel = new QLabel(tr("Library:"), main);
    _library = new LineEdit(main);
    _library->setMinCharWidth(40);
    libraryLabel->setBuddy(_library);

    QLabel* dbaUsernameLabel = new QLabel(tr("DBA Username:"), main);
    _dbaUsername = new LineEdit(main);
    _dbaUsername->setMinCharWidth(10);
    dbaUsernameLabel->setBuddy(_dbaUsername);

    QLabel* dbaPasswordLabel = new QLabel(tr("DBA Password:"), main);
    _dbaPassword = new LineEdit(main);
    _dbaPassword->setMinCharWidth(10);
    dbaPasswordLabel->setBuddy(_dbaPassword);

    QLabel* usernameLabel = new QLabel(tr("Username:"), main);
    _username = new LineEdit(main);
    _username->setMinCharWidth(10);
    usernameLabel->setBuddy(_username);

    QLabel* passwordLabel = new QLabel(tr("Password:"), main);
    _password = new LineEdit(main);
    _password->setMinCharWidth(10);
    passwordLabel->setBuddy(_password);

    QLabel* charSetLabel = new QLabel(tr("Character Set:"), main);
    _charSet = new LineEdit(main);
    _charSet->setMinCharWidth(20);
    charSetLabel->setBuddy(_charSet);

    QGridLayout* mainGrid = new QGridLayout(main);
    mainGrid->setSpacing(3);
    mainGrid->setMargin(3);
    mainGrid->setRowStretch(8, 1);
    mainGrid->setColStretch(1, 1);
    mainGrid->addWidget(hostnameLabel, 0, 0);
    mainGrid->addWidget(_hostname, 0, 1, AlignLeft | AlignVCenter);
    mainGrid->addWidget(portLabel, 1, 0);
    mainGrid->addWidget(_port, 1, 1, AlignLeft | AlignVCenter);
    mainGrid->addWidget(libraryLabel, 2, 0);
    mainGrid->addWidget(_library, 2, 1, AlignLeft | AlignVCenter);
    mainGrid->addWidget(dbaUsernameLabel, 3, 0);
    mainGrid->addWidget(_dbaUsername, 3, 1, AlignLeft | AlignVCenter);
    mainGrid->addWidget(dbaPasswordLabel, 4, 0);
    mainGrid->addWidget(_dbaPassword, 4, 1, AlignLeft | AlignVCenter);
    mainGrid->addWidget(usernameLabel, 5, 0);
    mainGrid->addWidget(_username, 5, 1, AlignLeft | AlignVCenter);
    mainGrid->addWidget(passwordLabel, 6, 0);
    mainGrid->addWidget(_password, 6, 1, AlignLeft | AlignVCenter);
    mainGrid->addWidget(charSetLabel, 7, 0);
    mainGrid->addWidget(_charSet, 7, 1, AlignLeft | AlignVCenter);

    _config.load(false);

    _hostname->setText(_config.hostname);
    _port->setInt(_config.port);
    if (_config.port == 0) _port->setText("");
    _library->setText(_config.library);
    _dbaUsername->setText(_config.dbaUsername);
    _dbaPassword->setText(_config.dbaPassword);
    _username->setText(_config.username);
    _password->setText(_config.password);
    _charSet->setText(_config.charSet);

    QFrame* buttons = new QFrame(this);
    QPushButton* test = new QPushButton(tr("Test"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);
    QPushButton* ok = new QPushButton(tr("OK"), buttons);

    ok->setDefault(true);
    connect(test, SIGNAL(clicked()), SLOT(test()));
    connect(cancel, SIGNAL(clicked()), SLOT(reject()));
    connect(ok, SIGNAL(clicked()), SLOT(accept()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(3);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(1, 1);
    buttonGrid->addWidget(test, 0, 0);
    buttonGrid->addWidget(cancel, 0, 2);
    buttonGrid->addWidget(ok, 0, 3);

    QGridLayout* grid = new QGridLayout(this);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->addWidget(main, 0, 0);
    grid->addWidget(buttons, 1, 0);

    setCaption(tr("PostgreSQL Driver Config"));
}

PostgresqlConfigDialog::~PostgresqlConfigDialog()
{
}

void
PostgresqlConfigDialog::test()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Validate hostname
    QString hostname = _hostname->text();
    QHostAddress address;
    if (hostname.isEmpty()) {
	warning("With no hostname set, connections\n"
		"will use unix-domain sockets which\n"
		"should be fine and is even a good\n"
		"choice over using localhost since\n"
		"unix-domain sockets should be faster\n"
		"but it does mean that different\n"
		"authentication methods will be used\n"
		"depending on the pg_hba.conf file");
    } else {
	struct hostent* host = NULL;
	int error = 0;
#ifdef WIN32
	WORD version = MAKEWORD(1, 0);
	WSADATA wsaData;
	if (WSAStartup(version, &wsaData) == 0) {
	    hostent = gethostbyname(hostname);
	    if (hostent == NULL) {
		unsigned long addr = inet_addr(hostname);
		hostent = gethostbyaddr((char*)(&addr), sizeof(addr), AF_INET);
	    }
	    error = WSAGetLastError();
	    WSACleanup();
	}
#else
	host = gethostbyname(hostname);
	error = h_errno;
#endif
	if (host == NULL) {
	    critical("Failed to lookup the hostname which can\n"
		     "be an error such as a mis-typed host name\n"
		     "or could be a temporary host resolution\n"
		     "problem or might be because the database\n"
		     "server computer is shutdown");
	} else {
	    int n1 = ((unsigned char*)host->h_addr)[0];
	    int n2 = ((unsigned char*)host->h_addr)[1];
	    int n3 = ((unsigned char*)host->h_addr)[2];
	    int n4 = ((unsigned char*)host->h_addr)[3];
	    QString a = QString("%1.%2.%3.%4").arg(n1).arg(n2).arg(n3).arg(n4);
	    address.setAddress(a);
	}
    }

    // Validate the port
    int port = _port->getInt();
    if (port == 0) {
	port = 5432;
	warning("A blank or zero port will default to using\n"
		"port 5432 which is the default for PostgreSQL\n"
		"and should be fine");
    }

    // Try to connect to port on hostname
    if (address.toString() != "0.0.0.0") {
	QSocketDevice socket;
	if (!socket.connect(address, port)) {
	    warning("Failed connecting to port on hostname\n"
		    "which may be because the server is down\n"
		    "or may be due to a firewall problem or\n"
		    "PostgreSQL not running on the server");
	}
    }

    // Validate library
    QString libraryFile = _library->text();
    if (libraryFile.isEmpty()) {
	warning("A blank library file will cause the\n"
		"Quasar driver to try and guess the\n"
		"correct library name but it may fail\n"
		"and its better to explicitly enter\n"
		"the library name here");
    } else if (!QFile::exists(libraryFile)) {
	warning("The library file doesn't exist which means\n"
		"either the database server hasn't been\n"
		"installed yet or else the library file is\n"
		"invalid.  This will have to be fixed before\n"
		"Quasar will work with PostgreSQL");
    } else {
	QLibrary* library = new QLibrary(libraryFile);
	if (!library->load()) {
	    warning("Loading the library failed which\n"
		    "means the driver won't work until\n"
		    "the library is installed or a valid\b"
		    "library is entered");
	}
	delete library;
    }

    // Validate dba username/password
    QString dbaUsername = _dbaUsername->text();
    QString dbaPassword = _dbaPassword->text();
    if (dbaUsername.isEmpty()) {
	critical("A username for dba connections is\n"
		 "required");
    }
    if (dbaPassword.isEmpty()) {
	warning("A blank dba password either means\n"
		"you've not entered one and you will\n"
		"need to or else your dba user has\n"
		"a blank password which would be a\n"
		"bad idea");
    }

    // Validate username/password
    QString username = _username->text();
    QString password = _password->text();
    if (username.isEmpty()) {
	critical("A username for non-dba connections\n"
		 "is required");
    } else if (password.isEmpty()) {
	warning("A blank non-dba password either means\n"
		"you've not entered one and you will\n"
		"need to or else your non-dba user has\n"
		"a blank password which would be a bad\n"
		"idea");
    }

    // Validate character set
    QString charSet = _charSet->text();
    if (charSet.isEmpty()) {
	warning("A blank character set will use the\n"
		"default for your locale which should\n"
		"be fine but it might be better to use\n"
		"UNICODE if you want to be able to\n"
		"handle a full range of string types");
    }

    // Save the current configuration to test with it
    PostgresqlConfig config = getConfig();
    if (!config.save(true)) {
	critical("Saving the configuration failed so\n"
		 "testing connecting to the security\n"
		 "database cannot occur");
    } else {
	// Try to connect to security.fdb and validate username/password
	PostgresqlDriver driver;
	if (!driver.initialize()) {
	    critical("The PostgreSQL driver failed to\n"
		     "initialize itself.  If there\n"
		     "have been other errors reported\n"
		     "before this then they should\n"
		     "explain what may be wrong.");
	} else {
	    PostgresqlConn conn(&driver);
	    if (!conn.dbaConnect("template1")) {
		critical("Failed to connect to the template1\n"
			 "database using the dba user.  If\n"
			 "you've received other error messages\n"
			 "then they should tell you what needs\n"
			 "to be fixed.  If you have not had any\n"
			 "other errors then your dba username or\n"
			 "password may be wrong");
	    }
	    if (!conn.connect("template1")) {
		critical("Failed to connect to the template1\n"
			 "database using the non-dba user.  If\n"
			 "you've received other error messages\n"
			 "then they should tell you what needs\n"
			 "to be fixed.  If you have not had any\n"
			 "other errors then your non-dba username\n"
			 "or password may be wrong");
	    }
	}

	// Restore old configuration for now
	if (!_config.save(true)) {
	    critical("Restoring the configuration failed so\n"
		     "your current configuration has been saved\n"
		     "and the original was not restored");
	}
    }

    QApplication::restoreOverrideCursor();
    QMessageBox::information(this, tr("Finished"), tr("Testing complete"));
}

void
PostgresqlConfigDialog::warning(const QString& message)
{
    QApplication::restoreOverrideCursor();

    qApp->beep();
    QMessageBox::warning(this, tr("Warning"), message);

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();
}

void
PostgresqlConfigDialog::critical(const QString& message)
{
    QApplication::restoreOverrideCursor();

    qApp->beep();
    QMessageBox::critical(this, tr("Error"), message);
    _critical = true;

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();
}

PostgresqlConfig
PostgresqlConfigDialog::getConfig()
{
    PostgresqlConfig config;
    config.hostname = _hostname->text();
    config.port = _port->getInt();
    config.library = _library->text();
    config.dbaUsername = _dbaUsername->text();
    config.dbaPassword = _dbaPassword->text();
    config.username = _username->text();
    config.password = _password->text();
    config.charSet = _charSet->text();
    return config;
}

void
PostgresqlConfigDialog::accept()
{
    _critical = false;
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Validate usernames
    QString dbaUsername = _dbaUsername->text();
    QString username = _username->text();
    if (dbaUsername.isEmpty()) {
	critical("A username for dba connections\n"
		 "is required");
    }
    if (username.isEmpty()) {
	critical("A username for non-dba connections\n"
		 "is required");
    }

    QApplication::restoreOverrideCursor();
    if (_critical) return;

    PostgresqlConfig config = getConfig();
    if (!config.save(true)) {
	QString message = tr("Saving configuration failed");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QString message = tr("Configuration data saved");
    QMessageBox::information(this, tr("Status"), message);

    QDialog::accept();
}

void
PostgresqlConfigDialog::reject()
{
    if (_config != getConfig()) {
	QString message = tr("Changes have been made to the\n"
			     "configuration.  Are you sure you\n"
			     "want to exit?");
	int ch = QMessageBox::warning(this, tr("Exit?"), message,
				      QMessageBox::No, QMessageBox::Yes);
	if (ch != QMessageBox::Yes) return;
    }

    QDialog::reject();
}
