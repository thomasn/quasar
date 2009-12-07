// $Id: stock_status.h,v 1.7 2004/01/31 01:50:31 arandell Exp $
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

#ifndef STOCK_STATUS_H
#define STOCK_STATUS_H

#include "quasar_window.h"
#include "id.h"

class LookupEdit;
class LineEdit;
class ListView;
class DeptLookup;
class SubdeptLookup;
class LocationLookup;
class QRadioButton;

class StockStatus: public QuasarWindow {
    Q_OBJECT
public:
    StockStatus(MainWindow* main);
    ~StockStatus();

    void setItem(Id item_id, const QString& number);

protected slots:
    void slotTypeChanged();
    void slotDeptChanged();
    void slotSubdeptChanged();
    void slotStoreChanged();
    void slotItemSelected();
    void slotRefresh();
    void slotPrint();

protected:
    QRadioButton* _single;
    LookupEdit* _item;
    LineEdit* _desc;
    LineEdit* _size;
    QRadioButton* _multiple;
    DeptLookup* _deptLookup;
    LookupEdit* _dept;
    SubdeptLookup* _subdeptLookup;
    LookupEdit* _subdept;
    LookupEdit* _group;
    LocationLookup* _locLookup;
    LookupEdit* _location;
    LookupEdit* _store;
    ListView* _list;

    void refreshSingle();
    void refreshMultiple();
};

#endif // STOCK_STATUS_H
