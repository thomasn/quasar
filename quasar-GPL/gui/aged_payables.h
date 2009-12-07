// $Id: aged_payables.h,v 1.10 2004/01/31 01:50:31 arandell Exp $
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

#ifndef AGED_PAYABLES_H
#define AGED_PAYABLES_H

#include "quasar_window.h"
#include "gltx.h"
#include "vendor.h"
#include "store.h"

class LookupEdit;
class ListView;
class NumberEdit;
class QComboBox;

class AgedPayables: public QuasarWindow {
    Q_OBJECT
public:
    AgedPayables(MainWindow* main);
    ~AgedPayables();

    void setStoreId(Id store_id);
    void setVendorId(Id vendor_id);

protected slots:
    void slotLevelChanged();
    void slotPickLine();
    void slotRefresh();
    void slotPrint();

protected:
    struct AgeInfo {
	Id id;
	QString name;
	QString number;
	fixed balance;
	fixed current;
	fixed pd30;
	fixed pd60;
	fixed pd90;
	fixed pd120;
    };
    vector<AgeInfo> _info;
    vector<Gltx> _gltxs;
    vector<Vendor> _vendors;
    vector<Store> _stores;

    void addGltx(const Gltx& gltx);
    bool findVendor(Id vendor_id, Vendor& vendor);
    bool findStore(Id store_id, Store& store);

    QComboBox* _level;
    LookupEdit* _store;
    LookupEdit* _account;
    LookupEdit* _vendor;
    LookupEdit* _group;
    ListView* _lines;
    NumberEdit* _balanceTotal;
    NumberEdit* _currentTotal;
    NumberEdit* _pd30Total;
    NumberEdit* _pd60Total;
    NumberEdit* _pd90Total;
    NumberEdit* _pd120Total;
    NumberEdit* _balancePercent;
    NumberEdit* _currentPercent;
    NumberEdit* _pd30Percent;
    NumberEdit* _pd60Percent;
    NumberEdit* _pd90Percent;
    NumberEdit* _pd120Percent;
};

#endif // AGED_PAYABLES_H
