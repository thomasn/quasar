// $Id: tx_summary.h,v 1.6 2005/01/30 04:25:31 bpepers Exp $
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

#ifndef TX_SUMMARY_H
#define TX_SUMMARY_H

#include "quasar_window.h"
#include "gltx.h"
#include "employee.h"
#include "store.h"
#include "account.h"
#include "tender.h"
#include "tax.h"
#include "dept.h"
#include "subdept.h"
#include "item.h"
#include "discount.h"

class DateRange;
class DatePopup;
class LookupEdit;
class ListView;
class ListViewItem;
class QTabWidget;
class QFrame;

struct EmployeeLine {
    EmployeeLine();

    int count;
    vector<Id> tenders;
    vector<fixed> amounts;

    void addAmount(Id tender_id, fixed amount);
    void addLine(const EmployeeLine& line);

    fixed getAmount(Id tender_id) const;
};

struct EmployeeInfo {
    EmployeeInfo();

    Employee employee;
    EmployeeLine sales;
    EmployeeLine returns;
    EmployeeLine payouts;
    EmployeeLine payments;
    EmployeeLine withdraws;
    EmployeeLine nosales;
    EmployeeLine others;
    EmployeeLine deposits;
    EmployeeLine overShorts;
    int voidTxCnt;
    int voidItemCnt;
    fixed discountTotal;
    fixed itemQtyTotal;

    void addInfo(const EmployeeInfo& info);
};

class TxSummary: public QuasarWindow {
    Q_OBJECT
public:
    TxSummary(MainWindow* main);
    ~TxSummary();

    void setDates(QDate from, QDate to);
    void refresh();

protected slots:
    void slotRefresh();
    void slotProcess();
    void slotPrint();

protected:
    DateRange* _range;
    DatePopup* _from;
    DatePopup* _to;
    LookupEdit* _store;
    LookupEdit* _station;
    LookupEdit* _employee;

    // Tabs for information on employees or stations
    QTabWidget* _tabs;

    // Processing for each type of transaction
    vector<Gltx> _gltxs;
    void processGltx(const Gltx& gltx);
    void processInvoice(EmployeeInfo& info, const Gltx& gltx);
    void processReturn(EmployeeInfo& info, const Gltx& gltx);
    void processPayout(EmployeeInfo& info, const Gltx& gltx);
    void processPayment(EmployeeInfo& info, const Gltx& gltx);
    void processWithdraw(EmployeeInfo& info, const Gltx& gltx);
    void processNosale(EmployeeInfo& info, const Gltx& gltx);
    void processTenderAdjust(EmployeeInfo& info, const Gltx& gltx);
    void processOther(EmployeeInfo& info, const Gltx& gltx);

    vector<Tender> _tenders;
    vector<int> _tenderCnts;
    void countTender(Id tender_id);
    bool findTender(Id tender_id, Tender& tender);

    vector<EmployeeInfo> _employees;
    EmployeeInfo& findEmployeeInfo(const Employee& employee);

    vector<QFrame*> _frames;
    void addFrame(EmployeeInfo& info);

    ListViewItem* _last;
    void listItem(ListView* l, const QString& n, const EmployeeLine& info);

    // Cache common data for speed
    bool findAccount(Id account_id, Account& account);
    bool findTax(Id tax_id, Tax& tax);
    bool findCard(Id card_id, Card& card);
    bool findEmployee(Id employee_id, Employee& employee);
    bool findStore(Id store_id, Store& store);
    bool findDept(Id dept_id, Dept& dept);
    bool findSubdept(Id subdept_id, Subdept& subdept);
    bool findItem(Id item_id, Item& item);
    bool findDiscount(Id discount_id, Discount& discount);

    vector<Account> _accountCache;
    vector<Tax> _taxCache;
    vector<Card> _cardCache;
    vector<Employee> _employeeCache;
    vector<Store> _storeCache;
    vector<Dept> _deptCache;
    vector<Subdept> _subdeptCache;
    vector<Item> _itemCache;
    vector<Discount> _discountCache;
};

#endif // TX_SUMMARY_H
