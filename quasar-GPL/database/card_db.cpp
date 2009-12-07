// $Id: card_db.cpp,v 1.30 2005/03/01 19:59:42 bpepers Exp $
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
#include "card.h"
#include "card_select.h"

// NOTE: the patgroup_id is handled a little oddly.  It is really
// controlled by the PatGroup class so its always created as NULL
// and never updated here but it is read in from the card table.
// So to add a card to a patronage group you have to update the
// PatGroup class.  Changing the customer or vendor and calling
// update will do nothing.

// Create a Card
bool
QuasarDB::create(Card& card)
{
    QString cmd = insertCmd("card", "card_id", "data_type,company,"
			    "first_name,last_name,name,number,street,"
			    "street2,city,province,country,postal,"
			    "phone_num,phone2_num,fax_num,email,web_page,"
			    "contact,comments,patgroup_id");
    Stmt stmt(_connection, cmd);

    insertData(card, stmt);
    stmtSetInt(stmt, card.dataType());
    stmtSetBool(stmt, card.company());
    stmtSetString(stmt, card.firstName());
    stmtSetString(stmt, card.lastName());
    stmtSetString(stmt, card.name());
    stmtSetString(stmt, card.number());
    stmtSetString(stmt, card.street());
    stmtSetString(stmt, card.street2());
    stmtSetString(stmt, card.city());
    stmtSetString(stmt, card.province());
    stmtSetString(stmt, card.country());
    stmtSetString(stmt, card.postal());
    stmtSetString(stmt, card.phoneNumber());
    stmtSetString(stmt, card.phone2Number());
    stmtSetString(stmt, card.faxNumber());
    stmtSetString(stmt, card.email());
    stmtSetString(stmt, card.webPage());
    stmtSetString(stmt, card.contact());
    stmtSetString(stmt, card.comments());
    stmtSetId(stmt, INVALID_ID); // NOTE: see comment at top of file

    if (!execute(stmt)) return false;
    return sqlCreateLines(card);
}

// Delete a Card
bool
QuasarDB::remove(const Card& card)
{
    if (!sqlDeleteLines(card)) return false;
    return removeData(card, "card", "card_id");
}

// Update a Card
bool
QuasarDB::update(const Card& orig, Card& card)
{
    if (orig.isActive() && !card.isActive()) {
	fixed balance = cardBalance(orig.id());
	if (balance != 0.0)
	    return error("Card has a balance");
    }

    if (orig.patGroupId() != card.patGroupId())
	return error("Can't change the patronage group through the card");

    QString cmd = updateCmd("card", "card_id", "data_type,company,"
			    "first_name,last_name,name,number,street,"
			    "street2,city,province,country,postal,"
			    "phone_num,phone2_num,fax_num,email,web_page,"
			    "contact,comments");
    Stmt stmt(_connection, cmd);

    updateData(orig, card, stmt);
    stmtSetInt(stmt, card.dataType());
    stmtSetBool(stmt, card.company());
    stmtSetString(stmt, card.firstName());
    stmtSetString(stmt, card.lastName());
    stmtSetString(stmt, card.name());
    stmtSetString(stmt, card.number());
    stmtSetString(stmt, card.street());
    stmtSetString(stmt, card.street2());
    stmtSetString(stmt, card.city());
    stmtSetString(stmt, card.province());
    stmtSetString(stmt, card.country());
    stmtSetString(stmt, card.postal());
    stmtSetString(stmt, card.phoneNumber());
    stmtSetString(stmt, card.phone2Number());
    stmtSetString(stmt, card.faxNumber());
    stmtSetString(stmt, card.email());
    stmtSetString(stmt, card.webPage());
    stmtSetString(stmt, card.contact());
    stmtSetString(stmt, card.comments());
    // NOTE: don't update patgroup_id here - see comment at top of file
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(card)) return false;

    return true;
}

// Lookup a Card.  This is just a shortcut for select.
bool
QuasarDB::lookup(Id card_id, Card& card)
{
    if (card_id == INVALID_ID) return false;
    CardSelect conditions;
    vector<Card> cards;

    conditions.id = card_id;
    if (!select(cards, conditions)) return false;
    if (cards.size() != 1) return false;

    card = cards[0];
    return true;
}

// Returns a count of Cards based on the conditions.
bool
QuasarDB::count(int& count, const CardSelect& conditions)
{
    count = 0;

    QString cmd = "select count(*) from card " + conditions.where();
    Stmt stmt(_connection, cmd);

    execute(stmt);
    while (stmt.next())
	count = stmtGetInt(stmt, 1);

    commit();
    return true;
}

// Returns a vector of Cards.  Returns cards sorted by name.
bool
QuasarDB::select(vector<Card>& cards, const CardSelect& conditions)
{
    cards.clear();

    QString cmd = selectCmd("card", "card_id", "data_type,company,"
			    "first_name,last_name,number,street,"
			    "street2,city,province,country,postal,"
			    "phone_num,phone2_num,fax_num,email,web_page,"
			    "contact,comments,patgroup_id", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Card card;
	int next = 1;
	selectData(card, stmt, next);
	card.setDataType(DataObject::DataType(stmtGetInt(stmt, next++)));
	selectCard(card, stmt, next);
	cards.push_back(card);
    }

    QString cmd1 = "select group_id from card_group where card_id = ? "
	"order by seq_num";
    Stmt stmt1(_connection, cmd1);

    QString cmd2 = "select discount_id from card_discount where "
	"card_id = ? order by seq_num";
    Stmt stmt2(_connection, cmd2);

    QString cmd3 = "select extra_id,data_value from card_extra "
	"where card_id = ? order by seq_num";
    Stmt stmt3(_connection, cmd3);

    for (unsigned int i = 0; i < cards.size(); ++i) {
	Id card_id = cards[i].id();

	stmtSetId(stmt1, card_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    Id group_id = stmtGetId(stmt1, 1);
	    cards[i].groups().push_back(group_id);
	}

	stmtSetId(stmt2, card_id);
	if (!execute(stmt2)) return false;
	while (stmt2.next()) {
	    Id discount_id = stmtGetId(stmt2, 1);
	    cards[i].discounts().push_back(discount_id);
	}

	stmtSetId(stmt3, card_id);
	if (!execute(stmt3)) return false;
	while (stmt3.next()) {
	    Id extra_id = stmtGetId(stmt3, 1);
	    QString value = stmtGetString(stmt3, 2);
	    cards[i].setValue(extra_id, value);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Card& card)
{
    if (!validate((DataObject&)card)) return false;

    if (card.company()) {
	if (card.lastName().stripWhiteSpace().isEmpty())
	    return error("Blank company name");
    } else {
	if (card.lastName().stripWhiteSpace().isEmpty())
	    return error("Blank last name");
    }

    // Check for duplicate number
    if (!card.number().isEmpty()) {
	CardSelect conditions;
	conditions.number = card.number();
	conditions.type = card.dataType();
	vector<Card> cards;
	select(cards, conditions);
	for (unsigned int i = 0; i < cards.size(); ++i)
	    if (cards[i].id() != card.id())
		return error("Number used for another card: " + card.number());
    }

    return true;
}

bool
QuasarDB::sqlCreateLines(const Card& card)
{
    QString cmd = insertText("card_group", "card_id", "seq_num,group_id");
    Stmt stmt(_connection, cmd);
    unsigned int i;
    for (i = 0; i < card.groups().size(); ++i) {
	stmtSetId(stmt, card.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, card.groups()[i]);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("card_discount", "card_id", "seq_num,discount_id");
    stmt.setCommand(cmd);
    for (i = 0; i < card.discounts().size(); ++i) {
	stmtSetId(stmt, card.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, card.discounts()[i]);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("card_extra", "card_id", "seq_num,extra_id,data_value");
    stmt.setCommand(cmd);
    for (i = 0; i < card.extra().size(); ++i) {
	const DataPair& line = card.extra()[i];

	stmtSetId(stmt, card.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.first);
	stmtSetString(stmt, line.second);
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const Card& card)
{
    if (!remove(card, "card_group", "card_id")) return false;
    if (!remove(card, "card_discount", "card_id")) return false;
    if (!remove(card, "card_extra", "card_id")) return false;
    return true;
}
