// $Id: inquiry.h,v 1.23 2005/01/30 04:25:31 bpepers Exp $
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

#ifndef INQUIRY_H
#define INQUIRY_H

#include "account.h"
#include "gltx.h"
#include "quasar_window.h"

class DateRange;
class DatePopup;
class LookupEdit;
class QTabWidget;
class InquiryWindow;
class DataEvent;

class Inquiry: public QuasarWindow {
    Q_OBJECT
public:
    Inquiry(MainWindow* main);
    ~Inquiry();

    void setCurrentTab(int tab);
    void setCurrentId(Id id);
    void setCurrentDates(QDate from, QDate to);

    void setStoreId(Id store_id);
    void setAccountId(Id account_id);
    void setCardId(Id card_id);
    void setItemId(Id item_id);

protected slots:
    void slotRefresh();
    void slotPrint();
    void slotEdit();
    void slotTabChange();
    void slotStoreChange();
    void slotAccountChange();
    void slotCardChange();
    void slotItemChange();
    void closeEvent(QCloseEvent* e);

protected:
    vector<Account> _accounts;
    bool _loading;

    DateRange* _dateRange;
    DatePopup* _from;
    DatePopup* _to;
    LookupEdit* _store;
    QTabWidget* _tabs;
    InquiryWindow* _account;
    InquiryWindow* _card;
    InquiryWindow* _item;
};

#endif // INQUIRY_H
