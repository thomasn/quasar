// $Id: round_rect_element.cpp,v 1.1 2004/08/04 00:11:32 bpepers Exp $
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

#include "round_rect_element.h"

RoundRectElement::RoundRectElement()
{
}

RoundRectElement::~RoundRectElement()
{
}

RoundRectElement*
RoundRectElement::clone() const
{
    RoundRectElement* rrect = new RoundRectElement();
    *rrect = *this;
    return rrect;
}

void
RoundRectElement::clear()
{
    ReportElement::clear();
    roundX = 25;
    roundY = 25;
}

bool
RoundRectElement::toXML(QDomElement& e) const
{
    ReportElement::toXML(e);

    e.setAttribute("roundX", QString::number(roundX));
    e.setAttribute("roundY", QString::number(roundY));

    return true;
}

bool
RoundRectElement::processAttribute(const QString& name, const QString& value)
{
    if (name == "roundX")
	roundX = getInt(name, value, 0);
    else if (name == "roundY")
	roundY = getInt(name, value, 0);
    else
	return ReportElement::processAttribute(name, value);

    return true;
}

void
RoundRectElement::generate(ReportInterp* interp, int offsetX, int offsetY,
			   QValueVector<PageElement>& elements)
{
    PageElement element;
    element.type = PageElement::ROUND_RECT;
    setPageElement(element, interp, offsetX, offsetY);

    element.roundX = roundX;
    element.roundY = roundY;

    elements.clear();
    elements.push_back(element);
}
