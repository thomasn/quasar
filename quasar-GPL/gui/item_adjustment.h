// $Id: item_adjustment.h,v 1.23 2005/04/01 22:13:37 bpepers Exp $
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

#ifndef ITEM_ADJUSTMENT_H
#define ITEM_ADJUSTMENT_H

#include "item_adjust.h"
#include "company.h"
#include "item.h"
#include "data_window.h"
#include "variant.h"

class GltxFrame;
class ItemLookup;
class Table;
class LookupEdit;
class NumberEdit;
class QComboBox;

class ItemAdjustment: public DataWindow {
    Q_OBJECT
public:
    ItemAdjustment(MainWindow* main, Id adjustment_id=INVALID_ID);
    ~ItemAdjustment();

protected slots:
    void slotStoreChanged();
    void slotReasonChanged();
    void slotSearch();
    void cellMoved(int row, int col);
    void cellChanged(int row, int col, Variant old);
    void focusNext(bool& leave, int& newRow, int& newcol, int type);
    void rowInserted(int row);
    void rowDeleted(int row);
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
    virtual void printItem(bool ask=false);

    bool findItem(Id item_id, Item& item);

    ItemAdjust _orig;
    ItemAdjust _curr;

    // Widgets
    GltxFrame* _gltxFrame;
    LookupEdit* _reason;
    ItemLookup* _lookup;
    Table* _items;
    QComboBox* _size;
    LookupEdit* _account;
    NumberEdit* _total;

    // Internal list of items
    struct AdjItem {
	Item item;
	QString number;
	QString size;
	fixed size_qty;
	fixed quantity;
	fixed inv_cost;
	fixed ext_deposit;
    };

    vector<AdjItem> _lines;
    vector<Item> _item_cache;
    Company _company;
};

#endif // ITEM_ADJUSTMENT_H
