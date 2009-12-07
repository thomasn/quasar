// $Id: company_defn.cpp,v 1.9 2005/04/12 08:56:14 bpepers Exp $
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

#include "company_defn.h"
#include "quasar_misc.h"

#include <qdom.h>
#include <qfile.h>

CompanyDefn::CompanyDefn()
{
}

CompanyDefn::CompanyDefn(const QString& filePath)
{
    load(filePath);
}

bool
CompanyDefn::load(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(IO_ReadOnly))
	return error("Failed to open \"" + filePath + "\" for reading");

    QDomDocument doc("QuasarCompany");
    if (!doc.setContent(&file))
	return error("Failed parsing \"" + filePath + "\" as XML");

    QDomElement root = doc.documentElement();
    QDomNodeList nodes = root.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "name")
	    _name = text;
	else if (tag == "version")
	    _version = text;
	else if (tag == "dbtype")
	    _dbtype = text;
	else if (tag == "database")
	    _database = text;
	else
	    qWarning("Unknown company tag: " + tag);
    }

    _filePath = filePath;
    return true;
}

bool
CompanyDefn::save(const QString& filePath, bool overwrite)
{
    if (!overwrite && QFile::exists(filePath))
	return error("File \"" + filePath + "\" already exists");
    if (filePath.isEmpty())
	return error("Blank company file path in save");

    QDomDocument doc("QuasarCompany");
    QDomElement root = doc.createElement("QuasarCompany");

    QDomElement nameNode = doc.createElement("name");
    nameNode.appendChild(doc.createTextNode(_name));
    root.appendChild(nameNode);

    QDomElement versionNode = doc.createElement("version");
    versionNode.appendChild(doc.createTextNode(_version));
    root.appendChild(versionNode);

    QDomElement typeNode = doc.createElement("dbtype");
    typeNode.appendChild(doc.createTextNode(_dbtype));
    root.appendChild(typeNode);

    QDomElement dbNode = doc.createElement("database");
    dbNode.appendChild(doc.createTextNode(_database));
    root.appendChild(dbNode);

    QString xmlInstruction = "version=\"1.0\" encoding=\"UTF-8\"";
    doc.appendChild(doc.createProcessingInstruction("xml", xmlInstruction));
    doc.appendChild(root);

    QFile file(filePath);
    if (!file.open(IO_WriteOnly))
	return error("Failed to open \"" + filePath + "\" for writing");

    QTextStream stream(&file);
    stream << doc.toString();

    file.close();
    setPermissions(filePath, 0640, "root", "quasar");

    return true;
}

bool
CompanyDefn::operator==(const CompanyDefn& rhs) const
{
    if (rhs._name != _name) return false;
    if (rhs._version != _version) return false;
    if (rhs._dbtype != _dbtype) return false;
    if (rhs._database != _database) return false;
    return true;
}

bool
CompanyDefn::operator!=(const CompanyDefn& rhs) const
{
    return !(*this == rhs);
}

bool
CompanyDefn::error(const QString& error)
{
    qWarning("Error: " + error);
    _last_error = error;
    return false;
}
