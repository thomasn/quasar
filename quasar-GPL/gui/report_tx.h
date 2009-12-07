// $Id: report_tx.h,v 1.14 2005/01/30 04:25:31 bpepers Exp $
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

#ifndef REPORT_TX_H
#define REPORT_TX_H

#include "quasar_window.h"
#include "object_cache.h"
#include "gltx.h"
#include "customer.h"
#include "store.h"
#include "account.h"
#include "tender.h"
#include "tax.h"
#include "dept.h"
#include "subdept.h"
#include "item.h"
#include "discount.h"
#include "invoice.h"
#include "receive.h"

class DateRange;
class DatePopup;
class LookupEdit;
class SubdeptLookup;
class ListView;
class ComboBox;
class QCheckBox;
class QTabWidget;
class QHeader;
class QFrame;
class Grid;

class ReportTx: public QuasarWindow {
    Q_OBJECT
public:
    ReportTx(MainWindow* main);
    ~ReportTx();

    void setDates(QDate from, QDate to);
    void setStoreId(Id store_id);
    void setStationId(Id station_id);
    void setEmployeeId(Id employee_id);
    void setShiftId(Id shift_id);
    void refresh();

protected slots:
    void slotTabChanged();
    void slotTypesSizeChange(int section, int oldSize, int newSize);
    void slotSalesSizeChange(int section, int oldSize, int newSize);
    void slotAccountsSizeChange(int section, int oldSize, int newSize);
    void slotTendersSizeChange(int section, int oldSize, int newSize);
    void slotReceivablesSizeChange(int section, int oldSize, int newSize);
    void slotSafeSizeChange(int section, int oldSize, int newSize);
    void slotTaxesSizeChange(int section, int oldSize, int newSize);
    void slotDiscountsSizeChange(int section, int oldSize, int newSize);
    void slotAllTypes();
    void slotAllDepts();
    void slotAllSubdepts();
    void slotAllAccounts();
    void slotAllTenders();
    void slotAllReceivables();
    void slotAllSafe();
    void slotAllTaxes();
    void slotAllDiscounts();
    void slotPickTypes();
    void slotPickSales();
    void slotPickAccounts();
    void slotPickTenders();
    void slotPickReceivables();
    void slotPickSafe();
    void slotPickTaxes();
    void slotPickDiscounts();
    void slotDeptChanged();
    void slotSubdeptChanged();
    void slotRefresh();
    void slotUpdateTypes();
    void slotUpdateSales();
    void slotUpdateAccounts();
    void slotUpdateTenders();
    void slotUpdateReceivables();
    void slotUpdateSafe();
    void slotUpdateTaxes();
    void slotUpdateDiscounts();
    void slotPrint();

protected:
    DateRange* _range;
    DatePopup* _from;
    DatePopup* _to;
    LookupEdit* _shift;
    LookupEdit* _station;
    LookupEdit* _employee;
    LookupEdit* _card;
    LookupEdit* _store;
    QCheckBox* _cashrecOnly;
    QTabWidget* _tabs;

    // Transaction types frame
    QFrame* _typeFrame;
    ComboBox* _type;
    ListView* _types;
    QHeader* _typesFooter;

    // Sales frame
    QFrame* _salesFrame;
    LookupEdit* _dept;
    SubdeptLookup* _subdeptLookup;
    LookupEdit* _subdept;
    ListView* _sales;
    QHeader* _salesFooter;

    // Account postings frame
    QFrame* _accountFrame;
    LookupEdit* _account;
    ListView* _accounts;
    QHeader* _accountsFooter;

    // Tenders frame
    QFrame* _tenderFrame;
    LookupEdit* _tender;
    ListView* _tenders;
    QHeader* _tendersFooter;

    // Receivables frame
    QFrame* _recFrame;
    LookupEdit* _recAccount;
    ListView* _recs;
    QHeader* _recsFooter;

    // Safe frame
    QFrame* _safeFrame;
    LookupEdit* _safeTender;
    ListView* _safe;
    QHeader* _safeFooter;

    // Taxes frame
    QFrame* _taxFrame;
    LookupEdit* _tax;
    ListView* _taxes;
    QHeader* _taxesFooter;

    // Discounts frame
    QFrame* _discountFrame;
    LookupEdit* _discount;
    ListView* _discounts;
    QHeader* _discountsFooter;

    // Initialize lists
    void initializeTypes();
    void initializeSales();
    void initializeAccounts();
    void initializeTenders();
    void initializeReceivables();
    void initializeSafe();
    void initializeTaxes();
    void initializeDiscounts();

    // Convert tab to grid for printing
    QString buildTitle(const QString& name);
    Grid* typesGrid();
    Grid* salesGrid();
    Grid* accountsGrid();
    Grid* tendersGrid();
    Grid* receivablesGrid();
    Grid* safeGrid();
    Grid* taxesGrid();
    Grid* discountsGrid();

    // Printing code
    void printTypes();
    void printSales();
    void printAccounts();
    void printTenders();
    void printReceivables();
    void printSafe();
    void printTaxes();
    void printDiscounts();
    void printSummary();

    // Cache common data for speed
    ObjectCache _cache;
    vector<Gltx> _gltxs;

    bool _dirtyTypes;
    bool _dirtySales;
    bool _dirtyAccounts;
    bool _dirtyTenders;
    bool _dirtyReceivables;
    bool _dirtySafe;
    bool _dirtyTaxes;
    bool _dirtyDiscounts;
};

#endif // REPORT_TX_H
