// $Id: column_defn.cpp,v 1.5 2004/12/01 01:16:32 bpepers Exp $
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

#include "column_defn.h"

ColumnDefn::ColumnDefn()
{
    clear();
}

ColumnDefn::ColumnDefn(QDomElement e)
{
    fromXML(e);
}

void
ColumnDefn::fromXML(QDomElement e)
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
	else if (tag == "type")
	    type = typeFromName(content);
	else if (tag == "size")
	    size = content.toInt();
	else if (tag == "decimals")
	    decimals = content.toInt();
	else if (tag == "manditory")
	    manditory = (content.lower() == "yes");
	else if (tag == "unique")
	    unique = (content.lower() == "yes");
	else if (tag == "table")
	    table = content;
	else
	    qWarning("Unknown column tag: " + tag);
    }
}

void
ColumnDefn::toXML(QDomElement& e) const
{
    QDomDocument doc = e.ownerDocument();

    QDomElement nm = doc.createElement("name");
    nm.appendChild(doc.createTextNode(name));
    e.appendChild(nm);

    QDomElement ds = doc.createElement("description");
    ds.appendChild(doc.createTextNode(description));
    e.appendChild(ds);

    QDomElement tp = doc.createElement("type");
    tp.appendChild(doc.createTextNode(typeName()));
    e.appendChild(tp);

    if (size != 0) {
	QDomElement sz = doc.createElement("size");
	sz.appendChild(doc.createTextNode(QString::number(size)));
	e.appendChild(sz);
    }

    if (decimals != 0) {
	QDomElement dc = doc.createElement("decimals");
	dc.appendChild(doc.createTextNode(QString::number(decimals)));
	e.appendChild(dc);
    }

    QDomElement md = doc.createElement("manditory");
    md.appendChild(doc.createTextNode(manditory ? "Yes" : "No"));
    e.appendChild(md);

    QDomElement un = doc.createElement("unique");
    un.appendChild(doc.createTextNode(unique ? "Yes" : "No"));
    e.appendChild(un);

    if (type == TYPE_ID) {
	QDomElement tb = doc.createElement("table");
	tb.appendChild(doc.createTextNode(table));
	e.appendChild(tb);
    }
}

void
ColumnDefn::clear()
{
    name = "";
    description = "";
    type = TYPE_STRING;
    size = 0;
    decimals = 0;
    manditory = true;
    unique = false;
    table = "";
}

QString
ColumnDefn::typeName(int type)
{
    switch (type) {
    case TYPE_CHAR:		return "Character";
    case TYPE_STRING:		return "String";
    case TYPE_DATE:		return "Date";
    case TYPE_TIME:		return "Time";
    case TYPE_BOOL:		return "Boolean";
    case TYPE_INT:		return "Integer";
    case TYPE_BIGINT:		return "Big Integer";
    case TYPE_NUMERIC:		return "Numeric";
    case TYPE_ID:		return "Identifier";
    case TYPE_MONEY:		return "Money";
    case TYPE_PERCENT:		return "Percent";
    case TYPE_QUANTITY:		return "Quantity";
    }
    return "Unknown";
}

int
ColumnDefn::typeFromName(const QString& name)
{
    if (name == "Character") return TYPE_CHAR;
    if (name == "String") return TYPE_STRING;
    if (name == "Date") return TYPE_DATE;
    if (name == "Time") return TYPE_TIME;
    if (name == "Boolean") return TYPE_BOOL;
    if (name == "Integer") return TYPE_INT;
    if (name == "Big Integer") return TYPE_BIGINT;
    if (name == "Numeric") return TYPE_NUMERIC;
    if (name == "Identifier") return TYPE_ID;
    if (name == "Money") return TYPE_MONEY;
    if (name == "Percent") return TYPE_PERCENT;
    if (name == "Quantity") return TYPE_QUANTITY;
    return TYPE_STRING;
}

QString
ColumnDefn::typeName() const
{
    return typeName(type);
}

bool
ColumnDefn::operator==(const ColumnDefn& rhs) const
{
    if (rhs.name != name) return false;
    if (rhs.description != description) return false;
    if (rhs.type != type) return false;
    if (rhs.size != size) return false;
    if (rhs.decimals != decimals) return false;
    if (rhs.manditory != manditory) return false;
    if (rhs.unique != unique) return false;
    if (rhs.table != table) return false;
    return true;
}

bool
ColumnDefn::operator!=(const ColumnDefn& rhs) const
{
    return !(*this == rhs);
}
