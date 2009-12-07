// $Id: label_defn.h,v 1.5 2004/02/03 00:13:26 arandell Exp $
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

#ifndef LABEL_DEFN_H
#define LABEL_DEFN_H

#include "label_item.h"

class LabelDefn {
public:
    LabelDefn();
    LabelDefn(const QString& filePath);

    // Identification
    QString name;		// Name to show users

    // Page definition
    fixed pageWidth;		// Width of paper without margins
    fixed pageHeight;		// Height of paper without margins
    fixed topMargin;		// Margin for top side
    fixed leftMargin;		// Margin for left side
    int rows;			// Number of rows of labels
    int columns;		// Number of column per row
    bool landscape;		// Landscape or portrait mode
    double scale;		// 1 for points, .1 for deci-points, ...

    // Label definition
    fixed labelWidth;		// Width of one label (printing area)
    fixed labelHeight;		// Height of one label (printing area)
    fixed horizontalGap;	// Horizontal gap between labels
    fixed verticalGap;		// Vertical gap between labels

    // Label contents
    QValueList<LabelItem*> items; // Stuff to show on each label

    // Error handling
    QString lastError;

    // Load and save to XML format file
    bool load(const QString& filePath);
    bool save(const QString& filePath);

    // Scale everything
    void dpiAdjust(double scaleX, double scaleY);

    // Draw the label
    void draw(QPainter* p, fixed x, fixed y, TextMap& mapping);

private:
    bool error(const QString& message);
};

#endif // LABEL_DEFN_H
