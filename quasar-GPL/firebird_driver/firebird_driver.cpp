// $Id: firebird_driver.cpp,v 1.46 2005/04/05 07:29:01 bpepers Exp $
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

#include "firebird_driver.h"
#include "firebird_config.h"
#include "firebird_config_dialog.h"
#include "company_defn.h"
#include "table_defn.h"
#include "sql_stmt.h"

#include <qglobal.h>
#include <qlibrary.h>
#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qtextcodec.h>
#include <assert.h>
#include <ibase.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

typedef ISC_STATUS (ISC_EXPORT *isc_attach_database_t)(ISC_STATUS*,short,char*,isc_db_handle*,short,char*);
typedef ISC_STATUS (ISC_EXPORT *isc_detach_database_t)(ISC_STATUS*,isc_db_handle*);
typedef ISC_STATUS (ISC_EXPORT *isc_drop_database_t)(ISC_STATUS*,isc_db_handle*);
typedef ISC_STATUS (ISC_EXPORT *isc_interprete_t)(char*,ISC_STATUS**);
typedef ISC_STATUS (ISC_EXPORT *isc_print_status_t)(ISC_STATUS*);
typedef ISC_STATUS (ISC_EXPORT_VARARG *isc_start_transaction_t)(ISC_STATUS*,isc_tr_handle*,short,...);
typedef ISC_STATUS (ISC_EXPORT *isc_commit_transaction_t)(ISC_STATUS*,isc_tr_handle*);
typedef ISC_STATUS (ISC_EXPORT *isc_rollback_transaction_t)(ISC_STATUS*,isc_tr_handle*);
typedef ISC_STATUS (ISC_EXPORT *isc_dsql_allocate_statement_t)(ISC_STATUS*,isc_db_handle*,isc_stmt_handle*);
typedef ISC_STATUS (ISC_EXPORT *isc_dsql_free_statement_t)(ISC_STATUS*,isc_stmt_handle*,unsigned short);
typedef ISC_STATUS (ISC_EXPORT *isc_dsql_prepare_t)(ISC_STATUS*,isc_tr_handle*,isc_stmt_handle*,unsigned short,char*,unsigned short,XSQLDA*);
typedef ISC_STATUS (ISC_EXPORT *isc_dsql_describe_t)(ISC_STATUS*,isc_stmt_handle*,unsigned short,XSQLDA*);
typedef ISC_STATUS (ISC_EXPORT *isc_dsql_describe_bind_t)(ISC_STATUS*,isc_stmt_handle*,unsigned short,XSQLDA*);
typedef ISC_STATUS (ISC_EXPORT *isc_dsql_execute_t)(ISC_STATUS*,isc_tr_handle*,isc_stmt_handle*,unsigned short,XSQLDA*);
typedef ISC_STATUS (ISC_EXPORT *isc_dsql_execute_immediate_t)(ISC_STATUS*,isc_db_handle*,isc_tr_handle*,unsigned short,char*,unsigned short,XSQLDA*);
typedef ISC_STATUS (ISC_EXPORT *isc_dsql_fetch_t)(ISC_STATUS*,isc_stmt_handle*,unsigned short,XSQLDA*);
typedef ISC_STATUS (ISC_EXPORT *isc_dsql_sql_info_t)(ISC_STATUS*,isc_stmt_handle*,short,char*,short,char*);
typedef ISC_STATUS (ISC_EXPORT *isc_encode_sql_date_t)(void*,ISC_DATE*);
typedef ISC_STATUS (ISC_EXPORT *isc_encode_sql_time_t)(void*,ISC_TIME*);
typedef ISC_STATUS (ISC_EXPORT *isc_decode_sql_date_t)(ISC_DATE*,void*);
typedef ISC_STATUS (ISC_EXPORT *isc_decode_sql_time_t)(ISC_TIME*,void*);
typedef ISC_STATUS (ISC_EXPORT *isc_decode_timestamp_t)(ISC_TIMESTAMP*,void*);
typedef ISC_STATUS (ISC_EXPORT *isc_vax_integer_t)(char*,short);

class FirebirdProcs {
public:
    FirebirdProcs(QLibrary* library);

    isc_attach_database_t isc_attach_database;
    isc_detach_database_t isc_detach_database;
    isc_drop_database_t isc_drop_database;
    isc_interprete_t isc_interprete;
    isc_print_status_t isc_print_status;
    isc_start_transaction_t isc_start_transaction;
    isc_commit_transaction_t isc_commit_transaction;
    isc_rollback_transaction_t isc_rollback_transaction;
    isc_dsql_allocate_statement_t isc_dsql_allocate_statement;
    isc_dsql_free_statement_t isc_dsql_free_statement;
    isc_dsql_prepare_t isc_dsql_prepare;
    isc_dsql_describe_t isc_dsql_describe;
    isc_dsql_describe_bind_t isc_dsql_describe_bind;
    isc_dsql_execute_t isc_dsql_execute;
    isc_dsql_execute_immediate_t isc_dsql_execute_immediate;
    isc_dsql_fetch_t isc_dsql_fetch;
    isc_dsql_sql_info_t isc_dsql_sql_info;
    isc_encode_sql_date_t isc_encode_sql_date;
    isc_encode_sql_time_t isc_encode_sql_time;
    isc_decode_sql_date_t isc_decode_sql_date;
    isc_decode_sql_time_t isc_decode_sql_time;
    isc_decode_timestamp_t isc_decode_timestamp;
    isc_vax_integer_t isc_vax_integer;
};

#define FIREBIRD_PROC(name) name = (name ## _t)library->resolve(#name)

FirebirdProcs::FirebirdProcs(QLibrary* library)
{
    FIREBIRD_PROC(isc_attach_database);
    FIREBIRD_PROC(isc_detach_database);
    FIREBIRD_PROC(isc_drop_database);
    FIREBIRD_PROC(isc_interprete);
    FIREBIRD_PROC(isc_print_status);
    FIREBIRD_PROC(isc_start_transaction);
    FIREBIRD_PROC(isc_commit_transaction);
    FIREBIRD_PROC(isc_rollback_transaction);
    FIREBIRD_PROC(isc_dsql_allocate_statement);
    FIREBIRD_PROC(isc_dsql_free_statement);
    FIREBIRD_PROC(isc_dsql_prepare);
    FIREBIRD_PROC(isc_dsql_describe);
    FIREBIRD_PROC(isc_dsql_describe_bind);
    FIREBIRD_PROC(isc_dsql_execute);
    FIREBIRD_PROC(isc_dsql_execute_immediate);
    FIREBIRD_PROC(isc_dsql_fetch);
    FIREBIRD_PROC(isc_dsql_sql_info);
    FIREBIRD_PROC(isc_encode_sql_date);
    FIREBIRD_PROC(isc_encode_sql_time);
    FIREBIRD_PROC(isc_decode_sql_date);
    FIREBIRD_PROC(isc_decode_sql_time);
    FIREBIRD_PROC(isc_decode_timestamp);
    FIREBIRD_PROC(isc_vax_integer);
}

FirebirdDriver::FirebirdDriver()
{
    _libFile = "";
    _library = NULL;
    _procs = NULL;
}

FirebirdDriver::~FirebirdDriver()
{
    delete _procs;
    delete _library;
}

bool
FirebirdDriver::initialize()
{
    if (_library != NULL)
	return true;

    FirebirdConfig config;
    if (!config.load())
	return error("Can't read firebird.cfg file");

    // Set firebird install directory
#ifdef WIN32
    SetEnvironmentVariable("INTERBASE", parseDir(config.installDir));
    SetEnvironmentVariable("FIREBIRD", parseDir(config.installDir));
#else
    setenv("INTERBASE", parseDir(config.installDir), true);
    setenv("FIREBIRD", parseDir(config.installDir), true);
#endif

    // Try to load the library
    QLibrary* library = new QLibrary(config.library);
    if (!library->load()) {
	libraryError();
	delete library;
	return error("Can't load firebird library: " + config.library);
    }

    _library = library;
    _procs = new FirebirdProcs(_library);

    return true;
}

bool
FirebirdDriver::create(CompanyDefn& company)
{
    if (!initialize())
	return error("Driver failed to initialize");

    FirebirdConfig config;
    if (!config.load())
	return error("Can't read firebird.cfg file");

    // Decide on database filepath
    QString name = company.name();
    name.replace(QRegExp(" "), "_");
    QString database = parseDir(config.databaseDir) + "/" + name +
	".fdb";

    // Determine existence of db to be created
    if (QFileInfo(database).exists())
        return error("A database of the same name already exists");

    // Database connection string
    QString dbName = config.hostname;
    if (config.port != 3050)
	dbName += "/" + QString::number(config.port);
    if (!dbName.isEmpty())
	dbName += ":";
    dbName += database;

    // Create database command
    QString cmd = "create database '" + dbName + "' user 'sysdba' password '" +
	config.dbaPassword + "' page_size " + QString::number(config.blockSize)
	+ " default character set " + config.charSet;

    ISC_STATUS status[20];
    isc_db_handle db = 0L;
    isc_tr_handle trans = 0L;
    char* command = strdup(cmd.latin1());
    _procs->isc_dsql_execute_immediate(status, &db, &trans, 0, command,
				       SQL_DIALECT_V6, NULL);
    free(command);
    if (status[0] == 1 && status[1]) {
	qWarning("Error in: %s", command);
	_procs->isc_print_status(status);
	return error("Failed creating database");
    }

    _procs->isc_detach_database(status, &db);

    company.setDatabase(database);
    company.setDBType("Firebird");
    return true;
}

bool
FirebirdDriver::remove(CompanyDefn& company)
{
    if (!initialize())
	return error("Driver failed to initialize");

    FirebirdConfig config;
    if (!config.load())
	return error("Can't read firebird.cfg file");

    FirebirdConn conn(this);
    if (!conn.dbaConnect(company.database()))
	return false;

    conn.dropDatabase();
    return true;
}

Connection*
FirebirdDriver::allocConnection()
{
    if (!initialize()) {
	error("Driver failed to initialize");
	return NULL;
    }

    return new FirebirdConn(this);
}

QDialog*
FirebirdDriver::configureDialog(QWidget* parent)
{
    return new FirebirdConfigDialog(parent);
}

FirebirdConn::FirebirdConn(FirebirdDriver* driver)
    : _driver(driver), _procs(_driver->_procs), _db(NULL), _tr(NULL)
{
}

FirebirdConn::~FirebirdConn()
{
    disconnect();
}

bool
FirebirdConn::connect(const QString& database, const QString& userName,
		      const QString& password)
{
    FirebirdConfig config;
    if (!config.load())
	return error("Can't read firebird.cfg file");

    // Get codec for text conversions
    QTextCodec* codec = QTextCodec::codecForName(config.charSet);
    if (codec == NULL)
	return error("Character set not supported by Qt: " + config.charSet);

    // Verify values are not too large
    if (userName.length() + password.length() + config.charSet.length() > 4000)
	return error("Username/password are too long");

    // Database param buffer
    char dpb_buffer[4096];
    char* dpb = dpb_buffer;

    // Initialize param buffer
    *dpb++ = isc_dpb_version1;

    // Turn on forced writes
    *dpb++ = isc_dpb_force_write;
    *dpb++ = 1;
    *dpb++ = 1;

    // Add username
    *dpb++ = isc_dpb_user_name;
    *dpb++ = strlen(userName);
    strcpy(dpb, userName);
    dpb += strlen(userName);

    // Add password
    *dpb++ = isc_dpb_password;
    *dpb++ = strlen(password);
    strcpy(dpb, password);
    dpb += strlen(password);

    // Add character set
    *dpb++ = isc_dpb_lc_ctype;
    *dpb++ = strlen(config.charSet);
    strcpy(dpb, config.charSet);
    dpb += strlen(config.charSet);

    // Build database connection name
    QString dbName = config.hostname;
    if (config.port != 3050)
	dbName += "/" + QString::number(config.port);
    if (!dbName.isEmpty())
	dbName += ":";
    dbName += database;

    // Connect
    ISC_STATUS status[20];
    isc_db_handle db = 0L;
    char* dbname = strdup(dbName.latin1());
    _procs->isc_attach_database(status, 0, dbname, &db, dpb - dpb_buffer,
				dpb_buffer);
    free(dbname);
    if (status[0] == 1 && status[1]) {
	_procs->isc_print_status(status);
	return error("connect failed");
    }

    disconnect();
    _db = db;
    _codec = codec;

    return true;
}

bool
FirebirdConn::connect(const QString& database)
{
    FirebirdConfig config;
    if (!config.load())
	return error("Can't read firebird.cfg file");

    return connect(database, config.username, config.password);
}

bool
FirebirdConn::dbaConnect(const QString& database)
{
    FirebirdConfig config;
    if (!config.load()) {
	qWarning("Can't read firebird.cfg file");
	return "";
    }

    return connect(database, "sysdba", config.dbaPassword);
}

void
FirebirdConn::disconnect()
{
    if (_db != NULL) {
	if (_autoCommit)
	    commit();
	else
	    rollback();

	ISC_STATUS status[20];
	_procs->isc_detach_database(status, &_db);
	if (status[0] == 1 && status[1]) {
	    _procs->isc_print_status(status);
	    error("isc_detach_database failed");
	}
    }
}

bool
FirebirdConn::connected() const
{
    return _db != NULL;
}

bool
FirebirdConn::setAutoCommit(bool flag)
{
    _autoCommit = flag;
    return true;
}

bool
FirebirdConn::setIsolationLevel(IsoLevel level)
{
    _isoLevel = level;
    return true;
}

bool
FirebirdConn::commit()
{
    if (_tr) {
	ISC_STATUS status[20];
	_procs->isc_commit_transaction(status, &_tr);
	if (status[0] == 1 && status[1]) {
	    _procs->isc_print_status(status);
	    return error("isc_commit_transaction failed");
	}
    }
    return true;
}

bool
FirebirdConn::rollback()
{
    if (_tr) {
	ISC_STATUS status[20];
	_procs->isc_rollback_transaction(status, &_tr);
	if (status[0] == 1 && status[1]) {
	    _procs->isc_print_status(status);
	    return error("isc_rollback_transaction failed");
	}
    }
    return true;
}

bool
FirebirdConn::execute(const QString& command)
{
    startTransaction();

    ISC_STATUS status[20];
    char* cmd = strdup(command.latin1());
    _procs->isc_dsql_execute_immediate(status, &_db, &_tr, 0, cmd,
				       SQL_DIALECT_V6, NULL);
    free(cmd);
    if (status[0] == 1 && status[1]) {
	qWarning("Execute failed: " + command);
	_procs->isc_print_status(status);
	return error("Execute failed");
    }

    if (_autoCommit) {
	if (!commit())
	    return error("Commit failed");
    }

    return true;
}

bool
FirebirdConn::create(const TableDefn& table)
{
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

    // Set quasar access permissions on tables
    if (table.name == "users") {
        command = "grant select on users to QUASAR";
	if (!execute(command))
	    return error("Failed setting permissions on table users");
    } else {
        command = "grant select, insert, update, delete on " + table.name +
	    " to quasar";
	if (!execute(command))
	    return error("Failed setting permissions on table " + table.name);
    }

    return true;
}

bool
FirebirdConn::create(const ConstraintDefn& constraint)
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
FirebirdConn::create(const IndexDefn& index)
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
FirebirdConn::allocStatement()
{
    return new FirebirdStmt(this);
}

bool
FirebirdConn::startTransaction()
{
    if (_tr) return true;

    // TODO: change this based on isolation level
    static char isc_tpb[] = { isc_tpb_version3, isc_tpb_write, isc_tpb_wait,
			      isc_tpb_read_committed, isc_tpb_no_rec_version };

    ISC_STATUS status[20];
    _procs->isc_start_transaction(status, &_tr, 1, &_db, sizeof(isc_tpb), isc_tpb);
    if (status[0] == 1 && status[1]) {
	_procs->isc_print_status(status);
	return error("isc_start_transaction failed");
    }
    return true;
}

bool
FirebirdConn::dropDatabase()
{
    rollback();

    ISC_STATUS status[20];
    _procs->isc_drop_database(status, &_db);
    if (status[0] == 1 && status[1]) {
	_procs->isc_print_status(status);
	return error("isc_drop_database failed");
    }
    return true;
}

FirebirdStmt::FirebirdStmt(FirebirdConn* conn)
    : _conn(conn), _procs(conn->_procs), _stmt(NULL)
{
    _outputDA = (XSQLDA*)_defaultColumns;
    _outputDA->version = SQLDA_VERSION1;
    _outputDA->sqln = DEFAULT_COLUMN_COUNT;

    _inputDA = (XSQLDA*)_defaultParams;;
    _inputDA->version = SQLDA_VERSION1;
    _inputDA->sqln = DEFAULT_PARAM_COUNT;
}

FirebirdStmt::~FirebirdStmt()
{
    clear();
}

void
FirebirdStmt::clear()
{
    if (_stmt != NULL) {
	ISC_STATUS status[20];
	_procs->isc_dsql_free_statement(status, &_stmt, DSQL_drop);
	if (status[0] == 1 && status[1]) {
	    _procs->isc_print_status(status);
	    error("isc_dsql_free_statement failed");
	}
    }

    _command = "";

    for (unsigned int param = 0; param < _params.size(); ++param)
	delete _params[param];
    _params.clear();

    for (unsigned int column = 0; column < _columns.size(); ++column)
	delete _columns[column];
    _columns.clear();

    if (_outputDA != (XSQLDA*)_defaultColumns)
	free(_outputDA);
    _outputDA = (XSQLDA*)_defaultColumns;

    if (_inputDA != (XSQLDA*)_defaultParams)
	free(_inputDA);
    _inputDA = (XSQLDA*)_defaultParams;
}

bool
FirebirdStmt::setCommand(const QString& cmd)
{
    clear();
    _command = cmd;
    ISC_STATUS status[20];
    _conn->startTransaction();

    _procs->isc_dsql_allocate_statement(status, &_conn->_db, &_stmt);
    if (status[0] == 1 && status[1]) {
	qWarning("Error in: " + cmd);
	_procs->isc_print_status(status);
	return error("isc_dsql_allocate_statement failed");
    }

    char* cmdString = strdup(_command);
    _procs->isc_dsql_prepare(status, &_conn->_tr, &_stmt, 0, cmdString,
			     SQL_DIALECT_V6, _outputDA);
    free(cmdString);
    if (status[0] == 1 && status[1]) {
	qWarning("Error in: " + cmd);
	_procs->isc_print_status(status);
	return error("isc_dsql_prepare failed");
    }

    int count = _outputDA->sqld;
    if (count > _outputDA->sqln) {
	_outputDA = (XSQLDA*)malloc(XSQLDA_LENGTH(count));
	_outputDA->version = SQLDA_VERSION1;
	_outputDA->sqln = count;

	_procs->isc_dsql_describe(status, &_stmt, SQL_DIALECT_V6, _outputDA);
	if (status[0] == 1 && status[1]) {
	    qWarning("Error in: " + cmd);
	    _procs->isc_print_status(status);
	    return error("isc_dsql_describe failed");
	}
    }

    _procs->isc_dsql_describe_bind(status, &_stmt, SQL_DIALECT_V6, _inputDA);
    if (status[0] == 1 && status[1]) {
	qWarning("Error in: " + cmd);
	_procs->isc_print_status(status);
	return error("isc_dsql_describe_bind failed");
    }

    count = _inputDA->sqld;
    if (count > _inputDA->sqln) {
	_inputDA = (XSQLDA*)malloc(XSQLDA_LENGTH(count));
	_inputDA->version = SQLDA_VERSION1;
	_inputDA->sqln = count;

	_procs->isc_dsql_describe_bind(status, &_stmt,SQL_DIALECT_V6,_inputDA);
	if (status[0] == 1 && status[1]) {
	    qWarning("Error in: " + cmd);
	    _procs->isc_print_status(status);
	    return error("isc_dsql_describe_bind failed");
	}
    }

    // Create the params
    for (int pnum = 0; pnum < _inputDA->sqld; ++pnum) {
	FirebirdParam* param = new FirebirdParam(this, pnum);
	_params.push_back(param);
    }

    // Create the columns
    for (int cnum = 0; cnum < _outputDA->sqld; ++cnum) {
	FirebirdColumn* column = new FirebirdColumn(this, cnum);
	_columns.push_back(column);
    }

    _nextParam = 0;
    return true;
}

int
FirebirdStmt::paramCount()
{
    return _params.size();
}

void
FirebirdStmt::setNull(int param)
{
    if (param == -1) param = _nextParam++;
    assert(param >= 0 && param < paramCount());
    _params[param]->setNull();
}

void
FirebirdStmt::setValue(int param, Variant value)
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
FirebirdStmt::setString(int param, const QString& value)
{
    if (param == -1) param = _nextParam++;
    assert(param >= 0 && param < paramCount());
    _params[param]->setString(value);
}

void
FirebirdStmt::setDouble(int param, double value)
{
    if (param == -1) param = _nextParam++;
    assert(param >= 0 && param < paramCount());
    _params[param]->setDouble(value);
}

void
FirebirdStmt::setLong(int param, long value)
{
    if (param == -1) param = _nextParam++;
    assert(param >= 0 && param < paramCount());
    _params[param]->setLong(value);
}

void
FirebirdStmt::setDate(int param, QDate value)
{
    if (param == -1) param = _nextParam++;
    assert(param >= 0 && param < paramCount());
    _params[param]->setDate(value);
}

void
FirebirdStmt::setTime(int param, QTime value)
{
    if (param == -1) param = _nextParam++;
    assert(param >= 0 && param < paramCount());
    _params[param]->setTime(value);
}

bool
FirebirdStmt::execute()
{
    ISC_STATUS status[20];
    _procs->isc_dsql_free_statement(status, &_stmt, DSQL_close);

    int retry_count = 0;
    while (1) {
	if (_procs->isc_dsql_execute(status, &_conn->_tr, &_stmt,
				     SQL_DIALECT_V6, _inputDA) == 0)
	    break;

	if ((status[1] == isc_deadlock || status[1] == isc_lock_conflict)) {
#ifdef WIN32
	    Sleep(1000);
#else
	    while (1) {
		struct timespec request = { 0, 100000};
		struct timespec remain = { 0, 100000};
		int result = nanosleep(&request, &remain);

		request = remain;
		if (result == -1 && errno == EINTR)
		    continue;
		break;
	    }
#endif

	    ++retry_count;
	    continue;
	}

	qWarning("Error in: " + _command);
	_procs->isc_print_status(status);
	return error("isc_dsql_execute failed");
    }

    // Commit if not a select and autoCommit is set
    if (_outputDA->sqld == 0 && _conn->_autoCommit)
	_conn->commit();

    // Clear param/column position
    _nextParam = 0;
    _nextColumn = 1;
    _result = 0;
    return true;
}

bool
FirebirdStmt::next()
{
    // If we already finished then just return false
    if (_result != 0)
	return false;

    ISC_STATUS status[20];
    _result = _procs->isc_dsql_fetch(status, &_stmt, SQL_DIALECT_V6,
				     _outputDA);

    // There was a next row so clear position
    if (_result == 0) {
	_nextParam = 0;
	_nextColumn = 1;
	return true;
    }

    // No next row so auto commit if required
    if (_result == 100) {
	if (_conn->_autoCommit) _conn->commit();
	return false;
    }

    _procs->isc_print_status(status);
    return error("isc_sql_fetch failed");
}

bool
FirebirdStmt::cancel()
{
    // TODO: implement
    return false;
}

int
FirebirdStmt::getUpdateCount()
{
    if (_outputDA->sqld > 0) return 0;

    static char requestInfo[] = { isc_info_sql_records,
				  isc_info_sql_stmt_type,
				  isc_info_end };
    char buffer[128];
    ISC_STATUS status[20];
    _procs->isc_dsql_sql_info(status, &_stmt, sizeof(requestInfo),
			      requestInfo, sizeof(buffer), buffer);

    int statementType = 0;
    int insertCount = 0;
    int deleteCount = 0;
    int updateCount = 0;

    for (char *p = buffer; *p != isc_info_end;) {
	char item = *p++;
	int length = _procs->isc_vax_integer (p, 2);
	p += 2;

	switch (item) {
	case isc_info_sql_records:
	    char* q;
	    for (q = p; *q != isc_info_end;) {
		char item = *q++;
		int l = _procs->isc_vax_integer(q, 2);
		q += 2;
		switch (item) {
		case isc_info_req_insert_count:
		    insertCount = _procs->isc_vax_integer(q, l);
		    break;
		case isc_info_req_delete_count:
		    deleteCount = _procs->isc_vax_integer(q, l);
		    break;
		case isc_info_req_update_count:
		    updateCount = _procs->isc_vax_integer(q, l);
		    break;
		}
		q += l;
	    }
	    break;

	case isc_info_sql_stmt_type:
	    statementType = _procs->isc_vax_integer(p, length);
	    break;
	}
	p += length;
    }

    int count = updateCount;
    if (insertCount > count) count = insertCount;
    if (deleteCount > count) count = deleteCount;

    return count;
}

int
FirebirdStmt::columnCount()
{
    return _outputDA->sqld;
}

int
FirebirdStmt::findColumn(const QString& name)
{
    for (int i = 1; i <= columnCount(); ++i)
	if (columnName(i).lower() == name.lower())
	    return i;
    return -1;
}

QString
FirebirdStmt::columnName(int column)
{
    assert(column > 0 && column <= columnCount());

    XSQLVAR* var = _outputDA->sqlvar + column - 1;
    char name[40];
    if (var->aliasname_length != 0) {
	memcpy(name, var->aliasname, var->aliasname_length);
	name[var->aliasname_length] = 0;
    } else {
	memcpy(name, var->sqlname, var->sqlname_length);
	name[var->sqlname_length] = 0;
    }
    return name;
}

bool
FirebirdStmt::isNull(int column)
{
    if (column == -1) column = _nextColumn++;
    assert(column > 0 && column <= columnCount());
    return _columns[column - 1]->isNull();
}

Variant
FirebirdStmt::getValue(int column)
{
    if (column == -1) column = _nextColumn++;
    assert(column > 0 && column <= columnCount());
    return _columns[column - 1]->getValue();
}

QString
FirebirdStmt::getString(int column)
{
    if (column == -1) column = _nextColumn++;
    assert(column > 0 && column <= columnCount());
    return _columns[column - 1]->getString();
}

double
FirebirdStmt::getDouble(int column)
{
    if (column == -1) column = _nextColumn++;
    assert(column > 0 && column <= columnCount());
    return _columns[column - 1]->getDouble();
}

long
FirebirdStmt::getLong(int column)
{
    if (column == -1) column = _nextColumn++;
    assert(column > 0 && column <= columnCount());
    return _columns[column - 1]->getLong();
}

QDate
FirebirdStmt::getDate(int column)
{
    if (column == -1) column = _nextColumn++;
    assert(column > 0 && column <= columnCount());
    return _columns[column - 1]->getDate();
}

QTime
FirebirdStmt::getTime(int column)
{
    if (column == -1) column = _nextColumn++;
    assert(column > 0 && column <= columnCount());
    return _columns[column - 1]->getTime();
}

FirebirdParam::FirebirdParam(FirebirdStmt* stmt, int number)
    : _stmt(stmt), _procs(stmt->_procs), _number(number), _null(0)
{
    _var = stmt->_inputDA->sqlvar + number;
    _var->sqldata = _buffer;
}

FirebirdParam::~FirebirdParam()
{
    clear();
}

void
FirebirdParam::clear()
{
    if (_var->sqldata != _buffer)
	free(_var->sqldata);
    _var->sqldata = _buffer;
    _var->sqllen = 0;
    _null = 0;
}

void
FirebirdParam::setNull()
{
    clear();
    _var->sqltype = SQL_TEXT + 1;
    _var->sqlind = &_null;
    _null = -1;
}

void
FirebirdParam::setString(const QString& value)
{
    clear();
    if (value.isNull()) {
	setNull();
    } else {
	QCString text = _stmt->_conn->_codec->fromUnicode(value);
	_var->sqltype = SQL_TEXT;
	_var->sqllen = text.length();
	if (_var->sqllen < 256)
	    memcpy(_buffer, text, _var->sqllen);
	else
	    _var->sqldata = strdup(text);
    }
}

void
FirebirdParam::setDouble(double value)
{
    clear();
    _var->sqltype = SQL_DOUBLE;
    _var->sqllen = sizeof(double);
    *((double*)_buffer) = value;
}

void
FirebirdParam::setLong(long value)
{
    clear();
    _var->sqltype = SQL_LONG;
    _var->sqllen = sizeof(long);
    *((long*)_buffer) = value;
}

void
FirebirdParam::setDate(QDate value)
{
    clear();
    if (value.isNull()) {
	setNull();
    } else {
	_var->sqltype = SQL_TYPE_DATE;
	_var->sqllen = sizeof(ISC_DATE);
	_procs->isc_encode_sql_date(makeTM(value), (ISC_DATE*)_buffer);
    }
}

void
FirebirdParam::setTime(QTime value)
{
    clear();
    if (!value.isValid()) {
	setNull();
    } else {
	_var->sqltype = SQL_TYPE_TIME;
	_var->sqllen = sizeof(ISC_TIME);
	_procs->isc_encode_sql_time(makeTM(value), (ISC_TIME*)_buffer);
    }
}

FirebirdColumn::FirebirdColumn(FirebirdStmt* stmt, int number)
    : _stmt(stmt), _procs(stmt->_procs), _number(number)
{
    _var = stmt->_outputDA->sqlvar + number;
    if (_var->sqllen < 256)
	_var->sqldata = _buffer;
    else
	_var->sqldata = (char*)malloc(_var->sqllen);
    _var->sqlind = &_null;
}

FirebirdColumn::~FirebirdColumn()
{
    if (_var->sqldata != _buffer)
	free(_var->sqldata);
}

bool
FirebirdColumn::isNull()
{
    if ((_var->sqltype & 1) == 0) return false;
    return *_var->sqlind;
}

Variant
FirebirdColumn::getValue()
{
    if (isNull()) return Variant();

    switch (_var->sqltype & ~1) {
    case SQL_TEXT:
    case SQL_VARYING:
	return getString();
    case SQL_SHORT:
    case SQL_LONG:
	if (_var->sqlscale == 0)
	    return int64_t(getLong());
	return getDouble();
    case SQL_INT64:
	if (_var->sqlscale == 0)
	    return Variant(int64_t(getDouble()));
	return getDouble();
    case SQL_FLOAT:
    case SQL_DOUBLE:
	return getDouble();
    case SQL_TYPE_DATE:
	return getDate();
    case SQL_TYPE_TIME:
	return getTime();
    default:
	break;
    }

    qWarning("Sqlda::getValue: invalid type: %d", _var->sqltype);
    return Variant();
}

QString
FirebirdColumn::getString()
{
    if (isNull()) return NULL;

    switch (_var->sqltype & ~1) {
    case SQL_TEXT:
	_var->sqldata[_var->sqllen] = 0;
	return _stmt->_conn->_codec->toUnicode(_var->sqldata);
    case SQL_VARYING:
	_var->sqldata[*((short*)_var->sqldata) + sizeof(short)] = 0;
	return _var->sqldata + sizeof(short);
    case SQL_SHORT:
    case SQL_LONG:
    case SQL_INT64:
	if (_var->sqlscale == 0)
	    sprintf(_internal, "%ld", getLong());
	else
	    sprintf(_internal, "%f", getDouble());
	return _internal;
    case SQL_FLOAT:
    case SQL_DOUBLE:
	sprintf(_internal, "%f", getDouble());
	return _internal;
    case SQL_TYPE_DATE:
	strcpy(_internal, getDate().toString());
	return _internal;
    case SQL_TYPE_TIME:
	strcpy(_internal, getTime().toString());
	return _internal;
    default:
	break;
    }

    qWarning("Sqlda::getString: invalid type: %d", _var->sqltype);
    return NULL;
}

double
FirebirdColumn::getDouble()
{
    if (isNull()) return 0.0;

    ISC_INT64 value;

    switch (_var->sqltype & ~1) {
    case SQL_SHORT:
	value = *((short*)_var->sqldata);
	if (_var->sqlscale == 0) return value;
	return double(value) * pow(10.0, _var->sqlscale);
    case SQL_LONG:
	value = *((long*)_var->sqldata);
	if (_var->sqlscale == 0) return value;
	return double(value) * pow(10.0, _var->sqlscale);
    case SQL_INT64:
	value = *((ISC_INT64*)_var->sqldata);
	if (_var->sqlscale == 0) return value;
	return double(value) * pow(10.0, _var->sqlscale);
    case SQL_FLOAT:
	return *((float*)_var->sqldata);
    case SQL_DOUBLE:
	return *((double*)_var->sqldata);
    default:
	break;
    }

    qWarning("getDouble: invalid type: %d", _var->sqltype);
    return 0.0;
}

long
FirebirdColumn::getLong()
{
    if (isNull()) return 0L;

    ISC_INT64 value;

    switch (_var->sqltype & ~1) {
    case SQL_SHORT:
	value = *((short*)_var->sqldata);
	if (_var->sqlscale == 0) return value;
	return int(double(value) * pow(10.0, _var->sqlscale));
    case SQL_LONG:
	value = *((long*)_var->sqldata);
	if (_var->sqlscale == 0) return value;
	return int(double(value) * pow(10.0, _var->sqlscale));
    case SQL_INT64:
	value = *((ISC_INT64*)_var->sqldata);
	if (_var->sqlscale == 0) return value;
	return int(double(value) * pow(10.0, _var->sqlscale));
    case SQL_FLOAT:
	return int(*((float*)_var->sqldata));
    case SQL_DOUBLE:
	return int(*((double*)_var->sqldata));
    default:
	break;
    }

    qWarning("getLong: invalid type: %d", _var->sqltype);
    return 0;
}

QDate
FirebirdColumn::getDate()
{
    if (isNull()) return QDate();
    struct tm tm;

    switch (_var->sqltype & ~1) {
    case SQL_TYPE_DATE:
	_procs->isc_decode_sql_date((ISC_DATE*)_var->sqldata, &tm);
	return makeDate(&tm);
    default:
	break;
    }

    qWarning("Sqlda::getDate: invalid type: %d", _var->sqltype);
    return QDate();
}

QTime
FirebirdColumn::getTime()
{
    if (isNull()) return QTime();
    struct tm tm;

    switch (_var->sqltype & ~1) {
    case SQL_TYPE_TIME:
	_procs->isc_decode_sql_time((ISC_TIME*)_var->sqldata, &tm);
	tm.tm_year = 100;
	tm.tm_mon = 0;
	tm.tm_mday = 1;
	return makeTime(&tm);
    default:
	break;
    }

    qWarning("Sqlda::getTime: invalid type: %d", _var->sqltype);
    return QTime();
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
    return new FirebirdDriver();
}

}
