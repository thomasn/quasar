// $Id: item_lookup.h,v 1.17 2004/03/20 01:33:45 bpepers Exp $
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

#ifndef ITEM_LOOKUP_H
#define ITEM_LOOKUP_H

#include "quasar_lookup.h"
#include "item.h"

class LookupEdit;
class LineEdit;
struct ItemSelect;

class ItemLookup: public QuasarLookup {
    Q_OBJECT
public:
    ItemLookup(MainWindow* main, QWidget* parent);
    ItemLookup(QuasarClient* quasar, QWidget* parent);
    ~ItemLookup();

    QString lookupById(Id item_id);
    vector<DataPair> lookupByText(const QString& text);
    int matchCount(const QString& text);

    Id store_id;
    bool purchasedOnly;
    bool soldOnly;
    bool inventoriedOnly;
    bool stockedOnly;
    bool activeOnly;
    bool checkOrderNum;
    LineEdit* desc;
    LookupEdit* dept;
    LookupEdit* subdept;
    LookupEdit* group;

    static bool lookup(QuasarDB* db, ItemSelect& conditions,
		       const QString& num, vector<Item>& items);

public slots:
    void refresh();
    QWidget* slotNew();
    QWidget* slotEdit(Id id);
};

#endif // ITEM_LOOKUP_H
