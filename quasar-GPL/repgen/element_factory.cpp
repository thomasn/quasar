// $Id: element_factory.cpp,v 1.5 2005/06/22 23:17:51 bpepers Exp $
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

#include "element_factory.h"
#include "point_element.h"
#include "line_element.h"
#include "rectangle_element.h"
#include "round_rect_element.h"
#include "ellipse_element.h"
#include "polygon_element.h"
#include "image_element.h"
#include "text_element.h"

template
QValueVector<ElementFactory::TypePair> ElementFactory::_types;

static ReportElement*
createPoint() { return new PointElement(); }

static ReportElement*
createLine() { return new LineElement(); }

static ReportElement*
createRectangle() { return new RectangleElement(); }

static ReportElement*
createRoundRect() { return new RoundRectElement(); }

static ReportElement*
createEllipse() { return new EllipseElement(); }

static ReportElement*
createPolygon() { return new PolygonElement(); }

static ReportElement*
createImage() { return new ImageElement(); }

static ReportElement*
createText() { return new TextElement(); }

template <>
void
ElementFactory::addBuiltinTypes()
{
    _types.push_back(TypePair("point", createPoint));
    _types.push_back(TypePair("line", createLine));
    _types.push_back(TypePair("rectangle", createRectangle));
    _types.push_back(TypePair("roundRect", createRoundRect));
    _types.push_back(TypePair("ellipse", createEllipse));
    _types.push_back(TypePair("polygon", createPolygon));
    _types.push_back(TypePair("image", createImage));
    _types.push_back(TypePair("text", createText));
}
