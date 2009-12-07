// $Id: time_edit.cpp,v 1.5 2005/01/31 23:22:34 bpepers Exp $
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

#include "time_edit.h"

TimeEdit::TimeEdit(QWidget* parent, const char* name)
    : ValconEdit(_timeValcon, parent, name)
{
    QString text = _timeValcon.format(QTime(20, 33, 33));
    _sizeChar = '9';
    _sizeLength = text.length();

    setMinimumWidth(calcMinimumWidth());
    connect(this, SIGNAL(validData()), SLOT(validData()));
}

TimeEdit::~TimeEdit()
{
}

int
TimeEdit::calcMinimumWidth()
{
    QString text = _timeValcon.format(QTime(20, 33, 33));
    return fontMetrics().width(text + "xx");
}

Variant
TimeEdit::value()
{
    Variant value;
    if (!_timeValcon.parse(text()))
	return value;

    value = _timeValcon.getTime();
    return value;
}

QTime
TimeEdit::getTime()
{
    QTime time;
    if (_timeValcon.parse(text()))
	time = _timeValcon.getTime();
    return time;
}

void
TimeEdit::setValue(Variant value)
{
    setText(_timeValcon.format(value));
}

void
TimeEdit::setTime(QTime time)
{
    setText(_timeValcon.format(time));
    validate(Return);
}

void
TimeEdit::validData()
{
    emit timeChanged(getTime());
}
