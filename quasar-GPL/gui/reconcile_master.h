// $Id: reconcile_master.h,v 1.6 2004/01/31 01:50:31 arandell Exp $
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

#ifndef RECONCILE_MASTER_H
#define RECONCILE_MASTER_H

#include "reconcile.h"
#include "data_window.h"

class LookupEdit;
class DatePopup;
class NumberEdit;
class ListView;
class QLabel;

class ReconcileMaster: public DataWindow {
    Q_OBJECT
public:
    ReconcileMaster(MainWindow* main, Id reconcile_id=INVALID_ID);
    ~ReconcileMaster();

protected slots:
    void slotAccountChanged();
    void slotStmtDateChanged();
    void slotEndDateChanged();
    void slotReconciledChanged();
    void recalculate();

protected:
    virtual void oldItem();
    virtual void newItem();
    virtual void cloneFrom(Id id);
    virtual bool fileItem();
    virtual bool deleteItem();
    virtual void restoreItem();
    virtual void cloneItem();
    virtual bool isChanged();
    virtual void dataToWidget();
    virtual void widgetToData();
    virtual bool dataValidate();
    virtual void printItem(bool ask=false);

    void loadData();

    Reconcile _orig;
    Reconcile _curr;

    // Widgets
    QPushButton* _print;
    LookupEdit* _account;
    DatePopup* _stmt_date;
    DatePopup* _end_date;
    NumberEdit* _open_balance;
    NumberEdit* _stmt_balance;
    ListView* _deposits;
    ListView* _cheques;
    QLabel* _deposit_cnt;
    QLabel* _deposit_total;
    QLabel* _cheque_cnt;
    QLabel* _cheque_total;
    QLabel* _stmt_total;
    QLabel* _cleared_total;
    QLabel* _difference;

    int depositCnt, chequeCnt;
    fixed depositTotal, chequeTotal;
    fixed clearedTotal, difference;
};

#endif // RECONCILE_MASTER_H
