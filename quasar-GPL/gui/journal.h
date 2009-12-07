// $Id: journal.h,v 1.18 2005/01/30 04:25:31 bpepers Exp $
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

#ifndef JOURNAL_H
#define JOURNAL_H

#include "account.h"
#include "gltx.h"
#include "store.h"
#include "quasar_window.h"

class DateRange;
class DatePopup;
class LookupEdit;
class QTabWidget;
class JournalWindow;
class ListViewItem;

class Journal: public QuasarWindow {
    Q_OBJECT
public:
    Journal(MainWindow* main);
    ~Journal();

    void setStoreId(Id store_id);
    void setCurrentTab(int tab);

protected slots:
    void slotRefresh();
    void slotRealRefresh();
    void slotPrint();
    void slotEdit();
    void closeEvent(QCloseEvent* e);

protected:
    void loadDate(QDate date);
    void addToList(JournalWindow* window, const Gltx& gltx);

    vector<Account> _accounts;
    vector<Gltx> _gltxs;
    bool _refreshPending;
    ListViewItem* _after;
    bool _loading;
    QDate _lastStart;
    QDate _lastEnd;

    vector<Store> _store_cache;
    bool findStore(Id store_id, Store& store);

    DateRange* _dateRange;
    DatePopup* _from;
    DatePopup* _to;
    LookupEdit* _store;
    QTabWidget* _tabs;
    JournalWindow* _general;
    JournalWindow* _disbursements;
    JournalWindow* _receipts;
    JournalWindow* _sales;
    JournalWindow* _purchases;
    JournalWindow* _inventory;
    JournalWindow* _all;
};

#endif // JOURNAL_H
