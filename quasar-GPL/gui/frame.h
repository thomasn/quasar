// $Id: frame.h,v 1.2 2004/01/31 01:50:31 arandell Exp $
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

#ifndef FRAME_H
#define FRAME_H

#include "fixed.h"
#include <qvaluelist.h>

class QPainter;

class Frame;
typedef QValueList<Frame*> FrameList;

class Frame {
public:
    Frame(Frame* parent=0);
    virtual ~Frame();

    Frame* parent();
    virtual void setParent(Frame* parent);

    // Position in parent
    fixed x() const;
    fixed y() const;
    void move(fixed x, fixed y);

    // Absolute positions
    fixed absoluteX() const;
    fixed absoluteY() const;

    // Size
    virtual fixed width();
    virtual fixed height();
    virtual void resize(fixed width, fixed height);
    virtual void preferredSize(fixed& width, fixed& height);
    fixed preferredWidth();
    fixed preferredHeight();

    // Painting
    virtual void paint(QPainter* p, int x=0, int y=0, int w=-1, int h=-1);
    virtual void paintChildren(QPainter* p);
    bool lastPageOnly() const;
    void setLastPageOnly(bool flag);

    // Children
    const FrameList& children();
    virtual void addChild(Frame* child);
    virtual void removeChild(Frame* child);
    virtual void layout();

    // Debugging
    virtual void print();

protected:
    void invalidate();
    const char* indentText();

    Frame* _parent;
    fixed _x, _y;
    fixed _width, _height;
    bool _lastPageOnly;
    FrameList _children;
    bool _valid;
    static int _indent;
};

inline fixed Frame::x() const { return _x; }
inline fixed Frame::y() const { return _y; }

#endif
