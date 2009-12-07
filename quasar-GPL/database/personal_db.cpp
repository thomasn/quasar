// $Id: personal_db.cpp,v 1.25 2005/03/01 19:59:42 bpepers Exp $
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

#include "personal.h"
#include "personal_select.h"

// Create a Personal
bool
QuasarDB::create(Personal& personal)
{
    if (!validate(personal)) return false;
    if (!create((Card&)personal)) return false;

    QString cmd = "insert into card_personal (personal_id) values (?)";
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, personal.id());
    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, personal);
    return true;
}

// Delete a Personal
bool
QuasarDB::remove(const Personal& personal)
{
    if (personal.id() == INVALID_ID) return false;
    if (!remove((Card&)personal)) return false;

    commit();
    dataSignal(DataEvent::Delete, personal);
    return true;
}

// Update a Personal
bool
QuasarDB::update(const Personal& orig, Personal& personal)
{
    if (orig.id() == INVALID_ID || personal.id() == INVALID_ID) return false;
    if (!validate(personal)) return false;
    if (!update(orig, (Card&)personal)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Personal.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id personal_id, Personal& personal)
{
    if (personal_id == INVALID_ID) return false;
    PersonalSelect conditions;
    vector<Personal> personals;

    conditions.id = personal_id;
    if (!select(personals, conditions)) return false;
    if (personals.size() != 1) return false;

    personal = personals[0];
    return true;
}

// Returns a vector of Personals.  Returns personals sorted by name.
bool
QuasarDB::select(vector<Personal>& personals, const PersonalSelect& conditions)
{
    personals.clear();

    QString table = "card_personal join card on card.card_id = "
	"card_personal.personal_id";

    QString cmd = selectCmd(table, "card_id", "company,first_name,"
			    "last_name,number,street,street2,city,"
			    "province,country,postal,phone_num,phone2_num,"
			    "fax_num,email,web_page,contact,comments,"
			    "patgroup_id", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Personal personal;
	int next = 1;
	selectData(personal, stmt, next);
	selectCard(personal, stmt, next);
	personals.push_back(personal);
    }

    QString cmd1 = "select group_id from card_group where card_id = ? "
	"order by seq_num";
    Stmt stmt1(_connection, cmd1);

    QString cmd2 = "select extra_id,data_value from card_extra "
	"where card_id = ? order by seq_num";
    Stmt stmt2(_connection, cmd2);

    for (unsigned int i = 0; i < personals.size(); ++i) {
	Id card_id = personals[i].id();

	stmtSetId(stmt1, card_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    Id group_id = stmtGetId(stmt1, 1);
	    personals[i].groups().push_back(group_id);
	}

	stmtSetId(stmt2, card_id);
	if (!execute(stmt2)) return false;
	while (stmt2.next()) {
	    Id extra_id = stmtGetId(stmt2, 1);
	    QString value = stmtGetString(stmt2, 2);
	    personals[i].setValue(extra_id, value);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Personal& personal)
{
    if (!validate((Card&)personal)) return false;

    return true;
}
