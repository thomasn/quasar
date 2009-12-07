// $Id: label_defn.cpp,v 1.5 2004/02/03 00:13:26 arandell Exp $
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

#include "label_defn.h"

#include <qapplication.h>
#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

#define tr(text) qApp->translate("LabelDefn",text)

LabelDefn::LabelDefn()
    : rows(0), columns(0), landscape(false), scale(1.0)
{
}

LabelDefn::LabelDefn(const QString& filePath)
{
    load(filePath);
}

bool
LabelDefn::load(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(IO_ReadOnly))
	return error(tr("Can't open label file: %1").arg(filePath));

    QDomDocument doc;
    if (!doc.setContent(&file))
	return error("Failed reading label file");
    if (doc.doctype().name() != "LABEL_DEFN")
	return error("Label definition file is not the right type");
    file.close();

    QDomElement root = doc.documentElement();
    for (uint i = 0; i < root.childNodes().count(); ++i) {
	QDomNode node = root.childNodes().item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;

	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "name") {
	    name = text;
	} else if (tag == "pageWidth") {
	    pageWidth = text.toDouble();
	} else if (tag == "pageHeight") {
	    pageHeight = text.toDouble();
	} else if (tag == "topMargin") {
	    topMargin = text.toDouble();
	} else if (tag == "leftMargin") {
	    leftMargin = text.toDouble();
	} else if (tag == "rows") {
	    rows = text.toInt();
	} else if (tag == "columns") {
	    columns = text.toInt();
	} else if (tag == "orientation") {
	    if (text == "landscape")
		landscape = true;
	    else if (text == "portrait")
		landscape = false;
	    else
		qWarning("Invalid orientation: " + text);
	} else if (tag == "scale") {
	    scale = text.toDouble();
	} else if (tag == "labelWidth") {
	    labelWidth = text.toDouble();
	} else if (tag == "labelHeight") {
	    labelHeight = text.toDouble();
	} else if (tag == "hgap") {
	    horizontalGap = text.toDouble();
	} else if (tag == "vgap") {
	    verticalGap = text.toDouble();
	} else if (tag == "line") {
	    QDomNodeList nodes = e.childNodes();
	    LineItem* item = new LineItem();
	    item->load(nodes);
	    items.push_back(item);
	} else if (tag == "rect") {
	    QDomNodeList nodes = e.childNodes();
	    RectItem* item = new RectItem();
	    item->load(nodes);
	    items.push_back(item);
	} else if (tag == "rrect") {
	    QDomNodeList nodes = e.childNodes();
	    RoundRectItem* item = new RoundRectItem();
	    item->load(nodes);
	    items.push_back(item);
	} else if (tag == "panel") {
	    QDomNodeList nodes = e.childNodes();
	    PanelItem* item = new PanelItem();
	    item->load(nodes);
	    items.push_back(item);
	} else if (tag == "ellipse") {
	    QDomNodeList nodes = e.childNodes();
	    EllipseItem* item = new EllipseItem();
	    item->load(nodes);
	    items.push_back(item);
	} else if (tag == "arc") {
	    QDomNodeList nodes = e.childNodes();
	    ArcItem* item = new ArcItem();
	    item->load(nodes);
	    items.push_back(item);
	} else if (tag == "chord") {
	    QDomNodeList nodes = e.childNodes();
	    ChordItem* item = new ChordItem();
	    item->load(nodes);
	    items.push_back(item);
	} else if (tag == "polygon") {
	    QDomNodeList nodes = e.childNodes();
	    PolygonItem* item = new PolygonItem();
	    item->load(nodes);
	    items.push_back(item);
	} else if (tag == "pixmap") {
	    QDomNodeList nodes = e.childNodes();
	    PixmapItem* item = new PixmapItem();
	    item->load(nodes);
	    items.push_back(item);
	} else if (tag == "image") {
	    QDomNodeList nodes = e.childNodes();
	    ImageItem* item = new ImageItem();
	    item->load(nodes);
	    items.push_back(item);
	} else if (tag == "text") {
	    QDomNodeList nodes = e.childNodes();
	    TextItem* item = new TextItem();
	    item->load(nodes);
	    items.push_back(item);
	} else if (tag == "barcode") {
	    QDomNodeList nodes = e.childNodes();
	    BarcodeItem* item = new BarcodeItem();
	    item->load(nodes);
	    items.push_back(item);
	} else {
	    qWarning("Unknown label defn tag: " + tag);
	}
    }

    return true;
}

bool
LabelDefn::save(const QString&)
{
    // TODO: implement
    return false;
}

void
LabelDefn::dpiAdjust(double scaleX, double scaleY)
{
    pageWidth = pageWidth * scaleX;
    pageHeight = pageHeight * scaleY;
    topMargin = topMargin * scaleY;
    leftMargin = leftMargin * scaleX;
    labelWidth = labelWidth * scaleX;
    labelHeight = labelHeight * scaleY;
    horizontalGap = horizontalGap * scaleX;
    verticalGap = verticalGap * scaleY;

    for (unsigned int i = 0; i < items.size(); ++i)
	items[i]->dpiAdjust(scaleX, scaleY);
}

void
LabelDefn::draw(QPainter* p, fixed x, fixed y, TextMap& mapping)
{
    TextItem::mapping = mapping;

    p->translate(x.toInt(), y.toInt());
    for (unsigned int i = 0; i < items.size(); ++i)
	items[i]->draw(p);
    p->translate(-x.toInt(), -y.toInt());
}

bool
LabelDefn::error(const QString& error)
{
    qWarning("Error: " + error);
    lastError = error;
    return false;
}
