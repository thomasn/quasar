// $Id: client_stmt.h,v 1.3 2005/03/01 19:59:42 bpepers Exp $
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

#ifndef CLIENT_STMT_H
#define CLIENT_STMT_H

#include "client_conn.h"
#include "tcl_object.h"
#include <qvaluevector.h>

class ClientStmt: public Statement {
public:
    ClientStmt(ClientConn* conn);
    ~ClientStmt();

    // Command
    bool setCommand(const QString& command);

    // Parameters for current command
    int paramCount();

    // Set parameter value
    void setNull(int param);
    void setValue(int param, Variant value);
    void setString(int param, const QString& value);
    void setDouble(int param, double value);
    void setLong(int param, long value);
    void setDate(int param, QDate value);
    void setTime(int param, QTime value);

    // Execute methods
    bool execute();
    bool next();
    bool cancel();

    // Number of rows effected by insert, update, or delete
    int getUpdateCount();

    // Columns in result set
    int columnCount();
    int findColumn(const QString& name);
    QString columnName(int column);

    // Values from current row
    bool isNull(int column);
    Variant getValue(int column);
    QString getString(int column);
    double getDouble(int column);
    long getLong(int column);
    QDate getDate(int column);
    QTime getTime(int column);

private:
    ClientConn* _conn;
    int _id;
    int _paramCount;
    QValueVector<Variant> _params;
    int _nextParam;
    int _nextColumn;
    int _updateCount;
    int _columnCount;
    int _currentRow;
    QStringList _data;
    TclObject _values;
    TclObject _nulls;
};

#endif // CLIENT_STMT_H
