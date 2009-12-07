// $Id: sales_report.h,v 1.11 2004/01/31 01:50:31 arandell Exp $
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

#ifndef SALES_REPORT_H
#define SALES_REPORT_H

#include "quasar_window.h"
#include "invoice.h"

class LookupEdit;
class ItemLookup;
class DateRange;
class DatePopup;
class ListView;
class NumberEdit;
class QComboBox;

class SalesReport: public QuasarWindow {
    Q_OBJECT
public:
    SalesReport(MainWindow* main);
    ~SalesReport();

    void setStoreId(Id store_id);
    void setCardId(Id card_id);

protected slots:
    void slotStoreChanged();
    void slotItemChanged();
    void slotSizeChanged();
    void slotPickLine();
    void slotRefresh();
    void slotPrint();

protected:
    void addInvoice(const Invoice& invoice);

    fixed _qtyTotal;
    fixed _priceTotal;

    LookupEdit* _customer;
    LookupEdit* _group;
    DateRange* _range;
    DatePopup* _from;
    DatePopup* _to;
    LookupEdit* _store;
    LookupEdit* _dept;
    LookupEdit* _subdept;
    ItemLookup* _lookup;
    LookupEdit* _item;
    QComboBox* _size;
    ListView* _list;
    NumberEdit* _totalQty;
    NumberEdit* _totalPrice;
};

#endif // SALES_REPORT_H
