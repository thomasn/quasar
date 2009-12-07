// $Id: withdraw_master.h,v 1.7 2004/06/16 06:30:14 bpepers Exp $
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

#ifndef WITHDRAW_MASTER_H
#define WITHDRAW_MASTER_H

#include "withdraw.h"
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

class WithdrawMaster: public DataWindow {
    Q_OBJECT
public:
    WithdrawMaster(MainWindow* main, Id withdraw_id=INVALID_ID);
    ~WithdrawMaster();

    void setStoreId(Id store_id);
    void setCardId(Id card_id);

protected slots:
    void slotCustomerChanged();
    void tenderCellChanged(int row, int col, Variant old);
    void tenderFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void referenceFocusNext(bool& leave, int& newRow, int& newcol, int type);

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

    Withdraw _orig;
    Withdraw _curr;

    // Widgets
    GltxFrame* _gltxFrame;
    LookupEdit* _customer;
    NumberEdit* _balance;
    NumberEdit* _amount;
    Table* _tenders;
    Table* _refs;
};

#endif // WITHDRAW_MASTER_H
