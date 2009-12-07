// $Id: report_page.cpp,v 1.18 2005/01/30 04:16:25 bpepers Exp $
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

#include "report_page.h"

#include <qpainter.h>
#include <qpaintdevicemetrics.h>

ReportPage::ReportPage()
{
}

ReportPage::ReportPage(QDomElement e)
{
    fromXML(e);
}

ReportPage::~ReportPage()
{
}

bool
ReportPage::fromXML(QDomElement root)
{
    elements.clear();

    QDomNodeList nodes = root.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString content = e.text();

	if (tag == "point")
	    elements.push_back(PageElement(e));
	else if (tag == "line")
	    elements.push_back(PageElement(e));
	else if (tag == "rectangle")
	    elements.push_back(PageElement(e));
	else if (tag == "roundRect")
	    elements.push_back(PageElement(e));
	else if (tag == "ellipse")
	    elements.push_back(PageElement(e));
	else if (tag == "polygon")
	    elements.push_back(PageElement(e));
	else if (tag == "image")
	    elements.push_back(PageElement(e));
	else if (tag == "text")
	    elements.push_back(PageElement(e));
	else
	    qWarning("Unknown report element: " + tag);
    }

    return true;
}

bool
ReportPage::toXML(QDomElement& root) const
{
    QDomDocument doc = root.ownerDocument();
    for (unsigned int i = 0; i < elements.size(); ++i) {
	QDomElement e = doc.createElement("element");
	elements[i].toXML(e);
	root.appendChild(e);
    }

    return true;
}

#define DPI_X(x) int((x) * metrics.logicalDpiX() * zoom / 600.0 + .5)
#define DPI_Y(y) int((y) * metrics.logicalDpiY() * zoom / 600.0 + .5)

void
ReportPage::paint(QPainter* p, double zoom) const
{
    QPaintDeviceMetrics metrics(p->device());

    // The elements
    for (unsigned int i = 0; i < elements.size(); ++i) {
	const PageElement& e = elements[i];

	QPen pen = e.pen;
	pen.setWidth(DPI_X(pen.width()));

	p->save();
	if (pen != p->pen())
	    p->setPen(pen);
	if (e.brush != p->brush())
	    p->setBrush(e.brush);
	if (e.backgroundMode != p->backgroundMode())
	    p->setBackgroundMode(Qt::BGMode(e.backgroundMode));
	if (QColor(e.backgroundColor) != p->backgroundColor())
	    p->setBackgroundColor(e.backgroundColor);

	p->translate(DPI_X(e.x), DPI_Y(e.y));
	if (e.scaleX != 1.0 || e.scaleY != 1.0)
	    p->scale(e.scaleX, e.scaleY);
	if (e.shearX != 0.0 || e.shearY != 0.0)
	    p->shear(e.shearX, e.shearY);
	if (e.rotate != 0.0)
	    p->rotate(e.rotate);

	QPointArray points;
	QImage image;
	QFont font;
	int newWidth, newHeight;
	double xScale, yScale, scale;
	int offsetX, offsetY;

	switch (e.type) {
	case PageElement::POINT:
	    p->drawPoint(0, 0);
	    break;
	case PageElement::LINE:
	    p->drawLine(0, 0, DPI_X(e.width), DPI_Y(e.height));
	    break;
	case PageElement::RECTANGLE:
	    p->drawRect(0, 0, DPI_X(e.width), DPI_Y(e.height));
	    break;
	case PageElement::ROUND_RECT:
	    p->drawRoundRect(0, 0, DPI_X(e.width), DPI_Y(e.height),
			     e.roundX, e.roundY);
	    break;
	case PageElement::ELLIPSE:
	    p->drawEllipse(0, 0, DPI_X(e.width), DPI_Y(e.height));
	    break;
	case PageElement::POLYGON:
	    points.resize(e.points.size());
	    for (unsigned int j = 0; j < e.points.size(); ++j) {
		int x = DPI_X(e.points[j].x());
		int y = DPI_Y(e.points[j].y());
		points.setPoint(j, x, y);
	    }
	    p->drawPolygon(points);
	    break;
	case PageElement::IMAGE:
	    xScale = e.width / double(e.image.width());
	    yScale = e.height / double(e.image.height());
	    scale = (xScale < yScale) ? xScale : yScale;
	    newWidth = int(e.image.width() * scale);
	    newHeight = int(e.image.height() * scale);
	    offsetX = (e.width - newWidth) / 2;
	    offsetY = (e.height - newHeight) / 2;
	    if (e.smoothScale)
		image = e.image.smoothScale(DPI_X(newWidth), DPI_Y(newHeight));
	    else
		image = e.image.scale(DPI_X(newWidth), DPI_Y(newHeight));
	    p->drawImage(DPI_X(offsetX), DPI_Y(offsetY), image);
	    break;
	case PageElement::TEXT:
	    font = e.font;
	    if (zoom != 1.0)
		font.setPointSize(int(font.pointSize() * zoom + .5));

	    if (font != p->font())
		p->setFont(font);

	    // Try to make text fit if it doesn't
	    do {
		QRect rect = p->boundingRect(0, 0, DPI_X(e.width),
					     DPI_Y(e.height), e.textFlags,
					     e.text);
		int diffWidth = rect.width() - DPI_X(e.width);
		int diffHeight = rect.height() - DPI_X(e.height);
		if (diffHeight < 2 && diffWidth < 2) break;

		QFont font = p->font();
		int newSize = font.pointSize() - 1;
		if (newSize < 6) break;
		font.setPointSize(newSize);
		p->setFont(font);
	    } while (true);

	    p->drawText(0, 0, DPI_X(e.width), DPI_Y(e.height), e.textFlags,
			e.text);
	    break;
	default:
	    qWarning("Invalid page element type: %d", e.type);
	    break;
	}

	p->restore();
    }
}
