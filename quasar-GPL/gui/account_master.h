// $Id: account_master.h,v 1.30 2004/01/31 01:50:31 arandell Exp $
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

#ifndef ACCOUNT_MASTER_H
#define ACCOUNT_MASTER_H

#include "account.h"
#include "data_window.h"
#include "variant.h"

class QComboBox;
class QCheckBox;
class LineEdit;
class LookupEdit;
class DateEdit;
class NumberEdit;
class AccountLookup;
class Table;

class AccountMaster: public DataWindow {
    Q_OBJECT
public:
    AccountMaster(MainWindow* main, Id account_id=INVALID_ID);
    ~AccountMaster();

    void setName(const QString& name);
    void setType(int type);

protected slots:
    void typeChanged(int type);
    void groupCellChanged(int row, int col, Variant old);

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

    Account _orig;
    Account _curr;

    // Widgets
    LineEdit* _name;
    LineEdit* _number;
    QComboBox* _type;
    AccountLookup* _parentLookup;
    LookupEdit* _parent;
    QCheckBox* _header;
    DateEdit* _lastRecon;
    NumberEdit* _nextNum;
    Table* _groups;
};

#endif // ACCOUNT_MASTER_H
