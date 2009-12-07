// $Id: account_list.h,v 1.26 2004/06/15 03:07:07 bpepers Exp $
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

#ifndef ACCOUNT_LIST_H
#define ACCOUNT_LIST_H

#include "active_list.h"
#include "account.h"
#include "company.h"

class AccountList: public ActiveList {
    Q_OBJECT
public:
    AccountList(MainWindow* main);
    ~AccountList();

public slots:
    void dataEvent(DataEvent* e);

protected slots:
    void slotActivities();
    void slotJournal();
    void slotReconcile();

protected:
    virtual void performRefresh();
    virtual void performPrint();
    virtual void performNew();
    virtual void performEdit();

    virtual bool isActive(Id item_id);
    virtual void setActive(Id item_id, bool active);

    fixed getBalance(const Account& account);

    Company _company;
    vector<Account> _accounts;
    vector<Id> _account_ids;
    vector<fixed> _balances;
};

#endif // ACCOUNT_LIST_H
