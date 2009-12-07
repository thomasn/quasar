// $Id: arrow_line.h,v 1.4 2004/02/03 00:56:02 arandell Exp $
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

#ifndef ARROW_LINE_H
#define ARROW_LINE_H

#include <qwidget.h>

class ArrowLine: public QWidget {
    Q_OBJECT
public:
    ArrowLine(QWidget* parent=0, const char* name=0);
    ArrowLine(int lineSides, QWidget* parent=0, const char* name=0);
    ~ArrowLine();

    enum Sides { Left=1, Right=2, Top=4, Bottom=8,
		 Horizontal=3, Vertical=12 };

    // Look and feel
    int lineWidth() const;
    int lineSides() const;
    int arrowSides() const;
    void setLineWidth(int width);
    void setLineSides(int sides);
    void setArrowSides(int sides);

    // Sizing
    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent* e);
    void drawContentsMask(QPainter* p);
    void drawLine(QPainter* p);

    int _lineWidth;
    int _lineSides;
    int _arrowSides;
};

#endif // ARROW_LINE_H
