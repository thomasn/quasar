// $Id: count_master.h,v 1.10 2004/03/18 22:40:04 bpepers Exp $
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

#ifndef COUNT_MASTER_H
#define COUNT_MASTER_H

#include "count.h"
#include "item.h"
#include "data_window.h"
#include "variant.h"

class LookupEdit;
class LocationLookup;
class ItemLookup;
class DatePopup;
class LineEdit;
class NumberEdit;
class Table;
class QCheckBox;
class QComboBox;

class CountMaster: public DataWindow {
    Q_OBJECT
public:
    CountMaster(MainWindow* main, Id count_id=INVALID_ID);
    ~CountMaster();

    void appendItem(const Item& item, const QString& number,
		    const QString& size, fixed qty);

protected slots:
    void slotStoreChanged();
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

    Count _orig;
    Count _curr;

    // Widgets
    LineEdit* _number;
    LineEdit* _description;
    LocationLookup* _locLookup;
    LookupEdit* _location;
    LookupEdit* _employee;
    LookupEdit* _store;
    DatePopup* _date;
    ItemLookup* _itemLookup;
    Table* _items;
    QComboBox* _size;

    bool findItem(Id item_id, Item& item);
    vector<Item> _item_cache;
};

#endif // COUNT_MASTER_H
