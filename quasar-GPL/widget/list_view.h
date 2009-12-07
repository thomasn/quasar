// $Id: list_view.h,v 1.10 2004/02/03 00:56:02 arandell Exp $
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

#ifndef LIST_VIEW_H
#define LIST_VIEW_H

#include <qlistview.h>
#include <qvaluevector.h>

class Valcon;
class ListViewItem;

class ListView: public QListView {
    Q_OBJECT
public:
    ListView(QWidget* parent=0, const char* name=0, WFlags flags=0);
    virtual ~ListView();

    int addColumn(const QString& label, int size=-1, Valcon* valcon=0);
    int addTextColumn(const QString& label, int length, int align=AlignLeft);
    int addNumberColumn(const QString& label, int digits=10);
    int addMoneyColumn(const QString& label, int digits=9);
    int addPercentColumn(const QString& label, int digits=4);
    int addDateColumn(const QString& label);
    int addCheckColumn(const QString& label);
    int addPriceColumn(const QString& label);

    Valcon* valcon(int column) const;
    void setValcon(int column, Valcon* valcon);

    int findColumn(const QString& title) const;

    ListViewItem* selectedItem() const;
    ListViewItem* currentItem() const;
    ListViewItem* firstChild() const;

protected:
    QValueVector<Valcon*> _valcons;
};

#endif
