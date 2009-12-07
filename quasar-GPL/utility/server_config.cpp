// $Id: server_config.cpp,v 1.26 2005/04/12 08:56:14 bpepers Exp $
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

#include "server_config.h"

#include <qdom.h>
#include <qfile.h>
#include <sys/stat.h>

ServerConfig::ServerConfig()
    : ConfigFile("server.cfg")
{
    clear();
}

ServerConfig::ServerConfig(const QString& configDir)
    : ConfigFile("server.cfg", configDir)
{
    clear();
}

ServerConfig::~ServerConfig()
{
}

void
ServerConfig::clear()
{
    port = 0;
    systemId = 0;
    adminPassword = "admin";
}

bool
ServerConfig::save(bool overwrite)
{
    if (_fileName.isEmpty())
	return error("Blank file name");
    if (configDir().isEmpty())
	return error("Blank config dir for server.cfg");
    if (!overwrite && QFile::exists(filePath()))
	return error("File \"" + filePath() + "\" already exists");

    QDomDocument doc("ServerConfig");
    QDomElement root = doc.createElement("ServerConfig");

    QString xmlInstruction = "version=\"1.0\" encoding=\"UTF-8\"";
    doc.appendChild(doc.createProcessingInstruction("xml", xmlInstruction));
    doc.appendChild(root);

    addNode(doc, "port", QString::number(port));
    addNode(doc, "systemId", QString::number(systemId));
    addNode(doc, "dataDir", dataDir);
    addNode(doc, "driverDir", driverDir);
    addNode(doc, "backupDir", backupDir);
    addNode(doc, "importDir", importDir);
    addNode(doc, "adminPassword", adminPassword);

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
ServerConfig::load(bool showError)
{
    QFile file(filePath());
    if (!file.open(IO_ReadOnly)) {
	if (showError)
	    error("Failed to open \"" + filePath() + "\" for reading");
	return false;
    }

    QDomDocument doc("ServerConfig");
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

	if (tag == "port") port = text.toInt();
	else if (tag == "systemId") systemId = text.toInt();
	else if (tag == "dataDir") dataDir = text;
	else if (tag == "driverDir") driverDir = text;
	else if (tag == "backupDir") backupDir = text;
	else if (tag == "importDir") importDir = text;
	else if (tag == "adminPassword") adminPassword = text;
	else qWarning("Warning: unknown server.cfg tag: " + tag);
    }

    return true;
}
