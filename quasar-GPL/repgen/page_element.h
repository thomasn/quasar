// $Id: page_element.h,v 1.2 2004/09/08 15:04:26 bpepers Exp $
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

#ifndef PAGE_ELEMENT_H
#define PAGE_ELEMENT_H

#include <qdom.h>
#include <qpen.h>
#include <qbrush.h>
#include <qfont.h>
#include <qimage.h>
#include <qvaluevector.h>

class ReportInterp;

class PageElement {
public:
    PageElement();
    PageElement(QDomElement e);
    ~PageElement();

    // Element types
    enum { POINT, LINE, RECTANGLE, ROUND_RECT, ELLIPSE, POLYGON, IMAGE,
	   TEXT };

    // Background modes
    enum { TRANSPARENT, OPAQUE };

    // Shared by all elements
    int type;			// Type of element
    int x, y;			// Position in band
    int width, height;		// Area element uses

    // Elements controlling drawing
    QPen pen;			// Pen for drawing
    QBrush brush;		// Brush for filling shapes
    QFont font;			// Font for text
    int backgroundMode;		// How the background is drawn
    QString backgroundColor;	// Background color

    // Transformations
    double scaleX;		// Scale in X direction
    double scaleY;		// Scale in Y direction
    double shearX;		// Shear in X direction
    double shearY;		// Shear in Y direction
    double rotate;		// Rotation

    // For round rectangles
    int roundX;			// Roundness from 0-99
    int roundY;			// Roundness from 0-99

    // For polygons
    QValueVector<QPoint> points;

    // For images
    QString imageFile;		// Set when image came from a file
    QString imageData;		// Set when image base64 data was given
    QImage image;		// Image to draw
    int imageDPI;		// DPI for image
    bool smoothScale;		// Smooth or normal scaling?

    // For text
    int textFlags;		// Alignment and other Qt text flags
    QString text;		// Text to put on page

    // This data is set by report generation but by the end it doesn't
    // matter so its not saved or loaded.  Its for resolving bound
    // text elements and by the end of generating the report they have
    // all been calculated with the real text stored in the text field
    QString format;		// Formatting method (money, percent, ...)
    int evalTime;		// When to evaluate
    QString expr;		// Evaluate to get text
    QString var;		// Variable to get test from
    QString code;		// Code to return result

    // Evaluate expr/var/code and set text
    QString evaluate(ReportInterp* interp);

    // Convert from/to XML element
    bool fromXML(QDomElement e);
    bool toXML(QDomElement& e) const;
};

#endif // PAGE_ELEMENT_H
