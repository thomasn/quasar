// $Id: shift_db.cpp,v 1.9 2005/03/01 19:59:42 bpepers Exp $
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

#include "shift.h"
#include "shift_select.h"

// Create a Shift
bool
QuasarDB::create(Shift& shift)
{
    if (!validate(shift)) return false;
    if (!create((Gltx&)shift)) return false;

    QString cmd = insertText("shift", "shift_id", "adjust_id,transfer_id");
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, shift.id());
    stmtSetId(stmt, shift.adjustmentId());
    stmtSetId(stmt, shift.transferId());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, shift);
    return true;
}

// Delete a Shift
bool
QuasarDB::remove(const Shift& shift)
{
    if (shift.id() == INVALID_ID) return false;
    if (shift.shiftId() != INVALID_ID) return false;

    QString cmd = "update gltx set shift_id = null where shift_id = ?";
    Stmt stmt(_connection, cmd);
    stmtSetId(stmt, shift.id());
    if (!execute(stmt)) return false;
    if (!remove((Gltx&)shift)) return false;

    commit();
    dataSignal(DataEvent::Delete, shift);
    return true;
}

// Update a Shift
bool
QuasarDB::update(const Shift& orig, Shift& shift)
{
    if (orig.id() == INVALID_ID || shift.id() == INVALID_ID) return false;
    if (!validate(shift)) return false;
    if (!update(orig, (Gltx&)shift)) return false;

    // Update the shift tables
    QString cmd = updateText("shift", "shift_id", "adjust_id,transfer_id");
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, shift.adjustmentId());
    stmtSetId(stmt, shift.transferId());
    stmtSetId(stmt, shift.id());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Shift.
bool
QuasarDB::lookup(Id shift_id, Shift& shift)
{
    if (shift_id == INVALID_ID) return false;
    ShiftSelect conditions;
    vector<Shift> shifts;

    conditions.id = shift_id;
    if (!select(shifts, conditions)) return false;
    if (shifts.size() != 1) return false;

    shift = shifts[0];
    return true;
}

// Returns a vector of Shifts.
bool
QuasarDB::select(vector<Shift>& shifts, const ShiftSelect& conditions)
{
    shifts.clear();

    QString cmd = gltxCmd("shift", "shift_id", "adjust_id,transfer_id",
			  conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Shift shift;
	int next = 1;
	selectData(shift, stmt, next);
	selectGltx(shift, stmt, next);
	shift.setAdjustmentId(stmtGetId(stmt, next++));
	shift.setTransferId(stmtGetId(stmt, next++));
	shifts.push_back(shift);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Shift& shift)
{
    return validate((Gltx&)shift);
}
