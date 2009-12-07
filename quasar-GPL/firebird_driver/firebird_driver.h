// $Id: firebird_driver.h,v 1.14 2005/04/04 06:11:25 bpepers Exp $
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

#ifndef FIREBIRD_DRIVER_H
#define FIREBIRD_DRIVER_H

#include "db_driver.h"
#include <qvaluevector.h>
#include <ibase.h>

class FirebirdProcs;
class FirebirdConn;
class FirebirdStmt;
class FirebirdParam;
class FirebirdColumn;
class QLibrary;
class QTextCodec;

#define DEFAULT_PARAM_COUNT 8
#define DEFAULT_COLUMN_COUNT 16

class FirebirdDriver: public Driver {
public:
    FirebirdDriver();
    ~FirebirdDriver();

    // Initialize the driver.  If this fails you shouldn't use the
    // driver except to get the configuration dialog and name.
    bool initialize();

    // Driver type name
    QString name() const { return "Firebird"; }

    // Create and delete databases
    bool create(CompanyDefn& company);
    bool remove(CompanyDefn& company);

    // Create a driver specific connection
    Connection* allocConnection();

    // Configuration dialog
    QDialog* configureDialog(QWidget* parent);

protected:
    QString _libFile;
    QLibrary* _library;
    FirebirdProcs* _procs;

    friend class FirebirdConn;
};

class FirebirdConn: public Connection {
public:
    FirebirdConn(FirebirdDriver* driver);
    ~FirebirdConn();

    // Auto commit (set before or after connected)
    bool setAutoCommit(bool autoCommit);

    // Isolation level (set before or after connected)
    bool setIsolationLevel(IsoLevel level);

    // Connect to a database
    bool connect(const QString& db, const QString& user, const QString& pwd);
    bool connect(const QString& database);
    bool dbaConnect(const QString& database);
    void disconnect();
    bool connected() const;

    // Transaction handling
    bool commit();
    bool rollback();

    // Execute a command immediately
    bool execute(const QString& command);

    // Create a driver specific statement
    Statement* allocStatement();

    // Create tables, constraints, and indexes
    bool create(const TableDefn& table);
    bool create(const ConstraintDefn& constraint);
    bool create(const IndexDefn& index);

protected:
    bool startTransaction();
    bool dropDatabase();

    FirebirdDriver* _driver;
    FirebirdProcs* _procs;
    QTextCodec* _codec;
    void* _db;
    void* _tr;

    friend class FirebirdDriver;
    friend class FirebirdStmt;
    friend class FirebirdParam;
    friend class FirebirdColumn;
};

class FirebirdStmt: public Statement {
public:
    FirebirdStmt(FirebirdConn* conn);
    ~FirebirdStmt();

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

protected:
    // Error handling
    void clear();

    FirebirdConn* _conn;
    FirebirdProcs* _procs;
    void* _stmt;
    XSQLDA* _inputDA;
    XSQLDA* _outputDA;
    char _defaultParams[XSQLDA_LENGTH(DEFAULT_PARAM_COUNT)];
    char _defaultColumns[XSQLDA_LENGTH(DEFAULT_COLUMN_COUNT)];
    QValueVector<FirebirdParam*> _params;
    QValueVector<FirebirdColumn*> _columns;
    ISC_STATUS _result;

    friend class FirebirdParam;
    friend class FirebirdColumn;
};

class FirebirdParam {
public:
    FirebirdParam(FirebirdStmt* stmt, int number);
    ~FirebirdParam();

    int number() const		{ return _number; }

    void setNull();
    void setString(const QString& value);
    void setDouble(double value);
    void setLong(long value);
    void setDate(QDate value);
    void setTime(QTime value);

protected:
    void clear();

    FirebirdStmt* _stmt;
    FirebirdProcs* _procs;
    int _number;

    XSQLVAR* _var;
    short _null;
    char _buffer[256];
};

class FirebirdColumn {
public:
    FirebirdColumn(FirebirdStmt* stmt, int number);
    ~FirebirdColumn();

    int number() const		{ return _number; }

    bool isNull();
    Variant getValue();
    QString getString();
    double getDouble();
    long getLong();
    QDate getDate();
    QTime getTime();

protected:
    FirebirdStmt* _stmt;
    FirebirdProcs* _procs;
    int _number;

    XSQLVAR* _var;
    short _null;
    char _buffer[256];
    char _internal[32];
};

#endif // FIREBIRD_DRIVER_H
