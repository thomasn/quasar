// $Id: text_element.cpp,v 1.3 2005/01/30 04:16:25 bpepers Exp $
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

#include "text_element.h"
#include <ctype.h>
#include <stdlib.h>

TextElement::TextElement()
{
    clear();
}

TextElement::~TextElement()
{
}

TextElement*
TextElement::clone() const
{
    TextElement* text = new TextElement();
    *text = *this;
    return text;
}

void
TextElement::clear()
{
    ReportElement::clear();
    font = QFont();
    flags = Qt::AlignLeft | Qt::AlignVCenter;
    format = "";
    evalTime = NOW;
    text = "";
    expr = "";
    var = "";
    code = "";
}

bool
TextElement::toXML(QDomElement& e) const
{
    QDomDocument doc = e.ownerDocument();

    ReportElement::toXML(e);

    if (!format.isEmpty())
	e.setAttribute("format", format);

    if (evalTime != NOW) {
	QString time = "Now";
	switch (evalTime) {
	case REPORT:	time = "Report"; break;
	case PAGE:	time = "Page"; break;
	}
	e.setAttribute("evalTime", time);
    }

    if (flags & Qt::AlignHCenter)
	e.setAttribute("halign", "Center");
    else if (flags & Qt::AlignRight)
	e.setAttribute("halign", "Right");

    if (flags & Qt::AlignTop)
	e.setAttribute("valign", "Top");
    else if (flags & Qt::AlignBottom)
	e.setAttribute("valign", "Bottom");

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

    QDomElement f = doc.createElement("text");
    if (!code.isEmpty()) {
	f.setTagName("code");
	f.appendChild(doc.createTextNode(code));
    } else if (!var.isEmpty()) {
	f.setTagName("var");
	f.appendChild(doc.createTextNode(var));
    } else if (!expr.isEmpty()) {
	f.setTagName("expr");
	f.appendChild(doc.createTextNode(expr));
    } else {
	f.appendChild(doc.createTextNode(text));
    }
    e.appendChild(f);

    return true;
}

bool
TextElement::processAttribute(const QString& name, const QString& value)
{
    if (name == "format")
	format = value;
    else if (name == "evalTime") {
	QString time = value.lower();
	if (time == "now") evalTime = NOW;
	else if (time == "report") evalTime = REPORT;
	else if (time == "page") evalTime = PAGE;
	else qWarning("Invalid evalTime: " + value);
    } else if (name == "halign") {
	QString align = value.lower();
	if (align == "left") flags |= Qt::AlignLeft;
	else if (align == "center") flags |= Qt::AlignHCenter;
	else if (align == "right") flags |= Qt::AlignRight;
	else qWarning("Unknown halign: " + value);
    } else if (name == "valign") {
	QString align = value.lower();
	if (align == "top") flags |= Qt::AlignTop;
	else if (align == "center") flags |= Qt::AlignVCenter;
	else if (align == "bottom") flags |= Qt::AlignBottom;
	else qWarning("Unknown valign: " + value);
    } else {
	return ReportElement::processAttribute(name, value);
    }

    return true;
}

bool
TextElement::processChildNode(QDomElement& e)
{
    QString tag = e.tagName();

    if (tag == "font") {
	font.setFamily(e.attribute("name"));
	font.setPointSize(getInt(e, "size", 10));
	font.setBold(getBoolean(e, "isBold", false));
	font.setItalic(getBoolean(e, "isItalic", false));
	font.setUnderline(getBoolean(e, "isUnderline", false));
	font.setStrikeOut(getBoolean(e, "isStrikeOut", false));
	if (e.hasAttribute("weight"))
	    font.setWeight(getInt(e, "weight", 0));
    } else if (tag == "text") {
	text = e.text();
    } else if (tag == "expr") {
	expr = e.text();
    } else if (tag == "var") {
	var = e.text();
    } else if (tag == "code") {
	code = e.text();
    } else {
	return ReportElement::processChildNode(e);
    }

    return true;
}

void
TextElement::generate(ReportInterp* interp, int offsetX, int offsetY,
		      QValueVector<PageElement>& elements)
{
    PageElement element;
    element.type = PageElement::TEXT;
    setPageElement(element, interp, offsetX, offsetY);

    element.font = font;
    element.textFlags = flags;
    element.format = format;
    element.evalTime = evalTime;
    element.text = text;
    element.expr = expr;
    element.var = var;
    element.code = code;

    elements.clear();
    elements.push_back(element);
}
