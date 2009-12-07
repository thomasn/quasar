// $Id: auto_order.h,v 1.5 2004/01/31 01:50:31 arandell Exp $
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

#ifndef AUTO_ORDER_H
#define AUTO_ORDER_H

#include "quasar_window.h"
#include "fixed.h"

class LookupEdit;
class DatePopup;
class IntegerEdit;
class PercentEdit;
class QRadioButton;
class QComboBox;
class Item;

class AutoOrder: public QuasarWindow {
    Q_OBJECT
public:
    AutoOrder(MainWindow* main);
    ~AutoOrder();

protected slots:
    void slotSelectChanged(int index);
    void slotMethodChanged(int index);
    void slotOk();

protected:
    LookupEdit* _vendor;
    LookupEdit* _store;
    QRadioButton* _itemSelect;
    LookupEdit* _dept;
    LookupEdit* _subdept;
    LookupEdit* _group;
    QRadioButton* _templateSelect;
    LookupEdit* _template;
    QRadioButton* _minMaxMethod;
    QRadioButton* _salesMethod;
    DatePopup* _endDate;
    IntegerEdit* _historyCnt;
    QComboBox* _historyType;
    PercentEdit* _percent;
    QComboBox* _percentType;
    IntegerEdit* _orderCnt;
    QComboBox* _orderType;

    fixed calculateOrderQty(const Item& item);
};

#endif // AUTO_ORDER_H
