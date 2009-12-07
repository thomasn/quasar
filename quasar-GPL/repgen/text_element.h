// $Id: text_element.h,v 1.3 2005/01/30 04:16:25 bpepers Exp $
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

#ifndef TEXT_ELEMENT_H
#define TEXT_ELEMENT_H

#include "report_element.h"

class TextElement: public ReportElement {
public:
    TextElement();
    ~TextElement();

    // Element type
    QString type() const { return "text"; }

    // Create a clone of the element
    TextElement* clone() const;

    // Clear all data to defaults
    void clear();

    // Generate page elements
    void generate(ReportInterp* interp, int offsetX, int offsetY,
		  QValueVector<PageElement>& elements);

    // Evaluation times
    enum { NOW, REPORT, PAGE };

    // Data
    QFont font;			// Font for text
    int flags;			// Alignment flags
    QString format;		// Formatting method (money, percent, ...)
    int evalTime;		// When to evaluate
    QString text;		// Static text
    QString expr;		// Evaluate to get text
    QString var;		// Variable to get test from
    QString code;		// Code to return result

    // Convert to XML
    bool toXML(QDomElement& e) const;

protected:
    bool processAttribute(const QString& name, const QString& value);
    bool processChildNode(QDomElement& e);
};

#endif // TEXT_ELEMENT_H
