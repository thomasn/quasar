// $Id: smtp.cpp,v 1.5 2004/02/03 00:13:26 arandell Exp $
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

#include "smtp.h"

#include <qdatetime.h>
#include <qtextstream.h>
#include <qsocket.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <time.h>

Smtp::Smtp(const QString& smtpHost, int port)
{
    _host = smtpHost;
    _port = port;
    _state = Init;
    _socket = NULL;
    _stream = NULL;
    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), SLOT(timeout()));
}

Smtp::~Smtp()
{
    delete _stream;
    delete _socket;
}

bool
Smtp::sendMail()
{
    // Check for required fields
    if (_host.isEmpty()) return false;
    if (_from.isEmpty()) return false;
    if (_to.count() == 0) return false;
    if (_subject.isEmpty()) return false;
    if (_body.isEmpty()) return false;

    // Build a RFC 822 compliant date
    time_t now;
    time(&now);
    struct tm* tm = localtime(&now);
    char date[1024];
    strftime(date, 1024, "%a, %d %b %Y %H:%M:%S %z", tm);

    // Build message taking care of leading periods on lines
    _message = QString::fromLatin1("From: ") + _from +
	       QString::fromLatin1("\nTo: ") + _to.join(",") +
	       QString::fromLatin1("\nSubject: ") + _subject +
	       QString::fromLatin1("\nDate: ") + QString(date) +
	       QString::fromLatin1("\n\n") + _body + "\n";
    _message.replace(QRegExp(QString::fromLatin1("\n")),
		     QString::fromLatin1("\r\n"));
    _message.replace(QRegExp(QString::fromLatin1("\r\n\\.")),
		     QString::fromLatin1("\r\n.."));

    // Connect to mail server
    emit status(tr("Connecting to %1").arg(_host));
    _socket = new QSocket(this);
    _socket->connectToHost(_host, _port);
    connect(_socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(_socket, SIGNAL(connected()), this, SLOT(connected()));
    _stream = new QTextStream(_socket);
    _toPos = 0;
    _state = Init;
    setTimeout(30);

    return true;
}

void
Smtp::cancelSend()
{
    done(false);
}

void
Smtp::connected()
{
    emit status(tr("Connected to %1").arg(_socket->peerName()));
}

void
Smtp::timeout()
{
    emit status(tr("Timeout"));
    done(false);
}

void
Smtp::readyRead()
{
    // SMTP is line-oriented
    if (_socket == NULL || !_socket->canReadLine())
	return;

    // Get results and truncate to result code
    QString response;
    QString responseLine;
    do {
	responseLine = _socket->readLine();
	response += responseLine;
    } while (_socket->canReadLine() && responseLine[3] != ' ');
    responseLine.truncate(3);

    // Very simple state machine
    setTimeout(30);
    if (_state == Init && responseLine[0] == '2') {
	*_stream << "HELO there\r\n";
	_state = Mail;
    } else if (_state == Mail && responseLine[0] == '2') {
	*_stream << "MAIL FROM: <" << _from << ">\r\n";
	_state = Rcpt;
    } else if (_state == Rcpt && responseLine[0] == '2') {
	*_stream << "RCPT TO: <" << _to[_toPos++] << ">\r\n";
	if (_toPos >= _to.count())
	    _state = Data;
    } else if (_state == Data && responseLine[0] == '2') {
	*_stream << "DATA\r\n";
	_state = Body;
    } else if (_state == Body && responseLine[0] == '3') {
	*_stream << _message << ".\r\n";
	_state = Quit;
    } else if (_state == Quit && responseLine[0] == '2') {
	*_stream << "QUIT\r\n";
	_state = Close;
    } else if (_state == Close) {
	emit status(tr("Message sent to %1").arg(_to[0]));
	done(true);
    } else {
	// Show error and finish
	QMessageBox::warning(qApp->activeWindow(), tr("Mail Error"),
			     tr("Unexpected reply from SMTP server:\n\n") +
			     response );
	done(false);
    }
}

void
Smtp::done(bool result)
{
    delete _stream;
    delete _socket;
    _stream = NULL;
    _socket = NULL;
    _timer->stop();
    emit finished(result);
}

void
Smtp::setTimeout(int seconds)
{
    _timer->stop();
    _timer->start(seconds * 1000, true);
}
