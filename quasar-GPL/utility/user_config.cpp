// $Id: user_config.cpp,v 1.17 2005/04/12 07:36:25 bpepers Exp $
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

#include "user_config.h"

#include <qdom.h>
#include <qfile.h>
#include <qdir.h>

QPalette UserConfig::defaultPalette;
QFont UserConfig::defaultFont;

UserConfig::UserConfig()
    : ConfigFile("user.cfg")
{
#ifdef WIN32
    _configDir = QDir::homeDirPath() + "/QuasarConfig";
#else
    _configDir = QDir::homeDirPath() + "/.quasar";
#endif
    clear();
}

UserConfig::~UserConfig()
{
}

void
UserConfig::clear()
{
    changeStyle = false;
    changeColor = false;
    changeFont = false;
    color = defaultPalette.active().background().name();
    font = defaultFont.rawName();
    defaultServer = "localhost";
    defaultUserName = "admin";
    smtpPort = 25;
    handheldDevice = "";
    preferA4 = false;
}

bool
UserConfig::save(bool overwrite)
{
    if (_fileName.isEmpty())
	return error("Blank file name");
    if (configDir().isEmpty())
	return error("Blank config dir for user.cfg");
    if (!overwrite && QFile::exists(filePath()))
	return error("File \"" + filePath() + "\" already exists");

    QDomDocument doc("UserConfig");
    QDomElement root = doc.createElement("UserConfig");

    QString xmlInstruction = "version=\"1.0\" encoding=\"UTF-8\"";
    doc.appendChild(doc.createProcessingInstruction("xml", xmlInstruction));
    doc.appendChild(root);

    addNode(doc, "cacheDir", cacheDir);
    addNode(doc, "changeStyle", changeStyle ? "Yes" : "No");
    addNode(doc, "changeColor", changeColor ? "Yes" : "No");
    addNode(doc, "changeFont", changeFont ? "Yes" : "No");
    addNode(doc, "style", style);
    addNode(doc, "color", color);
    addNode(doc, "font", font);
    addNode(doc, "mainPixmap", mainPixmap);
    addNode(doc, "locale", locale);
    addNode(doc, "defaultServer", defaultServer);
    addNode(doc, "defaultCompany", defaultCompany);
    addNode(doc, "defaultUserName", defaultUserName);
    addNode(doc, "defaultPassword", defaultPassword);
    addNode(doc, "smtpHost", smtpHost);
    addNode(doc, "smtpPort", QString::number(smtpPort));
    addNode(doc, "handheldDevice", handheldDevice);
    addNode(doc, "preferA4", preferA4 ? "Yes" : "No");

    QFile file(filePath());
    if (!file.open(IO_WriteOnly))
	return error("Failed to open \"" + filePath() + "\" for writing");

    QTextStream stream(&file);
    stream << doc.toString();

    file.close();
    setPermissions(0600, NULL, NULL);

    return true;
}

bool
UserConfig::load(bool showErrors)
{
    QFile file(filePath());
    if (!file.open(IO_ReadOnly)) {
	if (showErrors)
	    error("Failed to open \"" + filePath() + "\" for reading");
	return false;
    }

    QDomDocument doc("UserConfig");
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
	bool flag = (text == "Yes");

	if (tag == "cacheDir") cacheDir = text;
	else if (tag == "changeStyle") changeStyle = flag;
	else if (tag == "changeColor") changeColor = flag;
	else if (tag == "changeFont") changeFont = flag;
	else if (tag == "style") style = text;
	else if (tag == "color") color = text;
	else if (tag == "font") font = text;
	else if (tag == "mainPixmap") mainPixmap = text;
	else if (tag == "locale") locale = text;
	else if (tag == "defaultServer") defaultServer = text;
	else if (tag == "defaultCompany") defaultCompany = text;
	else if (tag == "defaultUserName") defaultUserName = text;
	else if (tag == "defaultPassword") defaultPassword = text;
	else if (tag == "smtpHost") smtpHost = text;
	else if (tag == "smtpPort") smtpPort = text.toInt();
	else if (tag == "handheldDevice") handheldDevice = text;
	else if (tag == "preferA4") preferA4 = flag;
	else qWarning("Warning: unknown user.cfg tag: " + tag);
    }

    return true;
}
