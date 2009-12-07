// $Id: item_select.h,v 1.10 2004/01/30 23:16:36 arandell Exp $
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

#ifndef ITEM_SELECT_H
#define ITEM_SELECT_H

#include "select.h"

struct ItemSelect: public Select {
    ItemSelect();
    virtual ~ItemSelect();

    // Selection criteria
    QString number;
    QString description;
    Id dept_id;
    Id subdept_id;
    Id group_id;
    Id vendor_id;
    Id store_id;
    Id location_id;
    bool stockedOnly;
    bool purchasedOnly;
    bool soldOnly;
    bool inventoriedOnly;
    bool checkOrderNum;

    virtual QString where() const;
};

#endif // ITEM_SELECT_H
