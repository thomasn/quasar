// $Id: tender_master.h,v 1.23 2004/01/31 01:50:31 arandell Exp $
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

#ifndef TENDER_MASTER_H
#define TENDER_MASTER_H

#include "tender.h"
#include "data_window.h"

class LineEdit;
class LookupEdit;
class NumberEdit;
class IdEdit;
class ComboBox;
class QCheckBox;
class Table;
class QTabWidget;
class QFrame;

class TenderMaster: public DataWindow {
    Q_OBJECT
public:
    TenderMaster(MainWindow* main, Id tender_id=INVALID_ID);
    ~TenderMaster();

protected slots:
    void denomCellChanged(int row, int col, Variant old);
    void slotTypeChanged();

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

    Tender _orig;
    Tender _curr;

    // Widgets
    LineEdit* _name;
    ComboBox* _type;
    NumberEdit* _limit;
    NumberEdit* _convertRate;
    IdEdit* _menuNum;
    QTabWidget* _tabs;
    QCheckBox* _overTender;
    QCheckBox* _openDrawer;
    QCheckBox* _forceAmount;
    QCheckBox* _secondRcpt;
    LookupEdit* _account;
    LookupEdit* _safe;
    LookupEdit* _bank;
    QFrame* _denomTab;
    Table* _denoms;
};

#endif // TENDER_MASTER_H
