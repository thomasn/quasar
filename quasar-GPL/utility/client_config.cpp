// $Id: client_config.cpp,v 1.9 2005/04/12 08:56:14 bpepers Exp $
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

#include "client_config.h"

#include <qdom.h>
#include <qfile.h>
#include <sys/stat.h>

ClientConfig::ClientConfig()
    : ConfigFile("client.cfg")
{
    clear();
}

ClientConfig::ClientConfig(const QString& configDir)
    : ConfigFile("client.cfg", configDir)
{
    clear();
}

ClientConfig::~ClientConfig()
{
}

void
ClientConfig::clear()
{
    localeDir = "";
    stationNumber = "";
}

bool
ClientConfig::save(bool overwrite)
{
    if (_fileName.isEmpty())
	return error("Blank file name");
    if (configDir().isEmpty())
	return error("Blank config dir for client.cfg");
    if (!overwrite && QFile::exists(filePath()))
	return error("File \"" + filePath() + "\" already exists");

    QDomDocument doc("ClientConfig");
    QDomElement root = doc.createElement("ClientConfig");

    QString xmlInstruction = "version=\"1.0\" encoding=\"UTF-8\"";
    doc.appendChild(doc.createProcessingInstruction("xml", xmlInstruction));
    doc.appendChild(root);

    addNode(doc, "localeDir", localeDir);
    addNode(doc, "stationNumber", stationNumber);

    QFile file(filePath());
    if (!file.open(IO_WriteOnly))
	return error("Failed to open \"" + filePath() + "\" for writing");

    QTextStream stream(&file);
    stream << doc.toString();

    file.close();
    setPermissions(0644, "root", "quasar");

    return true;
}

bool
ClientConfig::load(bool showErrors)
{
    QFile file(filePath());
    if (!file.open(IO_ReadOnly)) {
	if (showErrors)
	    error("Failed to open \"" + filePath() + "\" for reading");
	return false;
    }

    QDomDocument doc("ClientConfig");
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

	if (tag == "localeDir") localeDir = text;
	else if (tag == "stationNumber") stationNumber = text;
	else qWarning("Warning: unknown client.cfg tag: " + tag);
    }

    return true;
}
