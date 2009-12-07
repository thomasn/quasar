// $Id: business_object.cpp,v 1.2 2004/11/29 07:09:38 bpepers Exp $
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

#include "business_object.h"

BusinessObject::BusinessObject()
{
    clear();
}

BusinessObject::BusinessObject(QDomElement e)
{
    fromXML(e);
}

void
BusinessObject::fromXML(QDomElement e)
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

	if (tag == "name")
	    name = content;
	else if (tag == "description")
	    description = content;
	else if (tag == "number" || tag == "objectNumber")
	    number = content.toInt();
	else if (tag == "table")
	    tables.push_back(TableDefn(e));
	else
	    qWarning("Unknown business object tag: " + tag);
    }
}

void
BusinessObject::toXML(QDomElement& e) const
{
    QDomDocument doc = e.ownerDocument();

    QDomElement n = doc.createElement("name");
    n.appendChild(doc.createTextNode(name));
    e.appendChild(n);

    QDomElement d = doc.createElement("description");
    d.appendChild(doc.createTextNode(description));
    e.appendChild(d);

    if (number != -1) {
	QDomElement o = doc.createElement("number");
	o.appendChild(doc.createTextNode(QString::number(number)));
	e.appendChild(o);
    }

    for (unsigned int i = 0; i < tables.size(); ++i) {
	QDomElement t = doc.createElement("table");
	tables[i].toXML(t);
	e.appendChild(t);
    }
}

void
BusinessObject::clear()
{
    name = "";
    description = "";
    number = -1;
    tables.clear();
}

bool
BusinessObject::operator==(const BusinessObject& rhs) const
{
    if (rhs.name != name) return false;
    if (rhs.description != description) return false;
    if (rhs.number != number) return false;
    if (rhs.tables != tables) return false;
    return true;
}

bool
BusinessObject::operator!=(const BusinessObject& rhs) const
{
    return !(*this == rhs);
}
