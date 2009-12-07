// $Id: tax_db.cpp,v 1.28 2005/06/08 19:42:25 bpepers Exp $
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

#include "tax.h"
#include "tax_select.h"
#include "account.h"

// Create a Tax
bool
QuasarDB::create(Tax& tax)
{
    if (!validate(tax)) return false;

    QString cmd = insertCmd("tax", "tax_id", "name,description,grouped,"
			    "number,rate,collected_id,paid_id");
    Stmt stmt(_connection, cmd);

    insertData(tax, stmt);
    stmtSetString(stmt, tax.name());
    stmtSetString(stmt, tax.description());
    stmtSetBool(stmt, tax.isGroup());
    stmtSetString(stmt, tax.number());
    stmtSetFixed(stmt, tax.rate());
    stmtSetId(stmt, tax.collectedAccount());
    stmtSetId(stmt, tax.paidAccount());

    if (!execute(stmt)) return false;

    cmd = "insert into tax_on_tax (tax_id,seq_num,child_id) "
	"values (?,?,?)";
    stmt.setCommand(cmd);
    unsigned int line;
    for (line = 0; line < tax.tot_ids().size(); ++line) {
	stmtSetId(stmt, tax.id());
	stmtSetInt(stmt, line);
	stmtSetId(stmt, tax.tot_ids()[line]);
	if (!execute(stmt)) return false;
    }

    cmd = "insert into tax_group (tax_id,seq_num,child_id) "
	"values (?,?,?)";
    stmt.setCommand(cmd);
    for (line = 0; line < tax.group_ids().size(); ++line) {
	stmtSetId(stmt, tax.id());
	stmtSetInt(stmt, line);
	stmtSetId(stmt, tax.group_ids()[line]);
	if (!execute(stmt)) return false;
    }

    commit();
    dataSignal(DataEvent::Insert, tax);
    return true;
}

// Delete a Tax
bool
QuasarDB::remove(const Tax& tax)
{
    if (tax.id() == INVALID_ID) return false;
    if (!removeData(tax, "tax", "tax_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, tax);
    return true;
}

// Update a Tax
bool
QuasarDB::update(const Tax& orig, Tax& tax)
{
    if (orig.id() == INVALID_ID || tax.id() == INVALID_ID) return false;
    if (!validate(tax)) return false;

    // Update the tax table
    QString cmd = updateCmd("tax", "tax_id", "name,description,grouped,"
			    "number,rate,collected_id,paid_id");
    Stmt stmt(_connection, cmd);

    updateData(orig, tax, stmt);
    stmtSetString(stmt, tax.name());
    stmtSetString(stmt, tax.description());
    stmtSetBool(stmt, tax.isGroup());
    stmtSetString(stmt, tax.number());
    stmtSetFixed(stmt, tax.rate());
    stmtSetId(stmt, tax.collectedAccount());
    stmtSetId(stmt, tax.paidAccount());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");
    if (!remove(orig, "tax_on_tax", "tax_id")) return false;
    if (!remove(orig, "tax_group", "tax_id")) return false;

    cmd = "insert into tax_on_tax (tax_id,seq_num,child_id) "
	"values (?,?,?)";
    stmt.setCommand(cmd);
    unsigned int line;
    for (line = 0; line < tax.tot_ids().size(); ++line) {
	stmtSetId(stmt, tax.id());
	stmtSetInt(stmt, line);
	stmtSetId(stmt, tax.tot_ids()[line]);
	if (!execute(stmt)) return false;
    }

    cmd = "insert into tax_group (tax_id,seq_num,child_id) "
	"values (?,?,?)";
    stmt.setCommand(cmd);
    for (line = 0; line < tax.group_ids().size(); ++line) {
	stmtSetId(stmt, tax.id());
	stmtSetInt(stmt, line);
	stmtSetId(stmt, tax.group_ids()[line]);
	if (!execute(stmt)) return false;
    }

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Tax.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id tax_id, Tax& tax)
{
    if (tax_id == INVALID_ID) return false;
    TaxSelect conditions;
    vector<Tax> taxes;

    conditions.id = tax_id;
    if (!select(taxes, conditions)) return false;
    if (taxes.size() != 1) return false;

    tax = taxes[0];
    return true;
}

// Lookup a Tax.  This is just a shortcut to using select.
bool
QuasarDB::lookup(const QString& name, Tax& tax)
{
    if (name.isEmpty()) return false;
    TaxSelect conditions;
    vector<Tax> taxes;

    conditions.name = name;
    if (!select(taxes, conditions)) return false;
    if (taxes.size() != 1) return false;

    tax = taxes[0];
    return true;
}

// Returns a vector of Taxes.  Returns Taxes sorted by name.
bool
QuasarDB::select(vector<Tax>& taxes, const TaxSelect& conditions)
{
    taxes.clear();

    QString cmd = selectCmd("tax", "tax_id", "name,description,grouped,"
			    "number,rate,collected_id,paid_id",
			    conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Tax tax;
	int next = 1;
	selectData(tax, stmt, next);
	tax.setName(stmtGetString(stmt, next++));
	tax.setDescription(stmtGetString(stmt, next++));
	tax.setGroup(stmtGetBool(stmt, next++));
	tax.setNumber(stmtGetString(stmt, next++));
	tax.setRate(stmtGetFixed(stmt, next++));
	tax.setCollectedAccount(stmtGetId(stmt, next++));
	tax.setPaidAccount(stmtGetId(stmt, next++));
	taxes.push_back(tax);
    }

    cmd = "select child_id from tax_on_tax where tax_id = ? order by seq_num";
    stmt.setCommand(cmd);

    for (unsigned int i = 0; i < taxes.size(); ++i) {
	Id tax_id = taxes[i].id();

	stmtSetId(stmt, tax_id);
	if (!execute(stmt)) return false;
	while (stmt.next())
	    taxes[i].tot_ids().push_back(stmtGetId(stmt, 1));
    }

    cmd = "select child_id from tax_group where tax_id = ? order by seq_num";
    stmt.setCommand(cmd);

    for (unsigned int i = 0; i < taxes.size(); ++i) {
	Id tax_id = taxes[i].id();
	if (!taxes[i].isGroup()) continue;

	stmtSetId(stmt, tax_id);
	if (!execute(stmt)) return false;
	while (stmt.next())
	    taxes[i].group_ids().push_back(stmtGetId(stmt, 1));
    }

    commit();
    return true;
}

static bool
findLoops(QuasarDB* db, Id tax_id, const vector<Id>& tot_ids)
{
    for (unsigned int i = 0; i < tot_ids.size(); ++i) {
	Id tot_id = tot_ids[i];
	if (tot_id == tax_id)
	    return true;

	Tax child;
	db->lookup(tot_id, child);
	if (findLoops(db, tax_id, child.tot_ids()))
	    return true;
    }
    return false;
}

bool
QuasarDB::validate(const Tax& tax)
{
    if (!validate((DataObject&)tax)) return false;

    // Check for duplicate name
    TaxSelect conditions;
    conditions.name = tax.name();
    vector<Tax> taxes;
    select(taxes, conditions);
    for (unsigned int i = 0; i < taxes.size(); ++i)
	if (taxes[i].id() != tax.id())
	    return error("Name already used for another tax");

    if (tax.name().stripWhiteSpace().isEmpty())
	return error("Blank tax name");

    if (tax.description().stripWhiteSpace().isEmpty())
	return error("Blank tax description");

    if (tax.rate() < 0.0)
	return error("Negative tax rate");

    if (tax.isGroup()) {
	if (tax.collectedAccount() != INVALID_ID)
	    return error("Collected account not allowed for group tax");
	if (tax.paidAccount() != INVALID_ID)
	    return error("Paid account not allowed for group tax");

	for (unsigned int i = 0; i < tax.group_ids().size(); ++i) {
	    Tax child;
	    lookup(tax.group_ids()[i], child);
	    if (child.isGroup())
		return error("Group tax can't contain group taxes");
	}
    } else {
	Account account;
	if (!lookup(tax.collectedAccount(), account))
	    return error("Collected account doesn't exist");
	if (account.isHeader())
	    return error("Collected account is a header");

	if (!lookup(tax.paidAccount(), account))
	    return error("Paid account doesn't exist");
	if (account.isHeader())
	    return error("Paid account is a header");

	for (unsigned int i = 0; i < tax.tot_ids().size(); ++i) {
	    Tax child;
	    lookup(tax.tot_ids()[i], child);
	    if (child.isGroup())
		return error("Tax on tax can't contain group taxes");
	}

	if (findLoops(this, tax.id(), tax.tot_ids()))
	    return error("Loop in tax on taxes");
    }

    return true;
}
