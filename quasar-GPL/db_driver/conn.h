// $Id: conn.h,v 1.1 2005/03/01 19:45:35 bpepers Exp $
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

#ifndef CONN_H
#define CONN_H

#include "db_driver.h"

// Wrapper for Connection*
class Conn {
public:
    Conn(Driver* driver);
    Conn(const Conn& rhs);
    ~Conn();

    // Assignment
    Conn& operator=(const Conn& rhs);

    // Type conversions
    operator Connection*() { return _conn; }

    // Auto commit (set before or after connected)
    bool setAutoCommit(bool autoCommit);
    bool getAutoCommit() const;

    // Isolation level (set before or after connected)
    bool setIsolationLevel(IsoLevel level);
    IsoLevel getIsolationLevel() const;

    // Connect to a database
    bool connect(const QString& database);
    bool dbaConnect(const QString& database);
    void disconnect();
    bool connected() const;

    // Transaction handling
    bool commit();
    bool rollback();

    // Create a table, constraint, or index
    bool create(const TableDefn& table);
    bool create(const ConstraintDefn& constraint);
    bool create(const IndexDefn& index);

    // Create tables, constraints, and indexes for list of tables
    bool create(const QValueList<TableDefn>& tables);

    // Last error
    QString lastError() const;

protected:
    Connection* _conn;
};

#endif // CONN_H
