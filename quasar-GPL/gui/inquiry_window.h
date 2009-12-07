// $Id: inquiry_window.h,v 1.14 2005/01/30 04:25:31 bpepers Exp $
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

#ifndef INQUIRY_WINDOW_H
#define INQUIRY_WINDOW_H

#include <qframe.h>
#include "data_object.h"

class MainWindow;
class QuasarClient;
class LookupWindow;
class LookupEdit;
class DatePopup;
class ListView;
class QLabel;
class Grid;

class InquiryWindow: public QFrame {
    Q_OBJECT
public:
    InquiryWindow(MainWindow* main, QWidget* parent, const char* name,
		  LookupWindow* lookup);
    ~InquiryWindow();

    MainWindow* main;
    QuasarClient* quasar;
    LookupEdit* search;
    ListView* list;
    QLabel* labels[10];
    Grid* _grid;
    bool needsRefresh;
    bool loading;

public slots:
    virtual void slotRefresh(QDate from, QDate to, Id store_id)=0;
    virtual void slotPrint();
};

#endif // INQUIRY_WINDOW_H
