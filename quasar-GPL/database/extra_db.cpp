// $Id: extra_db.cpp,v 1.9 2005/03/01 19:59:42 bpepers Exp $
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

#include "extra.h"
#include "extra_select.h"

// Create a Extra
bool
QuasarDB::create(Extra& extra)
{
    if (!validate(extra)) return false;

    QString cmd = insertCmd("extra_data", "extra_id", "table_name,"
			    "data_name");
    Stmt stmt(_connection, cmd);

    insertData(extra, stmt);
    stmtSetString(stmt, extra.table());
    stmtSetString(stmt, extra.name());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, extra);
    return true;
}

// Delete a Extra
bool
QuasarDB::remove(const Extra& extra)
{
    if (extra.id() == INVALID_ID) return false;
    if (!removeData(extra, "extra_data", "extra_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, extra);
    return true;
}

// Update a Extra
bool
QuasarDB::update(const Extra& orig, Extra& extra)
{
    if (orig.id() == INVALID_ID || extra.id() == INVALID_ID) return false;
    if (!validate(extra)) return false;

    // Update the extra table
    QString cmd = updateCmd("extra_data", "extra_id", "table_name,"
			    "data_name");
    Stmt stmt(_connection, cmd);

    updateData(orig, extra, stmt);
    stmtSetString(stmt, extra.table());
    stmtSetString(stmt, extra.name());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Extra.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id extra_id, Extra& extra)
{
    if (extra_id == INVALID_ID) return false;
    ExtraSelect conditions;
    vector<Extra> extras;

    conditions.id = extra_id;
    if (!select(extras, conditions)) return false;
    if (extras.size() != 1) return false;

    extra = extras[0];
    return true;
}

// Returns a vector of Extra's.  Returns extras sorted by name.
bool
QuasarDB::select(vector<Extra>& extras, const ExtraSelect& conditions)
{
    extras.clear();

    QString cmd = selectCmd("extra_data", "extra_id", "table_name,"
			    "data_name", conditions, "data_name");
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Extra extra;
	int next = 1;
	selectData(extra, stmt, next);
	extra.setTable(stmtGetString(stmt, next++));
	extra.setName(stmtGetString(stmt, next++));
	extras.push_back(extra);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Extra& extra)
{
    if (!validate((DataObject&)extra)) return false;

    if (extra.table().stripWhiteSpace().isEmpty())
	return error("Blank table name");

    if (extra.name().stripWhiteSpace().isEmpty())
	return error("Blank name");

    ExtraSelect conditions;
    conditions.table = extra.table();
    conditions.name = extra.name();
    vector<Extra> extras;
    select(extras, conditions);
    for (unsigned int i = 0; i < extras.size(); ++i)
	if (extras[i].id() != extra.id())
	    return error("Duplicate data name");

    return true;
}
