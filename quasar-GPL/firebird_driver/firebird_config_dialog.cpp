// $Id: firebird_config_dialog.cpp,v 1.17 2005/03/09 23:14:51 bpepers Exp $
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

#include "firebird_config_dialog.h"
#include "firebird_driver.h"
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

extern "C" {
	extern char* crypt(const char* password, const char* salt);
}

FirebirdConfigDialog::FirebirdConfigDialog(QWidget* parent)
    : QDialog(parent, "FirebirdConfigDialog", true, WDestructiveClose)
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

    QLabel* installLabel = new QLabel(tr("Install Directory:"), main);
    _installDir = new LineEdit(main);
    _installDir->setMinCharWidth(40);
    installLabel->setBuddy(_installDir);

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

    QLabel* databaseLabel = new QLabel(tr("Database Directory:"), main);
    _databaseDir = new LineEdit(main);
    _databaseDir->setMinCharWidth(40);
    databaseLabel->setBuddy(_databaseDir);

    QLabel* blockSizeLabel = new QLabel(tr("Block Size:"), main);
    _blockSize = new IntegerEdit(7, main);
    blockSizeLabel->setBuddy(_blockSize);

    QLabel* charSetLabel = new QLabel(tr("Character Set:"), main);
    _charSet = new LineEdit(main);
    _charSet->setMinCharWidth(20);
    charSetLabel->setBuddy(_charSet);

    QGridLayout* mainGrid = new QGridLayout(main);
    mainGrid->setSpacing(3);
    mainGrid->setMargin(3);
    mainGrid->setRowStretch(10, 1);
    mainGrid->setColStretch(1, 1);
    mainGrid->addWidget(hostnameLabel, 0, 0);
    mainGrid->addWidget(_hostname, 0, 1, AlignLeft | AlignVCenter);
    mainGrid->addWidget(portLabel, 1, 0);
    mainGrid->addWidget(_port, 1, 1, AlignLeft | AlignVCenter);
    mainGrid->addWidget(libraryLabel, 2, 0);
    mainGrid->addWidget(_library, 2, 1, AlignLeft | AlignVCenter);
    mainGrid->addWidget(installLabel, 3, 0);
    mainGrid->addWidget(_installDir, 3, 1, AlignLeft | AlignVCenter);
    mainGrid->addWidget(dbaPasswordLabel, 4, 0);
    mainGrid->addWidget(_dbaPassword, 4, 1, AlignLeft | AlignVCenter);
    mainGrid->addWidget(usernameLabel, 5, 0);
    mainGrid->addWidget(_username, 5, 1, AlignLeft | AlignVCenter);
    mainGrid->addWidget(passwordLabel, 6, 0);
    mainGrid->addWidget(_password, 6, 1, AlignLeft | AlignVCenter);
    mainGrid->addWidget(databaseLabel, 7, 0);
    mainGrid->addWidget(_databaseDir, 7, 1, AlignLeft | AlignVCenter);
    mainGrid->addWidget(blockSizeLabel, 8, 0);
    mainGrid->addWidget(_blockSize, 8, 1, AlignLeft | AlignVCenter);
    mainGrid->addWidget(charSetLabel, 9, 0);
    mainGrid->addWidget(_charSet, 9, 1, AlignLeft | AlignVCenter);

    _config.load(false);

    _hostname->setText(_config.hostname);
    _port->setInt(_config.port);
    _library->setText(_config.library);
    _installDir->setText(_config.installDir);
    _dbaPassword->setText(_config.dbaPassword);
    _username->setText(_config.username);
    _password->setText(_config.password);
    _databaseDir->setText(_config.databaseDir);
    _blockSize->setInt(_config.blockSize);
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

    setCaption(tr("Firebird Driver Config"));
}

FirebirdConfigDialog::~FirebirdConfigDialog()
{
}

void
FirebirdConfigDialog::test()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Validate hostname
    QString hostname = _hostname->text();
    QHostAddress address;
    if (hostname.isEmpty()) {
	hostname = "localhost";
	address.setAddress("127.0.0.1");
	warning("A blank hostname will create a\n"
		"dedicated connection to the\n"
		"database file.  For shared use\n"
		"its better to use \"localhost\"\n"
		"for the hostname");
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
	port = 3050;
	warning("A blank or zero port will default to using\n"
		"port 3050 which is the default for Firebird\n"
		"and should be fine");
    }

    // Try to connect to port on hostname
    if (address.toString() != "0.0.0.0") {
	QSocketDevice socket;
	if (!socket.connect(address, port)) {
	    warning("Failed connecting to port on hostname\n"
		    "which may be because the server is down\n"
		    "or may be due to a firewall problem or\n"
		    "Firebird not running on the server");
	}
    }

    // Validate library
    QString libraryFile = _library->text();
    if (libraryFile.isEmpty()) {
	critical("A library file is required for Quasar\n"
		 "to use the Firebird database");
    } else if (!QFile::exists(libraryFile)) {
	warning("The library file doesn't exist which means\n"
		"either the database server hasn't been\n"
		"installed yet or else the library file is\n"
		"invalid.  This will have to be fixed before\n"
		"Quasar will work with Firebird");
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

    // Validate installation directory
    QString installDir = _installDir->text();
    if (installDir.isEmpty()) {
	critical("A installation directory is required for\n"
		 "Quasar to use the Firebird database");
    } else if (!QFile::exists(installDir)) {
	warning("The install directory doesn't exist which\n"
		"means either the database server hasn't been\n"
		"installed yet or else the install directory\n"
		"is invalid.  This will have to be fixed before\n"
		"Quasar will work with Firebird");
    } else if (!QFileInfo(installDir).isDir()) {
	critical("The installation directory is not set to\n"
		 "a directory");
    } else if (!QFile::exists(installDir + "/security.fdb")) {
	warning("The installation directory doesn't have\n"
		"the security.fdb file in it which likely\n"
		"means its not a proper Firebird install\n"
		"directory");
    }

    // Validate dba password
    QString dbaPassword = _dbaPassword->text();
    if (dbaPassword.isEmpty()) {
	warning("A blank dba password means the driver\n"
		"will try to read the password from the\n"
		"SYSDBA.password file in the Firebird\n"
		"installation directory but this file\n"
		"may not be readable by the Quasar server\n"
		"program");
    }

    // Validate username/password
    QString username = _username->text();
    QString password = _password->text();
    if (username.isEmpty()) {
	critical("A username for non-dba connections\n"
		 "is required");
    } else if (password.isEmpty()) {
	warning("A blank password for non-dba users\n"
		"is not recommended");
    }

    // Validate database dir
    QString databaseDir = parseDir(_databaseDir->text());
    if (address.toString() != "127.0.0.1") {
	warning("Your configuration is using a remote\n"
		"host for the Firebird database server\n"
		"so the database directory cannot be\n"
		"verified directly but if its wrong it\n"
		"will cause creating a company to fail");
    } else if (databaseDir.isEmpty()) {
	critical("A database directory is required for\n"
		 "Quasar to use the Firebird database");
    } else if (!QFile::exists(databaseDir)) {
	warning("The database directory doesn't exist which\n"
		"means either the directory hasn't been\n"
		"created yet or else the database directory\n"
		"is invalid.  This will have to be fixed before\n"
		"Quasar will work with Firebird");
    } else if (!QFileInfo(databaseDir).isDir()) {
	critical("The database directory is not set to\n"
		 "a directory");
    } else {
	// TODO: check permissions?
    }

    // Validate block size
    int blockSize = _blockSize->getInt();
    if (blockSize == 0) {
	critical("A block size is required");
    } else if (blockSize & 0x1ff != 0 || blockSize > 8192) {
	critical("The block size has to be a multiple of\n"
		 "512 and less than or equal to 8192 so\n"
		 "it can be 512, 1024, 2048, 4096, or\n"
		 "8192.  The suggested best value to use\n"
		 "is 4096");
    }

    // Validate character set
    QString charSet = _charSet->text();
    if (charSet.isEmpty()) {
	critical("A character set is required");
    } else if (QTextCodec::codecForName(charSet) == NULL) {
	critical("The character set chosen is not\n"
		 "supported by Qt so you will need\n"
		 "to enter a different one");
    }

    // Save the current configuration to test with it
    FirebirdConfig config = getConfig();
    if (!config.save(true)) {
	critical("Saving the configuration failed so\n"
		 "testing connecting to the security\n"
		 "database cannot occur");
    } else {
	// Try to connect to security.fdb and validate username/password
	FirebirdDriver driver;
	if (!driver.initialize()) {
	    critical("The Firebird driver failed to\n"
		     "initialize itself.  If there have\n"
		     "been other errors reported\n"
		     "before this then they should\n"
		     "explain what may be wrong.");
	} else {
	    FirebirdConn conn(&driver);
	    if (!conn.dbaConnect(installDir + "/security.fdb")) {
		critical("Failed to connect to the security\n"
			 "database.  If you've received other\n"
			 "error messages then they should\n"
			 "tell you what needs to be fixed.\n"
			 "If you have not had any other errors\n"
			 "then your dba password may be wrong\n"
			 "or your character set is not supported\n"
			 "by Firebird");
	    } else {
		const QString salt = "9z";
		QString onceCrypted = crypt(password, salt) + 2;
		QString twiceCrypted = crypt(onceCrypted, salt) + 2;

		QString cmd = "select passwd from users where user_name = '" +
		    username.upper() + "'";
		FirebirdStmt stmt(&conn);
		stmt.setCommand(cmd);
		QString encrypted;
		bool userExists = false;
		if (stmt.execute()) {
		    if (stmt.next()) {
			encrypted = stmt.getString(1);
			userExists = true;
		    }
		}

		if (!userExists) {
		    critical("The non-dba username doesn't exist\n"
			     "in the security database so it will\n"
			     "need to be created");
		} else if (encrypted != twiceCrypted) {
		    critical("The non-dba password doesn't match\n"
			     "the one in the security database\n"
			     "which will cause the client to fail\n"
			     "to connect.  You will need to set\n"
			     "the one in the Firebird security\n"
			     "database to match");
		}
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
FirebirdConfigDialog::warning(const QString& message)
{
    QApplication::restoreOverrideCursor();

    qApp->beep();
    QMessageBox::warning(this, tr("Warning"), message);

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();
}

void
FirebirdConfigDialog::critical(const QString& message)
{
    QApplication::restoreOverrideCursor();

    qApp->beep();
    QMessageBox::critical(this, tr("Error"), message);
    _critical = true;

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();
}

FirebirdConfig
FirebirdConfigDialog::getConfig()
{
    FirebirdConfig config;
    config.hostname = _hostname->text();
    config.port = _port->getInt();
    config.library = _library->text();
    config.installDir = _installDir->text();
    config.dbaPassword = _dbaPassword->text();
    config.username = _username->text();
    config.password = _password->text();
    config.databaseDir = _databaseDir->text();
    config.blockSize = _blockSize->getInt();
    config.charSet = _charSet->text();
    return config;
}

void
FirebirdConfigDialog::accept()
{
    _critical = false;
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Validate library
    QString libraryFile = _library->text();
    if (libraryFile.isEmpty()) {
	critical("A library file is required for Quasar\n"
		 "to use the Firebird database");
    }

    // Validate installation directory
    QString installDir = _installDir->text();
    if (installDir.isEmpty()) {
	critical("A installation directory is required for\n"
		 "Quasar to use the Firebird database");
    } else if (QFile::exists(installDir) && !QFileInfo(installDir).isDir()) {
	critical("The installation directory is not set to\n"
		 "a directory");
    }

    // Validate username/password
    QString username = _username->text();
    QString password = _password->text();
    if (username.isEmpty()) {
	critical("A username for non-dba connections\n"
		 "is required");
    }

    // Validate database directory
    QString hostname = _hostname->text();
    QString databaseDir = parseDir(_databaseDir->text());
    if (!hostname.isEmpty() && hostname != "localhost") {
	// Can't check the databaseDir in this case since its not on
	// the local computer.
    } else if (databaseDir.isEmpty()) {
	critical("A database directory is required for\n"
		 "Quasar to use the Firebird database");
    } else if (QFile::exists(databaseDir) && !QFileInfo(databaseDir).isDir()) {
	critical("The database directory is not set to\n"
		 "a directory");
    }

    // Validate block size
    int blockSize = _blockSize->getInt();
    if (blockSize == 0) {
	critical("A block size is required");
    } else if (blockSize & 0x1ff != 0 || blockSize > 8192) {
	critical("The block size has to be a multiple of\n"
		 "512 and less than or equal to 8192 so\n"
		 "it can be 512, 1024, 2048, 4096, or\n"
		 "8192.  The suggested best value to use\n"
		 "is 4096");
    }

    // Validate character set
    QString charSet = _charSet->text();
    if (charSet.isEmpty()) {
	critical("A character set is required");
    } else if (QTextCodec::codecForName(charSet) == NULL) {
	critical("The character set chosen is not\n"
		 "supported by Qt so you will need\n"
		 "to enter a different one");
    }

    QApplication::restoreOverrideCursor();
    if (_critical) return;

    FirebirdConfig config = getConfig();
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
FirebirdConfigDialog::reject()
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
