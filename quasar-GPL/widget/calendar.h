// $Id: calendar.h,v 1.9 2005/02/13 10:29:08 bpepers Exp $
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

#ifndef CALENDAR_H
#define CALENDAR_H

#include <qlabel.h>
#include <qcombobox.h>

#include "date_time.h"

class Calendar: public QFrame {
    Q_OBJECT
public:
    Calendar(QWidget* parent=0, const char* name=0, WFlags f=0);

    QDate date() { return _date; }
    void setFont(const QFont& font);

signals:
    void dateSelected(QDate);

public slots:
    void setDate(QDate date);
    void prevMonth();
    void nextMonth();
    void prevYear();
    void nextYear();
    void prevDay();
    void nextDay();
    void prevWeek();
    void nextWeek();

private slots:
    void setMonth(int month);
    void setYear(int year);

protected:
    void keyPressEvent(QKeyEvent* e);
    bool eventFilter(QObject* o, QEvent* e);

    void setPixmaps();
    int daysInMonth(int year, int month);
    void normalizeDate(int& year, int& month, int& day, bool truncateDay);
    void recalc();

    QLabel* _prevMonth;
    QLabel* _nextMonth;
    QComboBox* _month;
    QComboBox* _year;
    QLabel* _dayLabels[7];
    QFrame* _line;
    QLabel* _days[42];

    QDate _date;
    int _indexStart;
    int _indexEnd;
};

#endif
