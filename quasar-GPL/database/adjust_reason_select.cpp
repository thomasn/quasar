// $Id: adjust_reason_select.cpp,v 1.2 2005/01/09 10:02:53 bpepers Exp $
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

#include "adjust_reason_select.h"

AdjustReasonSelect::AdjustReasonSelect()
{
    name = "";
    number = "";
}

AdjustReasonSelect::~AdjustReasonSelect()
{
}

QString
AdjustReasonSelect::where() const
{
    QString where = Select::where();

    if (id != INVALID_ID)
	addIdCondition(where, "reason_id", id);
    if (name != "")
	addStringCondition(where, "name", name, 14);
    if (number != "")
	addStringCondition(where, "number", number, 5);

    return where;
}
