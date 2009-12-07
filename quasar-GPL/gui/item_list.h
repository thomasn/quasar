// $Id: item_list.h,v 1.13 2004/01/31 01:50:31 arandell Exp $
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

#ifndef ITEM_LIST_H
#define ITEM_LIST_H

#include "active_list.h"
#include "dept.h"
#include "subdept.h"

class LineEdit;
class LookupEdit;
class SubdeptLookup;
class LocationLookup;

class ItemList: public ActiveList {
    Q_OBJECT
public:
    ItemList(MainWindow* main);
    ~ItemList();

protected slots:
    void slotDeptChanged();
    void slotSubdeptChanged();
    void slotStoreChanged();
    void slotStockedChanged();
    void slotActivities();
    void slotInquiry();
    void slotItemHistory();
    void slotPriceCost();
    void slotSalesHistory();
    void slotStockStatus();
    void slotOnOrder();
    void slotItemMargin();

protected:
    virtual void addToPopup(QPopupMenu* menu);

    virtual void performRefresh();
    virtual void performPrint();
    virtual void performNew();
    virtual void performEdit();

    virtual bool isActive(Id item_id);
    virtual void setActive(Id item_id, bool active);

    LineEdit* _number;
    LineEdit* _description;
    LookupEdit* _dept;
    SubdeptLookup* _subLookup;
    LookupEdit* _subdept;
    LookupEdit* _group;
    LocationLookup* _locLookup;
    LookupEdit* _location;
    LookupEdit* _store;
    LookupEdit* _vendor;
    QCheckBox* _show_vendor;
    QCheckBox* _show_unstocked;

    bool findSubdept(Id subdept_id, Subdept& subdept);
    bool findDept(Id dept_id, Dept& dept);

    vector<Subdept> _subdept_cache;
    vector<Dept> _dept_cache;
};

#endif // ITEM_LIST_H
