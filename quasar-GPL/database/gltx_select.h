// $Id: gltx_select.h,v 1.24 2005/01/30 02:21:46 bpepers Exp $
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

#ifndef GLTX_SELECT_H
#define GLTX_SELECT_H

#include "select.h"
class Gltx;

struct GltxSelect: public Select {
    GltxSelect();
    virtual ~GltxSelect();

    // Select criteria
    int type;
    QString number;
    QString reference;
    QDate start_date;
    QDate end_date;
    Id station_id;
    Id employee_id;
    Id card_id;
    Id cust_type_id;
    Id card_group_id;
    Id card_patgroup_id;
    Id store_id;
    Id shift_id;
    Id link_id;
    bool unclosed;
    bool unprinted;
    bool unpaid;
    bool unlinked;
    int card_type;
    bool blankStation;
    bool blankEmployee;
    bool cashrecOnly;

    virtual QString where() const;
};

#endif // GLTX_SELECT_H
