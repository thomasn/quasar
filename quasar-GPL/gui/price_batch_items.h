// $Id: price_batch_items.h,v 1.3 2005/03/26 01:41:26 bpepers Exp $
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

#ifndef PRICE_BATCH_ITEMS_H
#define PRICE_BATCH_ITEMS_H

#include <qdialog.h>
#include "item.h"

class MainWindow;
class QuasarClient;
class QRadioButton;
class LookupEdit;
class ObjectCache;

class PriceBatchItems: public QDialog {
    Q_OBJECT
public:
    PriceBatchItems(QWidget* parent, MainWindow* main, Id store_id);
    virtual ~PriceBatchItems();

    void getItems(vector<Item>& items, vector<QString>& numbers,
		  vector<QString>& descs, vector<QString>& sizes,
		  vector<fixed>& costs, vector<Price>& prices);

public slots:
    void slotTypeChanged(int index);

protected slots:
    void accept();

protected:
    Id _store_id;
    QuasarClient* _quasar;

    QRadioButton* _useItems;
    LookupEdit* _dept;
    LookupEdit* _subdept;
    LookupEdit* _group;
    LookupEdit* _location;
    QRadioButton* _useOrder;
    LookupEdit* _order;
    QRadioButton* _useInvoice;
    LookupEdit* _invoice;
};

#endif // SET_PRICE_CALC_H
