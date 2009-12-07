// $Id: frame.cpp,v 1.2 2004/01/31 01:50:30 arandell Exp $
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

#include "frame.h"

#include <qstring.h>
#include <qpainter.h>

#include <assert.h>

int Frame::_indent = 0;

Frame::Frame(Frame* parent)
    : _parent(NULL), _x(0), _y(0), _width(10), _height(10),
      _lastPageOnly(false), _valid(false)
{
    setParent(parent);
}

Frame::~Frame()
{
    for (unsigned int i = 0; i < _children.size(); ++i)
	delete _children[i];
    _children.clear();
    if (_parent) _parent->removeChild(this);
}

Frame*
Frame::parent()
{
    return _parent;
}

void
Frame::setParent(Frame* parent)
{
    if (parent == _parent) return;

    if (_parent) _parent->removeChild(this);
    _parent = parent;
    if (_parent) _parent->addChild(this);
}

fixed
Frame::absoluteX() const
{
    fixed x = _x;
    if (_parent) x += _parent->absoluteX();
    return x;
}

fixed
Frame::absoluteY() const
{
    fixed y = _y;
    if (_parent) y += _parent->absoluteY();
    return y;
}

void
Frame::move(fixed x, fixed y)
{
    _x = x;
    _y = y;
    // TODO: need to repaint parent?
}

fixed
Frame::width()
{
    return _width;
}

fixed
Frame::height()
{
    return _height;
}

void
Frame::resize(fixed width, fixed height)
{
    _width = width;
    _height = height;
    layout();
}

void
Frame::preferredSize(fixed& width, fixed& height)
{
    width = _width;
    height = _height;
}

fixed
Frame::preferredWidth()
{
    fixed width, height;
    preferredSize(width, height);
    return width;
}

fixed
Frame::preferredHeight()
{
    fixed width, height;
    preferredSize(width, height);
    return height;
}

void
Frame::paint(QPainter* p, int, int, int, int)
{
    paintChildren(p);
}

void
Frame::paintChildren(QPainter* p)
{
    FrameList::iterator i = _children.begin();
    for (; i != _children.end(); ++i) {
	Frame* child = *i;

	p->translate(child->x().toDouble(), child->y().toDouble());
	child->paint(p);
	p->translate(-child->x().toDouble(), -child->y().toDouble());
    }
}

bool
Frame::lastPageOnly() const
{
    return _lastPageOnly;
}

void
Frame::setLastPageOnly(bool flag)
{
    _lastPageOnly = flag;
}

const FrameList&
Frame::children()
{
    return _children;
}

void
Frame::addChild(Frame* child)
{
    assert(child->parent() == this);
    _children.push_back(child);
}

void
Frame::removeChild(Frame* child)
{
    assert(child->parent() == this);
    _children.remove(child);
}

void
Frame::layout()
{
}

void
Frame::print()
{
    qDebug("%sframe: %.2f,%.2f %.2fx%.2f", indentText(), _x.toDouble(),
	   _y.toDouble(), _width.toDouble(), _height.toDouble());

    _indent += 4;
    FrameList::iterator i = _children.begin();
    for (; i != _children.end(); ++i)
	(*i)->print();
    _indent -= 4;
}

const char*
Frame::indentText()
{
    static char buffer[1024];
    assert(_indent < 1023);

    QString indent;
    indent.fill(' ', _indent);
    strcpy(buffer, indent);
    return buffer;
}

void
Frame::invalidate()
{
    _valid = false;
    FrameList::iterator i = _children.begin();
    for (; i != _children.end(); ++i)
	(*i)->invalidate();
}
