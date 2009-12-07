// $Id: page_element.cpp,v 1.8 2005/02/09 08:51:30 bpepers Exp $
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

#include "page_element.h"
#include "report_interp.h"
#include "text_format_factory.h"
#include "base64.h"

#include <qbuffer.h>
#include <qfile.h>

PageElement::PageElement()
    : type(POINT), x(0), y(0), width(0), height(0),
      backgroundMode(Qt::TransparentMode), backgroundColor("white"),
      scaleX(1.0), scaleY(1.0), shearX(0.0), shearY(0.0), rotate(0.0),
      roundX(25), roundY(25), textFlags(Qt::AlignLeft | Qt::AlignVCenter),
      text("")
{
}

PageElement::PageElement(QDomElement e)
    : type(POINT), x(0), y(0), width(0), height(0),
      backgroundMode(Qt::TransparentMode), backgroundColor("white"),
      scaleX(1.0), scaleY(1.0), shearX(0.0), shearY(0.0), rotate(0.0),
      roundX(25), roundY(25), textFlags(Qt::AlignLeft | Qt::AlignVCenter),
      text("")
{
    fromXML(e);
}

PageElement::~PageElement()
{
}

static bool
getBoolean(const QString& name, const QString& value, bool defaultValue)
{
    QString text = value.lower();
    if (text == "yes" || text == "true") return true;
    if (text == "no" || text == "false") return false;

    qWarning("Error: invalid boolean value for " + name + ": " + value);
    return defaultValue;
}

static bool
getBoolean(QDomElement e, const QString& name, bool defaultValue)
{
    QString value = e.attribute(name).lower();
    if (value.isEmpty()) return defaultValue;
    return getBoolean(name, value, defaultValue);
}

static int
getInt(const QString& name, const QString& value, int defaultValue)
{
    bool ok;
    int intValue = value.toInt(&ok);
    if (ok) return intValue;

    qWarning("Error: invalid integer value for " + name + ": " + value);
    return defaultValue;
}

static int
getInt(QDomElement e, const QString& name, int defaultValue)
{
    QString value = e.attribute(name).lower();
    if (value.isEmpty()) return defaultValue;
    return getInt(name, value, defaultValue);
}

bool
PageElement::fromXML(QDomElement e)
{
    // Initialize to defaults
    *this = PageElement();

    // Set the type
    if (e.tagName() == "point") type = POINT;
    else if (e.tagName() == "line") type = LINE;
    else if (e.tagName() == "rectangle") type = RECTANGLE;
    else if (e.tagName() == "roundRect") type = ROUND_RECT;
    else if (e.tagName() == "ellipse") type = ELLIPSE;
    else if (e.tagName() == "polygon") type = POLYGON;
    else if (e.tagName() == "image") type = IMAGE;
    else if (e.tagName() == "text") type = TEXT;
    else qWarning("Unknown page element type: " + e.tagName());

    // Process attributes
    QDomNamedNodeMap attrs = e.attributes();
    for (unsigned int i = 0; i < attrs.length(); ++i) {
	QDomNode attr = attrs.item(i);
	QString name = attr.nodeName();
	QString value = attr.nodeValue();

	if (name == "x")
	    x = getInt(name, value, 0);
	else if (name == "y")
	    y = getInt(name, value, 0);
	else if (name == "width")
	    width = getInt(name, value, 0);
	else if (name == "height")
	    height = getInt(name, value, 0);
	else if (name == "roundX" && type == ROUND_RECT)
	    roundX = getInt(name, value, 25);
	else if (name == "roundY" && type == ROUND_RECT)
	    roundY = getInt(name, value, 25);
	else if (name == "halign" && type == TEXT) {
	    QString align = value.lower();
	    if (align == "left") textFlags |= Qt::AlignLeft;
	    else if (align == "center") textFlags |= Qt::AlignHCenter;
	    else if (align == "right") textFlags |= Qt::AlignRight;
	    else qWarning("Unknown halign: " + value);
	} else if (name == "valign" && type == TEXT) {
	    QString align = value.lower();
	    if (align == "top") textFlags |= Qt::AlignTop;
	    else if (align == "center") textFlags = Qt::AlignVCenter;
	    else if (align == "bottom") textFlags = Qt::AlignBottom;
	    else qWarning("Unknown valign: " + value);
	} else {
	    qWarning("Unknown element attribute: " + name);
	}
    }

    QDomNodeList nodes = e.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();

	if (tag == "background") {
	    backgroundColor = e.attribute("color", "white");
	    QString mode = e.attribute("mode", "transparent").lower();
	    if (mode == "transparent") backgroundMode = Qt::TransparentMode;
	    else if (mode == "opaque") backgroundMode = Qt::OpaqueMode;
	    else qWarning("Unknown background mode: " + mode);
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
	    else if (style == "custom") brush.setStyle(Qt::CustomPattern);
	    else qWarning("Unknown brush style: " + style);

	    if (style == "custom") {
		QString text = e.attribute("pixmap");
		QByteArray bytes;
		if (!base64Decode(text, bytes))
		    qWarning("Error: failed decoding brush pixmap");

		QPixmap pixmap;
		if (!pixmap.loadFromData(bytes))
		    qWarning("Failed loading brush pixmap");
		else
		    brush.setPixmap(pixmap);
	    }
	} else if (tag == "image" && type == IMAGE) {
	    QString text = e.attribute("image");
	    QByteArray bytes;
	    if (!base64Decode(text, bytes))
		qWarning("Error: failed decoding image");

	    if (!image.loadFromData(bytes))
		qWarning("Failed loading image");

	    imageDPI = e.attribute("dpi").toInt();
	    smoothScale = e.attribute("scale") == "smooth";
	} else if (tag == "font" && type == TEXT) {
	    font.setFamily(e.attribute("name"));
	    font.setPointSize(getInt(e, "size", 10));
	    font.setBold(getBoolean(e, "isBold", false));
	    font.setItalic(getBoolean(e, "isItalic", false));
	    font.setUnderline(getBoolean(e, "isUnderline", false));
	    font.setStrikeOut(getBoolean(e, "isStrikeOut", false));
	    if (e.hasAttribute("weight"))
		font.setWeight(getInt(e, "weight", 0));
	} else if (tag == "point" && type == POLYGON) {
	    QPoint point;
	    point.setX(getInt(e, "x", 0));
	    point.setY(getInt(e, "y", 0));
	    points.push_back(point);
	} else if (tag == "scale") {
	    scaleX = e.attribute("x", "1").toDouble();
	    scaleY = e.attribute("y", "1").toDouble();
	} else if (tag == "shear") {
	    shearX = e.attribute("x", "0").toDouble();
	    shearY = e.attribute("y", "0").toDouble();
	} else if (tag == "rotate") {
	    rotate = e.attribute("angle", "0").toDouble();
	} else if (tag == "text" && type == TEXT) {
	    text = e.text();
	} else {
	    qWarning("Unknown page element tag: " + tag);
	}
    }

    return true;
}

bool
PageElement::toXML(QDomElement& e) const
{
    QDomDocument doc = e.ownerDocument();

    switch (type) {
    case POINT:		e.setTagName("point"); break;
    case LINE:		e.setTagName("line"); break;
    case RECTANGLE:	e.setTagName("rectangle"); break;
    case ROUND_RECT:	e.setTagName("roundRect"); break;
    case ELLIPSE:	e.setTagName("ellipse"); break;
    case POLYGON:	e.setTagName("polygon"); break;
    case IMAGE:		e.setTagName("image"); break;
    case TEXT:		e.setTagName("text"); break;
    }

    e.setAttribute("x", x);
    e.setAttribute("y", y);
    e.setAttribute("width", width);
    e.setAttribute("height", height);

    if (type == ROUND_RECT) {
	e.setAttribute("roundX", roundX);
	e.setAttribute("roundY", roundY);
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

    if (backgroundColor != "white" || backgroundMode != Qt::TransparentMode) {
	QString mode = "Transparent";
	if (backgroundMode == Qt::OpaqueMode)
	    mode = "Opaque";

	QDomElement f = doc.createElement("background");
	f.setAttribute("color", backgroundColor);
	f.setAttribute("mode", mode);
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

    if (type == IMAGE) {
#if QT_VERSION >= 0x030200
	QByteArray bytes;
	QBuffer buffer(bytes);
	buffer.open(IO_WriteOnly);
	image.save(&buffer, "PNG");
	QString text = base64Encode(bytes);
#else
	image.save("/tmp/image.png", "PNG");
	QFile file("/tmp/image.png");
	file.open(IO_ReadOnly);
	QByteArray bytes = file.readAll();
	QString text = base64Encode(bytes);
	file.remove();
#endif

	QDomElement f = doc.createElement("image");
	f.appendChild(doc.createTextNode(text));
	f.setAttribute("dpi", imageDPI);
	f.setAttribute("scale", smoothScale ? "smooth" : "normal");
	e.appendChild(f);
    }

    if (type == POLYGON) {
	for (unsigned int i = 0; i < points.size(); ++i) {
	    QDomElement f = doc.createElement("point");
	    f.setAttribute("x", points[i].x());
	    f.setAttribute("y", points[i].y());
	    e.appendChild(f);
	}
    }

    if (type == TEXT) {
	if (font != QFont()) {
	    QDomElement f = doc.createElement("font");
	    int weight = font.weight();
	    if (weight != QFont::Normal && weight != QFont::Bold)
		f.setAttribute("weight", weight);
	    if (font.strikeOut()) f.setAttribute("isStrikeOut", "true");
	    if (font.underline()) f.setAttribute("isUnderline", "true");
	    if (font.italic()) f.setAttribute("isItalic", "true");
	    if (font.bold()) f.setAttribute("isBold", "true");
	    f.setAttribute("size", font.pointSize());
	    if (!font.family().isEmpty())
		f.setAttribute("name", font.family());
	    e.appendChild(f);
	}

	QString halign = "Left";
	if (textFlags & Qt::AlignLeft) halign = "Left";
	if (textFlags & Qt::AlignRight) halign = "Right";
	if (textFlags & Qt::AlignHCenter) halign = "Center";
	e.setAttribute("halign", halign);

	QString valign = "Center";
	if (textFlags & Qt::AlignTop) valign = "Top";
	if (textFlags & Qt::AlignBottom) valign = "Bottom";
	if (textFlags & Qt::AlignVCenter) valign = "Center";
	e.setAttribute("valign", valign);

	QDomElement f = doc.createElement("text");
	f.appendChild(doc.createTextNode(text));
	e.appendChild(f);
    }

    return true;
}

static ReportTextFormat*
getFormat(const QString& type)
{
    static QValueVector<ReportTextFormat*> formats;
    for (unsigned int i = 0; i < formats.size(); ++i)
	if (formats[i]->type().lower() == type.lower())
	    return formats[i];

    ReportTextFormat* format = TextFormatFactory::create(type);
    if (format != NULL)
	formats.push_back(format);

    return format;
}

QString
PageElement::evaluate(ReportInterp* interp)
{
    QString value;
    if (!expr.isEmpty())
	value = interp->evaluate(expr).toString();
    else if (!var.isEmpty())
	value = interp->getVar(var).toString();
    else if (!code.isEmpty())
	value = interp->execute(code).toString();

    if (!format.isEmpty()) {
	QString leading, trailing;
	while (value.left(1) == " ") {
	    leading += " ";
	    value = value.mid(1);
	}
	while (value.right(1) == " ") {
	    trailing += " ";
	    value = value.left(value.length() - 1);
	}

	ReportTextFormat* fmt = getFormat(format);
	if (fmt != NULL)
	    value = leading + fmt->format(value) + trailing;
    }

    return value;
}
