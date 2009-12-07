// $Id: select.h,v 1.9 2004/01/30 23:16:36 arandell Exp $
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

#ifndef SELECT_H
#define SELECT_H

#include "data_object.h"

struct Select {
    Select();
    virtual ~Select();

    // Selection criteria
    Id id;
    fixed external_id;
    bool activeOnly;

    virtual QString where() const;

    void addCondition(QString& where, const QString& condition) const;
    void addStringCondition(QString& where, const QString& field,
			    const QString& value, int len=0) const;
    void addIdCondition(QString& where, const QString& field, Id value) const;
};

#endif // SELECT_H
