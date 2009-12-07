// $Id: general_master.h,v 1.15 2004/01/31 01:50:31 arandell Exp $
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

#ifndef GENERAL_MASTER_H
#define GENERAL_MASTER_H

#include "general.h"
#include "data_window.h"
#include "variant.h"

class GltxFrame;
class NumberEdit;
class Table;

class GeneralMaster: public DataWindow {
    Q_OBJECT
public:
    GeneralMaster(MainWindow* main, Id general_id=INVALID_ID);
    ~GeneralMaster();

    static bool checkReconciled(QWidget* parent, const Gltx& orig, Gltx& curr);

protected slots:
    void slotRecurring();
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

    void recalculate();

    General _orig;
    General _curr;

    // Widgets
    GltxFrame* _gltxFrame;
    Table* _table;
    NumberEdit* _debit_total;
    NumberEdit* _credit_total;
    NumberEdit* _difference;
    QCheckBox* _reverse;
};

#endif // GENERAL_MASTER_H
