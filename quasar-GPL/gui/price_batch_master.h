// $Id: price_batch_master.h,v 1.6 2005/03/26 01:41:26 bpepers Exp $
//
// Copyright (C) 1998-2004 Linux Canada Inc.  All rights reserved.
//
// This file is part of Quasar Acpricebatching
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
// information about Quasar Acpricebatching support and maintenance options.
//
// Contact sales@linuxcanada.com if any conditions of this licensing are
// not clear to you.

#ifndef PRICE_BATCH_MASTER_H
#define PRICE_BATCH_MASTER_H

#include "price_batch.h"
#include "item.h"
#include "data_window.h"
#include "variant.h"

class LineEdit;
class DateEdit;
class LookupEdit;
class ItemLookup;
class Table;
class QComboBox;
class ObjectCache;

class PriceBatchMaster: public DataWindow {
    Q_OBJECT
public:
    PriceBatchMaster(MainWindow* main, Id batch_id=INVALID_ID);
    ~PriceBatchMaster();

protected slots:
    void slotStoreChanged();
    void slotAddItems();
    void slotClearItems();
    void slotCalculate();
    void slotRound();
    void slotExecute();
    void slotPrint();
    void slotLabels();
    void cellMoved(int row, int col);
    void cellChanged(int row, int col, Variant old);
    void focusNext(bool& leave, int& newRow, int& newcol, int type);

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

    PriceBatch _orig;
    PriceBatch _curr;

    // Widgets
    LineEdit* _number;
    LineEdit* _description;
    LookupEdit* _store;
    DateEdit* _execDate;
    ItemLookup* _itemLookup;
    Table* _items;
    QComboBox* _size;

    ObjectCache* _cache;
    bool findItem(Id item_id, Item& item);
    void setItem(const Item& item);
    vector<Item> _item_cache;
};

#endif // PRICE_BATCH_MASTER_H
