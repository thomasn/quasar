// $Id: image_element.h,v 1.3 2005/01/24 10:06:36 bpepers Exp $
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

#ifndef IMAGE_ELEMENT_H
#define IMAGE_ELEMENT_H

#include "report_element.h"

class ImageElement: public ReportElement {
public:
    ImageElement();
    ~ImageElement();

    // Element type
    QString type() const { return "image"; }

    // Create a clone of the element
    ImageElement* clone() const;

    // Clear all data to defaults
    void clear();

    // Generate page elements
    void generate(ReportInterp* interp, int offsetX, int offsetY,
		  QValueVector<PageElement>& elements);

    // Data
    QString filePath;
    QImage image;
    int dpi;
    bool smoothScale;

    // Convert from/to XML element
    bool fromXML(QDomElement e);
    bool toXML(QDomElement& e) const;

protected:
    bool processAttribute(const QString& name, const QString& value);
};

#endif // IMAGE_ELEMENT_H
