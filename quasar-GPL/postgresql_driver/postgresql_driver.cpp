// $Id: postgresql_driver.cpp,v 1.26 2005/04/12 07:36:01 bpepers Exp $
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

#include "postgresql_driver.h"
#include "postgresql_config.h"
#include "postgresql_config_dialog.h"
#include "company_defn.h"
#include "table_defn.h"

#ifdef WIN32
#include <windows.h>
#define snprintf _snprintf
#else
#include <dlfcn.h>
#endif

#include <qlibrary.h>
#include <qdir.h>
#include <qregexp.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

typedef PGconn* (*PQsetdbLogin_t)(const char*, const char*, const char*,
				  const char*, const char*, const char*,
				  const char*);
typedef ConnStatusType (*PQstatus_t)(const PGconn*);
typedef char* (*PQerrorMessage_t)(const PGconn*);
typedef void (*PQfinish_t)(PGconn* conn);
typedef int (*PQsetClientEncoding_t)(PGconn* conn, const char*);
typedef PGresult* (*PQexec_t)(PGconn*, const char*);
typedef PGresult* (*PQexecParams_t)(PGconn*, const char*, int, const Oid*,
				    const char* const*, const int*,
				    const int*, int);
typedef ExecStatusType (*PQresultStatus_t)(const PGresult*);
typedef char* (*PQresultErrorMessage_t)(const PGresult*);
typedef int (*PQntuples_t)(const PGresult*);
typedef char* (*PQcmdTuples_t)(const PGresult*);
typedef int (*PQnfields_t)(const PGresult*);
typedef char* (*PQfname_t)(const PGresult*, int);
typedef void (*PQclear_t)(PGresult*);
typedef int (*PQgetisnull_t)(const PGresult*, int, int);
typedef char* (*PQgetvalue_t)(const PGresult*, int, int);
typedef size_t (*PQescapeString_t)(char*, const char*, size_t);

class PostgresqlProcs {
public:
    PostgresqlProcs(QLibrary* library);

    PQsetdbLogin_t PQsetdbLogin;
    PQstatus_t PQstatus;
    PQerrorMessage_t PQerrorMessage;
    PQfinish_t PQfinish;
    PQsetClientEncoding_t PQsetClientEncoding;
    PQexec_t PQexec;
    PQexecParams_t PQexecParams;
    PQresultStatus_t PQresultStatus;
    PQresultErrorMessage_t PQresultErrorMessage;
    PQntuples_t PQntuples;
    PQclear_t PQclear;
    PQcmdTuples_t PQcmdTuples;
    PQnfields_t PQnfields;
    PQfname_t PQfname;
    PQgetisnull_t PQgetisnull;
    PQgetvalue_t PQgetvalue;
    PQescapeString_t PQescapeString;
};

#define POSTGRESQL_PROC(name) name = (name ## _t)library->resolve(#name)

PostgresqlProcs::PostgresqlProcs(QLibrary* library)
{
    POSTGRESQL_PROC(PQsetdbLogin);
    POSTGRESQL_PROC(PQstatus);
    POSTGRESQL_PROC(PQerrorMessage);
    POSTGRESQL_PROC(PQfinish);
    POSTGRESQL_PROC(PQsetClientEncoding);
    POSTGRESQL_PROC(PQexec);
    POSTGRESQL_PROC(PQexecParams);
    POSTGRESQL_PROC(PQresultStatus);
    POSTGRESQL_PROC(PQresultErrorMessage);
    POSTGRESQL_PROC(PQntuples);
    POSTGRESQL_PROC(PQclear);
    POSTGRESQL_PROC(PQcmdTuples);
    POSTGRESQL_PROC(PQnfields);
    POSTGRESQL_PROC(PQfname);
    POSTGRESQL_PROC(PQgetisnull);
    POSTGRESQL_PROC(PQgetvalue);
    POSTGRESQL_PROC(PQescapeString);
}

PostgresqlDriver::PostgresqlDriver()
{
    _library = NULL;
    _procs = NULL;
}

PostgresqlDriver::~PostgresqlDriver()
{
    delete _procs;
    delete _library;
}

bool
PostgresqlDriver::initialize()
{
    if (_library != NULL)
	return true;

    PostgresqlConfig config;
    if (!config.load())
	return error("Can't read postgresql.cfg file");

    // Try to guess library name if not given
    if (config.library.isEmpty()) {
	QStringList libs;
	libs << "/usr/lib/libpq.so";
	libs << "/usr/lib/libpq.so.3";
	libs << "/usr/lib/libpq.so.3.1";

	for (unsigned int i = 0; i < libs.size(); ++i) {
	    if (QFileInfo(libs[i]).exists()) {
		config.library = libs[i];
		break;
	    }
	}

	// Save config if changed
	if (!config.library.isEmpty())
	    config.save(true);
    }

    // If still unknown, then its an error
    if (config.library.isEmpty())
	return error("Blank postgresql library name");

    // Try to load the library
    QLibrary* library = new QLibrary(config.library);
    if (!library->load()) {
	libraryError();
	delete library;
	return error("Can't load postgresql library: " + config.library);
    }

    _library = library;
    _procs = new PostgresqlProcs(_library);

    return true;
}

bool
PostgresqlDriver::create(CompanyDefn& company)
{
    if (!initialize())
	return error("Driver failed to initialize");

    PostgresqlConfig config;
    if (!config.load())
	return error("Can't read postgresql.cfg file");

    // Decide on database filePath (TODO: more conversions needed?)
    QString database = company.name();
    database.replace(QRegExp(" "), "_");
    database.replace(QRegExp("\\."), "_");
    database.replace(QRegExp("-"), "_");
    database = database.lower();

    PostgresqlConn conn(this);
    PostgresqlStmt stmt(&conn);
    conn.setAutoCommit(true);

    // Connect to template database
    if (!conn.dbaConnect("template1"))
	return error("Failed connecting to template1 database");

    // Create database
    QString cmd = "create database " + database;
    if (!config.charSet.isEmpty())
	cmd += " encoding '" + config.charSet + "'";
    stmt.setCommand(cmd);
    if (!stmt.execute())
	return error("Failed creating database: " + database);

    company.setDatabase(database);
    company.setDBType("PostgreSQL");
    return true;
}

bool
PostgresqlDriver::remove(CompanyDefn& company)
{
    if (!initialize())
	return error("Driver failed to initialize");

    PostgresqlConfig config;
    if (!config.load())
	return error("Can't read postgresql.cfg file");

    PostgresqlConn conn(this);
    PostgresqlStmt stmt(&conn);
    conn.setAutoCommit(true);

    // Connect to template database
    if (!conn.dbaConnect("template1"))
	return error("Failed connecting to template1 database");

    // Drop database
    QString cmd = "drop database " + company.database();
    stmt.setCommand(cmd);
    return stmt.execute();
}

Connection*
PostgresqlDriver::allocConnection()
{
    if (!initialize()) {
	error("Driver failed to initialize");
	return NULL;
    }

    return new PostgresqlConn(this);
}

QDialog*
PostgresqlDriver::configureDialog(QWidget* parent)
{
    return new PostgresqlConfigDialog(parent);
}

PostgresqlConn::PostgresqlConn(PostgresqlDriver* driver)
    : _driver(driver), _procs(_driver->_procs), _conn(NULL),
      _inTransaction(false)
{
}

PostgresqlConn::~PostgresqlConn()
{
    disconnect();
}

bool
PostgresqlConn::connect(const QString& database, const QString& userName,
			const QString& password)
{
    PostgresqlConfig config;
    if (!config.load(false))
	return error("Can't read postgresql.cfg file");

    QString port = QString::number(config.port);
    if (port == "0") port = "";

    PGconn* conn = _procs->PQsetdbLogin(config.hostname, port, NULL, NULL,
					database, userName, password);
    if (_procs->PQstatus(conn) != CONNECTION_OK) {
	QString message = _procs->PQerrorMessage(conn);
	qWarning("Connect failed: " + message);
	_procs->PQfinish(conn);
	return error("Connect failed: " + message);
    }

    if (_procs->PQsetClientEncoding(conn, "UNICODE") != 0) {
	_procs->PQfinish(conn);
	return error("Failed setting client encoding to unicode");
    }

    disconnect();
    _conn = conn;
    return true;
}

bool
PostgresqlConn::connect(const QString& database)
{
    PostgresqlConfig config;
    if (!config.load(false))
	return error("Can't read postgresql.cfg file");

    return connect(database, config.username, config.password);
}

bool
PostgresqlConn::dbaConnect(const QString& database)
{
    PostgresqlConfig config;
    if (!config.load(false))
	return error("Can't read postgresql.cfg file");

    return connect(database, config.dbaUsername, config.dbaPassword);
}

void
PostgresqlConn::disconnect()
{
    if (_conn != NULL) {
	if (_autoCommit)
	    commit();
	else
	    rollback();

	_procs->PQfinish(_conn);
	_conn = NULL;
    }
}

bool
PostgresqlConn::connected() const
{
    return _conn != NULL;
}

bool
PostgresqlConn::setAutoCommit(bool flag)
{
    _autoCommit = flag;
    return true;
}

bool
PostgresqlConn::setIsolationLevel(IsoLevel level)
{
    _isoLevel = level;
    return true;
}

bool
PostgresqlConn::commit()
{
    if (_inTransaction) {
	execute("commit");
	_inTransaction = false;
    }
    return true;
}

bool
PostgresqlConn::rollback()
{
    if (_inTransaction) {
	execute("rollback");
	_inTransaction = false;
    }
    return true;
}

bool
PostgresqlConn::execute(const QString& command)
{
    PGresult* result = _procs->PQexec(_conn, command.utf8());
    if (_procs->PQresultStatus(result) == PGRES_FATAL_ERROR) {
	QString message = _procs->PQresultErrorMessage(result);
	qWarning("Execute failed: " + message);
	_procs->PQclear(result);
	return error("Execute failed: " + message);
    }

    _procs->PQclear(result);
    return true;
}

bool
PostgresqlConn::create(const TableDefn& table)
{
    PostgresqlConfig config;
    if (!config.load(false))
	return error("Can't read postgresql.cfg file");

    QString command = "create table " + table.name + "(\n";
    for (unsigned int i = 0; i < table.columns.size(); ++i) {
	const ColumnDefn& column = table.columns[i];
	QString size = QString::number(column.size);

	command += "    " + column.name;
	switch (column.type) {
	case ColumnDefn::TYPE_CHAR:	command += " char(" + size + ")";break;
	case ColumnDefn::TYPE_STRING:	command += " varchar("+size+")"; break;
	case ColumnDefn::TYPE_DATE:	command += " date"; break;
	case ColumnDefn::TYPE_TIME:	command += " time"; break;
	case ColumnDefn::TYPE_BOOL:	command += " char(1)"; break;
	case ColumnDefn::TYPE_INT:	command += " integer"; break;
	case ColumnDefn::TYPE_BIGINT:	command += " numeric(14,0)"; break;
	case ColumnDefn::TYPE_NUMERIC:	command += " numeric(18,4)"; break;
	case ColumnDefn::TYPE_ID:	command += " "+Id::sqlDataType();break;
	case ColumnDefn::TYPE_MONEY:	command += " numeric(18,4)"; break;
	case ColumnDefn::TYPE_PERCENT:	command += " numeric(18,4)"; break;
	case ColumnDefn::TYPE_QUANTITY:	command += " numeric(18,4)"; break;
	}

	if (column.manditory) command += " not null";
	if (column.unique) command += " unique";

	command += ",\n";
    }
    command += "    primary key (" + table.primaryKey.join(", ") + "))";

    if (!execute(command))
        return error("Failed creating table "  + table.name);

    // Set access permissions on tables
    if (table.name == "users") {
        command = "grant select on users to " + config.username;
	if (!execute(command))
	    return error("Failed setting permissions on table users");
    } else {
        command = "grant select, insert, update, delete on " + table.name +
	    " to " + config.username;
	if (!execute(command))
	    return error("Failed setting permissions on table " + table.name);
    }

    return true;
}

bool
PostgresqlConn::create(const ConstraintDefn& constraint)
{
    QString command;

    switch (constraint.type) {
    case ConstraintDefn::TYPE_UNIQUE:
	command = "alter table " + constraint.table + " add unique (";
	command += constraint.columns.join(", ");
	command += ")";
	break;
    case ConstraintDefn::TYPE_CHECK:
	command = "alter table " + constraint.table + " add check (";
	command += constraint.check;
	command += ")";
	break;
    case ConstraintDefn::TYPE_FOREIGN_KEY:
	command = "alter table " + constraint.table + " add foreign key (";
	command += constraint.columns.join(", ");
	command += ") references " + constraint.referenceTable + " (";
	command += constraint.referenceColumns.join(", ");
	command += ")";
	if (constraint.referenceFlags & ConstraintDefn::DELETE_CASCADE)
	    command += " on delete cascade";
	if (constraint.referenceFlags & ConstraintDefn::DELETE_SET_NULL)
	    command += " on delete set null";
	break;
    }

    return execute(command);
}

bool
PostgresqlConn::create(const IndexDefn& index)
{
    QString command = "create ";
    if (index.unique) command += "unique ";
    command += "index " + index.name + " on " + index.table + " (";

    for (unsigned int i = 0; i < index.columns.size(); ++i) {
	const IndexColumn& column = index.columns[i];
	command += column.name;
	if (!column.ascending) command += " desc";
	if (i != index.columns.size() - 1) command += ", ";
    }
    command += ")";

    return execute(command);
}

Statement*
PostgresqlConn::allocStatement()
{
    return new PostgresqlStmt(this);
}

bool
PostgresqlConn::startTransaction()
{
    if (_inTransaction) return true;
    if (_autoCommit) return true;

    execute("start transaction");
    _inTransaction = true;

    return true;
}

PostgresqlStmt::PostgresqlStmt(PostgresqlConn* conn)
    : _conn(conn), _procs(conn->_procs), _result(NULL)
{
}

PostgresqlStmt::~PostgresqlStmt()
{
    cancel();
    clear();
}

void
PostgresqlStmt::clear()
{
    _internal = "";
    _params.clear();
}

bool
PostgresqlStmt::setCommand(const QString& cmd)
{
    clear();
    _command = cmd;
    _conn->startTransaction();

    int inSQuote = 0;
    int inDQuote = 0;
    for (unsigned int i = 0; i < cmd.length(); ++i) {
	QChar ch = cmd.at(i);

	// Allow things like \' or \" to escape quotes
	if (ch == '\\' && i != cmd.length() - 1) {
	    _internal += ch;
	    _internal += cmd.at(++i);
	    continue;
	}

	// Handle single quote start/end
	if (ch == '\'') {
	    inSQuote = !inSQuote;
	    _internal += ch;
	    continue;
	}

	// Handle double quote start/end
	if (ch == '"') {
	    inDQuote = !inDQuote;
	    _internal += ch;
	    continue;
	}

	// If in quotes or not a param marker, just transfer literally
	if (inSQuote || inDQuote || ch != '?') {
	    _internal += ch;
	    continue;
	}

	// Change param marker to PostgreSQL method
	_params << "";
	_internal += "$" + QString::number(_params.size());
    }

    _nextParam = 0;
    return true;
}

int
PostgresqlStmt::paramCount()
{
    return _params.size();
}

void
PostgresqlStmt::setNull(int param)
{
    if (param == -1) param = _nextParam++;
    assert(param >= 0 && param < paramCount());
    _params[param] = QString::null;
}

void
PostgresqlStmt::setValue(int param, Variant value)
{
    switch (value.type()) {
    case Variant::T_NULL:	setNull(param); break;
    case Variant::STRING:	setString(param, value.toString()); break;
    case Variant::FIXED:	setDouble(param, value.toDouble()); break;
    case Variant::BOOL:		setString(param, value.toString()); break;
    case Variant::DATE:		setDate(param, value.toDate()); break;
    case Variant::TIME:		setTime(param, value.toTime()); break;
    case Variant::PRICE:	setString(param, value.toString()); break;
    case Variant::ID:		setString(param, value.toString()); break;
    case Variant::PLU:		setString(param, value.toString()); break;
    }
}

void
PostgresqlStmt::setString(int param, const QString& value)
{
    if (param == -1) param = _nextParam++;
    assert(param >= 0 && param < paramCount());
    _params[param] = value;
}

void
PostgresqlStmt::setDouble(int param, double value)
{
    if (param == -1) param = _nextParam++;
    assert(param >= 0 && param < paramCount());
    _params[param] = QString::number(value, 'f', 4);
}

void
PostgresqlStmt::setLong(int param, long value)
{
    if (param == -1) param = _nextParam++;
    assert(param >= 0 && param < paramCount());
    _params[param] = QString::number(value);
}

void
PostgresqlStmt::setDate(int param, QDate value)
{
    if (param == -1) param = _nextParam++;
    assert(param >= 0 && param < paramCount());
    _params[param] = value.toString(Qt::ISODate);
}

void
PostgresqlStmt::setTime(int param, QTime value)
{
    if (param == -1) param = _nextParam++;
    assert(param >= 0 && param < paramCount());
    _params[param] = value.toString(Qt::ISODate);
}

bool
PostgresqlStmt::execute()
{
    cancel();

    if (_procs->PQexecParams == NULL) {
	QString command;
	int nextParam = 0;
	int inSQuote = 0;
	int inDQuote = 0;
	for (unsigned int i = 0; i < _command.length(); ++i) {
	    QChar ch = _command.at(i);

	    // Allow things like \' or \" to escape quotes
	    if (ch == '\\' && i != _command.length() - 1) {
		command += ch;
		command += _command.at(++i);
		continue;
	    }

	    // Handle single quote start/end
	    if (ch == '\'') {
		inSQuote = !inSQuote;
		command += ch;
		continue;
	    }

	    // Handle double quote start/end
	    if (ch == '"') {
		inDQuote = !inDQuote;
		command += ch;
		continue;
	    }

	    // If in quotes or not a param marker, just transfer literally
	    if (inSQuote || inDQuote || ch != '?') {
		command += ch;
		continue;
	    }

	    // Replace param marker with param value
	    QString value = _params[nextParam++];
	    if (value.isNull()) {
		command += "NULL";
	    } else {
		char buffer[value.length() * 3];
		_procs->PQescapeString(buffer, value.utf8(), value.utf8().length());
		command += "'" + QString(buffer) + "'";
	    }
	}

	_result = _procs->PQexec(_conn->_conn, command.utf8());
    } else {
	char** params = new char*[_params.size()];
	for (unsigned int i = 0; i < _params.size(); ++i) {
	    if (_params[i].isNull())
		params[i] = NULL;
	    else
		params[i] = strdup(_params[i].utf8());
	}

	_result = _procs->PQexecParams(_conn->_conn, _internal.utf8(),
				       _params.size(), NULL, params, NULL,
				       NULL, 0);

	for (unsigned int i = 0; i < _params.size(); ++i)
	    delete params[i];
	delete params;
    }

    if (_procs->PQresultStatus(_result) == PGRES_FATAL_ERROR) {
	QString message = _procs->PQresultErrorMessage(_result);
	qWarning("Execute failed: " + message);
	cancel();
	return error("Execute failed: " + message);
    }

    // Commit if not a select and autoCommit is set
    if (_conn->_autoCommit)
	_conn->commit();

    // Clear param/column position
    _nextParam = 0;
    _nextColumn = 1;
    _currentRow = -1;
    return true;
}

bool
PostgresqlStmt::next()
{
    int numRows = _procs->PQntuples(_result);
    _currentRow++;

    // There was a next row so clear position
    if (_currentRow < numRows) {
	_nextParam = 0;
	_nextColumn = 1;
	return true;
    }

    if (_conn->_autoCommit) _conn->commit();
    return false;
}

bool
PostgresqlStmt::cancel()
{
    if (_result != NULL) {
	_procs->PQclear(_result);
	_result = NULL;
    }
    return true;
}

int
PostgresqlStmt::getUpdateCount()
{
    return QString(_procs->PQcmdTuples(_result)).toInt();
}

int
PostgresqlStmt::columnCount()
{
    return _procs->PQnfields(_result);
}

int
PostgresqlStmt::findColumn(const QString& name)
{
    for (int i = 1; i <= columnCount(); ++i)
	if (columnName(i).lower() == name.lower())
	    return i;
    return -1;
}

QString
PostgresqlStmt::columnName(int column)
{
    assert(column > 0 && column <= columnCount());
    return _procs->PQfname(_result, column - 1);
}

bool
PostgresqlStmt::isNull(int column)
{
    if (column == -1) column = _nextColumn++;
    assert(column > 0 && column <= columnCount());
    return _procs->PQgetisnull(_result, _currentRow, column - 1);
}

Variant
PostgresqlStmt::getValue(int column)
{
    if (column == -1) column = _nextColumn++;
    assert(column > 0 && column <= columnCount());
    if (isNull(column)) return Variant();
    return QString::fromUtf8(_procs->PQgetvalue(_result, _currentRow, column - 1));
}

QString
PostgresqlStmt::getString(int column)
{
    if (column == -1) column = _nextColumn++;
    assert(column > 0 && column <= columnCount());
    if (isNull(column)) return QString();
    return QString::fromUtf8(_procs->PQgetvalue(_result, _currentRow, column - 1));
}

double
PostgresqlStmt::getDouble(int column)
{
    if (column == -1) column = _nextColumn++;
    assert(column > 0 && column <= columnCount());
    if (isNull(column)) return 0.0;
    return getString(column).toDouble();
}

long
PostgresqlStmt::getLong(int column)
{
    if (column == -1) column = _nextColumn++;
    assert(column > 0 && column <= columnCount());
    if (isNull(column)) return 0;
    return getString(column).toLong();
}

QDate
PostgresqlStmt::getDate(int column)
{
    if (column == -1) column = _nextColumn++;
    assert(column > 0 && column <= columnCount());
    if (isNull(column)) return QDate();
    return QDate::fromString(getString(column), Qt::ISODate);
}

QTime
PostgresqlStmt::getTime(int column)
{
    if (column == -1) column = _nextColumn++;
    assert(column > 0 && column <= columnCount());
    return QTime::fromString(getString(column));
}

extern "C" {

#ifdef WIN32
#define QUASAR_EXPORT __declspec(dllexport)
#else
#define QUASAR_EXPORT
#endif

QUASAR_EXPORT Driver*
getDriver()
{
    return new PostgresqlDriver();
}

}
