// $Id: date_popup.cpp,v 1.8 2005/01/30 04:40:58 bpepers Exp $
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

#include "date_popup.h"
#include "calendar.h"

#include <qpushbutton.h>
#include <qpixmap.h>

static const char* calendar_xpm[] = {
"16 16 2 1",
"       c None",
".      c #00000000FFFF",
"                ",
"                ",
"                ",
"  ...........   ",
"  .         .   ",
"  .         .   ",
"  ...........   ",
"  . . . . . .   ",
"  ...........   ",
"  . . . . . .   ",
"  ...........   ",
"  . . . . . .   ",
"  ...........   ",
"                ",
"                ",
"                "};

DatePopup::DatePopup(QWidget* parent, const char* name)
    : Popup(new DateEdit(0), new QPushButton(0), parent, name)
{
    buttonWidget()->setPixmap(QPixmap(calendar_xpm));
    buttonWidget()->setFocusPolicy(QWidget::NoFocus);

    _cal = new Calendar(this, "popup", Qt::WType_Popup);
    setPopupWidget(_cal);

    connect(_cal, SIGNAL(dateSelected(QDate)), SLOT(dateSelected(QDate)));
}

void
DatePopup::openPopup()
{
    _cal->setDate(getDate());
    Popup::openPopup();
}

void
DatePopup::dateSelected(QDate date)
{
    setDate(date);
    closePopup();
    dateWidget()->setFocus();
    dateWidget()->selectAll();
}
