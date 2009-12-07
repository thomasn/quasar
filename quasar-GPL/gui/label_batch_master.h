// $Id: label_batch_master.h,v 1.4 2004/09/20 17:35:55 bpepers Exp $
//
// Copyright (C) 1998-2004 Linux Canada Inc.  All rights reserved.
//
// This file is part of Quasar Aclabelbatching
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
// information about Quasar Aclabelbatching support and maintenance options.
//
// Contact sales@linuxcanada.com if any conditions of this licensing are
// not clear to you.

#ifndef LABEL_BATCH_MASTER_H
#define LABEL_BATCH_MASTER_H

#include "label_batch.h"
#include "item.h"
#include "data_window.h"
#include "variant.h"

class LookupEdit;
class LocationLookup;
class ItemLookup;
class LineEdit;
class NumberEdit;
class Table;
class ComboBox;

class LabelBatchMaster: public DataWindow {
    Q_OBJECT
public:
    LabelBatchMaster(MainWindow* main, Id batch_id=INVALID_ID);
    ~LabelBatchMaster();

protected slots:
    void slotStoreChanged();
    void slotAlignment();
    void slotPrint();
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

    LabelBatch _orig;
    LabelBatch _curr;

    // Widgets
    LineEdit* _number;
    LineEdit* _description;
    LookupEdit* _store;
    ItemLookup* _itemLookup;
    Table* _table;
    ComboBox* _size;
    ComboBox* _type;

    QStringList _labelNames;
    QStringList _labelFiles;

    bool findItem(Id item_id, Item& item);
    vector<Item> _item_cache;
};

#endif // LABEL_BATCH_MASTER_H
