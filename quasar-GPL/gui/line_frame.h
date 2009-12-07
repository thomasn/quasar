// $Id: line_frame.h,v 1.2 2004/01/31 01:50:31 arandell Exp $
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

#ifndef LINE_FRAME_H
#define LINE_FRAME_H

#include "frame.h"

class LineFrame: public Frame {
public:
    LineFrame(Frame* parent=0);
    ~LineFrame();

    void preferredSize(fixed& width, fixed& height);
    void paint(QPainter* p, int x=0, int y=0, int w=-1, int h=-1);
    void print();

protected:
    QString _color;
};

//inline const QString& LineFrame::color() const { return _color; }
//inline void LineFrame::setColor(const QString& color) { _color = color; }

#endif
