// $Id: index_defn.cpp,v 1.3 2004/11/27 10:07:43 bpepers Exp $
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

#include "index_defn.h"

IndexDefn::IndexDefn()
{
    clear();
}

IndexDefn::IndexDefn(QDomElement e)
{
    fromXML(e);
}

void
IndexDefn::addColumn(const QString& name, bool ascending)
{
    columns.push_back(IndexColumn(name, ascending));
}

void
IndexDefn::fromXML(QDomElement e)
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
	else if (tag == "column") {
	    QString sorting = e.attribute("sorting", "ascending").lower();
	    IndexColumn column;
	    column.name = content;
	    column.ascending = (sorting == "ascending");
	    columns.push_back(column);
	} else if (tag == "unique")
	    unique = (content.lower() == "yes");
	else
	    qWarning("Unknown index tag: " + tag);
    }
}

void
IndexDefn::toXML(QDomElement& e) const
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

    for (unsigned int i = 0; i < columns.size(); ++i) {
	QDomElement cl = doc.createElement("column");
	cl.appendChild(doc.createTextNode(columns[i].name));
	if (!columns[i].ascending)
	    cl.setAttribute("sorting", "descending");
	e.appendChild(cl);
    }

    QDomElement un = doc.createElement("unique");
    un.appendChild(doc.createTextNode(unique ? "Yes" : "No"));
    e.appendChild(un);
}

void
IndexDefn::clear()
{
    name = "";
    description = "";
    table = "";
    columns.clear();
    unique = false;
}

bool
IndexDefn::operator==(const IndexDefn& rhs) const
{
    if (rhs.name != name) return false;
    if (rhs.description != description) return false;
    if (rhs.table != table) return false;
    if (rhs.columns != columns) return false;
    if (rhs.unique != unique) return false;
    return true;
}

bool
IndexDefn::operator!=(const IndexDefn& rhs) const
{
    return !(*this == rhs);
}

IndexColumn::IndexColumn(const QString& _name, bool _ascending)
    : name(_name), ascending(_ascending)
{
}

bool
IndexColumn::operator==(const IndexColumn& rhs) const
{
    if (rhs.name != name) return false;
    if (rhs.ascending != ascending) return false;
    return true;
}

bool
IndexColumn::operator!=(const IndexColumn& rhs) const
{
    return !(*this == rhs);
}
