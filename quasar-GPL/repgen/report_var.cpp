// $Id: report_var.cpp,v 1.11 2005/01/30 04:16:25 bpepers Exp $
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

#include "report_var.h"
#include "report_interp.h"

ReportVar::ReportVar()
    : name(""), calcExpr(""), resetType(RESET_NONE), resetExpr("")
{
}

ReportVar::ReportVar(QDomElement e)
    : name(""), calcExpr(""), resetType(RESET_NONE), resetExpr("")
{
    fromXML(e);
}

ReportVar::~ReportVar()
{
}

bool
ReportVar::fromXML(QDomElement e)
{
    name = e.attribute("name", "");
    resetGroup = e.attribute("resetGroup", "");

    QString rType = e.attribute("resetType", "None").lower();
    if (rType == "none") resetType = RESET_NONE;
    else if (rType == "report") resetType = RESET_REPORT;
    else if (rType == "page") resetType = RESET_PAGE;
    else if (rType == "group") resetType = RESET_GROUP;
    else qWarning("Unknown report var reset type: " + rType);

    QDomNodeList nodes = e.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "calcExpr")
	    calcExpr = text;
	else if (tag == "resetExpr")
	    resetExpr = text;
	else if (tag == "sum") {
	    // TODO: save sum info so toXML can write it properly
	    resetExpr = "0";
	    calcExpr = text + " != {} ? [expr " + text + " + $var(" +
		name + ")] : $var(" + name + ")";
	} else
	    qWarning("Unknown variable tag: " + tag);
    }

    return true;
}

bool
ReportVar::toXML(QDomElement& e) const
{
    QDomDocument doc = e.ownerDocument();

    e.setAttribute("name", name);

    QDomElement f = doc.createElement("calcExpr");
    f.appendChild(doc.createTextNode(calcExpr));
    e.appendChild(f);

    if (resetType != RESET_NONE) {
	if (resetType == RESET_GROUP)
	    e.setAttribute("resetGroup", resetGroup);

	QString type = "";
	switch (resetType) {
	case RESET_REPORT:	type = "Report"; break;
	case RESET_PAGE:	type = "Page"; break;
	case RESET_GROUP:	type = "Group"; break;
	}

	QDomElement f = doc.createElement("resetType");
	f.appendChild(doc.createTextNode(type));
	e.appendChild(f);

	QDomElement g = doc.createElement("resetExpr");
	g.appendChild(doc.createTextNode(resetExpr));
	e.appendChild(g);
    }

    return true;
}

void
ReportVar::initialize(ReportInterp* interp)
{
    QVariant value = interp->evaluate(resetExpr);
    interp->setVar("var", name, value);
}

void
ReportVar::calculate(ReportInterp* interp)
{
    QVariant value = interp->evaluate(calcExpr);
    interp->setVar("var", name, value);
}
