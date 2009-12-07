// $Id: row_header.cpp,v 1.11 2004/02/03 00:56:02 arandell Exp $
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

#include "row_header.h"
#include <qpainter.h>

static const int QH_MARGIN = 4;

RowHeader::RowHeader(QWidget* parent, const char* name)
    : QHeader(parent, name)
{
}

RowHeader::RowHeader(int n, QWidget* parent, const char* name)
    : QHeader(n, parent, name)
{
}

RowHeader::~RowHeader()
{
}

void
RowHeader::paintSectionLabel(QPainter* p, int index, const QRect& fr)
{
    int section = mapToSection(index);
    if (section < 0)
	return;

    QString s = tr("%1").arg(section + 1);
    QRect r(fr.x() + QH_MARGIN, fr.y() + 2, fr.width() - 6, fr.height() - 4);

    p->setPen(colorGroup().buttonText());
    p->drawText(r, AlignVCenter | AlignRight, s);
}
