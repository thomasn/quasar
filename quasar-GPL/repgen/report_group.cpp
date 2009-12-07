// $Id: report_group.cpp,v 1.8 2005/01/30 04:16:25 bpepers Exp $
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

#include "report_group.h"
#include "report_interp.h"

ReportGroup::ReportGroup()
    : name(""), expression(""), currentValue("junk foo bar")
{
}

ReportGroup::ReportGroup(QDomElement e)
    : name(""), expression(""), currentValue("junk foo bar")
{
    fromXML(e);
}

ReportGroup::~ReportGroup()
{
}

bool
ReportGroup::fromXML(QDomElement e)
{
    name = e.attribute("name", "");

    QDomNodeList nodes = e.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;

	QString tag = e.tagName();
	if (tag == "expression")
	    expression = e.text();
	else if (tag == "header")
	    headers.push_back(ReportBand(e));
	else if (tag == "footer")
	    footers.push_back(ReportBand(e));
	else
	    qWarning("Unknown report group tag: " + tag);
    }

    return true;
}

bool
ReportGroup::toXML(QDomElement& e) const
{
    QDomDocument doc = e.ownerDocument();

    e.setAttribute("name", name);

    QDomElement f = doc.createElement("expression");
    f.appendChild(doc.createTextNode(expression));
    e.appendChild(f);

    for (unsigned int i = 0; i < headers.size(); ++i) {
	QDomElement f = doc.createElement("header");
	headers[i].toXML(f);
	e.appendChild(f);
    }

    for (unsigned int i = 0; i < footers.size(); ++i) {
	QDomElement f = doc.createElement("footer");
	footers[i].toXML(f);
	e.appendChild(f);
    }

    return true;
}

QVariant
ReportGroup::newValue(ReportInterp* interp)
{
    return interp->evaluate(expression);
}
