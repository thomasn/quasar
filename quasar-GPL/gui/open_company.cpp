// $Id: open_company.cpp,v 1.56 2005/03/22 17:55:14 bpepers Exp $
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
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_version.h"
#include "user_config.h"
#include "server_config.h"
#include "list_view_item.h"
#include "line_edit.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <qsocket.h>
#include <qdir.h>
#include <qprogressdialog.h>

OpenCompany::OpenCompany(MainWindow* main)
    : QMainWindow(NULL, "OpenCompany", WDestructiveClose),
      _main(main), _status(NULL)
{
    setCaption(tr("Open Company"));

    _quasar = new QuasarClient();
    _timer = new QTimer(this);

    QFrame* frame = new QFrame(this);

    _top = new QFrame(frame);
    QLabel* serverLabel = new QLabel(tr("&Server:"), _top);
    _server = new LineEdit(_top);
    _server->setMinCharWidth(30, 'x');
    serverLabel->setBuddy(_server);

    QGridLayout* topGrid = new QGridLayout(_top);
    topGrid->setSpacing(3);
    topGrid->setMargin(3);
    topGrid->setColStretch(1, 1);
    topGrid->addWidget(serverLabel, 0, 0);
    topGrid->addWidget(_server, 0, 1);

    _companies = new ListView(frame);
    _companies->addTextColumn(tr("Company Name"), 60);
    _companies->setAllColumnsShowFocus(true);
    _companies->setShowSortIndicator(true);
    _companies->setFocusPolicy(ClickFocus);

    _mid = new QFrame(frame);
    QLabel* usernameLabel = new QLabel(tr("&User Name:"), _mid);
    _username = new LineEdit(_mid);
    _username->setLength(16);
    usernameLabel->setBuddy(_username);

    QLabel* passwordLabel = new QLabel(tr("&Password:"), _mid);
    _password = new LineEdit(_mid);
    _password->setEchoMode(QLineEdit::Password);
    _password->setLength(16);
    passwordLabel->setBuddy(_password);

    QGridLayout* midGrid = new QGridLayout(_mid);
    midGrid->setSpacing(3);
    midGrid->setMargin(3);
    midGrid->setColStretch(1, 1);
    midGrid->addWidget(usernameLabel, 0, 0);
    midGrid->addWidget(_username, 0, 1, AlignLeft | AlignVCenter);
    midGrid->addWidget(passwordLabel, 1, 0);
    midGrid->addWidget(_password, 1, 1, AlignLeft | AlignVCenter);

    QFrame* buttons = new QFrame(frame);
    _login = new QPushButton(tr("&Login"), buttons);
    _cancel = new QPushButton(tr("&Cancel"), buttons);

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(3);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(_login, 0, 1);
    buttonGrid->addWidget(_cancel, 0, 2);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(1, 1);
    grid->setColStretch(0, 1);
    grid->addWidget(_top, 0, 0);
    grid->addWidget(_companies, 1, 0);
    grid->addWidget(_mid, 2, 0);
    grid->addWidget(buttons, 3, 0);

    setCentralWidget(frame);

    connect(_server, SIGNAL(validData()), SLOT(slotServerChanged()));
    connect(_login, SIGNAL(clicked()), SLOT(slotLogin()));
    connect(_cancel, SIGNAL(clicked()), SLOT(close()));

    // Server connecting
    connect(_quasar, SIGNAL(hostFound()), SLOT(slotHostFound()));
    connect(_quasar, SIGNAL(connected()), SLOT(slotConnected()));
    connect(_quasar, SIGNAL(connectionError(int)),
	    SLOT(slotConnectionError(int)));

    connect(_quasar, SIGNAL(disconnected()), SLOT(slotDisconnected()));
    connect(_timer, SIGNAL(timeout()), SLOT(slotTimeout()));

    _username->installEventFilter(this);
    _password->installEventFilter(this);

    setDisconnected();
}

OpenCompany::~OpenCompany()
{
    delete _quasar;
}

void
OpenCompany::setInfo(const QString& server, const QString& company,
		     const QString& username, const QString& password)
{
    _server->setText(server);
    _defaultCompany = company;
    _username->setText(username);
    _password->setText(password);
    QTimer::singleShot(100, this, SLOT(slotServerChanged()));
}

QuasarClient*
OpenCompany::getQuasarClient()
{
    QuasarClient* client = _quasar;
    _quasar = NULL;
    return client;
}

bool
OpenCompany::eventFilter(QObject* o, QEvent* e)
{
    if ((o == _username || o == _password) && e->type() == QEvent::KeyPress) {
	QKeyEvent* ke = (QKeyEvent*)e;

	switch (ke->key()) {
	case Key_Up:	qApp->sendEvent(_companies, e); break;
	case Key_Down:	qApp->sendEvent(_companies, e); break;
	case Key_Next:	qApp->sendEvent(_companies, e); break;
	case Key_Prior:	qApp->sendEvent(_companies, e); break;
	case Key_Home:	qApp->sendEvent(_companies, e); return true;
	case Key_End:	qApp->sendEvent(_companies, e); return true;
	case Key_Enter:  slotLogin(); break;
	case Key_Return: slotLogin(); break;
	case Key_Escape: close(); break;
	}
    }

    return QMainWindow::eventFilter(o, e);
}

void
OpenCompany::setStatus(const QString& message)
{
    if (_status == NULL) {
	_status = new QDialog(this, "status", true);
	_status->setCaption("Status");

	_statusText = new QLabel(message, _status);
	_statusText->setMinimumWidth(fontMetrics().width("_") * 30);

	QPushButton* abort = new QPushButton(tr("Abort"), _status);
	connect(abort, SIGNAL(clicked()), SLOT(slotAbort()));

	QGridLayout* grid = new QGridLayout(_status);
	grid->setSpacing(6);
	grid->setMargin(6);
	grid->setColStretch(0, 1);
	grid->addMultiCellWidget(_statusText, 0, 0, 0, 1);
	grid->addWidget(abort, 1, 1);

	_status->show();
    } else {
	_statusText->setText(message);
    }
    qApp->processEvents();
}

void
OpenCompany::clearStatus()
{
    delete _status;
    _status = NULL;
    _statusText = NULL;
}

void
OpenCompany::slotServerChanged()
{
    QString server = _server->text();
    if (_quasar->serverConnected() && server == _quasar->hostname())
	return;

    _quasar->serverDisconnect();
    setDisconnected();
    if (server.isEmpty())
	return;

    int port = DEFAULT_QUASARD_PORT;
    int pos = server.find(':');
    if (pos != -1) {
	port = server.mid(pos + 1).toInt();
	server = server.left(pos);
    }

    setStatus("Connecting...");
    _quasar->serverConnect(server, port);
}

void
OpenCompany::slotHostFound()
{
    setStatus("Connecting to server...");
}

void
OpenCompany::slotConnected()
{
    setStatus("Fetching companies...");

    QStringList fileNames;
    QStringList names;
    QStringList versions;
    if (!_quasar->resourceList("companies", "name", "version", fileNames,
			       names, versions)) {
	qWarning("Failed getting list of companies");
    }

    for (unsigned int i = 0; i < fileNames.size(); ++i) {
	if (versions[i] != QUASAR_DB_VERSION) continue;

	ListViewItem* item = new ListViewItem(_companies);
	item->extra.push_back(fileNames[i]);
	item->setText(0, names[i]);

	if (fileNames[i] == _defaultCompany) {
	    _companies->setCurrentItem(item);
	    _companies->setSelected(item, true);
	}
    }

    setConnected();

    if (_companies->selectedItem() == NULL) {
	ListViewItem* item = _companies->firstChild();
	_companies->setCurrentItem(item);
	_companies->setSelected(item, true);
    } else {
	QTimer::singleShot(50, this, SLOT(slotEnsureVisible()));
    }

    if (_username->text().isEmpty()) {
	_username->setFocus();
    } else {
	_password->setFocus();
	_password->selectAll();
    }
}

void
OpenCompany::slotConnectionError(int errorNum)
{
    setDisconnected();

    _server->setFocus();
    _server->selectAll();
    _server->markChanged();

    QString message;
    switch (errorNum) {
    case QSocket::ErrConnectionRefused:
	message = tr("Connection refused.  This usually means\n"
		     "that the Quasar server program is not\n"
		     "running on the server name you've entered.");
	break;
    case QSocket::ErrHostNotFound:
	message = tr("Server not found.  This usually means\n"
		     "that you've mistyped the server name\n"
		     "or else that the server is currently\n"
		     "turned off.");
	break;
    case QSocket::ErrSocketRead:
	message = tr("Error reading from server.  This shows\n"
		     "some type of problem in your network\n"
		     "which may be temporary.  Please try again.");
	break;
    default:
	message = tr("Unknown connection error.  This error\n"
		     "should never occur.  If it does please\n"
		     "contact the developer!");
    }

    qApp->beep();
    QMessageBox::critical(this, tr("Error"), message);
}

void
OpenCompany::slotEnsureVisible()
{
    _companies->ensureItemVisible(_companies->selectedItem());
}

void
OpenCompany::slotLogin()
{
    ListViewItem* item = _companies->currentItem();
    if (item == NULL) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), tr("No company chosen"));
	return;
    }

    if (_username->text().isEmpty()) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), tr("Blank user name"));
	_username->setFocus();
	return;
    }

    setStatus("Login to company...");

    // Open company
    QString fileName = item->extra[0].toString();
    QString username = _username->text();
    QString password = _password->text();
    if (!_quasar->openCompany(fileName, username, password)) {
	clearStatus();
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), _quasar->lastError());
	return;
    }

    setStatus("Read screen definition...");

    QString screenFile = _quasar->user().screen();
    if (screenFile.isEmpty()) screenFile = "default.xml";

    QString filePath;
    if (!_quasar->resourceFetch("screens", screenFile, filePath)) {
	clearStatus();
	qApp->beep();
	QString message = tr("Error fetching screen '%1'").arg(screenFile);
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    ScreenDefn screen;
    if (!screen.load(filePath)) {
	clearStatus();
	qApp->beep();
	QString message = tr("Screen '%1' is invalid").arg(screenFile);
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    // Save default login information
    UserConfig config;
    if (config.load()) {
	config.defaultServer = _server->text();
	config.defaultCompany = _companies->currentItem()->extra[0].toString();
	config.defaultUserName = _username->text();
	config.save(true);
    }

    // Change caption in case new one matches
    if (_main != NULL)
	_main->setCaption("Old");

    // Create new main window and delete old
    MainWindow* main = new MainWindow(_quasar, screen);
    main->show();
    qApp->setMainWidget(main);
    delete _main;

    // Set to NULL since the QuasarClient* has been passed on to the
    // ownership of MainWindow and we don't want the destructor to
    // delete it.
    _quasar = NULL;

    close();
}

void
OpenCompany::slotDisconnected()
{
    setDisconnected();

    _server->setFocus();
    _server->selectAll();
    _server->markChanged();

    QString message = tr("The server connection has been closed.\n"
			 "This may be caused by a network failure\n"
			 "or the shutdown of the server computer.");
    qApp->beep();
    QMessageBox::critical(this, tr("Error"), message);
}

void
OpenCompany::slotAbort()
{
    if (_quasar->serverConnected()) {
	// Restore from aborted login
	setConnected();
	_password->setFocus();
	qApp->beep();
    } else {
	// Restore from aborted connection
	setDisconnected();
	_server->setFocus();
	_server->selectAll();
	_server->markChanged();
	_quasar->serverDisconnect();
	qApp->beep();
    }
}

void
OpenCompany::slotTimeout()
{
    slotAbort();

    QString message = tr("The current operation has timed out.\n"
			 "This could be because of problems on\n"
			 "the network or with the server computer.\n");
    QMessageBox::critical(this, tr("Error"), message);
}

void
OpenCompany::setDisconnected()
{
    _timer->stop();
    _top->setEnabled(true);
    _mid->setEnabled(false);
    _login->setEnabled(false);
    _companies->clear();
    clearStatus();
}

void
OpenCompany::setProcessing(const QString& message)
{
    _timer->start(60000, true);
    _top->setEnabled(false);
    _mid->setEnabled(false);
    setStatus(message);
}

void
OpenCompany::setConnected()
{
    _timer->stop();
    _top->setEnabled(true);
    _mid->setEnabled(true);
    _login->setEnabled(true);
    clearStatus();
}
