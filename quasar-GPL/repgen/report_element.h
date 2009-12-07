// $Id: report_element.h,v 1.10 2004/08/04 03:43:56 bpepers Exp $
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

#ifndef REPORT_ELEMENT_H
#define REPORT_ELEMENT_H

#include "page_element.h"

class ReportInterp;

class ReportElement {
public:
    ReportElement();
    virtual ~ReportElement();

    // Type name
    virtual QString type() const=0;

    // Create a clone
    virtual ReportElement* clone() const=0;

    // Clear all data to defaults
    virtual void clear();

    // Generate page elements
    virtual void generate(ReportInterp* interp, int offsetX, int offsetY,
			  QValueVector<PageElement>& elements)=0;

    // Expression controlling printing (blank means always print)
    QString printWhen;

    // Drawing environment
    QPen pen;
    QBrush brush;
    int backgroundMode;
    QString backgroundColor;

    // Translations
    double scaleX, scaleY;
    double shearX, shearY;
    double rotate;

    // Position and size (strings because they can be an expression)
    QString x, y;
    QString width, height;

    // Convert from/to XML element
    virtual bool fromXML(QDomElement e);
    virtual bool toXML(QDomElement& e) const;

    // Evaluate printWhen and return result
    bool shouldPrint(ReportInterp* interp) const;

protected:
    // Assignment only in subclasses
    ReportElement& operator=(const ReportElement& rhs);

    // Implement these for element specific attributes and child nodes
    virtual bool processAttribute(const QString& name, const QString& value);
    virtual bool processChildNode(QDomElement& e);

    // Utility methods for XML conversions
    int getInt(const QString& name, const QString& value, int defaultValue);
    int getInt(QDomElement e, const QString& name, int defaultValue);
    bool getBoolean(const QString& name, const QString& value, bool defValue);
    bool getBoolean(QDomElement e, const QString& name, bool defaultValue);

    // Copy values into a cloned object
    void copyTo(ReportElement* element) const;

    // Transfer shared values to a page element
    void setPageElement(PageElement& element, ReportInterp* interp,
			int offsetX, int offsetY);

private:
    // No copy constructor allowed
    ReportElement(const ReportElement& e);
};

#endif // REPORT_ELEMENT_H
