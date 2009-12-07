// $Id: image_element.cpp,v 1.5 2005/01/24 10:06:36 bpepers Exp $
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

#include "image_element.h"
#include "base64.h"

#include <qbuffer.h>
#include <qfile.h>

ImageElement::ImageElement()
{
    clear();
}

ImageElement::~ImageElement()
{
}

ImageElement*
ImageElement::clone() const
{
    ImageElement* image = new ImageElement();
    *image = *this;
    return image;
}

void
ImageElement::clear()
{
    ReportElement::clear();
    filePath = "";
    image = QImage();
    dpi = 100;
    smoothScale = false;
}

bool
ImageElement::fromXML(QDomElement e)
{
    if (!ReportElement::fromXML(e)) return false;

    // If image file not given, assume text is base64 image data
    if (filePath.isEmpty()) {
	QByteArray bytes;
	if (!base64Decode(e.text(), bytes))
	    qWarning("Error: failed decoding image data");

	if (!image.loadFromData(bytes))
	    qWarning("Error: failed loading image data");
    }

    return true;
}

bool
ImageElement::toXML(QDomElement& e) const
{
    QDomDocument doc = e.ownerDocument();

    ReportElement::toXML(e);

    if (filePath.isEmpty()) {
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
	e.appendChild(doc.createTextNode(text));
    } else {
	e.setAttribute("file", filePath);
    }

    if (dpi != 100)
	e.setAttribute("dpi", QString::number(dpi));
    if (smoothScale)
	e.setAttribute("smoothScale", "Yes");

    return true;
}

bool
ImageElement::processAttribute(const QString& name, const QString& value)
{
    if (name == "dpi")
	dpi = value.toInt();
    else if (name == "file") {
	filePath = value;
	if (!image.load(filePath))
	    qWarning("Error: failed loading image file: " + filePath);
    } else if (name == "smoothScale") {
	smoothScale = (value.lower() == "yes");
    } else {
	return ReportElement::processAttribute(name, value);
    }

    return true;
}

void
ImageElement::generate(ReportInterp* interp, int offsetX, int offsetY,
		       QValueVector<PageElement>& elements)
{
    PageElement element;
    element.type = PageElement::IMAGE;
    setPageElement(element, interp, offsetX, offsetY);

    element.image = image;
    element.imageDPI = dpi;
    element.smoothScale = smoothScale;

    elements.clear();
    elements.push_back(element);
}
