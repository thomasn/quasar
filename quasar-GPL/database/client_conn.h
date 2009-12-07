// $Id: client_conn.h,v 1.3 2005/04/04 06:11:25 bpepers Exp $
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

#ifndef CLIENT_CONN_H
#define CLIENT_CONN_H

#include "db_driver.h"
#include "quasar_socket.h"

class ClientConn: public Connection {
public:
    ClientConn(QuasarSocket* socket);
    ~ClientConn();

    // Auto commit (set before or after connected)
    bool setAutoCommit(bool autoCommit);

    // Isolation level (set before or after connected)
    bool setIsolationLevel(IsoLevel level);

    // Real connection method for clients
    bool connect(const QString& filename, const QString& username,
		 const QString& password);

    // Connect to a database
    bool connect(const QString& database);
    bool dbaConnect(const QString& database);
    void disconnect();
    bool connected() const;

    // Transaction handling
    bool commit();
    bool rollback();

    // Execute a command immediately
    bool execute(const QString& command);

    // Allocate a statement
    Statement* allocStatement();

    // Create tables, constraints, and indexes
    bool create(const TableDefn&) { return false; }
    bool create(const ConstraintDefn&) { return false; }
    bool create(const IndexDefn&) { return false; }

protected:
    QStringList sendCmd(const QString& cmd, const QStringList& args);

    QuasarSocket* _socket;
    friend class ClientStmt;
};

#endif // CLIENT_CONN_H
