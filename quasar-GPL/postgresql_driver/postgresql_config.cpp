// $Id: postgresql_config.cpp,v 1.7 2005/04/12 08:56:14 bpepers Exp $
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

#include "postgresql_config.h"

#include <qdom.h>
#include <qfile.h>
#include <sys/stat.h>

PostgresqlConfig::PostgresqlConfig()
    : ConfigFile("postgresql.cfg")
{
    clear();
}

PostgresqlConfig::PostgresqlConfig(const QString& configDir)
    : ConfigFile("postgresql.cfg", configDir)
{
    clear();
}

PostgresqlConfig::~PostgresqlConfig()
{
}

void
PostgresqlConfig::clear()
{
    hostname = "";
    port = 5432;
    library = "";
    dbaUsername = "quasar_dba";
    dbaPassword = "quasar";
    username = "quasar";
    password = "";
    charSet = "UNICODE";
}

bool
PostgresqlConfig::save(bool overwrite)
{
    if (_fileName.isEmpty())
	return error("Blank file name");
    if (configDir().isEmpty())
	return error("Blank config dir for postgresql.cfg");
    if (!overwrite && QFile::exists(filePath()))
	return error("File \"" + filePath() + "\" already exists");

    QDomDocument doc("PostgresqlConfig");
    QDomElement root = doc.createElement("PostgresqlConfig");

    QString xmlInstruction = "version=\"1.0\" encoding=\"UTF-8\"";
    doc.appendChild(doc.createProcessingInstruction("xml", xmlInstruction));
    doc.appendChild(root);

    addNode(doc, "hostname", hostname);
    addNode(doc, "port", port == 0 ? "" : QString::number(port));
    addNode(doc, "library", library);
    addNode(doc, "dbaUsername", dbaUsername);
    addNode(doc, "dbaPassword", dbaPassword);
    addNode(doc, "username", username);
    addNode(doc, "password", password);
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
PostgresqlConfig::load(bool showErrors)
{
    QFile file(filePath());
    if (!file.open(IO_ReadOnly)) {
	if (showErrors)
	    error("Failed to open \"" + filePath() + "\" for reading");
	return false;
    }

    QDomDocument doc("PostgresqlConfig");
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
	else if (tag == "dbaUsername") dbaUsername = text;
	else if (tag == "dbaPassword") dbaPassword = text;
	else if (tag == "username") username = text;
	else if (tag == "password") password = text;
	else if (tag == "charSet") charSet = text;
	else qWarning("Warning: unknown postgresql.cfg tag: " + tag);
    }

    return true;
}

bool
PostgresqlConfig::operator==(const PostgresqlConfig& rhs) const
{
    if (rhs.hostname != hostname) return false;
    if (rhs.port != port) return false;
    if (rhs.library != library) return false;
    if (rhs.dbaUsername != dbaUsername) return false;
    if (rhs.dbaPassword != dbaPassword) return false;
    if (rhs.username != username) return false;
    if (rhs.password != password) return false;
    if (rhs.charSet != charSet) return false;
    return true;
}

bool
PostgresqlConfig::operator!=(const PostgresqlConfig& rhs) const
{
    return !(*this == rhs);
}
