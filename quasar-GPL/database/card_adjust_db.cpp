// $Id: card_adjust_db.cpp,v 1.5 2005/03/01 19:59:42 bpepers Exp $
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

#include "quasar_db.h"

#include "card_adjust.h"
#include "card_adjust_select.h"
#include "card.h"
#include "account.h"

// Create a CardAdjust
bool
QuasarDB::create(CardAdjust& adjustment)
{
    if (!validate(adjustment)) return false;
    if (!create((Gltx&)adjustment)) return false;

    commit();
    dataSignal(DataEvent::Insert, adjustment);
    return true;
}

// Delete a CardAdjust
bool
QuasarDB::remove(const CardAdjust& adjustment)
{
    if (adjustment.id() == INVALID_ID) return false;
    if (!remove((Gltx&)adjustment)) return false;

    commit();
    dataSignal(DataEvent::Delete, adjustment);
    return true;
}

// Update a CardAdjust
bool
QuasarDB::update(const CardAdjust& orig, CardAdjust& adjustment)
{
    if (orig.id() == INVALID_ID || adjustment.id() == INVALID_ID) return false;
    if (!validate(adjustment)) return false;
    if (!update(orig, (Gltx&)adjustment)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a CardAdjust.
bool
QuasarDB::lookup(Id adjustment_id, CardAdjust& adjustment)
{
    if (adjustment_id == INVALID_ID) return false;
    CardAdjustSelect conditions;
    vector<CardAdjust> adjustments;

    conditions.id = adjustment_id;
    if (!select(adjustments, conditions)) return false;
    if (adjustments.size() != 1) return false;

    adjustment = adjustments[0];
    return true;
}

// Returns a vector of CardAdjusts.
bool
QuasarDB::select(vector<CardAdjust>& adjusts, const CardAdjustSelect& sel)
{
    adjusts.clear();

    QString cmd = gltxCmd("gltx", "gltx_id", "", sel);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	CardAdjust adjustment;
	int next = 1;
	selectData(adjustment, stmt, next);
	selectGltx(adjustment, stmt, next);
	adjusts.push_back(adjustment);
    }

    for (unsigned int i = 0; i < adjusts.size(); ++i) {
	Id gltx_id = adjusts[i].id();
	GLTX_ACCOUNTS(adjusts);
	GLTX_CARDS(adjusts);
	GLTX_PAYMENTS(adjusts);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const CardAdjust& adjustment)
{
    return validate((Gltx&)adjustment);
}
