// $Id: text_frame.h,v 1.2 2004/01/31 01:50:31 arandell Exp $
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

#ifndef TEXT_FRAME_H
#define TEXT_FRAME_H

#include "frame.h"
#include "font.h"

// Justification
enum { ALIGN_LEFT=1, ALIGN_RIGHT=2, ALIGN_CENTER=3 };

class TextFrame: public Frame {
public:
    TextFrame(const QString& text, Frame* parent=0);
    ~TextFrame();

    virtual const QString& text();
    int padX() const;
    int padY() const;
    int justify() const;
    const Font& font() const;
    const QString& color() const;

    void setText(const QString& text);
    void setPadX(int size);
    void setPadY(int size);
    void setJustify(int align);
    void setFont(const Font& font);
    void setColor(const QString& color);

    void preferredSize(fixed& width, fixed& height);
    void paint(QPainter* p, int x=0, int y=0, int w=-1, int h=-1);
    void print();

protected:
    QString _text;
    int _padX, _padY;
    int _sticky;
    int _justify;
    Font _font;
    QString _color;
};

inline const QString& TextFrame::text() { return _text; }
inline int TextFrame::padX() const { return _padX; }
inline int TextFrame::padY() const { return _padY; }
inline int TextFrame::justify() const { return _justify; }
inline const Font& TextFrame::font() const { return _font; }
inline const QString& TextFrame::color() const { return _color; }

inline void TextFrame::setText(const QString& text) { _text = text; }
inline void TextFrame::setPadX(int size) { _padX = size; }
inline void TextFrame::setPadY(int size) { _padY = size; }
inline void TextFrame::setJustify(int align) { _justify = align; }
inline void TextFrame::setFont(const Font& font) { _font = font; }
inline void TextFrame::setColor(const QString& color) { _color = color; }

#endif
