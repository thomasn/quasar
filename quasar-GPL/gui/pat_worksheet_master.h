// $Id: pat_worksheet_master.h,v 1.11 2004/05/11 09:13:05 bpepers Exp $
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

#ifndef PAT_WORKSHEET_MASTER_H
#define PAT_WORKSHEET_MASTER_H

#include "pat_worksheet.h"
#include "data_window.h"
#include "object_cache.h"

class LineEdit;
class LookupEdit;
class DatePopup;
class NumberEdit;
class QTabWidget;
class ListView;
class Grid;

class PatWorksheetMaster: public DataWindow {
    Q_OBJECT
public:
    PatWorksheetMaster(MainWindow*, Id worksheet_id=INVALID_ID);
    ~PatWorksheetMaster();

protected slots:
    void slotReload();
    void slotPrint();
    void slotPost();
    void slotSalesDetail();
    void slotPurchaseDetail();
    void slotBalanceDetail();
    void slotAddAdjustment();
    void slotEditAdjustment();
    void slotRemoveAdjustment();
    void slotSalesReformat();
    void slotPurchasesReformat();
    void slotBalancesReformat();
    void slotRefreshSales(bool totals=true);
    void slotRefreshPurchases(bool totals=true);
    void slotRefreshBalances(bool totals=true);
    void slotRefreshAdjustments(bool totals=true);
    void slotRefreshTotals();

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
    void refresh();

    PatWorksheet _orig;
    PatWorksheet _curr;

    // Widgets
    LineEdit* _desc;
    DatePopup* _from;
    DatePopup* _to;
    NumberEdit* _equity_amt;
    LineEdit* _equity_memo;
    LookupEdit* _equity_id;
    NumberEdit* _credit_amt;
    LineEdit* _credit_memo;
    LookupEdit* _credit_id;
    QTabWidget* _tabs;
    ListView* _totals;
    ListView* _sales;
    QCheckBox* _sales_cust_detail;
    QCheckBox* _sales_dept_detail;
    ListView* _purchases;
    QCheckBox* _purch_vend_detail;
    NumberEdit* _purch_pnts;
    ListView* _balances;
    QCheckBox* _bal_cust_detail;
    NumberEdit* _bal_pnts;
    ListView* _adjusts;

    ObjectCache _cache;
};

#endif // PAT_WORKSHEET_MASTER_H
