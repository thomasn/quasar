// $Id: calendar.cpp,v 1.18 2005/03/05 11:04:05 bpepers Exp $
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

#include "calendar.h"
#include "icu_util.h"

#include <qpixmap.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qstyle.h>
#include <unicode/dtfmtsym.h>

Calendar::Calendar(QWidget* parent, const char* name, WFlags f)
    : QFrame(parent, name, f)
{
    setFrameStyle(WinPanel | Raised);

    _prevMonth = new QLabel(this);
    _nextMonth = new QLabel(this);
    _prevMonth->installEventFilter(this);
    _nextMonth->installEventFilter(this);
    setPixmaps();

    UErrorCode status = U_ZERO_ERROR;
    DateFormatSymbols symbols(status);
    int32_t count;
    const UnicodeString* months = symbols.getMonths(count);
    const UnicodeString* weekDays = symbols.getShortWeekdays(count);

    _month = new QComboBox(false, this);
    _month->setFocusPolicy(NoFocus);
    for (int month = 1; month <= 12; ++month) {
	UnicodeString uname = months[month - 1];
	QString name = convertToQt(uname);
	_month->insertItem(name);
    }
    connect(_month, SIGNAL(activated(int)), SLOT(setMonth(int)));

    _year = new QComboBox(false, this);
    _year->setFocusPolicy(NoFocus);
    for (int year = 2100; year >= 1900; --year)
	_year->insertItem(QString::number(year));
    connect(_year, SIGNAL(activated(int)), SLOT(setYear(int)));

    for (int dow = 0; dow < 7; ++dow) {
	UnicodeString uname = weekDays[dow+1];
	QString name = convertToQt(uname);

	QLabel* label = new QLabel(name, this);
	label->setAlignment(AlignCenter);
	_dayLabels[dow] = label;
    }

    _line = new QFrame(this);
    _line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    _line->setMinimumHeight(6);

    for (int day = 0; day < 42; ++day) {
	QLabel* label = new QLabel(" ", this);
	label->installEventFilter(this);
	label->setAlignment(AlignRight | AlignVCenter);
	_days[day] = label;
    }

    QVBoxLayout* l1 = new QVBoxLayout(this);
    l1->setMargin(4);
    l1->setSpacing(4);

    QGridLayout* l2 = new QGridLayout(l1, 1, 3, 2);
    l2->addWidget(_prevMonth, 0, 0);
    l2->addWidget(_month, 0, 1, AlignLeft);
    l2->addWidget(_year, 0, 2, AlignRight);
    l2->addWidget(_nextMonth, 0, 3);
    l2->setColStretch(1, 1);
    l2->setColStretch(2, 1);

    QGridLayout* l3 = new QGridLayout(l1, 8, 7, 2);
    for (int dow = 0; dow < 7; ++dow)
	l3->addWidget(_dayLabels[dow], 0, dow);
    l3->addMultiCellWidget(_line, 1, 1, 0, 6);
    for (int row = 0; row < 6; ++row) {
	for (int dow = 0; dow < 7; ++dow) {
	    l3->addWidget(_days[row * 7 + dow], row + 2, dow);
	    l3->setRowStretch(row + 2, 1);
	}
    }
}

void
Calendar::setFont(const QFont& font)
{
    QWidget::setFont(font);
    setPixmaps();
}

void
Calendar::setPixmaps()
{
    int width = fontMetrics().height();
    QPixmap pixmap(width, width);
    QPainter painter;

    pixmap.fill(this, 0, 0);
    painter.begin(&pixmap);
    QRect rect(2, 2, width - 4, width - 4);
    style().drawPrimitive(QStyle::PE_ArrowLeft, &painter, rect, colorGroup());
    painter.end();
    _prevMonth->setPixmap(pixmap);

    pixmap.fill(this, 0, 0);
    painter.begin(&pixmap);
    style().drawPrimitive(QStyle::PE_ArrowRight, &painter, rect, colorGroup());
    painter.end();
    _nextMonth->setPixmap(pixmap);
}

void
Calendar::setDate(QDate date)
{
    _date = date;
    recalc();
}

void
Calendar::prevMonth()
{
    int year = _date.year();
    int month = _date.month() - 1;
    int day = _date.day();
    normalizeDate(year, month, day, true);
    _date.setYMD(year, month, day);
    recalc();
}

void
Calendar::nextMonth()
{
    int year = _date.year();
    int month = _date.month() + 1;
    int day = _date.day();
    normalizeDate(year, month, day, true);
    _date.setYMD(year, month, day);
    recalc();
}

void
Calendar::prevYear()
{
    int year = _date.year() - 1;
    int month = _date.month();
    int day = _date.day();
    normalizeDate(year, month, day, true);
    _date.setYMD(year, month, day);
    recalc();
}

void
Calendar::nextYear()
{
    int year = _date.year() + 1;
    int month = _date.month();
    int day = _date.day();
    normalizeDate(year, month, day, true);
    _date.setYMD(year, month, day);
    recalc();
}

void
Calendar::prevDay()
{
    int year = _date.year();
    int month = _date.month();
    int day = _date.day() - 1;
    normalizeDate(year, month, day, false);
    _date.setYMD(year, month, day);
    recalc();
}

void
Calendar::nextDay()
{
    int year = _date.year();
    int month = _date.month();
    int day = _date.day() + 1;
    normalizeDate(year, month, day, false);
    _date.setYMD(year, month, day);
    recalc();
}

void
Calendar::prevWeek()
{
    int year = _date.year();
    int month = _date.month();
    int day = _date.day() - 7;
    normalizeDate(year, month, day, false);
    _date.setYMD(year, month, day);
    recalc();
}

void
Calendar::nextWeek()
{
    int year = _date.year();
    int month = _date.month();
    int day = _date.day() + 7;
    normalizeDate(year, month, day, false);
    _date.setYMD(year, month, day);
    recalc();
}

void
Calendar::setMonth(int newMonth)
{
    int year = _date.year();
    int month = newMonth + 1;
    int day = _date.day();
    normalizeDate(year, month, day, true);
    _date.setYMD(year, month, day);
    recalc();
}

void
Calendar::setYear(int newYear)
{
    int year = 2100 - newYear;
    int month = _date.month();
    int day = _date.day();
    normalizeDate(year, month, day, true);
    _date.setYMD(year, month, day);
    recalc();
}

int
Calendar::daysInMonth(int year, int month)
{
    int m = month;
    while (m < 1) m += 12;
    while (m > 12) m -= 12;
    return QDate(year, m, 1).daysInMonth();
}

void
Calendar::normalizeDate(int& year, int& month, int& day, bool truncateDay)
{
    // Adjust for day overflow
    if (truncateDay) {
	if (day > daysInMonth(year, month)) {
	    day = daysInMonth(year, month);
	}
    } else {
	while (day > daysInMonth(year, month)) {
	    day -= daysInMonth(year, month);
	    ++month;
	}
    }

    // Adjust for day underflow
    while (day < 1) {
	--month;
	day += daysInMonth(year, month);
    }

    // Adjust for month overflow
    while (month > 12) {
	++year;
	month -= 12;
    }

    // Adjust for month underflow
    while (month < 1) {
	--year;
	month += 12;
    }
}

void
Calendar::keyPressEvent(QKeyEvent* e)
{
    switch (e->key()) {
    case Key_Up:	prevWeek(); break;
    case Key_Down:	nextWeek(); break;
    case Key_Left:	prevDay(); break;
    case Key_Right:	nextDay(); break;
    case Key_PageUp:	prevMonth(); break;
    case Key_PageDown:	nextMonth(); break;
    case Key_Minus:	prevYear(); break;
    case Key_Plus:	nextYear(); break;
    case Key_Return:	emit dateSelected(_date); break;
    case Key_Enter:	emit dateSelected(_date); break;
    case Key_Space:	emit dateSelected(_date); break;
    case Key_Home:	setDate(QDate(_date.year(), 1, 1)); break;
    case Key_End:	setDate(QDate(_date.year(), 12, 31)); break;
    case Key_D:		setDate(QDate::currentDate()); break;
    default:		QWidget::keyPressEvent(e);
    }
}

bool
Calendar::eventFilter(QObject* obj, QEvent* e)
{
    if (e->type() != QEvent::MouseButtonRelease)
	return false;

    if (obj == _prevMonth) {
	prevMonth();
	return false;
    }

    if (obj == _nextMonth) {
	nextMonth();
	return false;
    }

    QLabel* label = NULL;
    int index = 0;

    for (int row = 0; row < 6 && label == NULL; ++row) {
	for (int dow = 0; dow < 7 && label == NULL; ++dow) {
	    index = row * 7 + dow;
	    if (_days[index] == obj)
		label = _days[index];
	}
    }

    if (label != NULL && index >= _indexStart && index < _indexEnd) {
	int day = index - _indexStart + 1;
	_date.setYMD(_date.year(), _date.month(), day);
	recalc();
	emit dateSelected(_date);
    }

    return false;
}

void
Calendar::recalc()
{
    if (_date.isNull()) _date = QDate::currentDate();

    _month->setCurrentItem(_date.month() - 1);
    _year->setCurrentItem(2100 - _date.year());

    QDate monthStart(_date.year(), _date.month(), 1);
    _indexStart = monthStart.dayOfWeek();
    _indexEnd = _indexStart + _date.daysInMonth();

    for (int row = 0; row < 6; ++row) {
	for (int dow = 0; dow < 7; ++dow) {
	    int index = row * 7 + dow;
	    QLabel* label = _days[index];

	    if (index < _indexStart || index >= _indexEnd) {
		label->setText("");
		label->setFrameStyle(QFrame::NoFrame);
	    } else {
		int day = index - _indexStart + 1;
		label->setText(QString::number(day));
		if (day == _date.day()) {
		    label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
		    label->setLineWidth(1);
		    label->setMargin(0);
		} else {
		    label->setFrameStyle(QFrame::NoFrame);
		    label->setLineWidth(0);
		    label->setMargin(1);
		}
	    }
	}
    }
}
