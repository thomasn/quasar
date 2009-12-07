// $Id: address_book.h,v 1.23 2004/10/09 00:35:20 bpepers Exp $
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

#ifndef ADDRESS_BOOK_H
#define ADDRESS_BOOK_H

#include "active_list.h"
#include "data_object.h"

class LineEdit;
class GroupLookup;
class LookupEdit;

class AddressBook: public ActiveList {
    Q_OBJECT
public:
    AddressBook(MainWindow* main);
    ~AddressBook();

protected slots:
    void slotActivities();
    void slotInquiry();
    void slotFindTx();
    void slotAgedAR();
    void slotSales();
    void slotReceipt();
    void slotWithdraw();
    void slotCheque();
    void slotAgedAP();
    void slotPurchases();
    void slotPayment();
    void slotEmail();
    void slotAllocate();
    void slotInactiveChanged();
    void tabChanged(QWidget* widget);

protected:
    virtual void addToPopup(QPopupMenu* menu);

    virtual void performRefresh();
    virtual void performPrint();
    virtual void performNew();
    virtual void performEdit();

    virtual bool isActive(Id item_id);
    virtual void setActive(Id item_id, bool active);

    void clearLists();
    ListView* _employee;
    ListView* _customer;
    ListView* _vendor;
    ListView* _personal;
    LineEdit* _name;
    LineEdit* _number;
    GroupLookup* _groupLookup;
    LookupEdit* _group;
    LookupEdit* _store;

    typedef vector<Id> IdVector;
    typedef IdVector::iterator IdVectorIt;

    IdVector _cardIds;
    vector<fixed> _balances;
};

#endif // ADDRESS_BOOK_H
