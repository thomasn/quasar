// $Id: client_conn.cpp,v 1.5 2005/04/04 06:11:25 bpepers Exp $
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

#include "client_conn.h"
#include "client_stmt.h"

ClientConn::ClientConn(QuasarSocket* socket)
    : _socket(socket)
{
}

ClientConn::~ClientConn()
{
}

bool
ClientConn::setAutoCommit(bool autoCommit)
{
    QStringList args;
    args << "auto_commit";
    args << (autoCommit ? "On" : "Off");
    QStringList results = sendCmd("set", args);
    if (results[0] != "set:")
	return error("set auto_commit failed: " + results[1]);
    _autoCommit = autoCommit;
    return true;
}

bool
ClientConn::setIsolationLevel(IsoLevel level)
{
    QStringList args;
    args << "isolation";
    switch (level) {
    case READ_UNCOMMITTED:	args << "read_uncommitted"; break;
    case READ_COMMITTED:	args << "read_committed"; break;
    case REPEATABLE_READ:	args << "repeatable_read"; break;
    case SERIALIZABLE:		args << "serializable"; break;
    }
    QStringList results = sendCmd("set", args);
    if (results[0] != "set:")
	return error("set isolation failed: " + results[1]);
    _isoLevel = level;
    return true;
}

bool
ClientConn::connect(const QString&)
{
    return false;
}

bool
ClientConn::dbaConnect(const QString&)
{
    return false;
}

bool
ClientConn::connect(const QString& filename, const QString& username,
		    const QString& password)
{
    QStringList args;
    args << filename;
    args << username;
    args << password;

    QStringList results = sendCmd("login", args);
    if (results[0] != "login:")
	return false;

    return true;
}

void
ClientConn::disconnect()
{
}

bool
ClientConn::connected() const
{
    return _socket->state() == QSocket::Connected;
}

bool
ClientConn::commit()
{
    QStringList results = sendCmd("commit", QStringList());
    if (results[0] != "commit:")
	return error("commit failed: " + results[1]);
    return true;
}

bool
ClientConn::rollback()
{
    QStringList results = sendCmd("rollback", QStringList());
    if (results[0] != "rollback:")
	return error("rollback failed: " + results[1]);
    return true;
}

bool
ClientConn::execute(const QString& command)
{
    QStringList results = sendCmd("exec_cmd", command);
    if (results[0] != "exec_cmd:")
	return error("exec_cmd failed: " + results[1]);
    return true;
}

Statement*
ClientConn::allocStatement()
{
    return new ClientStmt(this);
}

QStringList
ClientConn::sendCmd(const QString& cmd, const QStringList& args)
{
    _lastError = "";
    _socket->send(cmd, args);
    QStringList results = _socket->recv(0);
    if (results[0] == "error:")
	_lastError = results[1];
    return results;
}
