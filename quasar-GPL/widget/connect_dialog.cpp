// $Id: connect_dialog.cpp,v 1.5 2004/02/03 00:56:02 arandell Exp $
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

#include "connect_dialog.h"

#include <qapplication.h>
#include <qsocket.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>

ConnectDialog::ConnectDialog(QSocket* socket, const QString& hostname,
			     int port, QWidget* parent, const char* name)
    : QDialog(parent, name, true), _socket(socket)
{
    QLabel* statusLabel = new QLabel("Status:", this);
    _status = new QLineEdit(this);
    _status->setFocusPolicy(NoFocus);
    _status->setMinimumWidth(_status->fontMetrics().width("x") * 30);
    _status->setText("Finding server...");

    QPushButton* cancel = new QPushButton("Cancel", this);
    connect(cancel, SIGNAL(clicked()), SLOT(reject()));

    QGridLayout* grid = new QGridLayout(this);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setColStretch(1, 1);
    grid->addWidget(statusLabel, 0, 0);
    grid->addWidget(_status, 0, 1);
    grid->addMultiCellWidget(cancel, 1, 1, 0, 1, AlignRight | AlignVCenter);

    connect(socket, SIGNAL(hostFound()), SLOT(slotHostFound()));
    connect(socket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(socket, SIGNAL(error(int)), SLOT(slotError(int)));
    socket->connectToHost(hostname, port);

    setCaption("Server Connection");
}

ConnectDialog::~ConnectDialog()
{
}

void
ConnectDialog::slotHostFound()
{
    _status->setText("Connecting to server...");
}

void
ConnectDialog::slotConnected()
{
    _status->setText("Connected");
    qApp->processEvents();
    accept();
}

void
ConnectDialog::slotError(int error)
{
    QString message;
    switch (error) {
    case QSocket::ErrConnectionRefused:
	message = "Connection refused";
	break;
    case QSocket::ErrHostNotFound:
	message = "Host name not found";
	break;
    case QSocket::ErrSocketRead:
	message = "Read failed";
	break;
    default:
	message = "Connection error: " + QString::number(error);
    }

    qApp->beep();
    _status->setText(message);

    QPalette palette = _status->palette();
    palette.setColor(QPalette::Active, QColorGroup::Text, QColor(255, 10, 10));
    _status->setPalette(palette);
}
