// $Id: check_box.cpp,v 1.9 2004/10/21 05:45:06 bpepers Exp $
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

#include "check_box.h"

#include <qpainter.h>
#include <qstyle.h>
#include <qdrawutil.h>

CheckBox::CheckBox(QWidget* parent, const char* name)
    : QButton(parent, name, WRepaintNoErase | WResizeNoErase | WMouseNoMask)
{
    setToggleButton(true);
}

QSize
CheckBox::sizeHint() const
{
    QSize size = fontMetrics().size(0, "x");
    size.rwidth() += 4;
    size.rheight() += 4;
    return size;
}

void
CheckBox::drawButton(QPainter* p)
{
    // Draw contents
    if (isChecked()) {
	QRect rect(1, 1, width() - 2, height() - 2);
	style().drawPrimitive(QStyle::PE_CheckMark, p, rect, colorGroup());
    }

    if (hasFocus()) {
	QRect rect(2, 2, width() - 4, height() - 4);
	style().drawPrimitive(QStyle::PE_FocusRect, p, rect, colorGroup());
    }
}

QSizePolicy
CheckBox::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
}
