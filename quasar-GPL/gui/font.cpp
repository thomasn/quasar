// $Id: font.cpp,v 1.2 2004/01/31 01:50:30 arandell Exp $
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

#include "font.h"
#include "grid.h"

#include <qpainter.h>
#include <assert.h>

Font Font::defaultFont;

Font::Font(const QString& family, fixed size, int weight)
    : _family(family), _size(size), _weight(weight), _underline(false),
      _italic(false), _strikeout(false), _fixed(false)
{
}

fixed
Font::ascent()
{
    return getMetrics().ascent();
}

fixed
Font::descent()
{
    return getMetrics().descent();
}

fixed
Font::height()
{
    return ascent() + descent() + 1;
}

fixed
Font::leading()
{
    return getMetrics().leading();
}

fixed
Font::lineSpacing()
{
    return height() + leading();
}

fixed
Font::stringWidth(const QString& text)
{
    return getMetrics().width(text);
}

QFont
Font::getFont()
{
    QFont font(_family, _size.toInt(), _weight, _italic);
    font.setUnderline(_underline);
    font.setStrikeOut(_strikeout);
    font.setFixedPitch(_fixed);
    return font;
}

QFontMetrics
Font::getMetrics()
{
    QFont font = getFont();
    if (Grid::painter() == NULL)
	return QFontMetrics(font);
    Grid::painter()->setFont(font);
    return Grid::painter()->fontMetrics();
}

bool
Font::operator==(const Font& rhs) const
{
    if (rhs._family != _family) return false;
    if (rhs._size != _size) return false;
    if (rhs._weight != _weight) return false;
    if (rhs._underline != _underline) return false;
    if (rhs._italic != _italic) return false;
    if (rhs._strikeout != _strikeout) return false;
    if (rhs._fixed != _fixed) return false;
    return true;
}

bool
Font::operator!=(const Font& rhs) const
{
    return !(*this == rhs);
}
