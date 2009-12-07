// $Id: font.h,v 1.2 2004/01/31 01:50:31 arandell Exp $
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

#ifndef FONT_H
#define FONT_H

#include "fixed.h"
#include <qstring.h>
#include <qfont.h>
#include <qfontmetrics.h>

class Font {
public:
    Font(const QString& family="helvetica", fixed size=9.0, int weight=50);

    // Get values
    const QString& family() const;
    fixed pointSize() const;
    int weight() const;
    bool isUnderline() const;
    bool isItalic() const;
    bool isStrikeOut() const;
    bool isFixedPitch() const;

    // Set values
    void setFamily(const QString& family);
    void setPointSize(fixed size);
    void setWeight(int weight);
    void setUnderline(bool flag);
    void setItalic(bool flag);
    void setStrikeOut(bool flag);
    void setFixedPitch(bool flag);

    // Font metrics
    fixed ascent();
    fixed descent();
    fixed height();
    fixed leading();
    fixed lineSpacing();

    // Measure a string
    fixed stringWidth(const QString& text);

    // Convert to other types
    QFont getFont();
    QFontMetrics getMetrics();

    // Default font to use
    static Font defaultFont;

    // Operators
    bool operator==(const Font& rhs) const;
    bool operator!=(const Font& rhs) const;

protected:
    QString _family;
    fixed _size;
    int _weight;
    bool _underline;
    bool _italic;
    bool _strikeout;
    bool _fixed;
};

inline const QString& Font::family() const { return _family; }
inline fixed Font::pointSize() const { return _size; }
inline int Font::weight() const { return _weight; }
inline bool Font::isUnderline() const { return _underline; }
inline bool Font::isItalic() const { return _italic; }
inline bool Font::isStrikeOut() const { return _strikeout; }
inline bool Font::isFixedPitch() const { return _fixed; }

inline void Font::setFamily(const QString& family) { _family = family; }
inline void Font::setPointSize(fixed size) { _size = size; }
inline void Font::setWeight(int weight) { _weight = weight; }
inline void Font::setUnderline(bool flag) { _underline = flag; }
inline void Font::setItalic(bool flag) { _italic = flag; }
inline void Font::setStrikeOut(bool flag) { _strikeout = flag; }
inline void Font::setFixedPitch(bool flag) { _fixed = flag; }

#endif
