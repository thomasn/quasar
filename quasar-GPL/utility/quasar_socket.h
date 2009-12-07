// $Id: quasar_socket.h,v 1.9 2005/02/28 10:31:59 bpepers Exp $
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

#ifndef QUASAR_SOCKET_H
#define QUASAR_SOCKET_H

#include <qsocket.h>
#include <qstringlist.h>

// The QuasarSocket class implements the protocol between a client and
// a server.  This class can be used by the client or the server since
// the protocol is symetrical.  Requests are sent using the send() methods
// which are converted to a single line and sent through the socket.  On
// the receiving end the line is split back up and the received() signal
// is emitted.  The received signal should always be emitted with the
// same command and args as were passed into the send method.  The method
// used to do this is to convert the QStringList into a Tcl list and then
// into a line to be sent over the socket.  At the other end the line
// received is converted to a Tcl list and then to a QStringList.  There
// should be some special handling of control codes like newlines in the
// args but they are ignored for now.

class QuasarSocket: public QSocket {
    Q_OBJECT
public:
    // Constructors
    QuasarSocket(QObject* parent=NULL, const char* name=NULL);

    // Send request or results (NOTE: a version with a single arg is not
    // required since the QStringList one will be called if you pass a
    // single string and if you define a send method that takes one arg,
    // you get compile errors when you try to use it)
    void send(const QString& type);
    void send(const QString& type, const QString& arg1, const QString& arg2);
    void send(const QString& type, const QStringList& args);

    // Receive results from last command.  This will timeout after
    // a number of seconds.
    QStringList recv(int timeout=30);

public slots:
    void slotReadyRead();

signals:
    void received(QString type, QStringList args);

protected:
    QString _type;
    QStringList _args;
    QString _readLine;
};

#endif // QUASAR_SOCKET_H
