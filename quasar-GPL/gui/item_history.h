// $Id: item_history.h,v 1.9 2004/01/31 01:50:31 arandell Exp $
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

#ifndef ITEM_HISTORY_H
#define ITEM_HISTORY_H

#include "quasar_window.h"
#include "data_object.h"

class ItemLookup;
class LookupEdit;
class LineEdit;
class IntegerEdit;
class DateRange;
class DatePopup;
class ListView;
class ListViewItem;
class QComboBox;
class Grid;

class ItemHistory: public QuasarWindow {
    Q_OBJECT
public:
    ItemHistory(MainWindow* main);
    ~ItemHistory();

    void setStoreId(Id store_id);
    void setItem(Id item_id, const QString& number);

protected slots:
    void slotItemChanged();
    void slotStoreChanged();
    void slotRefresh();
    void slotPrint();
    void slotDoubleClick();

protected:
    void addLine(QString date, fixed qty, fixed cost, fixed sold_qty,
		 fixed sold_cost, fixed sold_price, fixed recv_qty,
		 fixed recv_cost, fixed adj_qty, fixed adj_cost,
		 fixed sizeQty);

    ItemLookup* _lookup;
    LookupEdit* _item;
    LineEdit* _desc;
    LookupEdit* _store;
    QComboBox* _size;
    DateRange* _dateRange;
    DatePopup* _from;
    DatePopup* _to;
    QComboBox* _qtySize;
    ListView* _list;

    Grid* _grid;
    ListViewItem* _last;
};

#endif // ITEM_HISTORY_H
