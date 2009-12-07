// $Id: date_popup.h,v 1.9 2005/01/30 04:40:58 bpepers Exp $
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

#ifndef DATE_POPUP_H
#define DATE_POPUP_H

#include "popup.h"
#include "date_edit.h"
class Calendar;

class DatePopup: public Popup {
    Q_OBJECT
public:
    DatePopup(QWidget* parent, const char* name=0);

    DateEdit* dateWidget() const { return (DateEdit*)textWidget(); }
    void setFocus() { dateWidget()->setFocus(); }

    Variant value() { return dateWidget()->value(); }
    void setValue(Variant value) { dateWidget()->setValue(value); }

    QDate getDate() { return dateWidget()->getDate(); }
    void setDate(QDate date) { dateWidget()->setDate(date); }

    bool valid() { return dateWidget()->valid(); }

public slots:
    void openPopup();
    void dateSelected(QDate date);

protected:
    Calendar* _cal;
};

#endif
