// $Id: report_param.cpp,v 1.10 2004/09/01 20:51:34 bpepers Exp $
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

#include "report_param.h"
#include "data_object.h"

ReportParam::ReportParam()
    : name(""), description(""), type("String"), defaultValue(""),
      manditory(true)
{
}

ReportParam::ReportParam(QDomElement e)
    : name(""), description(""), type("String"), defaultValue(""),
      manditory(true)
{
    fromXML(e);
}

ReportParam::~ReportParam()
{
}

QString
ReportParam::getAttribute(const QString& key) const
{
    return attributes[key];
}

void
ReportParam::setAttribute(const QString& key, const QString& value)
{
    attributes[key] = value;
}

bool
ReportParam::fromXML(QDomElement e)
{
    // Process attributes
    QDomNamedNodeMap attrs = e.attributes();
    for (unsigned int i = 0; i < attrs.length(); ++i) {
	QDomNode attr = attrs.item(i);
	QString key = attr.nodeName();
	QString value = attr.nodeValue();

	if (key == "name")
	    name = value;
	else if (key == "type")
	    type = value;
	else if (key == "description")
	    description = value;
	else if (key == "defaultValue")
	    defaultValue = value;
	else if (key == "manditory")
	    manditory = value.lower() == "true" || value.lower() == "yes";
	else
	    attributes[key] = value;
    }

    return true;
}

bool
ReportParam::toXML(QDomElement& e) const
{
    QDomDocument doc = e.ownerDocument();

    e.setAttribute("name", name);
    e.setAttribute("type", type);
    e.setAttribute("description", description);
    if (!defaultValue.isEmpty())
	e.setAttribute("defaultValue", defaultValue);
    e.setAttribute("manditory", manditory ? "yes" : "no");

    QMap<QString,QString>::ConstIterator it;
    for (it = attributes.begin(); it != attributes.end(); ++it)
	e.setAttribute(it.key(), it.data());

    return true;
}
