// $Id: db_driver.cpp,v 1.32 2005/03/01 19:48:42 bpepers Exp $
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

#include "db_driver.h"
#include "server_config.h"
#include "table_defn.h"

#include <qdir.h>
#include <qlibrary.h>
#include <qvaluevector.h>
#include <assert.h>

#ifdef WIN32
void libraryError()
{
}
#else
#include <dlfcn.h>
void libraryError()
{
    qWarning("Library load error: %s", dlerror());
}
#endif

static QValueVector<Driver*> drivers;

void
loadDrivers()
{
    static bool loaded = false;
    if (loaded) return;

    ServerConfig config;
    if (!config.load()) {
	qWarning("Error: can't locate database drivers");
	return;
    }

    QString driverDir = parseDir(config.driverDir);

#ifdef WIN32
    QDir dir(driverDir, "*.dll");
#else
    QDir dir(driverDir, "*.so");
#endif

    QStringList entries = dir.entryList();
    for (unsigned int i = 0; i < entries.size(); ++i) {
	QString fileName = entries[i];
	QString filePath = dir.filePath(fileName);

	QLibrary library(filePath);
	if (!library.load()) {
	    libraryError();
	    qWarning("Failed loading: " + filePath);
	    continue;
	}

	GetDriverFunc getDriver = (GetDriverFunc)library.resolve("getDriver");
	if (getDriver == NULL) {
	    qWarning("Missing getDriver symbol in: " + filePath);
	    continue;
	}

	Driver* driver = getDriver();
	library.setAutoUnload(false);
	drivers.push_back(driver);
    }

    loaded = true;
}

Driver::Driver()
{
}

Driver::~Driver()
{
}

QStringList
Driver::types()
{
    QStringList types;

    loadDrivers();
    for (unsigned int i = 0; i < drivers.size(); ++i)
	types.push_back(drivers[i]->name());

    return types;
}

bool
Driver::typeIsAvailable(const QString& type)
{
    return types().contains(type);
}

void
Driver::registerDriver(Driver* driver)
{
    drivers.push_back(driver);
}

Driver*
Driver::getDriver(const QString& type)
{
    loadDrivers();
    for (unsigned int i = 0; i < drivers.size(); ++i)
	if (drivers[i]->name() == type)
	    return drivers[i];
    return NULL;
}

bool
Driver::error(const QString& message)
{
    _lastError = message;
    return false;
}

Connection::Connection()
    : _driver(NULL), _autoCommit(false), _isoLevel(READ_COMMITTED),
      _refCount(0)
{
}

Connection::~Connection()
{
}

Statement*
Connection::allocStatement(const QString& command)
{
    Statement* stmt = allocStatement();
    if (stmt != NULL) stmt->setCommand(command);
    return stmt;
}

bool
Connection::create(const QValueList<TableDefn>& tables)
{
    // First create all the tables
    for (unsigned int i = 0; i < tables.size(); ++i) {
	const TableDefn& table = tables[i];
	if (!create(table))
	    return error("Failed creating table: " + table.name);
    }
    commit();

    // Then add all constraints
    for (unsigned int i = 0; i < tables.size(); ++i) {
	const TableDefn& table = tables[i];
	for (unsigned int j = 0; j < table.constraints.size(); ++j) {
	    const ConstraintDefn& constraint = table.constraints[j];
	    if (!create(constraint))
		return error("Failed creating constraint: " + constraint.name);
	}
    }
    commit();

    // Then create the indexes
    for (unsigned int i = 0; i < tables.size(); ++i) {
	const TableDefn& table = tables[i];
	for (unsigned int j = 0; j < table.indexes.size(); ++j) {
	    const IndexDefn& index = table.indexes[j];
	    if (!create(index))
		return error("Failed creating index: " + index.name);
	}
    }
    commit();

    return true;
}

bool
Connection::error(const QString& message)
{
    _lastError = message;
    return false;
}

Statement::Statement()
    : _conn(NULL), _nextParam(0), _nextColumn(0), _refCount(0)
{
}

Statement::~Statement()
{
}

bool
Statement::error(const QString& message)
{
    _lastError = message;
    return false;
}
