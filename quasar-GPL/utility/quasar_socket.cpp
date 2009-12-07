// $Id: quasar_socket.cpp,v 1.15 2005/03/15 04:54:42 bpepers Exp $
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

#include "quasar_socket.h"
#include "tcl_object.h"

QuasarSocket::QuasarSocket(QObject* parent, const char* name)
    : QSocket(parent, name)
{
}

void
QuasarSocket::send(const QString& type)
{
    send(type, QStringList());
}

void
QuasarSocket::send(const QString& type, const QString& a1, const QString& a2)
{
    QStringList args;
    args << a1;
    args << a2;
    send(type, args);
}

void
QuasarSocket::send(const QString& type, const QStringList& args)
{
    if (type.isEmpty()) return;

    TclObject list;
    list.lappend(type);
    for (unsigned int i = 0; i < args.size(); ++i)
	list.lappend(args[i]);

    QString message = list.toString();
    if (message.contains('\n'))
	message = "\002" + message + "\003";
    else
	message += "\n";

    writeBlock(message.utf8(), strlen(message.utf8()));
    flush();
}

void
QuasarSocket::slotReadyRead()
{
    while (1) {
	char buffer[8192];
	int count = readBlock(buffer, 8191);
	if (count <= 0) break;

	buffer[count] = 0;
	_readLine += QString::fromUtf8(buffer);

	QString data;
	if (_readLine.left(1) == "\002") {
	    int index = _readLine.find('\003');
	    if (index == -1) continue;

	    data = _readLine.mid(1, index - 1);
	    _readLine = _readLine.mid(index + 1);
	} else {
	    int index = _readLine.find('\n');
	    if (index == -1) continue;

	    data = _readLine.left(index);
	    _readLine = _readLine.mid(index + 1);

	    while (data.right(1) == "\r")
		data = data.left(data.length() - 1);
	}

	TclObject list(data);
	if (list.llength() == 0) return;

	_type = list[0].toString();
	_args.clear();
	for (int i = 1; i < list.llength(); ++i)
	    _args << list[i].toString();
	emit received(_type, _args);
    }
}

QStringList
QuasarSocket::recv(int timeout)
{
    int timeoutCnt = 0;

    while (true) {
	char buffer[8192];
	int count = readBlock(buffer, 8191);
	if (count == -1) {
	    QStringList results;
	    results.push_back("error:");
	    results.push_back("Error reading from server");
	    return results;
	}
	if (count == 0) {
	    waitForMore(1000);
	    if (++timeoutCnt < timeout || timeout == 0) continue;

	    QStringList results;
	    results.push_back("error:");
	    results.push_back("Timeout reading from server");
	    return results;
	}

	buffer[count] = 0;
	_readLine += QString::fromUtf8(buffer);
	timeoutCnt = 0;

	QString data;
	if (_readLine.left(1) == "\002") {
	    int index = _readLine.find('\003');
	    if (index == -1) continue;

	    data = _readLine.mid(1, index - 1);
	    _readLine = _readLine.mid(index + 1);
	} else {
	    int index = _readLine.find('\n');
	    if (index == -1) continue;

	    data = _readLine.left(index);
	    _readLine = _readLine.mid(index + 1);

	    while (data.right(1) == "\r")
		data = data.left(data.length() - 1);
	}

	return TclObject(data).toStringList();
    }

    return QStringList();
}
