// $Id: cash_reconcile.h,v 1.11 2004/01/31 01:50:31 arandell Exp $
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

#ifndef CASH_RECONCILE_H
#define CASH_RECONCILE_H

#include "quasar_window.h"
#include "company.h"
#include "shift.h"
#include "tender_count.h"
#include "tender.h"

class DatePopup;
class LookupEdit;
class ListView;
class QTabWidget;
class QFrame;
class QPushButton;

struct SummaryLine {
    Tender tender;
    fixed shiftsAmt;
    fixed countsAmt;
};

class CashReconcile: public QuasarWindow {
    Q_OBJECT
public:
    CashReconcile(MainWindow* main);
    ~CashReconcile();

protected slots:
    void slotLineChanged();
    void slotLineSelected();
    void slotShiftSelected();
    void slotCountSelected();
    void slotShiftClose();
    void slotCreateCount();
    void slotBankDeposit();
    void slotTenderAdjust();
    void slotTenderTransfer();
    void slotRefresh();
    void slotSummary();
    void slotReconcile();

protected:
    DatePopup* _date;
    LookupEdit* _store;
    ListView* _lines;
    QTabWidget* _tabs;
    QFrame* _info;
    ListView* _shiftList;
    ListView* _countList;
    QFrame* _setup;
    LookupEdit* _safeStore;
    LookupEdit* _safeId;
    LookupEdit* _adjust;
    LookupEdit* _transfer;
    QPushButton* _shiftClose;
    QPushButton* _countCreate;
    QPushButton* _bankDeposit;
    QPushButton* _reconcile;

    Company _company;
    vector<Shift> _shifts;
    vector<TenderCount> _counts;
    vector<Tender> _tenders;
    vector<SummaryLine> _summary;

    Id stationId();
    Id employeeId();
    void addShiftAmt(Id tender_id, fixed shift_amt);
    void addCountAmt(Id tender_id, fixed count_amt);
    bool findTender(Id tender_id, Tender& tender);
};

#endif // CASH_RECONCILE_H
