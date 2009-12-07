// $Id: db_driver.h,v 1.28 2005/04/04 06:11:25 bpepers Exp $
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

#ifndef DB_DRIVER_H
#define DB_DRIVER_H

#include "variant.h"
#include <qstringlist.h>

class Connection;
class Statement;
class CompanyDefn;
class TableDefn;
class ConstraintDefn;
class IndexDefn;
class QDialog;
class QWidget;

// Driver for a database type
class Driver {
public:
    // Supported driver types
    static QStringList types();
    static bool typeIsAvailable(const QString& type);
    static void registerDriver(Driver* driver);

    // Get a pointer to a driver by type name.  Don't delete this pointer
    // since it belongs to the Driver class.
    static Driver* getDriver(const QString& type);

    // Initialize the driver.  If this fails you shouldn't use the
    // driver except to get the configuration dialog and name.
    virtual bool initialize()=0;

    // Driver type name
    virtual QString name() const=0;

    // Create and delete databases
    virtual bool create(CompanyDefn& company)=0;
    virtual bool remove(CompanyDefn& company)=0;

    // Allocate a connection
    virtual Connection* allocConnection()=0;

    // Configuration dialog
    virtual QDialog* configureDialog(QWidget* parent)=0;

    // Last error
    QString lastError() const { return _lastError; }

protected:
    // Protected constructor/destructor so you can only create a Driver
    // using the getDriver() method and can't delete it.
    Driver();
    virtual ~Driver();

    // Set last error and return false
    bool error(const QString& message);

    // Driver data
    QString _lastError;

private:
    // These are not allowed on drivers
    Driver(const Driver& driver);
    Driver& operator=(const Driver& driver);
};

// Isolation levels
enum IsoLevel { READ_UNCOMMITTED, READ_COMMITTED, REPEATABLE_READ,
		SERIALIZABLE };

// Command types
enum CmdType { CMD_SELECT, CMD_INSERT, CMD_DELETE, CMD_UPDATE,
	       CMD_OTHER };

// Connection to a database
class Connection {
public:
    virtual ~Connection();

    // Auto commit (set before or after connected)
    virtual bool setAutoCommit(bool autoCommit)=0;
    bool getAutoCommit() const { return _autoCommit; }

    // Isolation level (set before or after connected)
    virtual bool setIsolationLevel(IsoLevel level)=0;
    IsoLevel getIsolationLevel() const { return _isoLevel; }

    // Connect to a database
    virtual bool connect(const QString& database)=0;
    virtual bool dbaConnect(const QString& database)=0;
    virtual void disconnect()=0;
    virtual bool connected() const=0;

    // Transaction handling
    virtual bool commit()=0;
    virtual bool rollback()=0;

    // Execute a command immediately
    virtual bool execute(const QString& command)=0;

    // Allocate a statement
    virtual Statement* allocStatement()=0;
    Statement* allocStatement(const QString& command);

    // Create a table, constraint, or index
    virtual bool create(const TableDefn& table)=0;
    virtual bool create(const ConstraintDefn& constraint)=0;
    virtual bool create(const IndexDefn& index)=0;

    // Create tables, constraints, and indexes for list of tables
    bool create(const QValueList<TableDefn>& tables);

    // Reference counting
    void incrRefCount() { _refCount++; }
    void decrRefCount() { _refCount--; }
    int refCount() const { return _refCount; }

    // Last error
    QString lastError() const { return _lastError; }

protected:
    // Protected constructor so you can only create a Connection
    // using the driver->allocConnection() method.
    Connection();

    // Set last error and return false
    bool error(const QString& message);

    // Connection data
    Driver* _driver;
    bool _autoCommit;
    IsoLevel _isoLevel;
    int _refCount;
    QString _lastError;

private:
    // These are not allowed on connections
    Connection(const Connection& con);
    Connection& operator=(const Connection& con);
};

// Statement on a connection
class Statement {
public:
    virtual ~Statement();

    // Command
    virtual bool setCommand(const QString& command)=0;
    QString getCommand() const { return _command; }

    // Parameters for current command
    virtual int paramCount()=0;

    // Set parameter value
    virtual void setNull(int param)=0;
    virtual void setValue(int param, Variant value)=0;
    virtual void setString(int param, const QString& value)=0;
    virtual void setDouble(int param, double value)=0;
    virtual void setLong(int param, long value)=0;
    virtual void setDate(int param, QDate value)=0;
    virtual void setTime(int param, QTime value)=0;

    // Execute methods
    virtual bool execute()=0;
    virtual bool next()=0;
    virtual bool cancel()=0;

    // Number of rows effected by insert, update, or delete
    virtual int getUpdateCount()=0;

    // Columns in result set
    virtual int columnCount()=0;
    virtual int findColumn(const QString& name)=0;
    virtual QString columnName(int column)=0;

    // Values from current row
    virtual bool isNull(int column)=0;
    virtual Variant getValue(int column)=0;
    virtual QString getString(int column)=0;
    virtual double getDouble(int column)=0;
    virtual long getLong(int column)=0;
    virtual QDate getDate(int column)=0;
    virtual QTime getTime(int column)=0;

    // Reference counting
    void incrRefCount() { _refCount++; }
    void decrRefCount() { _refCount--; }
    int refCount() const { return _refCount; }

    // Last error
    QString lastError() const { return _lastError; }

protected:
    // Protected constructor so you can only create a Statement
    // using the connection->allocStatement() method.
    Statement();

    // Set last error and return false
    bool error(const QString& message);

    // Statement data
    Connection* _conn;
    QString _command;
    int _nextParam;
    int _nextColumn;
    int _refCount;
    QString _lastError;

private:
    // These are not allowed on statements
    Statement(const Statement& stmt);
    Statement& operator=(const Statement& stmt);
};

// Function specification for runtime loaded drivers
typedef Driver* (*GetDriverFunc)();

// Operating specific error when loading a library fails
extern void libraryError();

#endif // DB_DRIVER_H
