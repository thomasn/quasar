// $Id: date_edit.h,v 1.19 2005/01/30 04:40:58 bpepers Exp $
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

#ifndef DATE_EDIT_H
#define DATE_EDIT_H

#include "valcon_edit.h"
#include "date_valcon.h"

class DateEdit: public ValconEdit {
    Q_OBJECT
public:
    DateEdit(QWidget* parent, const char* name=0);
    ~DateEdit();

    int calcMinimumWidth();

    Variant value();
    QDate getDate();

signals:
    void dateChanged(QDate date);

public slots:
    void setValue(Variant value);
    void setDate(QDate date);
    bool popup();

protected slots:
    void validData();

protected:
    DateValcon _dateValcon;
};

#endif // DATE_EDIT_H
