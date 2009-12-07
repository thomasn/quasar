// $Id: text_frame.cpp,v 1.2 2004/01/31 01:50:31 arandell Exp $
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

#include "text_frame.h"
#include "grid.h"

#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qstringlist.h>

TextFrame::TextFrame(const QString& text, Frame* parent)
    : Frame(parent), _text(text), _padX(2), _padY(0), _justify(ALIGN_LEFT)
{
    _font = Font::defaultFont;
}

TextFrame::~TextFrame()
{
}

void
TextFrame::preferredSize(fixed& width, fixed& height)
{
    width = 0.0;
    height = 0.0;

    QStringList lines = QStringList::split('\n', text(), true);
    for (QStringList::Iterator it = lines.begin(); it != lines.end(); ++it) {
	QString& line = *it;
	fixed lineWidth = _font.stringWidth(line);
	if (lineWidth > width) width = lineWidth;
	height += _font.lineSpacing();
    }

    int padX = _padX;
    int padY = _padY;
    if (Grid::painter() != NULL) {
	QPaintDeviceMetrics metrics(Grid::painter()->device());
	padX = int(padX * metrics.logicalDpiX() / 72.0 + .5);
	padY = int(padY * metrics.logicalDpiY() / 72.0 + .5);
    }

    // Add on padding
    width += 2 * padX;
    height += 2 * padY;
}

void
TextFrame::paint(QPainter* p, int, int, int, int)
{
    if (_lastPageOnly && !Grid::lastPage() || text().isEmpty())
	return;

    // Figure out proper alignment
    int flags = Qt::AlignVCenter;
    if (_justify == ALIGN_LEFT)   flags |= Qt::AlignLeft;
    if (_justify == ALIGN_RIGHT)  flags |= Qt::AlignRight;
    if (_justify == ALIGN_CENTER) flags |= Qt::AlignHCenter;

    int padX = _padX;
    int padY = _padY;
    if (Grid::painter() != NULL) {
	QPaintDeviceMetrics metrics(Grid::painter()->device());
	padX = int(padX * metrics.logicalDpiX() / 72.0 + .5);
	padY = int(padY * metrics.logicalDpiY() / 72.0 + .5);
    }

    // Save state and setup pen, font, ...
    p->save();
    p->setFont(_font.getFont());
    if (!_color.isEmpty()) p->setPen(QColor(_color));
    p->drawText(padX, padY, _width.toInt(), _height.toInt(),
		flags, text());
    p->restore();

    paintChildren(p);
}

void
TextFrame::print()
{
    qDebug("%stext: %.2f,%.2f %.2fx%.2f (%s)", indentText(), _x.toDouble(),
	   _y.toDouble(), _width.toDouble(), _height.toDouble(),
	   text().latin1());
}
