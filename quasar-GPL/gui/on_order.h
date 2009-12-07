// $Id: on_order.h,v 1.9 2004/01/31 01:50:31 arandell Exp $
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

#ifndef ON_ORDER_H
#define ON_ORDER_H

#include "quasar_window.h"
#include "gltx.h"

class LookupEdit;
class ItemLookup;
class ListView;
class NumberEdit;
class QComboBox;

class OnOrder: public QuasarWindow {
    Q_OBJECT
public:
    OnOrder(MainWindow* main);
    ~OnOrder();

    void setStoreId(Id store_id);
    void setItem(Id item_id, const QString& number);

protected slots:
    void slotStoreChanged();
    void slotItemChanged();
    void slotSizeChanged();
    void slotAllVendors();
    void slotPickLine();
    void slotRefresh();
    void slotPrint();

protected:
    LookupEdit* _vendor;
    LookupEdit* _store;
    LookupEdit* _dept;
    LookupEdit* _subdept;
    ItemLookup* _lookup;
    LookupEdit* _item;
    QComboBox* _size;
    ListView* _list;
    NumberEdit* _totalQty;
    NumberEdit* _totalCost;
};

#endif // ON_ORDER_H
