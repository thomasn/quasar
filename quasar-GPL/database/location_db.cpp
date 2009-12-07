// $Id: location_db.cpp,v 1.10 2005/03/01 19:59:42 bpepers Exp $
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

#include "location.h"
#include "location_select.h"

// Create a Location
bool
QuasarDB::create(Location& location)
{
    if (!validate(location)) return false;

    QString cmd = insertCmd("location", "location_id", "name,store_id,"
			    "section,fixture,bin");
    Stmt stmt(_connection, cmd);

    insertData(location, stmt);
    stmtSetString(stmt, location.canonicalName());
    stmtSetId(stmt, location.storeId());
    stmtSetString(stmt, location.section());
    stmtSetString(stmt, location.fixture());
    stmtSetString(stmt, location.bin());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, location);
    return true;
}

// Delete a Location
bool
QuasarDB::remove(const Location& location)
{
    if (location.id() == INVALID_ID) return false;
    if (!removeData(location, "location", "location_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, location);
    return true;
}

// Update a Location
bool
QuasarDB::update(const Location& orig, Location& location)
{
    if (orig.id() == INVALID_ID || location.id() == INVALID_ID) return false;
    if (!validate(location)) return false;

    // Update the location table
    QString cmd = updateCmd("location", "location_id", "name,store_id,"
			    "section,fixture,bin");
    Stmt stmt(_connection, cmd);

    updateData(orig, location, stmt);
    stmtSetString(stmt, location.canonicalName());
    stmtSetId(stmt, location.storeId());
    stmtSetString(stmt, location.section());
    stmtSetString(stmt, location.fixture());
    stmtSetString(stmt, location.bin());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Location.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id location_id, Location& location)
{
    if (location_id == INVALID_ID) return false;
    LocationSelect conditions;
    vector<Location> locations;

    conditions.id = location_id;
    if (!select(locations, conditions)) return false;
    if (locations.size() != 1) return false;

    location = locations[0];
    return true;
}

// Returns a vector of Location's.  Returns locations sorted by name.
bool
QuasarDB::select(vector<Location>& locations, const LocationSelect& conditions)
{
    locations.clear();

    QString cmd = selectCmd("location", "location_id", "name,store_id,"
			    "section,fixture,bin", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Location location;
	int next = 1;
	selectData(location, stmt, next);
	location.setName(stmtGetString(stmt, next++));
	location.setStoreId(stmtGetId(stmt, next++));
	location.setSection(stmtGetString(stmt, next++));
	location.setFixture(stmtGetString(stmt, next++));
	location.setBin(stmtGetString(stmt, next++));
	locations.push_back(location);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Location& location)
{
    if (!validate((DataObject&)location)) return false;

    if (location.storeId() == INVALID_ID)
	return error("Store is required");

    QString section = location.section().stripWhiteSpace();
    QString fixture = location.fixture().stripWhiteSpace();
    QString bin = location.bin().stripWhiteSpace();

    if (section.isEmpty())
	return error("Section can't be blank");

    if (fixture.isEmpty() && !bin.isEmpty())
	return error("Fixture can't be blank if Bin is set");

    return true;
}
