// $Id: list_view_item.h,v 1.12 2004/02/03 00:56:02 arandell Exp $
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

#ifndef LIST_VIEW_ITEM_H
#define LIST_VIEW_ITEM_H

#include "list_view.h"
#include "variant.h"
#include "id.h"

class ListViewItem: public QListViewItem {
public:
    ListViewItem(ListView* parent, Id id=INVALID_ID);
    ListViewItem(ListView* parent, ListViewItem* after, Id id=INVALID_ID);
    ListViewItem(ListViewItem* parent, Id id=INVALID_ID);
    ListViewItem(ListViewItem* parent, ListViewItem* after, Id id=INVALID_ID);
    virtual ~ListViewItem();

    ListView* listView() const;
    ListViewItem* firstChild() const;
    ListViewItem* nextSibling() const;
    ListViewItem* parent() const;
    ListViewItem* itemAbove();
    ListViewItem* itemBelow();

    QString key(int column, bool ascending) const;
    int compare(QListViewItem* i, int col, bool ascending) const;

    Variant value(int column) const;
    void setValue(int column, Variant value);
    void setText(int column, const QString& value);

    void paintCell(QPainter* p, const QColorGroup& cg, int column,
		   int width, int alignment);

    Id id;
    bool isLast;
    QValueVector<Variant> values;
    QValueVector<Variant> extra;
};

#endif // LIST_VIEW_ITEM_H
