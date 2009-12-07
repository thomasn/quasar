// $Id: image_frame.cpp,v 1.2 2004/01/31 01:50:30 arandell Exp $
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

#include "image_frame.h"
#include "grid.h"

#include <qpainter.h>
#include <qpaintdevicemetrics.h>

ImageFrame::ImageFrame(const QString& filename, Frame* parent)
    : Frame(parent), _filename(filename)
{
    _image.load(filename);
}

ImageFrame::~ImageFrame()
{
}

void
ImageFrame::preferredSize(fixed& width, fixed& height)
{
    width = _image.width();
    height = _image.height();

    if (Grid::painter() != NULL) {
	QPaintDeviceMetrics metrics(Grid::painter()->device());
	double scaleX = metrics.logicalDpiX() / 73.0;
	double scaleY = metrics.logicalDpiY() / 73.0;
	width = width * scaleX;
	height = height * scaleY;
    }
}

void
ImageFrame::paint(QPainter* p, int x, int y, int w, int h)
{
    if (w == -1) w = _width.toInt();
    if (h == -1) h = _height.toInt();

    QImage scaled = _image.smoothScale(w, h);
    p->drawImage(x, y, scaled);
}

void
ImageFrame::print()
{
}
