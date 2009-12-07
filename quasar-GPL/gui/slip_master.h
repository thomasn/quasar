// $Id: slip_master.h,v 1.13 2005/03/13 22:17:01 bpepers Exp $
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

#ifndef SLIP_MASTER_H
#define SLIP_MASTER_H

#include "slip.h"
#include "vendor.h"
#include "item.h"
#include "data_window.h"
#include "variant.h"

class LookupEdit;
class DatePopup;
class LineEdit;
class NumberEdit;
class IntegerEdit;
class ItemLookup;
class Table;
class ComboBox;

class SlipMaster: public DataWindow {
    Q_OBJECT
public:
    SlipMaster(MainWindow* main, Id slip_id=INVALID_ID);
    ~SlipMaster();

    static bool postSlip(QuasarClient* quasar, Slip& slip, bool& create);

protected slots:
    void itemCellMoved(int row, int col);
    void itemCellChanged(int row, int col, Variant old);
    void itemFocusNext(bool& leave, int& newRow, int& newcol, int type);

protected slots:
    void slotVendorChanged();
    void slotStoreChanged();
    void slotPost();
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

    void updateItemInfo(int row);
    bool findItem(Id item_id, Item& item);

    Slip _orig;
    Slip _curr;

    // Widgets
    LookupEdit* _vend_id;
    LineEdit* _number;
    LineEdit* _orderNum;
    LineEdit* _invoiceNum;
    LookupEdit* _store;
    DatePopup* _shipDate;
    LineEdit* _waybill;
    LineEdit* _carrier;
    IntegerEdit* _pieces;
    DatePopup* _postDate;
    ComboBox* _status;
    ItemLookup* _lookup;
    Table* _items;
    ComboBox* _size;
    NumberEdit* _item_cnt;
    NumberEdit* _used;

    Vendor _vendor;
    vector<Item> _item_cache;
    int _item_row;
};

#endif // SLIP_MASTER_H
