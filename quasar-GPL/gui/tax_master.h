// $Id: tax_master.h,v 1.17 2004/01/31 01:50:31 arandell Exp $
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

#ifndef TAX_MASTER_H
#define TAX_MASTER_H

#include "tax.h"
#include "data_window.h"
#include "variant.h"

class LineEdit;
class NumberEdit;
class LookupEdit;
class Table;
class QCheckBox;
class QTabWidget;
class QFrame;

class TaxMaster: public DataWindow {
    Q_OBJECT
public:
    TaxMaster(MainWindow* main, Id tax_id=INVALID_ID);
    ~TaxMaster();

    void setName(const QString& name);

protected slots:
    void groupChanged();
    void totCellChanged(int row, int col, Variant old);
    void totFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void groupCellChanged(int row, int col, Variant old);
    void groupFocusNext(bool& leave, int& newRow, int& newcol, int type);

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

    Tax _orig;
    Tax _curr;

    // Widgets
    LineEdit* _name;
    LineEdit* _desc;
    QCheckBox* _group;
    QTabWidget* _tabs;
    QFrame* _controlFrame;
    QFrame* _totFrame;
    QFrame* _groupFrame;
    LineEdit* _number;
    NumberEdit* _rate;
    LookupEdit* _collected;
    LookupEdit* _paid;
    Table* _tots;
    Table* _groups;
};

#endif // TAX_MASTER_H
