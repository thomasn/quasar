// $Id: constraint_defn.cpp,v 1.4 2004/12/11 01:30:32 bpepers Exp $
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

#include "constraint_defn.h"

ConstraintDefn::ConstraintDefn()
{
    clear();
}

ConstraintDefn::ConstraintDefn(QDomElement e)
{
    fromXML(e);
}

void
ConstraintDefn::fromXML(QDomElement e)
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
	else if (tag == "table")
	    table = content;
	else if (tag == "type")
	    type = typeFromName(content);
	else if (tag == "columns")
	    columns = QStringList::split(",", content);
	else if (tag == "check")
	    check = content;
	else if (tag == "referenceTable")
	    referenceTable = content;
	else if (tag == "referenceColumns")
	    referenceColumns = QStringList::split(",", content);
	else if (tag == "referenceFlags")
	    referenceFlags = content.toInt();
	else
	    qWarning("Unknown constraint tag: " + tag);
    }
}

void
ConstraintDefn::toXML(QDomElement& e) const
{
    QDomDocument doc = e.ownerDocument();

    QDomElement nm = doc.createElement("name");
    nm.appendChild(doc.createTextNode(name));
    e.appendChild(nm);

    QDomElement ds = doc.createElement("description");
    ds.appendChild(doc.createTextNode(description));
    e.appendChild(ds);

    QDomElement tb = doc.createElement("table");
    tb.appendChild(doc.createTextNode(table));
    e.appendChild(tb);

    QDomElement tp = doc.createElement("type");
    tp.appendChild(doc.createTextNode(typeName()));
    e.appendChild(tp);

    if (columns.size() > 0) {
	QDomElement cl = doc.createElement("columns");
	cl.appendChild(doc.createTextNode(columns.join(",")));
	e.appendChild(cl);
    }

    if (!check.isEmpty()) {
	QDomElement ch = doc.createElement("check");
	ch.appendChild(doc.createTextNode(check));
	e.appendChild(ch);
    }

    if (!referenceTable.isEmpty()) {
	QDomElement rt = doc.createElement("referenceTable");
	rt.appendChild(doc.createTextNode(referenceTable));
	e.appendChild(rt);
    }

    if (referenceColumns.size() > 0) {
	QDomElement rc = doc.createElement("referenceColumns");
	rc.appendChild(doc.createTextNode(referenceColumns.join(",")));
	e.appendChild(rc);
    }

    if (referenceFlags != 0) {
	QDomElement rf = doc.createElement("referenceFlags");
	rf.appendChild(doc.createTextNode(QString::number(referenceFlags)));
	e.appendChild(rf);
    }
}

void
ConstraintDefn::clear()
{
    name = "";
    description = "";
    table = "";
    type = TYPE_CHECK;
    columns.clear();
    check = "";
    referenceTable = "";
    referenceColumns.clear();
    referenceFlags = 0;
}

QString
ConstraintDefn::typeName(int type)
{
    switch (type) {
    case TYPE_UNIQUE:		return "Unique";
    case TYPE_CHECK:		return "Check";
    case TYPE_FOREIGN_KEY:	return "Foreign Key";
    }
    return "Unknown";
}

int
ConstraintDefn::typeFromName(const QString& name)
{
    if (name == "Unique") return TYPE_UNIQUE;
    if (name == "Check") return TYPE_CHECK;
    if (name == "Foreign Key") return TYPE_FOREIGN_KEY;
    return TYPE_UNIQUE;
}

QString
ConstraintDefn::typeName() const
{
    return typeName(type);
}

bool
ConstraintDefn::operator==(const ConstraintDefn& rhs) const
{
    if (rhs.name != name) return false;
    if (rhs.description != description) return false;
    if (rhs.table != table) return false;
    if (rhs.type != type) return false;
    if (rhs.columns != columns) return false;
    if (rhs.check != check) return false;
    if (rhs.referenceTable != referenceTable) return false;
    if (rhs.referenceColumns != referenceColumns) return false;
    if (rhs.referenceFlags != referenceFlags) return false;
    return true;
}

bool
ConstraintDefn::operator!=(const ConstraintDefn& rhs) const
{
    return !(*this == rhs);
}
