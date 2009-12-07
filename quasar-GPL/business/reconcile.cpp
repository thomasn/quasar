// $Id: reconcile.cpp,v 1.8 2004/12/30 00:07:58 bpepers Exp $
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

#include "reconcile.h"

Reconcile::Reconcile()
    : _account_id(INVALID_ID), _stmt_balance(0.0), _reconciled(false)
{
    _data_type = RECONCILE;
}

Reconcile::~Reconcile()
{
}

bool
Reconcile::operator==(const Reconcile& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._account_id != _account_id) return false;
    if (rhs._stmt_balance != _stmt_balance) return false;
    if (rhs._stmt_date != _stmt_date) return false;
    if (rhs._end_date != _end_date) return false;
    if (rhs._reconciled != _reconciled) return false;
    if (rhs._lines != _lines) return false;
    return true;
}

bool
Reconcile::operator!=(const Reconcile& rhs) const
{
    return !(*this == rhs);

}

ReconcileLine::ReconcileLine(Id tx_id, int seq)
    : gltx_id(tx_id), seq_num(seq)
{
}

bool
ReconcileLine::operator==(const ReconcileLine& rhs) const
{
    if (gltx_id != rhs.gltx_id) return false;
    if (seq_num != rhs.seq_num) return false;
    return true;
}

bool
ReconcileLine::operator!=(const ReconcileLine& rhs) const
{
    return !(*this == rhs);
}
