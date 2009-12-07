// $Id: label_item.cpp,v 1.8 2004/07/31 19:40:21 bpepers Exp $
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

#include "label_item.h"

#include <qapplication.h>
#include <qdrawutil.h>
#include <qbitmap.h>
#include <qdom.h>
#include <qtextstream.h>
#include <qvaluevector.h>
#include <ctype.h>
#include <stdlib.h>

TextMap TextItem::mapping;

LabelItem::LabelItem()
    : rotate(0.0), scaleX(1.0), scaleY(1.0)
{
    pen.setWidth(1);
}

LabelItem::~LabelItem()
{
}

void
LabelItem::load(QDomNodeList& nodes)
{
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "pen") {
	    QDomNodeList nodes = e.childNodes();
	    loadPen(nodes);
	} else if (tag == "brush") {
	    QDomNodeList nodes = e.childNodes();
	    loadBrush(nodes);
	} else if (tag == "font") {
	    QDomNodeList nodes = e.childNodes();
	    loadFont(nodes);
	} else if (tag == "rotate") {
	    rotate = text.toDouble();
	} else if (tag == "scaleX") {
	    scaleX = text.toDouble();
	} else if (tag == "scaleY") {
	    scaleY = text.toDouble();
	} else if (tag == "scale") {
	    scaleX = scaleY = text.toDouble();
	}
    }
}

void
LabelItem::loadPen(QDomNodeList& nodes)
{
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "style") {
	    if (text == "no pen")
		pen.setStyle(Qt::NoPen);
	    else if (text == "solid")
		pen.setStyle(Qt::SolidLine);
	    else if (text == "dash")
		pen.setStyle(Qt::DashLine);
	    else if (text == "dot")
		pen.setStyle(Qt::DotLine);
	    else if (text == "dash dot")
		pen.setStyle(Qt::DashDotLine);
	    else if (text == "dash dot dot")
		pen.setStyle(Qt::DashDotDotLine);
	    else 
		qWarning("Unknown pen style: " + text);
	} else if (tag == "width") {
	    pen.setWidth(text.toInt());
	} else if (tag == "capStyle") {
	    if (text == "flat")
		pen.setCapStyle(Qt::FlatCap);
	    else if (text == "square")
		pen.setCapStyle(Qt::SquareCap);
	    else if (text == "round")
		pen.setCapStyle(Qt::RoundCap);
	    else
		qWarning("Unknown cap style: " + text);
	} else if (tag == "joinStyle") {
	    if (text == "miter")
		pen.setJoinStyle(Qt::MiterJoin);
	    else if (text == "bevel")
		pen.setJoinStyle(Qt::BevelJoin);
	    else if (text == "round")
		pen.setJoinStyle(Qt::RoundJoin);
	    else
		qWarning("Unknown join style: " + text);
	} else if (tag == "color") {
	    pen.setColor(QColor(text));
	} else {
	    qWarning("Unknown pen tag: " + tag);
	}
    }
}

void
LabelItem::loadBrush(QDomNodeList& nodes)
{
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "style") {
	    if (text == "no brush")
		brush.setStyle(Qt::NoBrush);
	    else if (text == "solid")
		brush.setStyle(Qt::SolidPattern);
	    else if (text == "dense1")
		brush.setStyle(Qt::Dense1Pattern);
	    else if (text == "dense2")
		brush.setStyle(Qt::Dense2Pattern);
	    else if (text == "dense3")
		brush.setStyle(Qt::Dense3Pattern);
	    else if (text == "dense4")
		brush.setStyle(Qt::Dense4Pattern);
	    else if (text == "dense5")
		brush.setStyle(Qt::Dense5Pattern);
	    else if (text == "dense6")
		brush.setStyle(Qt::Dense6Pattern);
	    else if (text == "dense7")
		brush.setStyle(Qt::Dense7Pattern);
	    else if (text == "horizontal")
		brush.setStyle(Qt::HorPattern);
	    else if (text == "vertical")
		brush.setStyle(Qt::VerPattern);
	    else if (text == "cross")
		brush.setStyle(Qt::CrossPattern);
	    else if (text == "bw diag")
		brush.setStyle(Qt::BDiagPattern);
	    else if (text == "fw diag")
		brush.setStyle(Qt::FDiagPattern);
	    else if (text == "diag cross")
		brush.setStyle(Qt::DiagCrossPattern);
	    else
		qWarning("Unknown brush style: " + text);
	} else if (tag == "color") {
	    brush.setStyle(Qt::SolidPattern);
	    brush.setColor(QColor(text));
	} else if (tag == "pixmap") {
	    QPixmap pixmap(text);
	    brush.setStyle(Qt::CustomPattern);
	    brush.setPixmap(pixmap);
	} else {
	    qWarning("Unknown brush tag: " + tag);
	}
    }
}

void
LabelItem::loadFont(QDomNodeList& nodes)
{
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "family")
	    font.setFamily(text);
	else if (tag == "size")
	    font.setPointSize(text.toInt());
	else if (tag == "weight") {
	    if (text == "normal")
	        font.setWeight(QFont::Normal);
	    else if (text == "bold")
	        font.setWeight(QFont::Bold);
	    else
		font.setWeight(text.toInt());
	} else if (tag == "underline")
	    font.setUnderline(text == "Yes");
	else if (tag == "italic")
	    font.setItalic(text == "Yes");
	else if (tag == "strikeout")
	    font.setStrikeOut(text == "Yes");
	else if (tag == "fixed_pitch")
	    font.setFixedPitch(text == "Yes");
	else
	    qWarning("Unknown font tag: " + tag);
    }
}

void
LabelItem::save(QTextStream& stream)
{
    // TODO: output pen, brush, and font
    stream << "  <rotate>" << rotate << "</rotate>\n";
}

void
LabelItem::dpiAdjust(double scaleX, double)
{
    pen.setWidth(int(pen.width() * scaleX));
}

void
LabelItem::draw(QPainter* p)
{
    if (pen != p->pen()) p->setPen(pen);
    if (brush != p->brush()) p->setBrush(brush);
    if (font != p->font()) p->setFont(font);

    if (rotate != 0.0 || scaleX != 1.0 || scaleY != 1.0) {
	p->save();
	if (rotate != 0.0) p->rotate(rotate);
	if (scaleX != 1.0 || scaleY != 1.0) p->scale(scaleX, scaleY);
    }
}

void
LabelItem::cleanup(QPainter* p)
{
    if (rotate != 0.0 || scaleX != 1.0 || scaleY != 1.0) {
	p->restore();
    }
}

LineItem::LineItem()
    : x1(0), y1(0), x2(0), y2(0)
{
}

LineItem::~LineItem()
{
}

void
LineItem::load(QDomNodeList& nodes)
{
    LabelItem::load(nodes);

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "x1")
	    x1 = text.toDouble();
	else if (tag == "y1")
	    y1 = text.toDouble();
	else if (tag == "x2")
	    x2 = text.toDouble();
	else if (tag == "y2")
	    y2 = text.toDouble();
    }
}

void
LineItem::save(QTextStream& stream)
{
    stream << " <line>" << endl;
    stream << "  <x1>" << x1.toString() << "</x1>" << endl;
    stream << "  <y1>" << y1.toString() << "</y1>" << endl;
    stream << "  <x2>" << x2.toString() << "</x2>" << endl;
    stream << "  <y2>" << y2.toString() << "</y2>" << endl;
    LabelItem::save(stream);
    stream << " </line>" << endl;
}

void
LineItem::dpiAdjust(double scaleX, double scaleY)
{
    x1 = x1 * scaleX;
    x2 = x2 * scaleX;
    y1 = y1 * scaleY;
    y2 = y2 * scaleY;
}

void
LineItem::draw(QPainter* p)
{
    LabelItem::draw(p);
    p->drawLine(x1.toInt(), y1.toInt(), x2.toInt(), y2.toInt());
    LabelItem::cleanup(p);
}

RectItem::RectItem()
    : x(0), y(0), width(0), height(0)
{
}

RectItem::~RectItem()
{
}

void
RectItem::load(QDomNodeList& nodes)
{
    LabelItem::load(nodes);

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "x")
	    x = text.toDouble();
	else if (tag == "y")
	    y = text.toDouble();
	else if (tag == "width")
	    width = text.toDouble();
	else if (tag == "height")
	    height = text.toDouble();
    }
}

void
RectItem::save(QTextStream& stream)
{
    stream << " <rect>" << endl;
    stream << "  <x>" << x.toString() << "</x>" << endl;
    stream << "  <y>" << y.toString() << "</y>" << endl;
    stream << "  <width>" << width.toString() << "</width>" << endl;
    stream << "  <height>" << height.toString() << "</height>" << endl;
    LabelItem::save(stream);
    stream << " </rect>" << endl;
}

void
RectItem::dpiAdjust(double scaleX, double scaleY)
{
    x = x * scaleX;
    y = y * scaleY;
    width = width * scaleX;
    height = height * scaleY;
}

void
RectItem::draw(QPainter* p)
{
    LabelItem::draw(p);
    p->drawRect(x.toInt(), y.toInt(), width.toInt(), height.toInt());
    LabelItem::cleanup(p);
}

RoundRectItem::RoundRectItem()
    : xRound(50), yRound(50)
{
}

RoundRectItem::~RoundRectItem()
{
}

void
RoundRectItem::load(QDomNodeList& nodes)
{
    RectItem::load(nodes);

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "xr")
	    xRound = text.toDouble();
	else if (tag == "yr")
	    yRound = text.toDouble();
    }
}

void
RoundRectItem::save(QTextStream& stream)
{
    stream << " <rrect>" << endl;
    stream << "  <x>" << x.toString() << "</x>" << endl;
    stream << "  <y>" << y.toString() << "</y>" << endl;
    stream << "  <width>" << width.toString() << "</width>" << endl;
    stream << "  <height>" << height.toString() << "</height>" << endl;
    stream << "  <xr>" << xRound.toString() << "</xr>" << endl;
    stream << "  <yr>" << yRound.toString() << "</yr>" << endl;
    LabelItem::save(stream);
    stream << " </rrect>" << endl;
}

void
RoundRectItem::draw(QPainter* p)
{
    LabelItem::draw(p);
    p->drawRoundRect(x.toInt(), y.toInt(), width.toInt(), height.toInt(),
		     xRound.toInt(), yRound.toInt());
    LabelItem::cleanup(p);
}

PanelItem::PanelItem()
    : sunken(false), lineWidth(2)
{
    colorGroup = qApp->palette().active();
}

PanelItem::~PanelItem()
{
}

void
PanelItem::load(QDomNodeList& nodes)
{
    RectItem::load(nodes);

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "lineWidth")
	    lineWidth = text.toInt();
	else if (tag == "outline")
	    sunken = (text == "sunken");
	// TODO: color group
	// TODO: fill brush
    }
}

void
PanelItem::save(QTextStream& stream)
{
    stream << " <panel>" << endl;
    stream << "  <x>" << x.toString() << "</x>" << endl;
    stream << "  <y>" << y.toString() << "</y>" << endl;
    stream << "  <width>" << width.toString() << "</width>" << endl;
    stream << "  <height>" << height.toString() << "</height>" << endl;
    if (sunken)
	stream << "  <outline>" << "sunken" << "</outline>" << endl;
    else
	stream << "  <outline>" << "raised" << "</outline>" << endl;
    stream << "  <lineWidth>" << lineWidth << "</lineWidth>" << endl;
    // TODO: color group
    // TODO: fill brush
    LabelItem::save(stream);
    stream << " </panel>" << endl;
}

void
PanelItem::draw(QPainter* p)
{
    LabelItem::draw(p);
    qDrawShadePanel(p, x.toInt(), y.toInt(), width.toInt(), height.toInt(),
		    colorGroup, sunken, lineWidth, NULL);
    LabelItem::cleanup(p);
}

EllipseItem::EllipseItem()
{
}

EllipseItem::~EllipseItem()
{
}

void
EllipseItem::load(QDomNodeList& nodes)
{
    RectItem::load(nodes);
}

void
EllipseItem::save(QTextStream& stream)
{
    stream << " <ellipse>" << endl;
    stream << "  <x>" << x.toString() << "</x>" << endl;
    stream << "  <y>" << y.toString() << "</y>" << endl;
    stream << "  <width>" << width.toString() << "</width>" << endl;
    stream << "  <height>" << height.toString() << "</height>" << endl;
    LabelItem::save(stream);
    stream << " </ellipse>" << endl;
}

void
EllipseItem::draw(QPainter* p)
{
    LabelItem::draw(p);
    p->drawEllipse(x.toInt(), y.toInt(), width.toInt(), height.toInt());
    LabelItem::cleanup(p);
}

ArcItem::ArcItem()
    : angle(0), arcLength(0)
{
}

ArcItem::~ArcItem()
{
}

void
ArcItem::load(QDomNodeList& nodes)
{
    RectItem::load(nodes);

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "angle")
	    angle = text.toDouble();
	else if (tag == "length")
	    arcLength = text.toDouble();
    }
}

void
ArcItem::save(QTextStream& stream)
{
    stream << " <arc>" << endl;
    stream << "  <x>" << x.toString() << "</x>" << endl;
    stream << "  <y>" << y.toString() << "</y>" << endl;
    stream << "  <width>" << width.toString() << "</width>" << endl;
    stream << "  <height>" << height.toString() << "</height>" << endl;
    stream << "  <angle>" << angle.toString() << "</angle>" << endl;
    stream << "  <length>" << arcLength.toString() << "</length>" << endl;
    LabelItem::save(stream);
    stream << " </arc>" << endl;
}

void
ArcItem::draw(QPainter* p)
{
    LabelItem::draw(p);
    p->drawArc(x.toInt(), y.toInt(), width.toInt(), height.toInt(),
	       angle.toInt(), arcLength.toInt());
    LabelItem::cleanup(p);
}

ChordItem::ChordItem()
    : angle(0), arcLength(0)
{
}

ChordItem::~ChordItem()
{
}

void
ChordItem::load(QDomNodeList& nodes)
{
    RectItem::load(nodes);

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "angle")
	    angle = text.toDouble();
	else if (tag == "length")
	    arcLength = text.toDouble();
    }
}

void
ChordItem::save(QTextStream& stream)
{
    stream << " <chord>" << endl;
    stream << "  <x>" << x.toString() << "</x>" << endl;
    stream << "  <y>" << y.toString() << "</y>" << endl;
    stream << "  <width>" << width.toString() << "</width>" << endl;
    stream << "  <height>" << height.toString() << "</height>" << endl;
    stream << "  <angle>" << angle.toString() << "</angle>" << endl;
    stream << "  <length>" << arcLength.toString() << "</length>" << endl;
    LabelItem::save(stream);
    stream << " </chord>" << endl;
}

void
ChordItem::draw(QPainter* p)
{
    LabelItem::draw(p);
    p->drawChord(x.toInt(), y.toInt(), width.toInt(), height.toInt(),
		 angle.toInt(), arcLength.toInt());
    LabelItem::cleanup(p);
}

PolygonItem::PolygonItem()
    : points(16), winding(false)
{
}

PolygonItem::~PolygonItem()
{
}

void
PolygonItem::load(QDomNodeList& nodes)
{
    RectItem::load(nodes);

    unsigned int pos = 0;
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "winding")
	    winding = (text == "Yes");
	else if (tag == "point") {
	    QStringList numbers = QStringList::split(",", text);
	    QPoint point(numbers[0].toInt(), numbers[1].toInt());
	    if (points.size() <= pos) points.resize(points.size() * 2);
	    points.setPoint(pos++, point);
	}
    }
    points.resize(pos);
}

void
PolygonItem::save(QTextStream&)
{
}

void
PolygonItem::dpiAdjust(double scaleX, double scaleY)
{
    RectItem::dpiAdjust(scaleX, scaleY);

    for (unsigned int i = 0; i < points.count(); ++i) {
        QPoint point = points.point(i);
	point.setX(int(point.x() * scaleX));
	point.setY(int(point.y() * scaleY));
	points.setPoint(i, point);
    }
}

void
PolygonItem::draw(QPainter* p)
{
    p->translate(x.toInt(), y.toInt());
    LabelItem::draw(p);
    p->drawPolygon(points, winding);
    LabelItem::cleanup(p);
    p->translate(-x.toInt(), -y.toInt());
}

PixmapItem::PixmapItem()
    : x(0), y(0)
{
}

PixmapItem::~PixmapItem()
{
}

void
PixmapItem::load(QDomNodeList& nodes)
{
    LabelItem::load(nodes);

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "x")
	    x = text.toDouble();
	else if (tag == "y")
	    y = text.toDouble();
	else if (tag == "file")
	    pixmap.load(text);
    }
}

void
PixmapItem::save(QTextStream&)
{
}

void
PixmapItem::dpiAdjust(double scaleX, double scaleY)
{
    x = x * scaleX;
    y = y * scaleY;
}

void
PixmapItem::draw(QPainter* p)
{
    pixmap.setMask(pixmap.createHeuristicMask());

    p->translate(x.toInt(), y.toInt());
    LabelItem::draw(p);
    p->drawPixmap(0, 0, pixmap);
    LabelItem::cleanup(p);
    p->translate(-x.toInt(), -y.toInt());
}

ImageItem::ImageItem()
    : x(0), y(0)
{
}

ImageItem::~ImageItem()
{
}

void
ImageItem::load(QDomNodeList& nodes)
{
    LabelItem::load(nodes);

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "x")
	    x = text.toDouble();
	else if (tag == "y")
	    y = text.toDouble();
	else if (tag == "file")
	    image.load(text);
    }
}

void
ImageItem::save(QTextStream&)
{
}

void
ImageItem::dpiAdjust(double scaleX, double scaleY)
{
    x = x * scaleX;
    y = y * scaleY;
}

void
ImageItem::draw(QPainter* p)
{
    image.setAlphaBuffer(true);

    QPixmap pixmap;
    pixmap.convertFromImage(image);

    QBitmap mask;
    //mask.convertFromImage(image.createAlphaMask());
    mask.convertFromImage(image.createHeuristicMask());
    pixmap.setMask(mask);

    p->translate(x.toInt(), y.toInt());
    LabelItem::draw(p);
    p->drawPixmap(0, 0, pixmap);
    LabelItem::cleanup(p);
    p->translate(-x.toInt(), -y.toInt());
}

TextItem::TextItem()
    : halign(Qt::AlignLeft), valign(Qt::AlignVCenter), wordWrap(false)
{
}

TextItem::~TextItem()
{
}

void
TextItem::load(QDomNodeList& nodes)
{
    RectItem::load(nodes);

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString txt = e.text();

	if (tag == "text") {
	    text = txt;
	} else if (tag == "halign") {
	    if (txt == "left")
		halign = Qt::AlignLeft;
	    else if (txt == "right")
		halign = Qt::AlignRight;
	    else if (txt == "center")
		halign = Qt::AlignHCenter;
	    else
		qWarning("Unknown halign: " + txt);
	} else if (tag == "valign") {
	    if (txt == "top")
		valign = Qt::AlignTop;
	    else if (txt == "bottom")
		valign = Qt::AlignBottom;
	    else if (txt == "center")
		valign = Qt::AlignVCenter;
	    else
		qWarning("Unknown valign: " + txt);
	} else if (tag == "wordWrap") {
	    wordWrap = (text == "On");
	}
    }
}

void
TextItem::save(QTextStream&)
{
}

void
TextItem::draw(QPainter* p)
{
    QString saveText = text;
    if (text.left(1) == "@") text = mapping[text.mid(1)];
    int flags = halign | valign;
    if (wordWrap) flags |= Qt::WordBreak;

    LabelItem::draw(p);
    p->drawText(x.toInt(), y.toInt(), width.toInt(), height.toInt(),
		flags, text);
    LabelItem::cleanup(p);

    text = saveText;
}

BarcodeItem::BarcodeItem()
    : format(AUTO), adjust(1.0)
{
}

BarcodeItem::~BarcodeItem()
{
}

void
BarcodeItem::load(QDomNodeList& nodes)
{
    TextItem::load(nodes);

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "format") {
	    if (text == "auto")
		format = AUTO;
	    else if (text == "upc-a")
		format = UPC_A;
	    else if (text == "upc-e")
		format = UPC_E;
	    else if (text == "ean-13")
		format = EAN13;
	    else if (text == "ean-8")
		format = EAN8;
	    else if (text == "code39")
		format = CODE39;
	    else if (text == "code93")
		format = CODE93;
	    else if (text == "code128")
		format = CODE128;
	    else if (text == "i25")
		format = I25;
	    else if (text == "plessy")
		format = PLESSY;
	    else if (text == "msi")
		format = MSI_PLESSY;
	    else if (text == "codabar")
		format = CODABAR;
	    else
		qWarning("Unknown barcode format: " + text);
	}
    }
}

void
BarcodeItem::save(QTextStream&)
{
}

void
BarcodeItem::dpiAdjust(double scaleX, double scaleY)
{
    TextItem::dpiAdjust(scaleX, scaleY);
    adjust = scaleX;
}

void
BarcodeItem::draw(QPainter* p)
{
    QString saveText = text;
    if (text.left(1) == "@") text = mapping[text.mid(1)];

    QString encoded;
    QStringList numbers;

    int fmt = format;
    if (fmt == AUTO) {
	if (verifyUpcA()) fmt = UPC_A;
	else if (verifyUpcE()) fmt = UPC_E;
	else if (verifyEan13()) fmt = EAN13;
	else if (verifyEan8()) fmt = EAN8;
	else if (verifyCode128()) fmt = CODE128;
	else if (verifyCode39()) fmt = CODE39;
	else if (verifyI25()) fmt = I25;
	else if (verifyCodabar()) fmt = CODABAR;
	else if (verifyPlessy()) fmt = PLESSY;
	else if (verifyMSIPlessy()) fmt = MSI_PLESSY;
	else if (verifyCode93()) fmt = CODE93;
	else return;
    }

    bool good = false;
    switch (fmt) {
    case UPC_A:
	good = encodeUpcA(encoded, numbers);
	if (!good) qWarning("Invalid UPC-A barcode: " + text);
	break;
    case UPC_E:
	good = encodeUpcE(encoded, numbers);
	if (!good) qWarning("Invalid UPC-E barcode: " + text);
	break;
    case EAN13:
	good = encodeEan13(encoded, numbers);
	if (!good) qWarning("Invalid EAN-13 barcode: " + text);
	break;
    case EAN8:
	good = encodeEan8(encoded, numbers);
	if (!good) qWarning("Invalid EAN-8 barcode: " + text);
	break;
    case CODE39:
	good = encodeCode39(encoded, numbers);
	if (!good) qWarning("Invalid Code39 barcode: " + text);
	break;
    case CODE93:
	good = encodeCode93(encoded, numbers);
	if (!good) qWarning("Invalid Code93 barcode: " + text);
	break;
    case CODE128:
	good = encodeCode128(encoded, numbers);
	if (!good) qWarning("Invalid Code128 barcode: " + text);
	break;
    case I25:
	good = encodeI25(encoded, numbers);
	if (!good) qWarning("Invalid Interleave 2 of 5 barcode: " + text);
	break;
    case PLESSY:
	good = encodePlessy(encoded, numbers);
	if (!good) qWarning("Invalid Plessy barcode: " + text);
	break;
    case MSI_PLESSY:
	good = encodeMSIPlessy(encoded, numbers);
	if (!good) qWarning("Invalid MSI-Plessy barcode: " + text);
	break;
    case CODABAR:
	good = encodeCodabar(encoded, numbers);
	if (!good) qWarning("Invalid Codabar barcode: " + text);
	break;
    default:
	qWarning("Unsupported barcode format: %d", format);
    }

    if (!good) {
	text = saveText;
	return;
    }

    p->translate(x.toInt(), y.toInt());
    LabelItem::draw(p);

    int offset = 0;
    unsigned int i;
    for (i = 0; i < encoded.length(); ++i) {
        int barHeight;
	int count;
	if (encoded.at(i).latin1() >= 'a') {
	    barHeight = height.toInt();
	    count = encoded.at(i).latin1() - 'a' + 1;
	} else {
	    barHeight = int(height.toInt() - 4 * adjust);
	    count = encoded.at(i).digitValue();
	}

	int barWidth = count * width.toInt();
	if (i % 2)
	    p->fillRect(offset, 0, barWidth, barHeight, QColor("black"));
	offset += barWidth;
    }

    for (i = 0; i < numbers.count(); ++i) {
	QStringList info = QStringList::split(":", numbers[i]);
	int offset = info[0].toInt();
	int size = info[1].toInt();
	QString text = info[2];

	if (font.pointSize() != size) {
	    font.setPointSize(size);
	    p->setFont(font);
	}

	int x = int(offset * adjust);
	int y = int(height.toInt() + 6 * adjust);
	p->drawText(x, y, text);
    }

    LabelItem::cleanup(p);
    p->translate(-x.toInt(), -y.toInt());

    text = saveText;
}

static int
width_of_partial(const char* partial)
{
    int i = 0;
    while (*partial) {
	if (isdigit(*partial))
	    i += *partial - '0';
	else if (islower(*partial))
	    i += *partial - 'a' + 1;
	partial++;
    }
    return i;
}

static QString
upc_e_to_a0(const QString& text)
{
    QString result;
    switch(text.at(5).latin1()) {
    case '0':
    case '1':
    case '2':
	result = "0" + text.left(2) + text.at(5) + "0000" + text.mid(2, 3);
	break;
    case '3':
	result = "0" + text.left(3) + "00000" + text.mid(3, 2);
	break;
    case '4':
	result = "0" + text.left(4) + "00000" + text.at(4);
	break;
    default:
	result = "0" + text.left(5) + "0000" + text.at(5);
	break;
    }
    return result;
}

static int
ean_make_checksum(const QString& text, int mode)
{
    int length = text.find(' ');
    if (length == -1) length = text.length();
    int esum = 0, osum = 0;
    bool even = true;

    while (length-- > 0) {
	if (even) esum += text.at(length).digitValue();
	else      osum += text.at(length).digitValue();
	even = !even;
    }

    if (!mode) {
	// Standard upc/ean checksum
	int checksum = (3 * esum + osum) % 10;
	return (10 - checksum) % 10;
    }

    // Add-5 checksum
    int checksum = (3 * esum + 9 * osum);
    return checksum % 10;
}

/* Try to expand an UPC-E barcode to its UPC-A equivalent.
 * Accept 6, 7 or 8-digit sequence (not counting the addon):
 *  6:  only the middle part, encoding "0", w/o checksum.
 *  7:  the middle part, encoding "0" with a correct checksum
 *    or
 *      the middle part, encoding "0" or "1" prepended
 *  8:  fully qualified UPC-E with checksum.
 *
 * Returns a 11 digit UPC-A (w/o checksum) for valid EPC-E barcode
 * or an empty string for an invalid one.
 *
 * The checksum for UPC-E is calculated using its UPC-A equivalent.
 */
static QString
upc_e_to_a(const QString& text)
{
    QString result;
    int checksum;

    int length = text.find(' ');
    if (length == -1) length = text.length();

    switch (length) {
    case 6:
	return upc_e_to_a0(text);
    case 7:
	// If starts with 0 or 1, valid barcode without checksum
	if (text.at(0) == '0' || text.at(0) == '1') {
	    result = upc_e_to_a0(text.mid(1));
	    result[0] = text.at(0);
	    return result;
	}

	// Find out whether the 7th char is correct checksum
	result = upc_e_to_a0(text);
	checksum = ean_make_checksum(result, 0);
	if (checksum == (text.at(length - 1).digitValue()))
	    return result;
	break;
    case 8:
	if (text.at(0) == '0' || text.at(0) == '1') {
	    result = upc_e_to_a0(text.mid(1));
	    result[0] = text.at(0);
	    checksum = ean_make_checksum(result, 0);
	    if (checksum == (text.at(length - 1).digitValue()))
		return result;
	}
	break;
    }

    // Invalid representation for UPC-E
    return "";
}

// Accept a 11 or 12 digit UPC-A barcode and shrink it into
// an 8-digit UPC-E equivalent if possible.  Return "" if 
// impossible, the UPC-E barcode if possible.

static QString
upc_a_to_e(const QString& text)
{
    QString result;
    int checksum;

    switch (text.length()) {
    case 12:
	result = text.left(11);
	checksum = ean_make_checksum(result, 0);
	if (checksum != (text.at(11).digitValue()))
	    return "";
	break;
    case 11:
	checksum = ean_make_checksum(text, 0);
	break;
    default:
	return "";
    }

    // UPC-E can only be used with number system 0 or 1
    if (text.at(0) != '0' && text.at(0) != '1')
    	return "";

    result = text.left(1);

    if ((text.at(3) == '0' || text.at(3) == '1' || text.at(3) == '2')
	    && text.mid(4, 4) == "0000") {
	result += text.mid(1, 2) + text.mid(8, 3) + text.at(3);
    } else if (text.mid(4, 5) == "00000") {
	result += text.mid(1, 3) + text.mid(9, 2) + "3";
    } else if (text.mid(5, 5) == "00000") {
	result += text.mid(1, 4) + text.at(10) + "4";
    } else if ((text.at(5) != '0') && text.mid(6,4) == "0000" &&
	       text.at(10) >= '5' && text.at(10) <= '9') {
	result += text.mid(1, 5) + text.at(10);
    } else {
	return "";
    }

    result += checksum + '0';
    return result;
}

bool
BarcodeItem::verifyUpcA()
{
    for (unsigned int i = 0; i < text.length(); ++i)
	if (!text.at(i).isDigit()) return false;

    switch (text.length()) {
    case 11:
	// All 11 digit number are accepted
	break;
    case 12:
	// Check that last digit is valid checksum
	if (text.at(11) != (ean_make_checksum(text.left(11), 0) + '0'))
	    return false;
	break;
    default:
	return false;
    }

    return true;
}

bool
BarcodeItem::encodeUpcA(QString& encoded, QStringList& numbers)
{
    int xpos = 0;

    QStringList digits;
    digits << "3211" << "2221" << "2122" << "1411" << "1132";
    digits << "1231" << "1114" << "1312" << "1213" << "3112";

    // Left side filler with space for outside digit
    encoded += "9a1a";
    xpos += width_of_partial("9a1a");

    // Let side barcode
    unsigned int i;
    for (i = 0; i < 6; ++i) {
	if (!text.at(i).isDigit()) return false;

	encoded += digits[text.at(i).digitValue()];
	if (i == 0) {
	    numbers << "0:10:" + text.at(i);
	} else {
	    numbers << QString::number(xpos) + ":12:" + text.at(i);
	}
	xpos += 7;
    }

    // Middle filler
    encoded += "1a1a1";
    xpos += width_of_partial("1a1a1");

    // Right side barcode
    for (i = 6; i < 12; ++i) {
	if (!text.at(i).isDigit()) return false;

	encoded += digits[text.at(i).digitValue()];
	if (i == 11) {
	    numbers << QString::number(xpos + 13) + ":10:" + text.at(i);
	} else {
	    numbers << QString::number(xpos) + ":12:" + text.at(i);
	}
	xpos += 7;
    }

    // Right side filler
    encoded += "a1a";
    xpos += width_of_partial("a1a");

    return true;
}

bool
BarcodeItem::verifyUpcE()
{
    for (unsigned int i = 0; i < text.length(); ++i)
	if (!text.at(i).isDigit()) return false;

    switch (text.length()) {
    case 6:
    case 7:
    case 8:
	if (upc_e_to_a(text).isEmpty())
	    return false;
	break;
    default:
	return false;
    }

    return true;
}

bool
BarcodeItem::encodeUpcE(QString& encoded, QStringList& numbers)
{
    int xpos = 0;

    QStringList digits;
    digits << "3211" << "2221" << "2122" << "1411" << "1132";
    digits << "1231" << "1114" << "1312" << "1213" << "3112";

    static char* upc_mirrortab[] = {
	"---111","--1-11","--11-1","--111-","-1--11",
	"-11--1","-111--","-1-1-1","-1-11-","-11-1-"
    };
    static char *upc_mirrortab1[] = {
	"111---","11-1--","11--1-","11---1","1-11--",
	"1--11-","1---11","1-1-1-","1-1--1","1--1-1"
    };

    // Convert text
    QString ctext = upc_a_to_e(upc_e_to_a(text));
    int checksum = ctext.at(7).digitValue();

    // Number on left side
    numbers << "0:10:" + ctext.at(0);

    // Left side filler with space for outside digit
    encoded += "9a1a";
    xpos += width_of_partial("9a1a");

    QString mirror;
    if (ctext.at(0) == '0')
	mirror = upc_mirrortab[checksum];
    else
	mirror = upc_mirrortab1[checksum];

    // Barcode contents
    for (unsigned int i = 0; i < 6; ++i) {
	if (!ctext.at(i+1).isDigit()) return false;

	QString bar = digits[ctext.at(i+1).digitValue()];
	if (mirror.at(i) == '1') {
	    encoded += bar;
	} else {
	    encoded += bar[3];
	    encoded += bar[2];
	    encoded += bar[1];
	    encoded += bar[0];
	}

	numbers << QString::number(xpos) + ":12:" + ctext.at(i+1);
	xpos += 7;
    }

    // Number on right side
    numbers << QString::number(xpos + 10) + ":10:" + ctext.at(7);

    // Right side filler
    encoded += "1a1a1a";
    xpos += width_of_partial("1a1a1a");

    return true;
}

bool
BarcodeItem::verifyEan13()
{
    // TODO: support EAN-13
    return false;
}

bool
BarcodeItem::encodeEan13(QString&, QStringList&)
{
    // TODO: support EAN-13
    return false;
}

bool
BarcodeItem::verifyEan8()
{
    // TODO: support EAN-8
    return false;
}

bool
BarcodeItem::encodeEan8(QString&, QStringList&)
{
    // TODO: support EAN-8
    return false;
}

bool
BarcodeItem::verifyCode39()
{
    QString alphabet = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ-. *$/+%";

    if (text.length() == 0) return false;
    for (unsigned int i = 0; i < text.length(); ++i)
	if (alphabet.find(text.at(i), 0, false) == -1)
	    return false;
    return true;
}

bool
BarcodeItem::encodeCode39(QString& encoded, QStringList& numbers)
{
    int checksum = 0;
    int numPos = 22;

    QString alphabet = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ-. *$/+%";
    QString checkbet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%";

    QStringList bars;
    bars << "31113" << "13113" << "33111" << "11313" << "31311";
    bars << "13311" << "11133" << "31131" << "13131" << "11331";

    QStringList spaces;
    spaces << "1311" << "1131" << "1113" << "3111";

    QStringList specialBars;
    specialBars << "11111" << "11111" << "11111" << "11111";

    QStringList specialSpaces;
    specialSpaces << "3331" << "3313" << "3133" << "1333";

    // Left side filler
    encoded += "0a3a1c1c1a";

    // Barcode contents
    for (unsigned int i = 0; i < text.length(); ++i) {
	int code = alphabet.find(text.at(i), 0, false);
	if (code == -1) return false;

	QString b,s;
	if (code < 40) {
	    b = bars[code % 10];
	    s = spaces[code / 10];
	} else {
	    b = specialBars[code - 40];
	    s = specialSpaces[code - 40];
	}
	encoded += "1" + b[0] + s[0] + b[1] + s[1] + b[2] + s[2] +
	    b[3] + s[3] + b[4];

	int checkCode = checkbet.find(alphabet[code]);
	if (checkCode != -1)
	    checksum += checkCode;

	numbers << QString::number(numPos) + ":12:" + text.at(i).upper();
	numPos += 16;
    }

    // Checksum
    int code = alphabet.find(checkbet[checksum % 43]);
    QString b,s;
    if (code < 40) {
	b = bars[code % 10];
	s = spaces[code / 10];
    } else {
	b = specialBars[code - 40];
	s = specialSpaces[code - 40];
    }
    encoded += "1" + b[0] + s[0] + b[1] + s[1] + b[2] + s[2] +
	b[3] + s[3] + b[4];

    // Right side filler
    encoded += "1a3a1c1c1a";

    return true;
}

bool
BarcodeItem::verifyCode93()
{
    if (text.length() == 0) return false;
    for (unsigned int i = 0; i < text.length(); ++i) {
	unsigned char ch = text.at(i).latin1();
	if (ch > 127)
	    return false;
    }
    return true;
}

bool
BarcodeItem::encodeCode93(QString& encoded, QStringList& numbers)
{
    QValueVector<int> checksums;
    int numPos = 22;

    QString alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%";

    QStringList bars;
    bars << "131112" << "111213" << "111312" << "111411" << "121113";
    bars << "121212" << "121311" << "111114" << "131211" << "141111";
    bars << "211113" << "211212" << "211311" << "221112" << "221211";
    bars << "231111" << "112113" << "112212" << "112311" << "122112";
    bars << "132111" << "111123" << "111222" << "111321" << "121122";
    bars << "131121" << "212112" << "212211" << "211122" << "211221";
    bars << "221121" << "222111" << "112122" << "112221" << "122121";
    bars << "123111" << "121131" << "311112" << "311211" << "321111";
    bars << "112131" << "113121" << "211131" << "121221" << "312111";
    bars << "311121" << "122211" << "111141";

    const char* shiftset1 =
	"%$$$$$$$$$$$$$$$$$$$$$$$$$$%%%%%"
	" ///  ///// /             /%%%%%"
	"%                          %%%%%"
	"%++++++++++++++++++++++++++%%%%%";
    const char* shiftset2 =
	"UABCDEFGHIJKLMNOPQRSTUVWXYZABCDE"
	" ABC  FGHIJ L             ZFGHIJ"
	"V                          KLMNO"
	"WABCDEFGHIJLKMNOPQRSTUVWXYZPQRST";

    int START_STOP = 47;
    int EXTEND_DOLLAR = 43;
    int EXTEND_PERCENT = 44;
    int EXTEND_SLASH = 45;
    int EXTEND_PLUS = 46;

    // Left side filler
    encoded += "0" + bars[START_STOP];

    // Barcode contents
    unsigned int i;
    for (i = 0; i < text.length(); ++i) {
	int code = alphabet.find(text.at(i));
	if (code == -1) {
	    switch (shiftset1[text.at(i).latin1()]) {
	    case '$':	code = EXTEND_DOLLAR; break;
	    case '%':	code = EXTEND_PERCENT; break;
	    case '/':	code = EXTEND_SLASH; break;
	    case '+':	code = EXTEND_PLUS; break;
	    default:	code = 0; break;
	    }
	    encoded += bars[code];
	    checksums.push_back(code);

	    code = alphabet.find(shiftset2[text.at(i).latin1()]);
	    encoded += bars[code];
	    checksums.push_back(code);
	} else {
	    encoded += bars[code];
	    checksums.push_back(code);
	}

	numbers << QString::number(numPos) + ":12:" + text.at(i);
	numPos += 9;
    }

    // Checksum
    int c_checksum = 0;
    int k_checksum = 0;
    for (i = 1; i <= checksums.size(); ++i) {
	c_checksum += i * int(checksums[checksums.size() - i]);
	k_checksum += (i+1) * int(checksums[checksums.size() - i]);
    }
    c_checksum = c_checksum % 47;
    k_checksum += c_checksum;
    k_checksum = k_checksum % 47;
    encoded += bars[c_checksum] + bars[k_checksum];

    // Right side filler
    encoded += bars[START_STOP] + "1";

    return true;
}

bool
BarcodeItem::verifyCode128()
{
    if (text.length() == 0) return false;
    for (unsigned int i = 0; i < text.length(); ++i) {
	unsigned char ch = text.at(i).latin1();
	if (ch > 0x80 && (ch < 0xc1 || ch > 0xc4))
	    return false;
    }
    return true;
}

#define NEED_CODE_A(c) ((c)<32 || (c)==0x80) 
#define NEED_CODE_B(c) ((c)>=96 && (c)<128)

static int
Barcode_a_or_b(const QString& text)
{
    for (unsigned int i = 0; i < text.length(); ++i) {
	unsigned char ch = text.at(i).latin1();
	if (NEED_CODE_A(ch))
	    return 'A';
	if (NEED_CODE_B(ch))
	    return 'B';
    }
    return 0; /* any */
}

// Code is either 'A' or 'B', and value must be valid
static int
Barcode_encode_as(int code, QChar ch)
{
    int value = ch.latin1();

    int CODE_A = 101; /* only B and C */
    int CODE_B = 100; /* only A and C */
    int FUNC_1 = 102; /* all of them */
    int FUNC_2 =  97; /* only A and B */
    int FUNC_3 =  96; /* only A and B */
    // FUNC_4 is CODE_A when in A and CODE_B when in B

    // First check the special chars
    if (value == 0xC1) return FUNC_1;
    if (value == 0xC2) return FUNC_2;
    if (value == 0xC3) return FUNC_3;
    if (value == 0xC4) { /* F4 */
	if (code == 'A') return CODE_A;
	return CODE_B;
    }

    /* then check ascii values */
    if (value >= 0x20 && value <= 0x5F)
	return value - 0x20; /* both codes */
    if (value == 0x80) return 64; /* code A */
    if (value < 0x20) return value+64; /* code A */
    if (value >= 0x60) return value - 0x20; /* code B */
    /* can't happen */
    return -1;
}

static void
Barcode_128_make_array(const QString& text, QValueVector<int>& codes)
{
    int code;
    unsigned int i, cnt;

    int CODE_A = 101; /* only B and C */
    int CODE_B = 100; /* only A and C */
    int CODE_C =  99; /* only A and B */
    int FUNC_1 = 102; /* all of them */
    int START_A = 103;
    int STOP    = 106;
    int SHIFT   =  98; /* only A and B */

    // Choose the starting code
    if (text.at(2) == '\0' && text.at(0).isDigit() && text.at(1).isDigit()) {
	code = 'C';
    } else if (text.at(0).isDigit() && text.at(1).isDigit() && text.at(2).isDigit()
	       && text.at(3).isDigit()) {
	code = 'C';
    } else {
	code = Barcode_a_or_b(text);
	if (!code) code = 'B';
    }
    codes.push_back(START_A + code - 'A');

    for (i = 0; i < text.length(); ) {
	unsigned char ch = text.at(i).latin1();

	switch (code) {
	case 'C':
	    if (ch == 0xC1) {
		codes.push_back(FUNC_1);
		++i;
	    } else if (text.at(i).isDigit() && text.at(i+1).isDigit()) {
		codes.push_back(text.at(i).digitValue() * 10 +
				text.at(i+1).digitValue());
		i += 2;
	    } else {
		code = Barcode_a_or_b(text.mid(i));
		if (!code) code = 'B';
		codes.push_back((code == 'A') ? CODE_A : CODE_B);
	    }
	    break;
	case 'B':
	case 'A':
	    for (cnt = 0; text.at(i+cnt).isDigit(); ++cnt)
		;
	    if (cnt >= 4) {
		// If there are 4 or more digits, turn to C
		if (cnt & 1) {
		    // Odd number: encode one first
		    codes.push_back(text.at(i++).latin1() - ' ');
		}
		codes.push_back(CODE_C);
		code = 'C';
	    } else if (code == 'A' && NEED_CODE_B(ch)) {
		// Check whether we should use SHIFT or change code
		if (Barcode_a_or_b(text.mid(i+1)) == 'B') {
		    codes.push_back(CODE_B);
		    code = 'B';
		} else {
		    codes.push_back(SHIFT);
		    codes.push_back(Barcode_encode_as('B', text.at(i++)));
		}
	    } else if (code == 'B' && NEED_CODE_A(ch)) {
		// Check whether we should use SHIFT or change code
		if (Barcode_a_or_b(text.mid(i+1)) == 'A') {
		    codes.push_back(CODE_A);
		    code = 'A';
		} else {
		    codes.push_back(SHIFT);
		    codes.push_back(Barcode_encode_as('A', text.at(i++)));
		}
	    } else {
		codes.push_back(Barcode_encode_as(code, text.at(i++)));
	    }
	    break;
	}
    }

    // Add the checksum
    int checksum = codes[0];
    for (i = 1; i < codes.size(); ++i)
	checksum += i * codes[i];
    checksum %= 103;
    codes.push_back(checksum);
    codes.push_back(STOP);
}

bool
BarcodeItem::encodeCode128(QString& encoded, QStringList& numbers)
{
    static char* codeset[] = {
	"212222", "222122", "222221", "121223", "121322",  /*  0 -  4 */
	"131222", "122213", "122312", "132212", "221213",
	"221312", "231212", "112232", "122132", "122231",  /* 10 - 14 */
	"113222", "123122", "123221", "223211", "221132",
	"221231", "213212", "223112", "312131", "311222",  /* 20 - 24 */
	"321122", "321221", "312212", "322112", "322211",
	"212123", "212321", "232121", "111323", "131123",  /* 30 - 34 */
	"131321", "112313", "132113", "132311", "211313",
	"231113", "231311", "112133", "112331", "132131",  /* 40 - 44 */
	"113123", "113321", "133121", "313121", "211331",
	"231131", "213113", "213311", "213131", "311123",  /* 50 - 54 */
	"311321", "331121", "312113", "312311", "332111",
	"314111", "221411", "431111", "111224", "111422",  /* 60 - 64 */
	"121124", "121421", "141122", "141221", "112214",
	"112412", "122114", "122411", "142112", "142211",  /* 70 - 74 */
	"241211", "221114", "413111", "241112", "134111",
	"111242", "121142", "121241", "114212", "124112",  /* 80 - 84 */
	"124211", "411212", "421112", "421211", "212141",
	"214121", "412121", "111143", "111341", "131141",  /* 90 - 94 */
	"114113", "114311", "411113", "411311", "113141",
	"114131", "311141", "411131", "b1a4a2", "b1a2a4",  /* 100 - 104 */
	"b1a2c2", "b3c1a1b"
    };

    int SYMBOL_WID = 11;	// All of them are 11-bars

    QValueVector<int> codes;
    Barcode_128_make_array(text, codes);

    encoded = "0";
    unsigned int i;
    for (i = 0; i < codes.size(); ++i)
	encoded += QString(codeset[codes[i]]);

    for (i = 0; i < encoded.length(); ++i)
	if (encoded.at(i).isLetter())
	    encoded.at(i) = '1' + encoded.at(i).latin1() - 'a';

    double size = int(180.0 * (codes.size()-3) / text.length() + .5) / 10.0;
    if (size > 12.0) size = 12.0;
    double step = int(10 * (size / 18.0 * SYMBOL_WID) + .5) / 10.0;

    double x = SYMBOL_WID;
    for (i = 0; i < text.length(); ++i) {
	unsigned char ch = text.at(i).latin1();
	if (ch < 32 || ch == 0x80) ch = '_';
	if (ch > 0xc0) ch = ' ';
	numbers << QString::number(int(x)) + ":" + QString::number(int(size))
	    + ":" + QChar(ch);
	x += step;
    }

    return true;
}

bool
BarcodeItem::verifyI25()
{
    if (text.length() == 0) return false;
    for (unsigned int i = 0; i < text.length(); ++i)
	if (!text.at(i).isDigit()) return false;
    return true;
}

bool
BarcodeItem::encodeI25(QString& encoded, QStringList& numbers)
{
    int numPos = 16;

    QStringList codes;
    codes << "11331" << "31113" << "13113" << "33111" << "11313";
    codes << "31311" << "13311" << "11133" << "31131" << "13131";

    // Add a leading if needed
    QString ctext = text;
    if ((ctext.length() + 1) % 2)
	ctext = "0" + ctext;

    // Add the checksum
    int sum[2];
    sum[0] = sum[1] = 0;
    unsigned int i;
    for (i = 0; i < ctext.length(); ++i)
	sum[i%2] += ctext.at(i).digitValue();
    int checksum = sum[0] * 3 + sum[1];
    checksum = (10 - (checksum % 10)) % 10;
    ctext += '0' + checksum;

    // Left side filler
    encoded += "0a1a1";
    numPos = 4;

    // Barcode contents
    for (i = 0; i < ctext.length(); i += 2) {
	if (!ctext.at(i).isDigit() || !ctext.at(i+1).isDigit())
	    return false;

	QString p1 = codes[ctext.at(i).digitValue()];
	QString p2 = codes[ctext.at(i+1).digitValue()];
	for (unsigned int j = 0; j < p1.length(); ++j) {
	    encoded += p1.at(j);
	    encoded += p2.at(j);
	}

	if (ctext.mid(i) == 2)
	    numbers << QString::number(numPos) + ":12:" + ctext.at(i);
	else
	    numbers << QString::number(numPos) + ":12:" + ctext.at(i)
		    << QString::number(numPos+9) + ":12:" + ctext.at(i+1);
	numPos += 18;
    }

    // Right side filler
    encoded += "c1a";

    return true;
}

bool
BarcodeItem::verifyPlessy()
{
    QString alphabet = "0123456789ABCDEF";

    if (text.length() == 0) return false;
    for (unsigned int i = 0; i < text.length(); ++i)
	if (alphabet.find(text.at(i), 0, false) == -1)
	    return false;
    return true;
}

bool
BarcodeItem::encodePlessy(QString& encoded, QStringList& numbers)
{
    int numPos = 16;
    char check[9] = { 1,1,1,1,0,1,0,0,1 };

    QString alphabet = "0123456789ABCDEF";

    QStringList patterns;
    patterns << "13" << "31";

    char* checks = (char*)malloc(text.length() * 4 + 8);
    memset(checks, 0, text.length() * 4 + 8);

    // Left side filler
    encoded += "031311331";

    // Barcode contents
    unsigned int i;
    for (i = 0; i < text.length(); ++i) {
	int code = alphabet.find(text.at(i), 0, false);
	if (code == -1) return false;

	encoded += patterns[(code >> 0) & 1];
	encoded += patterns[(code >> 1) & 1];
	encoded += patterns[(code >> 2) & 1];
	encoded += patterns[(code >> 3) & 1];

	numbers << QString::number(numPos) + ":12:" + text.at(i).upper();
	numPos += 16;

	checks[i * 4 + 0] = (code >> 0) & 1;
	checks[i * 4 + 1] = (code >> 1) & 1;
	checks[i * 4 + 2] = (code >> 2) & 1;
	checks[i * 4 + 3] = (code >> 3) & 1;
    }

    // Checksum
    for (i = 0; i < 4 * text.length(); ++i) {
	if (checks[i]) {
	    for (int j = 0; j < 9; ++j)
		checks[i+j] ^= check[j];
	}
    }
    for (i = 0; i < 8; ++i)
	encoded += patterns[checks[text.length() * 4 + i]];

    // Right side filler
    encoded += "331311313";

    free(checks);
    return true;
}

bool
BarcodeItem::verifyMSIPlessy()
{
    if (text.length() == 0) return false;
    for (unsigned int i = 0; i < text.length(); ++i)
	if (!text.at(i).isDigit())
	    return false;
    return true;
}

bool
BarcodeItem::encodeMSIPlessy(QString& encoded, QStringList& numbers)
{
    int checksum = 0;
    int numPos = 6;

    QStringList patterns;
    patterns << "13" << "31";

    // Left side filler
    encoded += "031";

    // Barcode contents
    for (unsigned int i = 0; i < text.length(); ++i) {
	if (!text.at(i).isDigit()) return false;
	int code = text.at(i).digitValue();
	encoded += patterns[(code >> 3) & 1];
	encoded += patterns[(code >> 2) & 1];
	encoded += patterns[(code >> 1) & 1];
	encoded += patterns[(code >> 0) & 1];

	numbers << QString::number(numPos) + ":12:" + text.at(i);
	numPos += 16;

	if ((i ^ text.length()) & 1)
	    checksum += 2 * code + (2 * code) / 10;
	else
	    checksum += code;
    }

    // Checksum
    int code = (checksum + 9) / 10 * 10 - checksum;
    encoded += patterns[(code >> 3) & 1];
    encoded += patterns[(code >> 2) & 1];
    encoded += patterns[(code >> 1) & 1];
    encoded += patterns[(code >> 0) & 1];

    // Right side filler
    encoded += "131";

    return true;
}

bool
BarcodeItem::verifyCodabar()
{
    QString alphabet = "0123456789-$:/.+ABCD";
    bool startPresent = false;

    if (text.length() == 0) return false;
    for (unsigned int i = 0; i < text.length(); ++i) {
	int code = alphabet.find(text.at(i), 0, false);
	if (code == -1) return false;
	if (i == 0 && code >= 16)
	    startPresent = true;
	else if (code >= 16 && (!startPresent || i != text.length()-1))
	    return false;
    }
    return true;
}

bool
BarcodeItem::encodeCodabar(QString& encoded, QStringList& numbers)
{
    int checksum = 0;
    bool startPresent;
    int numPos = 0;

    QString alphabet = "0123456789-$:/.+ABCD";
    int CODE_A = 16;
    int CODE_B = 17;
    int NARROW = 12;
    int WIDE = 14;

    QStringList patterns;
    patterns << "1111133" << "1111331" << "1113113" << "3311111" << "1131131";
    patterns << "3111131" << "1311113" << "1311311" << "1331111" << "3113111";
    patterns << "1113311" << "1133111" << "3111313" << "3131113" << "3131311";
    patterns << "1131313" << "1133131" << "1313113" << "1113133" << "1113331";

    // If no start character, use A as a start character
    if (!text.at(0).isLetter()) {
	encoded += "1" + patterns[CODE_A];
	checksum = CODE_A;
	startPresent = false;
	numPos = WIDE;
    } else {
	startPresent = true;
    }

    // Barcode contents
    for (unsigned int i = 0; i < text.length(); ++i) {
	int code = alphabet.find(text.at(i), 0, false);
	if (code == -1) return false;

	encoded += "1" + patterns[code];
	checksum += code;

	numbers << QString::number(numPos) + ":12:" + text.at(i).upper();
	numPos += (code < 12) ? NARROW : WIDE;

	if (startPresent && i == text.length() - 2) {
	    code = alphabet.find(text.at(i+1), 0, false);
	    checksum += code;

	    checksum = (checksum + 15) / 16 * 16 - checksum;
	    encoded += "1" + patterns[checksum];
	}
    }

    if (!startPresent) {
	checksum += CODE_B;
	checksum = (checksum + 15) / 16 * 16 - checksum;
	encoded += "1" + patterns[checksum];
	encoded += "1" + patterns[CODE_B];
    }

    return true;
}
