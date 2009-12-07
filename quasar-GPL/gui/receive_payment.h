// $Id: receive_payment.h,v 1.8 2004/01/31 01:50:31 arandell Exp $
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

#ifndef RECEIVE_PAYMENT_H
#define RECEIVE_PAYMENT_H

#include <qdialog.h>
#include "receive.h"
#include "variant.h"

class MainWindow;
class QuasarClient;
class LookupEdit;
class DatePopup;
class NumberEdit;
class LineEdit;
class Table;

class ReceivePayment: public QDialog {
    Q_OBJECT
public:
    ReceivePayment(QWidget* parent, MainWindow* main, Id receive_id);
    ~ReceivePayment();

protected slots:
    void cellChanged(int row, int col, Variant old);
    void focusNext(bool& leave, int& newRow, int& newcol, int type);

protected slots:
    void slotAutoAlloc();
    void slotQuickCheque();
    void recalculate();
    void accept();

protected:
    MainWindow* _main;
    QuasarClient* _quasar;
    Receive _receive;
    fixed _sign;

    // Widgets
    LineEdit* _number;
    LookupEdit* _vendor;
    DatePopup* _date;
    NumberEdit* _balance;
    NumberEdit* _total;
    NumberEdit* _paid;
    LineEdit* _memo;
    Table* _table;
};

#endif // RECEIVE_PAYMENT_H
