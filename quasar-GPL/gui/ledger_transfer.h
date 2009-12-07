// $Id: ledger_transfer.h,v 1.6 2004/01/31 01:50:31 arandell Exp $
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

#ifndef LEDGER_TRANSFER_H
#define LEDGER_TRANSFER_H

#include "general.h"
#include "company.h"
#include "data_window.h"
#include "variant.h"

class GltxFrame;
class NumberEdit;
class LineEdit;
class DatePopup;
class LookupEdit;
class Table;

class LedgerTransfer: public DataWindow {
    Q_OBJECT
public:
    LedgerTransfer(MainWindow* main, Id transfer_id=INVALID_ID);
    ~LedgerTransfer();

protected slots:
    void slotAccountChanged();
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

    General _orig;
    General _curr;

    // Widgets
    GltxFrame* _gltxFrame;
    LookupEdit* _account;
    NumberEdit* _balance;
    NumberEdit* _amount;
    Table* _table;
    LookupEdit* _transfer_acct;
    NumberEdit* _alloc;
    NumberEdit* _difference;

    Company _company;
    vector<General> _origLinks;
    vector<General> _links;
};

#endif // LEDGER_TRANSFER_H
