// $Id: pat_group_master.h,v 1.4 2004/01/31 01:50:31 arandell Exp $
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

#ifndef PAT_GROUP_MASTER_H
#define PAT_GROUP_MASTER_H

#include "pat_group.h"
#include "data_window.h"
#include "variant.h"

class LineEdit;
class Table;
class LookupEdit;

class PatGroupMaster: public DataWindow {
    Q_OBJECT
public:
    PatGroupMaster(MainWindow*, Id group_id=INVALID_ID);
    ~PatGroupMaster();

protected slots:
    void cardCellChanged(int row, int col, Variant old);
    void cardFocusNext(bool& leave, int& newRow, int& newCol, int type);

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

    PatGroup _orig;
    PatGroup _curr;

    // Widgets
    LineEdit* _name;
    LineEdit* _number;
    Table* _cards;
    LookupEdit* _equity;
    LookupEdit* _credit;
};

#endif // PAT_GROUP_MASTER_H
