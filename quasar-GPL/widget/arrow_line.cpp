// $Id: arrow_line.cpp,v 1.6 2004/02/03 00:56:02 arandell Exp $
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

#include "arrow_line.h"

#include <qpainter.h>

ArrowLine::ArrowLine(QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    _lineWidth = 2;
    _lineSides = 0;
    _arrowSides = 0;
}

ArrowLine::ArrowLine(int lineSides, QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    _lineWidth = 2;
    _lineSides = lineSides;
    _arrowSides = 0;
}

ArrowLine::~ArrowLine()
{
}

int
ArrowLine::lineWidth() const
{
    return _lineWidth;
}

int
ArrowLine::lineSides() const
{
    return _lineSides;
}

int
ArrowLine::arrowSides() const
{
    return _arrowSides;
}

void
ArrowLine::setLineWidth(int width)
{
    _lineWidth = width;
    update();
}

void
ArrowLine::setLineSides(int sides)
{
    _lineSides = sides;
    update();
}

void
ArrowLine::setArrowSides(int sides)
{
    _arrowSides = sides;
    update();
}

QSize
ArrowLine::sizeHint() const
{
    QSize size(40, 40);
    return size;
}

void
ArrowLine::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    // Pseudo tranparency
    if (parentWidget() && parentWidget()->backgroundPixmap()) {
	p.drawTiledPixmap(0, 0, width(), height(),
			   *parentWidget()->backgroundPixmap(), x(), y());
    } else if (parentWidget()) {
	p.fillRect(0, 0, width(), height(), parentWidget()->backgroundColor());
    }

    drawLine(&p);
}

void
ArrowLine::drawContentsMask(QPainter* p)
{
    p->setPen(color1);
    drawLine(p);
}

void
ArrowLine::drawLine(QPainter* p)
{
    int mx = width() / 2;
    int my = height() / 2;

    // Draw lines
    if (_lineSides & Left)
	p->drawLine(0, my, mx, my);
    if (_lineSides & Right)
	p->drawLine(mx, my, width(), my);
    if (_lineSides & Top)
	p->drawLine(mx, 0, mx, my);
    if (_lineSides & Bottom)
	p->drawLine(mx, my, mx, height());

    // TODO: draw arrows
}
