// $Id: report_element.cpp,v 1.18 2005/02/09 08:50:45 bpepers Exp $
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

#include "report_element.h"
#include "report_interp.h"
#include "base64.h"

#include <qbuffer.h>
#include <qfile.h>

ReportElement::ReportElement()
{
    clear();
}

ReportElement::~ReportElement()
{
}

ReportElement&
ReportElement::operator=(const ReportElement& rhs)
{
    printWhen = rhs.printWhen;
    pen = rhs.pen;
    brush = rhs.brush;
    backgroundMode = rhs.backgroundMode;
    backgroundColor = rhs.backgroundColor;
    scaleX = rhs.scaleX;
    scaleY = rhs.scaleY;
    shearX = rhs.shearX;
    shearY = rhs.shearY;
    rotate = rhs.rotate;
    x = rhs.x;
    y = rhs.y;
    width = rhs.width;
    height = rhs.height;
    return *this;
}

void
ReportElement::clear()
{
    printWhen = "";
    pen = QPen();
    brush = QBrush();
    backgroundMode = Qt::TransparentMode;
    backgroundColor = "white";
    scaleX = 1.0;
    scaleY = 1.0;
    shearX = 0.0;
    shearY = 0.0;
    rotate = 0.0;
    x = "0";
    y = "0";
    width = "0";
    height = "0";
}

bool
ReportElement::fromXML(QDomElement e)
{
    clear();

    // Process attributes
    QDomNamedNodeMap attrs = e.attributes();
    for (unsigned int i = 0; i < attrs.length(); ++i) {
	QDomNode attr = attrs.item(i);
	QString name = attr.nodeName();
	QString value = attr.nodeValue();

	processAttribute(name, value);
    }

    // Process child nodes
    QDomNodeList nodes = e.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;

	processChildNode(e);
    }

    return true;
}

int
ReportElement::getInt(const QString& name, const QString& value, int defValue)
{
    bool ok;
    int intValue = value.toInt(&ok);
    if (ok) return intValue;

    qWarning("Error: invalid integer value for " + name + ": " + value);
    return defValue;
}

int
ReportElement::getInt(QDomElement e, const QString& name, int defaultValue)
{
    QString value = e.attribute(name).lower();
    if (value.isEmpty()) return defaultValue;
    return getInt(name, value, defaultValue);
}

bool
ReportElement::getBoolean(const QString& name, const QString& value, bool def)
{
    QString text = value.lower();
    if (text == "yes" || text == "true") return true;
    if (text == "no" || text == "false") return false;

    qWarning("Error: invalid boolean value for " + name + ": " + value);
    return def;
}

bool
ReportElement::getBoolean(QDomElement e, const QString& name, bool defValue)
{
    QString value = e.attribute(name).lower();
    if (value.isEmpty()) return defValue;
    return getBoolean(name, value, defValue);
}

bool
ReportElement::processAttribute(const QString& name, const QString& value)
{
    if (name == "x")
	x = value;
    else if (name == "y")
	y = value;
    else if (name == "width")
	width = value;
    else if (name == "height")
	height = value;
    else {
	qWarning("Unknown element attribute: " + name);
	return false;
    }

    return true;
}

bool
ReportElement::processChildNode(QDomElement& e)
{
    QString tag = e.tagName();

    if (tag == "printWhen") {
	printWhen = e.text();
    } else if (tag == "pen") {
	pen.setWidth(getInt(e, "width", 0));
	pen.setColor(e.attribute("color", "black"));

	QString style = e.attribute("style", "solid").lower();
	if (style == "nopen") pen.setStyle(Qt::NoPen);
	else if (style == "solid") pen.setStyle(Qt::SolidLine);
	else if (style == "dash") pen.setStyle(Qt::DashLine);
	else if (style == "dot") pen.setStyle(Qt::DotLine);
	else if (style == "dashdot") pen.setStyle(Qt::DashDotLine);
	else if (style == "dashdotdot") pen.setStyle(Qt::DashDotDotLine);
	else qWarning("Unknown pen style: " + style);

	QString capStyle = e.attribute("capStyle", "flat").lower();
	if (capStyle == "flat") pen.setCapStyle(Qt::FlatCap);
	else if (capStyle == "square") pen.setCapStyle(Qt::SquareCap);
	else if (capStyle == "round") pen.setCapStyle(Qt::RoundCap);
	else qWarning("Unknown pen capStyle: " + capStyle);

	QString joinStyle = e.attribute("joinStyle", "miter").lower();
	if (joinStyle == "miter") pen.setJoinStyle(Qt::MiterJoin);
	else if (joinStyle == "bevel") pen.setJoinStyle(Qt::BevelJoin);
	else if (joinStyle == "round") pen.setJoinStyle(Qt::RoundJoin);
	else qWarning("Unknown pen joinStyle: " + joinStyle);
    } else if (tag == "brush") {
	brush.setColor(e.attribute("color", "black"));

	QString style = e.attribute("style", "solid").lower();
	if (style == "nobrush") brush.setStyle(Qt::NoBrush);
	else if (style == "solid") brush.setStyle(Qt::SolidPattern);
	else if (style == "dense1") brush.setStyle(Qt::Dense1Pattern);
	else if (style == "dense2") brush.setStyle(Qt::Dense2Pattern);
	else if (style == "dense3") brush.setStyle(Qt::Dense3Pattern);
	else if (style == "dense4") brush.setStyle(Qt::Dense4Pattern);
	else if (style == "dense5") brush.setStyle(Qt::Dense5Pattern);
	else if (style == "dense6") brush.setStyle(Qt::Dense6Pattern);
	else if (style == "dense7") brush.setStyle(Qt::Dense7Pattern);
	else if (style == "horizontal") brush.setStyle(Qt::HorPattern);
	else if (style == "vertical") brush.setStyle(Qt::VerPattern);
	else if (style == "cross") brush.setStyle(Qt::CrossPattern);
	else if (style == "bdiag") brush.setStyle(Qt::BDiagPattern);
	else if (style == "fdiag") brush.setStyle(Qt::FDiagPattern);
	else if (style == "cdiag") brush.setStyle(Qt::DiagCrossPattern);
	else qWarning("Unknown brush style: " + style);

	QString file = e.attribute("pixmap");
	if (!file.isEmpty()) {
	    QPixmap pixmap;
	    if (!pixmap.load(file))
		qWarning("Failed loading brush pixmap: " + file);
	    else
		brush.setPixmap(pixmap);
	}
    } else if (tag == "background") {
	backgroundColor = e.attribute("color", "white");
	QString mode = e.attribute("mode", "transparent").lower();
	if (mode == "transparent") backgroundMode = Qt::TransparentMode;
	else if (mode == "opaque") backgroundMode = Qt::OpaqueMode;
	else qWarning("Unknown background mode: " + mode);
    } else if (tag == "scale") {
	scaleX = e.attribute("x", "1").toDouble();
	scaleY = e.attribute("y", "1").toDouble();
    } else if (tag == "shear") {
	shearX = e.attribute("x", "0").toDouble();
	shearY = e.attribute("y", "0").toDouble();
    } else if (tag == "rotate") {
	rotate = e.attribute("angle", "0").toDouble();
    } else {
	qWarning("Unknown element tag: " + tag);
	return false;
    }

    return true;
}

bool
ReportElement::toXML(QDomElement& e) const
{
    QDomDocument doc = e.ownerDocument();

    e.setTagName(type());

    e.setAttribute("x", x);
    e.setAttribute("y", y);
    e.setAttribute("width", width);
    e.setAttribute("height", height);

    if (!printWhen.isEmpty()) {
	QDomElement f = doc.createElement("printWhen");
	f.appendChild(doc.createTextNode(printWhen));
	e.appendChild(f);
    }

    if (pen != QPen()) {
	QString style;
	if (pen.style() == Qt::NoPen) style = "NoPen";
	else if (pen.style() == Qt::DashLine) style = "Dash";
	else if (pen.style() == Qt::DotLine) style = "Dot";
	else if (pen.style() == Qt::DashDotLine) style = "DashDot";
	else if (pen.style() == Qt::DashDotDotLine) style = "DashDotDot";
	else style = "Solid";

	QString capStyle;
	if (pen.capStyle() == Qt::SquareCap) capStyle = "Square";
	else if (pen.capStyle() == Qt::RoundCap) capStyle = "Round";
	else capStyle = "Flat";

	QString joinStyle;
	if (pen.joinStyle() == Qt::BevelJoin) joinStyle = "Bevel";
	else if (pen.joinStyle() == Qt::RoundJoin) joinStyle = "Round";
	else joinStyle = "Miter";

	QDomElement f = doc.createElement("pen");
	f.setAttribute("color", pen.color().name());
	f.setAttribute("width", pen.width());
	f.setAttribute("style", style);
	if (capStyle != "Flat")
	    f.setAttribute("capStyle", capStyle);
	if (joinStyle != "Miter")
	    f.setAttribute("joinStyle", joinStyle);
	e.appendChild(f);
    }

    if (brush != QBrush()) {
	QDomElement f = doc.createElement("brush");

	QString style = "NoBrush";
	switch (brush.style()) {
	case Qt::NoBrush:		style = "NoBrush"; break;
	case Qt::SolidPattern:		style = "Solid"; break;
	case Qt::Dense1Pattern:		style = "Dense1"; break;
	case Qt::Dense2Pattern:		style = "Dense2"; break;
	case Qt::Dense3Pattern:		style = "Dense3"; break;
	case Qt::Dense4Pattern:		style = "Dense4"; break;
	case Qt::Dense5Pattern:		style = "Dense5"; break;
	case Qt::Dense6Pattern:		style = "Dense6"; break;
	case Qt::Dense7Pattern:		style = "Dense7"; break;
	case Qt::HorPattern:		style = "Horizontal"; break;
	case Qt::VerPattern:		style = "Vertical"; break;
	case Qt::CrossPattern:		style = "Cross"; break;
	case Qt::BDiagPattern:		style = "BDiag"; break;
	case Qt::FDiagPattern:		style = "FDiag"; break;
	case Qt::DiagCrossPattern:	style = "CDiag"; break;
	case Qt::CustomPattern:
#if QT_VERSION >= 0x030200
	    QByteArray bytes;
	    QBuffer buffer(bytes);
	    buffer.open(IO_WriteOnly);
	    brush.pixmap()->save(&buffer, "PNG");
	    QString text = base64Encode(bytes);
#else
	    brush.pixmap()->save("/tmp/pixmap.png", "PNG");
	    QFile file("/tmp/pixmap.png");
	    file.open(IO_ReadOnly);
	    QByteArray bytes = file.readAll();
	    QString text = base64Encode(bytes);
	    file.remove();
#endif
	    style = "Custom";
	    QDomElement g = doc.createElement("pixmap");
	    g.appendChild(doc.createTextNode(text));
	    f.appendChild(g);
	}

	f.setAttribute("color", brush.color().name());
	f.setAttribute("style", style);
	e.appendChild(f);
    }

    if (backgroundColor != "white" || backgroundMode != Qt::TransparentMode) {
	QString mode = "Transparent";
	if (backgroundMode == Qt::OpaqueMode)
	    mode = "Opaque";

	QDomElement f = doc.createElement("background");
	f.setAttribute("color", backgroundColor);
	f.setAttribute("mode", mode);
	e.appendChild(f);
    }

    if (scaleX != 1.0 || scaleY != 1.0) {
	QDomElement f = doc.createElement("scale");
	f.setAttribute("x", scaleX);
	f.setAttribute("y", scaleY);
	e.appendChild(f);
    }

    if (shearX != 0.0 || shearY != 0.0) {
	QDomElement f = doc.createElement("shear");
	f.setAttribute("x", shearX);
	f.setAttribute("y", shearY);
	e.appendChild(f);
    }

    if (rotate != 0.0) {
	QDomElement f = doc.createElement("rotate");
	f.setAttribute("angle", rotate);
	e.appendChild(f);
    }

    return true;
}

bool
ReportElement::shouldPrint(ReportInterp* interp) const
{
    if (printWhen.isEmpty()) return true;
    return interp->evalBoolean(printWhen, true);
}

void
ReportElement::copyTo(ReportElement* element) const
{
    element->printWhen = printWhen;
    element->pen = pen;
    element->brush = brush;
    element->backgroundMode = backgroundMode;
    element->backgroundColor = backgroundColor;
    element->scaleX = scaleX;
    element->scaleY = scaleY;
    element->shearX = shearX;
    element->shearY = shearY;
    element->rotate = rotate;
    element->x = x;
    element->y = y;
    element->width = width;
    element->height = height;
}

void
ReportElement::setPageElement(PageElement& element, ReportInterp* interp,
			      int offsetX, int offsetY)
{
    // Convert each value either as number or as expression
    bool ok;
    int realX = x.toInt(&ok);
    if (!ok) realX = interp->evaluate(x).toInt();
    int realY = y.toInt(&ok);
    if (!ok) realY = interp->evaluate(y).toInt();
    int realWidth = width.toInt(&ok);
    if (!ok) realWidth = interp->evaluate(width).toInt();
    int realHeight = height.toInt(&ok);
    if (!ok) realHeight = interp->evaluate(height).toInt();

    element.pen = pen;
    element.brush = brush;
    element.backgroundMode = backgroundMode;
    element.backgroundColor = backgroundColor;
    element.scaleX = scaleX;
    element.scaleY = scaleY;
    element.shearX = shearX;
    element.shearY = shearY;
    element.rotate = rotate;
    element.x = realX + offsetX;
    element.y = realY + offsetY;
    element.width = realWidth;
    element.height = realHeight;
}
