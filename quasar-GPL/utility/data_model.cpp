// $Id: data_model.cpp,v 1.1 2004/11/27 10:07:43 bpepers Exp $
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

#include "data_model.h"
#include <qfile.h>

DataModel::DataModel()
{
    clear();
}

TableDefnList
DataModel::tables() const
{
    TableDefnList list;
    for (unsigned int i = 0; i < objects.size(); ++i)
	for (unsigned int j = 0; j < objects[i].tables.size(); ++j)
	    list.push_back(objects[i].tables[j]);
    return list;
}

bool
DataModel::load(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(IO_ReadOnly)) {
	qWarning("Failed to open \"" + filePath + "\" for reading");
	return false;
    }

    QDomDocument doc("DataModel");
    QString errorMsg;
    int errorLine, errorCol;
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorCol)) {
	qWarning("Error processing the model file on line %d, "
		 "column %d:\n    " + errorMsg, errorLine, errorCol);
	return false;
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
	QString content = e.text();

	if (tag == "version")
	    version = content;
	else if (tag == "description")
	    description = content;
	else if (tag == "fromVersion")
	    fromVersion = content;
	else if (tag == "businessObject")
	    objects.push_back(BusinessObject(e));
	else if (tag == "update")
	    updates.push_back(UpdateDefn(e));
	else
	    qWarning("Unknown data model tag: " + tag);
    }

    return true;
}

bool
DataModel::save(const QString& filePath) const
{
    QDomDocument doc("DataModel");
    QDomElement root = doc.createElement("DataModel");

    QString xmlInstruction = "version=\"1.0\" encoding=\"UTF-8\"";
    doc.appendChild(doc.createProcessingInstruction("xml", xmlInstruction));
    doc.appendChild(root);

    QDomElement v = doc.createElement("version");
    v.appendChild(doc.createTextNode(version));
    root.appendChild(v);

    QDomElement d = doc.createElement("description");
    d.appendChild(doc.createTextNode(description));
    root.appendChild(d);

    QDomElement f = doc.createElement("fromVersion");
    f.appendChild(doc.createTextNode(fromVersion));
    root.appendChild(f);

    for (unsigned int i = 0; i < objects.size(); ++i) {
	QDomElement e = doc.createElement("businessObject");
	objects[i].toXML(e);
	root.appendChild(e);
    }

    for (unsigned int i = 0; i < updates.size(); ++i) {
	QDomElement e = doc.createElement("update");
	updates[i].toXML(e);
	root.appendChild(e);
    }

    QFile file(filePath);
    if (!file.open(IO_WriteOnly)) {
	qWarning("Failed to open \"" + filePath + "\" for writing");
	return false;
    }

    QTextStream stream(&file);
    stream << doc.toString();

    file.close();
    return true;
}

void
DataModel::clear()
{
    version = "";
    description = "";
    fromVersion = "";
    objects.clear();
    updates.clear();
}

bool
DataModel::operator==(const DataModel& rhs) const
{
    if (rhs.version != version) return false;
    if (rhs.description != description) return false;
    if (rhs.fromVersion != fromVersion) return false;
    if (rhs.objects != objects) return false;
    if (rhs.updates != updates) return false;
    return true;
}

bool
DataModel::operator!=(const DataModel& rhs) const
{
    return !(*this == rhs);
}
