// $Id: line_frame.cpp,v 1.2 2004/01/31 01:50:30 arandell Exp $
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

#include "line_frame.h"
#include "grid.h"

#include <qpainter.h>

LineFrame::LineFrame(Frame* parent)
    : Frame(parent)
{
}

LineFrame::~LineFrame()
{
}

void
LineFrame::preferredSize(fixed& width, fixed& height)
{
    width = 0.0;
    height = 0.5;
}

void
LineFrame::paint(QPainter* p, int, int, int, int)
{
    if (_lastPageOnly && !Grid::lastPage())
	return;

    double x1 = 0.0;
    double x2 = x1 + _width.toDouble();
    double y1 = _height.toDouble() / 2.0;
    double y2 = y1;

    p->drawLine(int(x1), int(y1), int(x2), int(y2));
    paintChildren(p);
}

void
LineFrame::print()
{
    qDebug("%sline: %.2f,%.2f %.2fx%.2f", indentText(), _x.toDouble(),
	   _y.toDouble(), _width.toDouble(), _height.toDouble());
}
