// $Id: company_master.h,v 1.9 2005/03/13 22:09:17 bpepers Exp $
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

#ifndef COMPANY_MASTER_H
#define COMPANY_MASTER_H

#include "company.h"
#include "data_window.h"

class LineEdit;
class LookupEdit;
class DatePopup;
class ComboBox;
class Table;
class QLabel;
class QGroupBox;
class QCheckBox;

class CompanyMaster: public DataWindow {
    Q_OBJECT
public:
    CompanyMaster(MainWindow*, Id company_id=INVALID_ID);
    ~CompanyMaster();

protected slots:
    void slotSetSafeWidget();
    void slotRoundingCellChanged(int row, int col, Variant);

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

    Company _orig;
    Company _curr;

    // Widgets
    LineEdit* _name;
    LineEdit* _number;
    QGroupBox* _addr;
    LineEdit* _street;
    LineEdit* _street2;
    LineEdit* _city;
    LineEdit* _province;
    LineEdit* _country;
    LineEdit* _postal;
    LineEdit* _phone_num;
    LineEdit* _phone2_num;
    LineEdit* _fax_num;
    LineEdit* _email;
    LineEdit* _web_page;

    // General Ledger
    LookupEdit* _defaultStore;
    DatePopup* _startDate;
    DatePopup* _closeDate;
    ComboBox* _shiftMethod;
    LookupEdit* _safeStore;
    QLabel* _safeStationLabel;
    LookupEdit* _safeStation;
    QLabel* _safeEmployeeLabel;
    LookupEdit* _safeEmployee;

    // Linked Accounts
    LookupEdit* _reAccount;
    LookupEdit* _hbAccount;
    LookupEdit* _bankAccount;
    LookupEdit* _customerAcct;
    LookupEdit* _vendorAcct;
    LookupEdit* _custTermsAcct;
    LookupEdit* _vendTermsAcct;
    LookupEdit* _depositAccount;
    LookupEdit* _transferAccount;
    LookupEdit* _physicalAccount;
    LookupEdit* _splitAccount;
    LookupEdit* _chargeAccount;
    LookupEdit* _overShortAccount;

    // Price rounding
    Table* _rounding;
};

#endif // COMPANY_MASTER_H
