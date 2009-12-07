// $Id: payout_master.h,v 1.3 2004/01/31 01:50:31 arandell Exp $
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

#ifndef PAYOUT_MASTER_H
#define PAYOUT_MASTER_H

#include "payout.h"
#include "data_window.h"
#include "variant.h"

class GltxFrame;
class Table;
class NumberEdit;

class PayoutMaster: public DataWindow {
    Q_OBJECT
public:
    PayoutMaster(MainWindow* main, Id payout_id=INVALID_ID);
    ~PayoutMaster();

protected slots:
    void expenseCellChanged(int row, int col, Variant old);
    void expenseFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void tenderCellChanged(int row, int col, Variant old);
    void tenderFocusNext(bool& leave, int& newRow, int& newcol, int type);

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

    void recalculate();

    Payout _orig;
    Payout _curr;

    // Widgets
    GltxFrame* _gltxFrame;
    Table* _expenses;
    Table* _taxes;
    Table* _tenders;
    NumberEdit* _expenseTotal;
    NumberEdit* _taxTotal;
    NumberEdit* _total;
    NumberEdit* _tenderTotal;
    NumberEdit* _diff;

    void addTax(fixed taxable, vector<Id>& ids, vector<fixed>& amts);
    vector<TaxLine> _tax_info;
};

#endif // PAYOUT_MASTER_H
