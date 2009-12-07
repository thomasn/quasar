// $Id: pay_bills.h,v 1.9 2004/01/31 01:50:31 arandell Exp $
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

#ifndef PAY_BILLS_H
#define PAY_BILLS_H

#include "quasar_window.h"
#include "gltx.h"
#include "term.h"
#include "data_window.h"
#include "variant.h"
#include <qguardedptr.h>

class DatePopup;
class TimeEdit;
class LookupEdit;
class AccountLookup;
class NumberEdit;
class MoneyEdit;
class Table;
class QRadioButton;
class InvoiceInfo;

class PayBills: public QuasarWindow {
    Q_OBJECT
public:
    PayBills(MainWindow* main);
    ~PayBills();

public slots:
    void setDiscount(fixed disc_amt);

protected slots:
    void slotOk();
    void slotNext();
    void slotCancel();
    void slotAccountChanged();
    void slotPayClicked();
    void slotPayChanged(int button);
    void slotShowChanged(int button);
    void slotPayAll();
    void slotPayDue();
    void slotClearAll();
    void slotInvoiceInfo();
    void slotRefresh();
    void cellMoved(int row, int col);
    void cellChanged(int row, int col, Variant old);
    void focusNext(bool& leave, int& newRow, int& newcol, int type);
    void recalculate();

protected:
    vector<Gltx> _gltxs;
    vector<Term> _terms;
    vector<fixed> _suggest;
    QGuardedPtr<InvoiceInfo> _info;

    DatePopup* _date;
    TimeEdit* _time;
    LookupEdit* _store;
    LookupEdit* _station;
    LookupEdit* _employee;
    NumberEdit* _nextCheque;
    AccountLookup* _lookup;
    LookupEdit* _account;
    QRadioButton* _showAll;
    QRadioButton* _showDue;
    DatePopup* _dueDate;
    LookupEdit* _disc;
    Table* _table;
    MoneyEdit* _current;
    MoneyEdit* _total;
    MoneyEdit* _ending;
};

#endif // PAY_BILLS_H
