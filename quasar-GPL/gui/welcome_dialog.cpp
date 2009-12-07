// $Id: welcome_dialog.cpp,v 1.7 2005/04/06 06:44:25 bpepers Exp $
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

#include "welcome_dialog.h"
#include "quasar_client.h"
#include "server_config.h"
#include "combo_box.h"
#include "line_edit.h"
#include "images.h"
#include "icu_util.h"

#include <qvbox.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qtextcodec.h>
#include <qtimer.h>
#include <qsocket.h>

static Locale systemLocale;

WelcomeDialog::WelcomeDialog()
    : QDialog(NULL, "WelcomeDialog", true)
{
    setCaption(tr("Welcome To Quasar"));

    QLabel* image = new QLabel(this);
    image->setPixmap(login_image);

    QFrame* right = new QFrame(this);

    _localeBox = new QGroupBox(right);
    QGridLayout* localeGrid = new QGridLayout(_localeBox, 2, 2,
					      _localeBox->frameWidth() * 2);
    localeGrid->addRowSpacing(0, _localeBox->fontMetrics().height());
    localeGrid->setMargin(6);
    localeGrid->setSpacing(3);
    localeGrid->setColStretch(1, 1);

    _localeMsg = new QLabel(_localeBox);
    _localeLabel = new QLabel(_localeBox);
    _locale = new ComboBox(_localeBox);
    _localeLabel->setBuddy(_locale);
    _locale->setMaximumWidth(320);

    localeGrid->addMultiCellWidget(_localeMsg, 1, 1, 0, 1);
    localeGrid->addWidget(_localeLabel, 2, 0);
    localeGrid->addWidget(_locale, 2, 1, AlignLeft | AlignVCenter);

    _serverBox = new QGroupBox(right);
    QGridLayout* serverGrid = new QGridLayout(_serverBox, 2, 2,
					      _serverBox->frameWidth() * 2);
    serverGrid->addRowSpacing(0, _serverBox->fontMetrics().height());
    serverGrid->setMargin(6);
    serverGrid->setSpacing(3);
    serverGrid->setColStretch(1, 1);

    _serverMsg = new QLabel(_serverBox);
    _serverLabel = new QLabel(_serverBox);
    _server = new LineEdit(_serverBox);
    _serverLabel->setBuddy(_server);

    serverGrid->addMultiCellWidget(_serverMsg, 1, 1, 0, 1);
    serverGrid->addWidget(_serverLabel, 2, 0);
    serverGrid->addWidget(_server, 2, 1, AlignLeft | AlignVCenter);

    QGridLayout* rightGrid = new QGridLayout(right);
    rightGrid->setSpacing(3);
    rightGrid->setMargin(3);
    rightGrid->setRowStretch(1, 1);
    rightGrid->setRowStretch(3, 1);
    rightGrid->setColStretch(0, 1);
    rightGrid->addWidget(_localeBox, 0, 0);
    rightGrid->addWidget(_serverBox, 2, 0);

    QFrame* buttons = new QFrame(this);
    _status = new QLabel(buttons);
    _ok = new QPushButton(tr("OK"), buttons);
    _cancel = new QPushButton(tr("Cancel"), buttons);

    _ok->setDefault(true);
    _ok->setMinimumSize(_cancel->sizeHint());
    _cancel->setMinimumSize(_cancel->sizeHint());
    connect(_ok, SIGNAL(clicked()), SLOT(accept()));
    connect(_cancel, SIGNAL(clicked()), SLOT(reject()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(_status, 0, 0);
    buttonGrid->addWidget(_ok, 0, 1);
    buttonGrid->addWidget(_cancel, 0, 2);

    _locales = Locale::getAvailableLocales(_localesCnt);
    slotLocaleChanged();

    QGridLayout* grid = new QGridLayout(this);
    grid->setSpacing(3);
    grid->setMargin(6);
    grid->setRowStretch(0, 1);
    grid->setColStretch(1, 1);
    grid->addWidget(image, 0, 0);
    grid->addWidget(right, 0, 1);
    grid->addMultiCellWidget(buttons, 1, 1, 0, 1);

    _quasar = new QuasarClient(this);
    _timer = new QTimer(this);

    connect(_locale, SIGNAL(activated(int)), SLOT(slotLocaleChanged()));
    connect(_quasar, SIGNAL(hostFound()), SLOT(slotHostFound()));
    connect(_quasar, SIGNAL(connected()), SLOT(slotConnected()));
    connect(_quasar, SIGNAL(connectionError(int)),
	    SLOT(slotConnectionError(int)));
    connect(_timer, SIGNAL(timeout()), SLOT(slotTimeout()));

    _server->setText("localhost");
    _locale->setFocus();
}

WelcomeDialog::~WelcomeDialog()
{
}

QString
WelcomeDialog::locale() const
{
    // Return the locale they have picked unless its the system one
    // in which case just return "" so that Quasar will continue to
    // use the system locale.
    QString name = _locales[_locale->currentItem()].getName();
    if (name == systemLocale.getName()) return "";
    return name;
}

QString
WelcomeDialog::serverName() const
{
    return _server->text();
}

void
WelcomeDialog::slotLocaleChanged()
{
    bool firstTime = (_locale->count() == 0);
    int index = _locale->currentItem();
    QString systemLocaleName = systemLocale.getName();

    // Set default locale for ICU
    if (!firstTime) {
	Locale locale = _locales[index];
	UErrorCode status = U_ZERO_ERROR;
	Locale::setDefault(locale, status);
	if (U_FAILURE(status)) {
	    QString msg = tr("Failed setting locale to %1")
		.arg(locale.getName());
	    QMessageBox::critical(this, tr("Error"), msg);
	}
    }

    _locale->clear();
    for (int32_t i = 0; i < _localesCnt; ++i) {
	UnicodeString uname;
	_locales[i].getDisplayName(uname);

	QString name = convertToQt(uname);
	_locale->insertItem(name);

	if (firstTime && _locales[i].getName() == systemLocaleName) {
	    firstTime = false;
	    index = _locale->count() - 1;
	}
    }

    QString localeMsg = tr("Select the locale you are using.  This will "
			   "control the language\nused for messages "
			   "and the format of dates, money, ...\n\n");

    QString serverMsg = tr("Select the server to connect to.  The "
			   "default is \"localhost\"\nwhich will connect "
			   "to a server on your own computer.\n\n");

    _locale->setCurrentItem(index);
    _localeBox->setTitle(tr("Locale"));
    _localeMsg->setText(localeMsg);
    _localeLabel->setText(tr("Locale:"));
    _serverBox->setTitle(tr("Server"));
    _serverMsg->setText(serverMsg);
    _serverLabel->setText(tr("Server:"));
    _ok->setText(tr("OK"));
    _cancel->setText(tr("Cancel"));
}

void
WelcomeDialog::accept()
{
    setEnabled(false);
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    QString server = serverName();
    int port = DEFAULT_QUASARD_PORT;

    int pos = server.find(':');
    if (pos != -1) {
	port = server.mid(pos + 1).toInt();
	server = server.left(pos);
    }

    _timer->start(60000, true);
    _quasar->serverConnect(server, port);
    _status->setText("Connecting...");
}

void
WelcomeDialog::slotHostFound()
{
    _status->setText("Connecting to server...");
}

void
WelcomeDialog::slotConnected()
{
    QApplication::restoreOverrideCursor();
    QDialog::accept();
}

void
WelcomeDialog::slotConnectionError(int errorNum)
{
    _timer->stop();
    _status->clear();
    setEnabled(true);

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
    QApplication::restoreOverrideCursor();
    QMessageBox::critical(this, tr("Error"), message);

    _server->setFocus();
    _server->selectAll();
}

void
WelcomeDialog::slotTimeout()
{
    _timer->stop();
    _status->clear();
    setEnabled(true);

    QString message = tr("The current operation has timed out.\n"
			 "This could be because of problems on\n"
			 "the network or with the server computer.\n");

    qApp->beep();
    QApplication::restoreOverrideCursor();
    QMessageBox::critical(this, tr("Error"), message);

    _server->setFocus();
    _server->selectAll();
}
