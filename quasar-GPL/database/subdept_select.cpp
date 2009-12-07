// $Id: subdept_select.cpp,v 1.11 2004/11/10 10:36:52 bpepers Exp $
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

#include "subdept_select.h"

SubdeptSelect::SubdeptSelect()
{
    name = "";
    number = "";
    dept_id = INVALID_ID;
}

SubdeptSelect::~SubdeptSelect()
{
}

QString
SubdeptSelect::where() const
{
    QString where = Select::where();

    if (id != INVALID_ID)
	addIdCondition(where, "subdept_id", id);
    if (name != "")
	addStringCondition(where, "name", name, 30);
    if (number != "")
	addStringCondition(where, "number", number, 5);
    if (dept_id != INVALID_ID)
	addIdCondition(where, "dept_id", dept_id);

    return where;
}
