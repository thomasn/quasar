// $Id: station_db.cpp,v 1.6 2005/03/01 19:59:42 bpepers Exp $
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

#include "station.h"
#include "station_select.h"

// Create a Station
bool
QuasarDB::create(Station& station)
{
    if (!validate(station)) return false;

    // Auto allocate station number
    if (station.number().stripWhiteSpace() == "#") {
	fixed number = uniqueNumber("station", "number");
	station.setNumber(number.toString());
    }

    QString cmd = insertCmd("station", "station_id", "name,number,"
			    "store_id,offline_num");
    Stmt stmt(_connection, cmd);

    insertData(station, stmt);
    stmtSetString(stmt, station.name());
    stmtSetString(stmt, station.number());
    stmtSetId(stmt, station.storeId());
    stmtSetFixed(stmt, station.offlineNumber());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, station);
    return true;
}

// Delete a Station
bool
QuasarDB::remove(const Station& station)
{
    if (station.id() == INVALID_ID) return false;
    if (!removeData(station, "station", "station_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, station);
    return true;
}

// Update a Station
bool
QuasarDB::update(const Station& orig, Station& station)
{
    if (orig.id() == INVALID_ID || station.id() == INVALID_ID) return false;
    if (!validate(station)) return false;

    if (station.number().stripWhiteSpace() == "#")
	return error("Can't change number to '#'");

    // Update the station table
    QString cmd = updateCmd("station", "station_id", "name,number,"
			    "store_id,offline_num");
    Stmt stmt(_connection, cmd);

    updateData(orig, station, stmt);
    stmtSetString(stmt, station.name());
    stmtSetString(stmt, station.number());
    stmtSetId(stmt, station.storeId());
    stmtSetFixed(stmt, station.offlineNumber());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Station.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id station_id, Station& station)
{
    if (station_id == INVALID_ID) return false;
    StationSelect conditions;
    vector<Station> stations;

    conditions.id = station_id;
    if (!select(stations, conditions)) return false;
    if (stations.size() != 1) return false;

    station = stations[0];
    return true;
}

// Lookup a Station by its number
bool
QuasarDB::lookup(const QString& number, Station& station)
{
    if (number.isEmpty()) return false;
    StationSelect conditions;
    vector<Station> stations;

    conditions.number = number;
    if (!select(stations, conditions)) return false;
    if (stations.size() != 1) return false;

    station = stations[0];
    return true;
}

// Returns a vector of Station's.  Returns stations sorted by name.
bool
QuasarDB::select(vector<Station>& stations, const StationSelect& conditions)
{
    stations.clear();

    QString cmd = selectCmd("station", "station_id", "name,number,"
			    "store_id,offline_num", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Station station;
	int next = 1;
	selectData(station, stmt, next);
	station.setName(stmtGetString(stmt, next++));
	station.setNumber(stmtGetString(stmt, next++));
	station.setStoreId(stmtGetId(stmt, next++));
	station.setOfflineNumber(stmtGetFixed(stmt, next++));
	stations.push_back(station);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Station& station)
{
    if (!validate((DataObject&)station)) return false;

    if (station.name().stripWhiteSpace().isEmpty())
	return error("Blank station name");

    if (station.number().stripWhiteSpace().isEmpty())
	return error("Blank station number");

    if (station.offlineNumber() < 1)
	return error("Invalid offline number");

    return true;
}
