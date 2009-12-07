// $Id: client_stmt.cpp,v 1.5 2005/04/05 07:29:01 bpepers Exp $
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

#include "client_stmt.h"

#include <assert.h>
#include <qregexp.h>

ClientStmt::ClientStmt(ClientConn* conn)
    : _conn(conn), _id(-1), _paramCount(0), _currentRow(-1)
{
}

ClientStmt::~ClientStmt()
{
    // Free statement if it was allocated
    if (_id != -1)
	_conn->sendCmd("free_stmt", QString::number(_id));
}

bool
ClientStmt::setCommand(const QString& command)
{
    _command = command;
    _nextParam = 0;
    _params.clear();

    // Convert newlines to spaces
    _command.replace(QRegExp("\n"), " ");

    // If have stmt allocated, set set_stmt and parse result
    if (_id != -1) {
	QStringList args;
	args << QString::number(_id);
	args << _command;

	QStringList results = _conn->sendCmd("set_stmt", args);
	if (results[0] != "set_stmt:")
	    return error("set_stmt failed: " + results[1]);
	_paramCount = results[1].toInt();
    }

    return true;
}

int
ClientStmt::paramCount()
{
    // If no stmt allocated, set alloc_stmt and parse result
    if (_id == -1) {
	QStringList results = _conn->sendCmd("alloc_stmt", _command);
	if (results[0] != "alloc_stmt:")
	    return -1;

	_id = results[1].toInt();
	_paramCount = results[2].toInt();
    }
    return _paramCount;
}

void
ClientStmt::setNull(int param)
{
    if (param == -1) param = _nextParam++;
    setValue(param, Variant());
}

void
ClientStmt::setValue(int param, Variant value)
{
    if (param == -1) param = _nextParam++;
    while (param >= int(_params.size()))
	_params.push_back(Variant());
    _params[param] = value;
}

void
ClientStmt::setString(int param, const QString& value)
{
    if (param == -1) param = _nextParam++;
    setValue(param, value);
}

void
ClientStmt::setDouble(int param, double value)
{
    if (param == -1) param = _nextParam++;
    setValue(param, value);
}

void
ClientStmt::setLong(int param, long value)
{
    if (param == -1) param = _nextParam++;
    setValue(param, int64_t(value));
}

void
ClientStmt::setDate(int param, QDate value)
{
    if (param == -1) param = _nextParam++;
    setValue(param, value);
}

void
ClientStmt::setTime(int param, QTime value)
{
    if (param == -1) param = _nextParam++;
    setValue(param, value);
}

bool
ClientStmt::execute()
{
    // If stmt not allocated, allocate it now
    if (_id == -1) {
	QStringList results = _conn->sendCmd("alloc_stmt", _command);
	if (results[0] != "alloc_stmt:")
	    return error("alloc_stmt failed: " + results[1]);

	_id = results[1].toInt();
	_paramCount = results[2].toInt();
    }

    // Verify parameter count
    if (_paramCount != int(_params.size()))
	return error("Invalid parameter count");

    // Parameters for execute
    TclObject params;
    TclObject nulls;
    for (unsigned int i = 0; i < _params.size(); ++i) {
	Variant value = _params[i];
	params.lappend(value.toString());
	if (value.isNull())
	    nulls.lappend(QString::number(i));
    }

    QStringList args;
    args.push_back(QString::number(_id));
    args.push_back(params.toString());
    args.push_back(nulls.toString());

    QStringList results = _conn->sendCmd("execute", args);
    if (results[0] != "execute:")
	return error("execute failed: " + results[1]);

    _nextParam = 0;
    _nextColumn = 1;
    _updateCount = results[1].toInt();
    _columnCount = results[2].toInt();
    _currentRow = -1;
    _data.clear();

    return true;
}

bool
ClientStmt::next()
{
    if (_id == -1)
	return error("Statement not executed");

    // All data received from server
    if (_currentRow != -1 && _data.size() == 0)
	return false;

    // Do we need to grab more rows to work with?
    bool fetchNeeded = true;
    if (_currentRow != -1) {
	_currentRow++;
	if (_currentRow < int(_data.size()))
	    fetchNeeded = false;
    }

    // Fetch more rows
    if (fetchNeeded) {
	QStringList results = _conn->sendCmd("next", QString::number(_id));
	if (results[0] != "next:")
	    return error("next failed: " + results[1]);

	results.pop_front();

	_data = results;
	_currentRow = 0;

	if (_data.size() == 0) return false;
    }

    TclObject row = _data[_currentRow];
    _values = row[0];
    _nulls = row[1];

    _nextParam = 0;
    _nextColumn = 1;

    return true;
}

bool
ClientStmt::cancel()
{
    // TODO: ignored for now but should it do something?
    return true;
}

int
ClientStmt::getUpdateCount()
{
    return _updateCount;
}

int
ClientStmt::columnCount()
{
    return _columnCount;
}

int
ClientStmt::findColumn(const QString&)
{
    // TODO: fetch column info if needed and find by name
    assert(false);
    return -1;
}

QString
ClientStmt::columnName(int)
{
    // TODO: fetch column info if needed and return name
    assert(false);
    return "";
}

bool
ClientStmt::isNull(int column)
{
    if (column == -1) column = _nextColumn++;
    if (_currentRow < 0 || _currentRow >= int(_data.size()))
	return false;
    if (column < 1 || column > _columnCount)
	return false;

    return _nulls.toStringList().contains(QString::number(column - 1));
}

Variant
ClientStmt::getValue(int column)
{
    if (column == -1) column = _nextColumn++;
    return getString(column);
}

QString
ClientStmt::getString(int column)
{
    if (column == -1) column = _nextColumn++;
    if (_currentRow < 0 || _currentRow >= int(_data.size()))
	return "";
    if (column < 1 || column > _columnCount)
	return "";
    return _values.toStringList()[column - 1];
}

double
ClientStmt::getDouble(int column)
{
    if (column == -1) column = _nextColumn++;
    QString value = getString(column);
    if (value.isEmpty()) return 0.0;
    return value.toDouble();
}

long
ClientStmt::getLong(int column)
{
    if (column == -1) column = _nextColumn++;
    QString value = getString(column);
    if (value.isEmpty()) return 0;
    return value.toLong();
}

QDate
ClientStmt::getDate(int column)
{
    if (column == -1) column = _nextColumn++;
    QString value = getString(column);
    if (value.isEmpty()) return QDate();
    return QDate::fromString(value, Qt::ISODate);
}

QTime
ClientStmt::getTime(int column)
{
    if (column == -1) column = _nextColumn++;
    QString value = getString(column);
    if (value.isEmpty()) return QTime();
    return QTime::fromString(value, Qt::ISODate);
}
