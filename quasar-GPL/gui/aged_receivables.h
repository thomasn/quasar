// $Id: aged_receivables.h,v 1.17 2004/09/08 15:05:32 bpepers Exp $
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

#ifndef AGED_RECEIVABLES_H
#define AGED_RECEIVABLES_H

#include "quasar_window.h"
#include "gltx.h"
#include "customer.h"
#include "store.h"

class LookupEdit;
class ListView;
class NumberEdit;
class DatePopup;
class QFrame;
class QRadioButton;
class QTabWidget;

struct AgeTab {
    QFrame* frame;
    ListView* lines;
    NumberEdit* balanceTotal;
    NumberEdit* currentTotal;
    NumberEdit* pd30Total;
    NumberEdit* pd60Total;
    NumberEdit* pd90Total;
    NumberEdit* pd120Total;
    NumberEdit* balancePercent;
    NumberEdit* currentPercent;
    NumberEdit* pd30Percent;
    NumberEdit* pd60Percent;
    NumberEdit* pd90Percent;
    NumberEdit* pd120Percent;
};

class AgedReceivables: public QuasarWindow {
    Q_OBJECT
public:
    AgedReceivables(MainWindow* main);
    ~AgedReceivables();

    void setStoreId(Id store_id);
    void setCustomerId(Id customer_id);

protected slots:
    void slotTypeChanged(int index);
    void slotPickLine();
    void slotRefresh();
    void slotPrint();

protected:
    struct AgeInfo {
	Id id;
	QString name;
	QString number;
	QString cardNum;
	fixed balance;
	fixed current;
	fixed pd30;
	fixed pd60;
	fixed pd90;
	fixed pd120;
    };
    vector<AgeInfo> _info;
    vector<Gltx> _gltxs;
    vector<Customer> _customers;
    vector<Store> _stores;

    void buildTab(AgeTab& tab);
    void addGltx(Gltx& gltx);
    bool findCustomer(Id customer_id, Customer& customer);
    bool findStore(Id store_id, Store& store);

    QRadioButton* _all;
    LookupEdit* _type;
    LookupEdit* _group;
    LookupEdit* _patgroup;
    QRadioButton* _one;
    LookupEdit* _customer;
    DatePopup* _date;
    LookupEdit* _store;
    QTabWidget* _tabs;
    AgeTab _debits;
    AgeTab _credits;
};

#endif // AGED_RECEIVABLES_H
