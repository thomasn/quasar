// $Id: general_db.cpp,v 1.20 2005/03/01 19:59:42 bpepers Exp $
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

#include "general.h"
#include "general_select.h"

// Create a General
bool
QuasarDB::create(General& general)
{
    if (!validate(general)) return false;
    if (!create((Gltx&)general)) return false;

    QString cmd = "insert into general (general_id) values (?)";
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, general.id());
    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, general);
    return true;
}

// Delete a General
bool
QuasarDB::remove(const General& general)
{
    if (general.id() == INVALID_ID) return false;
    if (!remove((Gltx&)general)) return false;

    commit();
    dataSignal(DataEvent::Delete, general);
    return true;
}

// Update a General
bool
QuasarDB::update(const General& orig, General& general)
{
    if (orig.id() == INVALID_ID || general.id() == INVALID_ID) return false;
    if (!validate(general)) return false;
    if (!update(orig, (Gltx&)general)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a General.
bool
QuasarDB::lookup(Id general_id, General& general)
{
    if (general_id == INVALID_ID) return false;
    GeneralSelect conditions;
    vector<General> generals;

    conditions.id = general_id;
    if (!select(generals, conditions)) return false;
    if (generals.size() != 1) return false;

    general = generals[0];
    return true;
}

// Returns a vector of Generals.
bool
QuasarDB::select(vector<General>& generals, const GeneralSelect& conditions)
{
    generals.clear();

    QString cmd = gltxCmd("general", "general_id", "", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	General general;
	int next = 1;
	selectData(general, stmt, next);
	selectGltx(general, stmt, next);
	generals.push_back(general);
    }

    for (unsigned int i = 0; i < generals.size(); ++i) {
	Id gltx_id = generals[i].id();
	GLTX_ACCOUNTS(generals);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const General& general)
{
    if (!validate((Gltx&)general)) return false;

    return true;
}
