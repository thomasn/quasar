// $Id: pos_work_select.h,v 1.2 2004/08/02 03:46:13 bpepers Exp $
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

#ifndef POS_WORK_SELECT_H
#define POS_WORK_SELECT_H

#include "select.h"

struct PosWorkSelect: public Select {
    PosWorkSelect();
    virtual ~PosWorkSelect();

    // Selection criteria
    QString type;
    QString status;
    Id store_id;
    Id station_id;
    Id employee_id;
    bool no_errors;

    virtual QString where() const;
};

#endif // POS_WORK_SELECT_H
