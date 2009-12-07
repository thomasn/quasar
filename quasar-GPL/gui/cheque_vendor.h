// $Id: cheque_vendor.h,v 1.18 2004/01/31 01:50:31 arandell Exp $
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

#ifndef CHEQUE_VENDOR_H
#define CHEQUE_VENDOR_H

#include "cheque.h"
#include "gltx.h"
#include "term.h"
#include "data_window.h"
#include "variant.h"
#include <qguardedptr.h>

class LookupEdit;
class GltxFrame;
class MoneyEdit;
class LineEdit;
class Table;
class QCheckBox;
class InvoiceInfo;

class ChequeVendor: public DataWindow {
    Q_OBJECT
public:
    ChequeVendor(MainWindow* main, Id cheque_id=INVALID_ID);
    ~ChequeVendor();

    void setStoreId(Id store_id);
    void setCardId(Id card_id);

    void payReceiving(Id receive_id);

public slots:
    void setDiscount(fixed disc_amt);

protected slots:
    void slotRecurring();
    void slotAccountChanged();
    void slotVendorChanged();
    void slotPayClicked();
    void slotPayAll();
    void slotClearAll();
    void slotInvoiceInfo();
    void slotRefresh();
    void cellMoved(int row, int col);
    void cellChanged(int row, int col, Variant old);
    void focusNext(bool& leave, int& newRow, int& newcol, int type);
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
    virtual void printItem(bool ask=false);

    fixed getChequeNumber(Id bankId);
    vector<Gltx> _gltxs;
    vector<Term> _terms;
    vector<fixed> _suggest;
    QGuardedPtr<InvoiceInfo> _info;

    Cheque _orig;
    Cheque _curr;

    // Widgets
    LookupEdit* _account;
    LookupEdit* _discount;
    MoneyEdit* _current;
    MoneyEdit* _ending;
    GltxFrame* _gltxFrame;
    LookupEdit* _vendor;
    MoneyEdit* _balance;
    MoneyEdit* _amount;
    LineEdit* _memo;
    Table* _table;
    MoneyEdit* _payment;
    MoneyEdit* _discAmt;
    MoneyEdit* _total;
    QCheckBox* _printed;
};

#endif // CHEQUE_VENDOR_H
