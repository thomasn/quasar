// $Id: active_list.h,v 1.10 2004/01/31 01:50:31 arandell Exp $
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

#ifndef ACTIVE_LIST_H
#define ACTIVE_LIST_H

#include "list_window.h"

class QCheckBox;
class QGridLayout;

class ActiveList: public ListWindow {
    Q_OBJECT
public:
    ActiveList(MainWindow* main, const char* name, bool tabs=false);
    ~ActiveList();

public slots:
    void slotSetActive();
    void slotSetInactive();

protected slots:
    virtual void slotInactiveChanged();
    virtual void slotActivities();

protected:
    virtual void addToPopup(QPopupMenu* menu);

    virtual bool isActive(Id id)=0;
    virtual void setActive(Id id, bool active)=0;

    QCheckBox* _inactive;
    QGridLayout* _grid;
};

#endif // ACTIVE_LIST_H
