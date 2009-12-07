// $Id: sales_history.h,v 1.23 2005/01/30 04:25:31 bpepers Exp $
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

#ifndef SALES_HISTORY_H
#define SALES_HISTORY_H

#include "quasar_window.h"
#include "data_object.h"

class ItemLookup;
class LookupEdit;
class LineEdit;
class IntegerEdit;
class NumberEdit;
class DatePopup;
class ListView;
class ComboBox;
class Grid;

class SalesHistory: public QuasarWindow {
    Q_OBJECT
public:
    SalesHistory(MainWindow* main);
    ~SalesHistory();

    void setStoreId(Id store_id);
    void setItem(Id item_id, const QString& number);

protected slots:
    void slotItemChanged();
    void slotStoreChanged();
    void slotRefresh();
    void slotPrint();

protected:
    void addLine(const QString& period, QDate date, fixed qty, fixed cost,
		 fixed price, fixed sizeQty);

    ItemLookup* _lookup;
    LookupEdit* _item;
    LineEdit* _desc;
    LookupEdit* _store;
    NumberEdit* _onHand;
    NumberEdit* _onOrder;
    NumberEdit* _minQty;
    NumberEdit* _maxQty;
    ComboBox* _size;
    DatePopup* _date;
    ComboBox* _qtySize;
    ComboBox* _period;
    IntegerEdit* _count;
    ListView* _list;
    Grid* _grid;
};

#endif // SALES_HISTORY_H
