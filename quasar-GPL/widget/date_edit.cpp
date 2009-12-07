// $Id: date_edit.cpp,v 1.25 2005/01/30 04:40:58 bpepers Exp $
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

#include "date_edit.h"
#include "calendar.h"

DateEdit::DateEdit(QWidget* parent, const char* name)
    : ValconEdit(_dateValcon, parent, name)
{
    QString text = _dateValcon.format(QDate(2000, 12, 30));
    _sizeChar = '9';
    _sizeLength = text.length();

    setMinimumWidth(calcMinimumWidth());
    connect(this, SIGNAL(validData()), SLOT(validData()));
}

DateEdit::~DateEdit()
{
}

int
DateEdit::calcMinimumWidth()
{
    int width = 0;
    for (int month = 1; month < 12; ++month) {
	QString text = _dateValcon.format(QDate(2000, month, 28));
	int monthWidth = fontMetrics().width(text + "xx");
	if (monthWidth > width) width = monthWidth;
    }
    return width;
}

Variant
DateEdit::value()
{
    Variant value;
    if (!_dateValcon.parse(text()))
	return value;

    value = _dateValcon.getDate();
    return value;
}

QDate
DateEdit::getDate()
{
    QDate date;
    if (_dateValcon.parse(text()))
	date = _dateValcon.getDate();
    return date;
}

void
DateEdit::setValue(Variant value)
{
    setText(_dateValcon.format(value));
}

void
DateEdit::setDate(QDate date)
{
    setText(_dateValcon.format(date));
    validate(Return);
}

bool
DateEdit::popup()
{
#if 0
    Calendar* cal = new Calendar(this, "popup", true, Qt::WType_Popup);
    bool result = cal->exec();
    if (result) setDate(cal->date());

    delete cal;
    return result;
#else
    return true;
#endif
}

void
DateEdit::validData()
{
    emit dateChanged(getDate());
}
