// $Id: report_band.cpp,v 1.12 2005/02/09 08:51:10 bpepers Exp $
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

#include "report_band.h"
#include "report_interp.h"
#include "element_factory.h"

ReportBand::ReportBand()
    : height(0), printWhen(""), beforeCode(""), afterCode(""),
      onNewPage(""), newPageAfter(""), onBottom("")
{
}

ReportBand::ReportBand(QDomElement e)
    : height(0), printWhen(""), beforeCode(""), afterCode(""),
      onNewPage(""), newPageAfter(""), onBottom("")
{
    fromXML(e);
}

ReportBand::ReportBand(const ReportBand& band)
    : height(band.height), printWhen(band.printWhen),
      beforeCode(band.beforeCode), afterCode(band.afterCode),
      onNewPage(band.onNewPage), newPageAfter(band.newPageAfter),
      onBottom(band.onBottom)
{
    for (unsigned int i = 0; i < band.elements.size(); ++i)
	elements.push_back(band.elements[i]->clone());
}

ReportBand::~ReportBand()
{
    for (unsigned int i = 0; i < elements.size(); ++i)
	delete elements[i];
}

ReportBand&
ReportBand::operator=(const ReportBand& rhs)
{
    if (&rhs != this) {
	height = rhs.height;
	printWhen = rhs.printWhen;
	beforeCode = rhs.beforeCode;
	afterCode = rhs.afterCode;
	onNewPage = rhs.onNewPage;
	newPageAfter = rhs.newPageAfter;
	onBottom = rhs.onBottom;

	for (unsigned int i = 0; i < elements.size(); ++i)
	    delete elements[i];
	elements.clear();
	for (unsigned int i = 0; i < rhs.elements.size(); ++i)
	    elements.push_back(rhs.elements[i]->clone());
    }
    return *this;
}

bool
ReportBand::fromXML(QDomElement e)
{
    height = e.attribute("height", "0").toInt();

    QDomNodeList nodes = e.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;

	QString tag = e.tagName();
	if (tag == "printWhen")
	    printWhen = e.text();
	else if (tag == "beforeCode")
	    beforeCode = e.text();
	else if (tag == "afterCode")
	    afterCode = e.text();
	else if (tag == "onNewPage")
	    onNewPage = e.text();
	else if (tag == "newPageAfter")
	    newPageAfter = e.text();
	else if (tag == "onBottom")
	    onBottom = e.text();
	else {
	    ReportElement* element = ElementFactory::create(tag);
	    if (element != NULL) {
		element->fromXML(e);
		elements.push_back(element);
	    } else {
		qWarning("Unknown report element: " + tag);
	    }
	}
    }

    return true;
}

bool
ReportBand::toXML(QDomElement& e) const
{
    QDomDocument doc = e.ownerDocument();

    e.setAttribute("height", QString::number(height));

    if (!printWhen.isEmpty()) {
	QDomElement f = doc.createElement("printWhen");
	f.appendChild(doc.createTextNode(printWhen));
	e.appendChild(f);
    }

    if (!beforeCode.isEmpty()) {
	QDomElement f = doc.createElement("beforeCode");
	f.appendChild(doc.createTextNode(beforeCode));
	e.appendChild(f);
    }

    if (!afterCode.isEmpty()) {
	QDomElement f = doc.createElement("afterCode");
	f.appendChild(doc.createTextNode(afterCode));
	e.appendChild(f);
    }

    if (!onNewPage.isEmpty()) {
	QDomElement f = doc.createElement("onNewPage");
	f.appendChild(doc.createTextNode(onNewPage));
	e.appendChild(f);
    }

    if (!newPageAfter.isEmpty()) {
	QDomElement f = doc.createElement("newPageAfter");
	f.appendChild(doc.createTextNode(newPageAfter));
	e.appendChild(f);
    }

    if (!onBottom.isEmpty()) {
	QDomElement f = doc.createElement("onBottom");
	f.appendChild(doc.createTextNode(onBottom));
	e.appendChild(f);
    }

    for (unsigned int i = 0; i < elements.size(); ++i) {
	QDomElement f = doc.createElement("element");
	elements[i]->toXML(f);
	e.appendChild(f);
    }

    return true;
}

bool
ReportBand::shouldPrint(ReportInterp* interp) const
{
    if (printWhen.isEmpty()) return true;
    return interp->evalBoolean(printWhen, true);
}
