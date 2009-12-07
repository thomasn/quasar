// $Id: count_list.h,v 1.11 2005/03/04 00:04:58 bpepers Exp $
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

#ifndef COUNT_LIST_H
#define COUNT_LIST_H

#include "active_list.h"
#include "count.h"
#include "item.h"

class LookupEdit;

class CountList: public ActiveList {
    Q_OBJECT
public:
    CountList(MainWindow* main);
    ~CountList();

public slots:
    void dataEvent(DataEvent* e);

protected slots:
    void slotItemSearch();
    void slotNotCounted();
    void slotDiscReport();
    void slotProcess();

protected:
    virtual void performRefresh();
    virtual void performPrint();
    virtual void performNew();
    virtual void performEdit();

    virtual bool isActive(Id tax_id);
    virtual void setActive(Id tax_id, bool active);

    void discrepancies(vector<Count>& counts, bool include_zero, Id store_id,
		       vector<Item>& items, vector<QString>& numbers,
		       vector<QString>& sizes, vector<fixed>& on_hands,
		       vector<fixed>& qtys, vector<fixed>& costs,
		       vector<fixed>& deposits);

    LookupEdit* _store;
};

#endif // COUNT_LIST_H
