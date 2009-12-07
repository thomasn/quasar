// $Id: firebird_config.cpp,v 1.8 2005/04/12 08:56:14 bpepers Exp $
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

#include "firebird_config.h"

#include <qdom.h>
#include <qfile.h>
#include <sys/stat.h>

FirebirdConfig::FirebirdConfig()
    : ConfigFile("firebird.cfg")
{
    clear();
}

FirebirdConfig::FirebirdConfig(const QString& configDir)
    : ConfigFile("firebird.cfg", configDir)
{
    clear();
}

FirebirdConfig::~FirebirdConfig()
{
}

void
FirebirdConfig::clear()
{
    hostname = "localhost";
    port = 3050;
    library = "";
    installDir = "";
    dbaPassword = "masterkey";
    username = "quasar";
    password = "quasar";
    databaseDir = "";
    blockSize = 4096;
    charSet = "ISO8859_1";
}

bool
FirebirdConfig::save(bool overwrite)
{
    if (_fileName.isEmpty())
	return error("Blank file name");
    if (configDir().isEmpty())
	return error("Blank config dir for firebird.cfg");
    if (!overwrite && QFile::exists(filePath()))
	return error("File \"" + filePath() + "\" already exists");

    QDomDocument doc("FirebirdConfig");
    QDomElement root = doc.createElement("FirebirdConfig");

    QString xmlInstruction = "version=\"1.0\" encoding=\"UTF-8\"";
    doc.appendChild(doc.createProcessingInstruction("xml", xmlInstruction));
    doc.appendChild(root);

    addNode(doc, "hostname", hostname);
    addNode(doc, "port", QString::number(port));
    addNode(doc, "library", library);
    addNode(doc, "installDir", installDir);
    addNode(doc, "dbaPassword", dbaPassword);
    addNode(doc, "username", username);
    addNode(doc, "password", password);
    addNode(doc, "databaseDir", databaseDir);
    addNode(doc, "blockSize", QString::number(blockSize));
    addNode(doc, "charSet", charSet);

    QFile file(filePath());
    if (!file.open(IO_WriteOnly))
	return error("Failed to open \"" + filePath() + "\" for writing");

    QTextStream stream(&file);
    stream << doc.toString();

    file.close();
    setPermissions(0640, "root", "quasar");

    return true;
}

bool
FirebirdConfig::load(bool showErrors)
{
    QFile file(filePath());
    if (!file.open(IO_ReadOnly)) {
	if (showErrors)
	    error("Failed to open \"" + filePath() + "\" for reading");
	return false;
    }

    QDomDocument doc("FirebirdConfig");
    QString errorMsg;
    int errorLine, errorCol;
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorCol)) {
	QString message = "Failed parsing \"%1\" as XML\n"
	    "    Error on line %2, column %3:\n        %4";
	return error(message.arg(filePath()).arg(errorLine).arg(errorCol)
		     .arg(errorMsg));
    }

    clear();
    QDomElement root = doc.documentElement();
    QDomNodeList nodes = root.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "hostname") hostname = text;
	else if (tag == "port") port = text.toInt();
	else if (tag == "library") library = text;
	else if (tag == "installDir") installDir = text;
	else if (tag == "dbaPassword") dbaPassword = text;
	else if (tag == "username") username = text;
	else if (tag == "password") password = text;
	else if (tag == "databaseDir") databaseDir = text;
	else if (tag == "blockSize") blockSize = text.toInt();
	else if (tag == "charSet") charSet = text;
	else qWarning("Warning: unknown firebird.cfg tag: " + tag);
    }

    return true;
}

bool
FirebirdConfig::operator==(const FirebirdConfig& rhs) const
{
    if (rhs.hostname != hostname) return false;
    if (rhs.port != port) return false;
    if (rhs.library != library) return false;
    if (rhs.installDir != installDir) return false;
    if (rhs.dbaPassword != dbaPassword) return false;
    if (rhs.username != username) return false;
    if (rhs.password != password) return false;
    if (rhs.databaseDir != databaseDir) return false;
    if (rhs.blockSize != blockSize) return false;
    if (rhs.charSet != charSet) return false;
    return true;
}

bool
FirebirdConfig::operator!=(const FirebirdConfig& rhs) const
{
    return !(*this == rhs);
}
