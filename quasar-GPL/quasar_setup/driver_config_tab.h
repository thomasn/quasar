// $Id: driver_config_tab.h,v 1.3 2005/03/01 19:53:02 bpepers Exp $
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

#ifndef DRIVER_CONFIG_TAB_H
#define DRIVER_CONFIG_TAB_H

#include "config_tab.h"

class LineEdit;
class ListView;

class DriverConfigTab: public ConfigTab
{
    Q_OBJECT
public:
    // Constructors
    DriverConfigTab(QWidget* parent);
    virtual ~DriverConfigTab();

public slots:
    void loadData();
    bool saveChanges();

private slots:
    void slotConfigure();

private:
    ListView* _drivers;
};

#endif // DRIVER_CONFIG_TAB_H
