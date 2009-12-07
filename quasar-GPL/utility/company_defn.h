// $Id: company_defn.h,v 1.3 2004/11/27 10:04:49 bpepers Exp $
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
//

#ifndef COMPANY_DEFN_H
#define COMPANY_DEFN_H

// The company definition class is for storage of the information needed
// to open the database by the clients.  They all need a company name and
// database name but the rest of the data depends on the database type.
// For example the Firebird database needs to specify the file path to the
// actual database file.  The company definition is written to a file when
// the company is created by a database and passed on to the clients when
// they need it where they pass it over to the Database class to get an
// actual connection to the company database.

#include <qstring.h>
#include <qstringlist.h>

class CompanyDefn {
public:
    CompanyDefn();
    CompanyDefn(const QString& filePath);

    // Error handling
    QString lastError() const			{ return _last_error; }

    // Get information
    QString name() const			{ return _name; }
    QString version() const			{ return _version; }
    QString dbType() const			{ return _dbtype; }
    QString database() const			{ return _database; }

    // Set information
    void setName(const QString& name)		{ _name = name; }
    void setVersion(const QString& version)	{ _version = version; }
    void setDBType(const QString& type)		{ _dbtype = type; }
    void setDatabase(const QString& database)	{ _database = database; }

    // Load/save from file in XML format
    bool load(const QString& filePath);
    bool save(const QString& filePath, bool overwrite=false);
    QString filePath() const { return _filePath; }

    // Comparisons
    bool operator==(const CompanyDefn& rhs) const;
    bool operator!=(const CompanyDefn& rhs) const;

protected:
    bool error(const QString& error);
    QString _last_error;

    QString _name;
    QString _version;
    QString _dbtype;
    QString _database;
    QString _filePath;
};

#endif // COMPANY_DEFN_H
