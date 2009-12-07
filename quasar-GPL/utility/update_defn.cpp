// $Id: update_defn.cpp,v 1.1 2004/11/27 10:07:43 bpepers Exp $
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

#include "update_defn.h"

UpdateDefn::UpdateDefn()
{
    clear();
}

UpdateDefn::UpdateDefn(QDomElement e)
{
    fromXML(e);
}

void
UpdateDefn::fromXML(QDomElement e)
{
    clear();

    QDomNodeList nodes = e.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString content = e.text();

	if (tag == "number")
	    number = content.toInt();
	else if (tag == "description")
	    description = content;
	else if (tag == "databases")
	    databases = QStringList::split(",", content);
	else if (tag == "updateCmd")
	    updateCmds.push_back(content);
	else if (tag == "restoreCmd")
	    restoreCmds.push_back(content);
	else if (tag == "cleanupCmd")
	    cleanupCmds.push_back(content);
	else
	    qWarning("Unknown update tag: " + tag);
    }
}

void
UpdateDefn::toXML(QDomElement& e) const
{
    QDomDocument doc = e.ownerDocument();

    QDomElement nb = doc.createElement("number");
    nb.appendChild(doc.createTextNode(QString::number(number)));
    e.appendChild(nb);

    QDomElement de = doc.createElement("description");
    de.appendChild(doc.createTextNode(description));
    e.appendChild(de);

    QDomElement db = doc.createElement("databases");
    db.appendChild(doc.createTextNode(databases.join(",")));
    e.appendChild(db);

    for (unsigned int i = 0; i < updateCmds.size(); ++i) {
	QDomElement t = doc.createElement("updateCmd");
	t.appendChild(doc.createTextNode(updateCmds[i]));
	e.appendChild(t);
    }

    for (unsigned int i = 0; i < restoreCmds.size(); ++i) {
	QDomElement t = doc.createElement("restoreCmd");
	t.appendChild(doc.createTextNode(restoreCmds[i]));
	e.appendChild(t);
    }

    for (unsigned int i = 0; i < cleanupCmds.size(); ++i) {
	QDomElement t = doc.createElement("cleanupCmd");
	t.appendChild(doc.createTextNode(cleanupCmds[i]));
	e.appendChild(t);
    }
}

void
UpdateDefn::clear()
{
    number = 1;
    description = "";
    databases.clear();
    updateCmds.clear();
    restoreCmds.clear();
    cleanupCmds.clear();
}

bool
UpdateDefn::operator==(const UpdateDefn& rhs) const
{
    if (rhs.number != number) return false;
    if (rhs.description != description) return false;
    if (rhs.databases != databases) return false;
    if (rhs.updateCmds != updateCmds) return false;
    if (rhs.restoreCmds != restoreCmds) return false;
    if (rhs.cleanupCmds != cleanupCmds) return false;
    return true;
}

bool
UpdateDefn::operator!=(const UpdateDefn& rhs) const
{
    return !(*this == rhs);
}
