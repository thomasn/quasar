// $Id: polygon_element.cpp,v 1.1 2004/08/04 00:11:32 bpepers Exp $
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

#include "polygon_element.h"

PolygonElement::PolygonElement()
{
    clear();
}

PolygonElement::~PolygonElement()
{
}

PolygonElement*
PolygonElement::clone() const
{
    PolygonElement* polygon = new PolygonElement();
    *polygon = *this;
    return polygon;
}

void
PolygonElement::clear()
{
    ReportElement::clear();
    points.clear();
}

bool
PolygonElement::toXML(QDomElement& e) const
{
    QDomDocument doc = e.ownerDocument();

    ReportElement::toXML(e);

    for (unsigned int i = 0; i < points.size(); ++i) {
	QDomElement f = doc.createElement("point");
	f.setAttribute("x", points[i].x());
	f.setAttribute("y", points[i].y());
	e.appendChild(f);
    }

    return true;
}

bool
PolygonElement::processChildNode(QDomElement& e)
{
    QString tag = e.tagName();

    if (tag == "point") {
	QPoint point;
	point.setX(getInt(e, "x", 0));
	point.setY(getInt(e, "y", 0));
	points.push_back(point);
    } else {
	return ReportElement::processChildNode(e);
    }

    return true;
}

void
PolygonElement::generate(ReportInterp* interp, int offsetX, int offsetY,
			 QValueVector<PageElement>& elements)
{
    PageElement element;
    element.type = PageElement::POLYGON;
    setPageElement(element, interp, offsetX, offsetY);

    element.points = points;

    elements.clear();
    elements.push_back(element);
}
