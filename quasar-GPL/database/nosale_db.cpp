// $Id: nosale_db.cpp,v 1.6 2005/03/01 19:59:42 bpepers Exp $
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

#include "nosale.h"
#include "nosale_select.h"

// Create a Nosale
bool
QuasarDB::create(Nosale& nosale)
{
    if (!validate(nosale)) return false;
    if (!create((Gltx&)nosale)) return false;

    commit();
    dataSignal(DataEvent::Insert, nosale);
    return true;
}

// Delete a Nosale
bool
QuasarDB::remove(const Nosale& nosale)
{
    if (nosale.id() == INVALID_ID) return false;
    if (!remove((Gltx&)nosale)) return false;

    commit();
    dataSignal(DataEvent::Delete, nosale);
    return true;
}

// Update a Nosale
bool
QuasarDB::update(const Nosale& orig, Nosale& nosale)
{
    if (orig.id() == INVALID_ID || nosale.id() == INVALID_ID) return false;
    if (!validate(nosale)) return false;
    if (!update(orig, (Gltx&)nosale)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Nosale.
bool
QuasarDB::lookup(Id nosale_id, Nosale& nosale)
{
    if (nosale_id == INVALID_ID) return false;
    NosaleSelect conditions;
    vector<Nosale> nosales;

    conditions.id = nosale_id;
    if (!select(nosales, conditions)) return false;
    if (nosales.size() != 1) return false;

    nosale = nosales[0];
    return true;
}

// Returns a vector of Nosales.
bool
QuasarDB::select(vector<Nosale>& nosales, const NosaleSelect& conditions)
{
    nosales.clear();

    QString cmd = gltxCmd("gltx", "gltx_id", "", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Nosale nosale;
	int next = 1;
	selectData(nosale, stmt, next);
	selectGltx(nosale, stmt, next);
	nosales.push_back(nosale);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Nosale& nosale)
{
    return validate((Gltx&)nosale);
}
