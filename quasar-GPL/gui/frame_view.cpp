// $Id: frame_view.cpp,v 1.2 2004/01/31 01:50:30 arandell Exp $
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

#include "frame_view.h"
#include <qpainter.h>

FrameView::FrameView(QWidget* parent, const char* name, WFlags f)
    : QScrollView(parent, name, f), _frame(NULL), _printing(false)
{
    viewport()->setBackgroundMode(PaletteBase);
}

FrameView::~FrameView()
{
}

void
FrameView::setPrinting(bool flag)
{
    _printing = flag;
}

void
FrameView::setFrame(Frame* frame)
{
    _frame = NULL;
    erase();
    resizeContents(1, 1);

    _frame = frame;
    if (_frame != NULL) {
	fixed width, height;
	frame->preferredSize(width, height);

	fixed vWidth = (visibleWidth() - 20);
	fixed vHeight = visibleHeight();

	if (width < vWidth) width = vWidth;
	if (height < vHeight) height = vHeight;

	frame->resize(width, height);
	frame->layout();

	resizeContents(width.toInt(), height.toInt());
	viewport()->update();
    }
}

void
FrameView::resizeEvent(QResizeEvent* e)
{
    if (_frame != NULL) {
	fixed width = _frame->width();
	fixed height = _frame->height();
	fixed prefWidth, prefHeight;
	_frame->preferredSize(prefWidth, prefHeight);

	fixed newWidth = width;
	fixed newHeight = height;
	if (e->size().width() - 20 > prefWidth)
	    newWidth = e->size().width() - 20;
	if (e->size().height() > prefHeight)
	    newHeight = e->size().height() - 4;

	if (newWidth != prefWidth || newHeight != height) {
	    _frame->resize(newWidth, newHeight);
	    viewport()->erase();
	    resizeContents(newWidth.toInt(), newHeight.toInt());
	}
    }

    QScrollView::resizeEvent(e);
}

void
FrameView::drawContents(QPainter* p, int cx, int cy, int cw, int ch)
{
    if (_frame == NULL || _printing) return;

    fixed offset = (contentsWidth() - _frame->width()) / 2;
    p->translate(offset.toInt(), 0);
    _frame->paint(p, cx, cy, cw, ch);
}
