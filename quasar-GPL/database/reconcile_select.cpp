// $Id: reconcile_select.cpp,v 1.6 2005/02/24 03:06:12 bpepers Exp $
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

#include "reconcile_select.h"

ReconcileSelect::ReconcileSelect()
{
    account_id = INVALID_ID;
    reconciledOnly = false;
    unreconciledOnly = false;
}

ReconcileSelect::~ReconcileSelect()
{
}

QString
ReconcileSelect::where() const
{
    QString where = Select::where();

    if (id != INVALID_ID)
	addIdCondition(where, "reconcile_id", id);
    if (account_id != INVALID_ID)
	addIdCondition(where, "account_id", account_id);
    if (!stmt_date.isNull())
	addCondition(where, "stmt_date = '" +
		     stmt_date.toString(Qt::ISODate) + "'");
    if (reconciledOnly)
	addCondition(where, "reconciled = 'Y'");
    if (unreconciledOnly)
	addCondition(where, "reconciled = 'N'");

    return where;
}
