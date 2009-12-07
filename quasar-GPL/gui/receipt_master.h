// $Id: receipt_master.h,v 1.23 2004/06/16 06:30:14 bpepers Exp $
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

#ifndef RECEIPT_MASTER_H
#define RECEIPT_MASTER_H

#include "receipt.h"
#include "gltx.h"
#include "term.h"
#include "data_window.h"
#include "variant.h"
#include <qguardedptr.h>

class GltxFrame;
class LookupEdit;
class NumberEdit;
class Table;
class InvoiceInfo;

class ReceiptMaster: public DataWindow {
    Q_OBJECT
public:
    ReceiptMaster(MainWindow* main, Id receipt_id=INVALID_ID);
    ~ReceiptMaster();

    void setStoreId(Id store_id);
    void setCardId(Id card_id);

    void payInvoice(Id invoice_id);

public slots:
    void setDiscount(fixed disc_amt);

protected slots:
    void slotCustomerChanged();
    void slotTenderChanged();
    void slotAmountChanged();
    void slotPayClicked();
    void slotPayOldest();
    void slotClearAll();
    void slotInvoiceInfo();
    void slotRefresh();
    void cellMoved(int row, int col);
    void cellChanged(int row, int col, Variant old);
    void focusNext(bool& leave, int& newRow, int& newcol, int type);
    void tenderCellChanged(int row, int col, Variant old);
    void tenderFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void referenceFocusNext(bool& leave, int& newRow, int& newcol, int type);
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

    vector<Gltx> _gltxs;
    vector<Term> _terms;
    vector<fixed> _suggest;
    QGuardedPtr<InvoiceInfo> _info;

    Receipt _orig;
    Receipt _curr;

    // Widgets
    GltxFrame* _gltxFrame;
    LookupEdit* _tender;
    LookupEdit* _discount;
    NumberEdit* _discAmt;
    LookupEdit* _customer;
    NumberEdit* _balance;
    NumberEdit* _amount;
    NumberEdit* _alloc;
    NumberEdit* _diff;
    Table* _table;
    Table* _refs;
    Table* _tenders;
};

#endif // RECEIPT_MASTER_H
