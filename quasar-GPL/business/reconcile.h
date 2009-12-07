// $Id: reconcile.h,v 1.7 2005/01/30 00:51:13 bpepers Exp $
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

#ifndef RECONCILE_H
#define RECONCILE_H

#include "data_object.h"

struct ReconcileLine {
    ReconcileLine(Id gltx_id=INVALID_ID, int seq_num=-1);

    Id gltx_id;
    int seq_num;

    bool operator==(const ReconcileLine& rhs) const;
    bool operator!=(const ReconcileLine& rhs) const;
};

class Reconcile: public DataObject {
public:
    // Constructors and Destructor
    Reconcile();
    ~Reconcile();

    // Get methods
    Id accountId()			const { return _account_id; }
    fixed statementBalance()		const { return _stmt_balance; }
    QDate statementDate()		const { return _stmt_date; }
    QDate endDate()			const { return _end_date; }
    bool isReconciled()			const { return _reconciled; }

    // Set methods
    void setAccountId(Id account_id)		{ _account_id = account_id; }
    void setStatementBalance(fixed amount)	{ _stmt_balance = amount; }
    void setStatementDate(QDate date)		{ _stmt_date = date; }
    void setEndDate(QDate date)			{ _end_date = date; }
    void setReconciled(bool flag)		{ _reconciled = flag; }

    // Reconcile lines
    vector<ReconcileLine>& lines()		{ return _lines; }
    const vector<ReconcileLine>& lines()  const { return _lines; }

    // Operations
    bool operator==(const Reconcile& rhs) const;
    bool operator!=(const Reconcile& rhs) const;

protected:
    Id _account_id;			// Bank account reconciling
    fixed _stmt_balance;		// Balance on statement to match to
    QDate _stmt_date;			// Statement date
    QDate _end_date;			// Ending date (limits tx shown)
    bool _reconciled;			// Reconcile done?
    vector<ReconcileLine> _lines;	// Accounts and amounts
};

#endif // RECONCILE_H
