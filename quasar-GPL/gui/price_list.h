// $Id: price_list.h,v 1.9 2004/10/15 05:18:52 bpepers Exp $
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

#ifndef PRICE_LIST_H
#define PRICE_LIST_H

#include "active_list.h"

class LookupEdit;

class PriceList: public ActiveList {
    Q_OBJECT
public:
    PriceList(MainWindow* main, bool prices=true);
    ~PriceList();

    void setStoreId(Id store_id);
    void setItemId(Id item_id);
    void setCardId(Id card_id);

public slots:
    void dataEvent(DataEvent* e);

protected slots:
    void slotInactiveChanged();

protected:
    virtual void performRefresh();
    virtual void performPrint();
    virtual void performNew();
    virtual void performEdit();

    virtual bool isActive(Id price_id);
    virtual void setActive(Id price_id, bool active);

    void clearLists();
    ListView* _cost;
    LookupEdit* _item;
    LookupEdit* _card;
    LookupEdit* _batch;
    LookupEdit* _store;
};

#endif // PRICE_LIST_H
