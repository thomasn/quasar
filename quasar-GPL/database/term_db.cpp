// $Id: term_db.cpp,v 1.10 2005/03/01 19:59:42 bpepers Exp $
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

#include "term.h"
#include "term_select.h"

// Create a Term
bool
QuasarDB::create(Term& term)
{
    if (!validate(term)) return false;

    QString cmd = insertCmd("term", "term_id", "name,cod,due_days,"
			    "disc_days,discount");
    Stmt stmt(_connection, cmd);

    insertData(term, stmt);
    stmtSetString(stmt, term.name());
    stmtSetBool(stmt, term.isCOD());
    stmtSetInt(stmt, term.dueDays());
    stmtSetInt(stmt, term.discountDays());
    stmtSetFixed(stmt, term.discount());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, term);
    return true;
}

// Delete a Term
bool
QuasarDB::remove(const Term& term)
{
    if (term.id() == INVALID_ID) return false;
    if (!removeData(term, "term", "term_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, term);
    return true;
}

// Update a Term
bool
QuasarDB::update(const Term& orig, Term& term)
{
    if (orig.id() == INVALID_ID || term.id() == INVALID_ID) return false;
    if (!validate(term)) return false;

    // Update the term table
    QString cmd = updateCmd("term", "term_id", "name,cod,due_days,"
			    "disc_days,discount");
    Stmt stmt(_connection, cmd);

    updateData(orig, term, stmt);
    stmtSetString(stmt, term.name());
    stmtSetBool(stmt, term.isCOD());
    stmtSetInt(stmt, term.dueDays());
    stmtSetInt(stmt, term.discountDays());
    stmtSetFixed(stmt, term.discount());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Term.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id term_id, Term& term)
{
    if (term_id == INVALID_ID) return false;
    TermSelect conditions;
    vector<Term> terms;

    conditions.id = term_id;
    if (!select(terms, conditions)) return false;
    if (terms.size() != 1) return false;

    term = terms[0];
    return true;
}

// Returns a vector of Terms.  Returns Terms sorted by name.
bool
QuasarDB::select(vector<Term>& terms, const TermSelect& conditions)
{
    terms.clear();

    QString cmd = selectCmd("term", "term_id", "cod,due_days,"
			    "disc_days,discount", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Term term;
	int next = 1;
	selectData(term, stmt, next);
	term.setCOD(stmtGetBool(stmt, next++));
	term.setDueDays(stmtGetInt(stmt, next++));
	term.setDiscountDays(stmtGetInt(stmt, next++));
	term.setDiscount(stmtGetFixed(stmt, next++));
	terms.push_back(term);
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Term& term)
{
    if (!validate((DataObject&)term)) return false;

    // Check for duplicate name
    TermSelect conditions;
    conditions.name = term.name();
    vector<Term> terms;
    select(terms, conditions);
    for (unsigned int i = 0; i < terms.size(); ++i)
	if (terms[i].id() != term.id())
	    return error("Name already used for another term");

    if (term.name().stripWhiteSpace().isEmpty())
	return error("Blank term name");

    if (term.dueDays() < 0)
	return error("Negative due days");

    if (term.discountDays() < 0)
	return error("Negative discount days");

    if (term.discountDays() > term.dueDays())
	return error("Discount days larger than due days");

    if (term.discount() < 0.0)
	return error("Negative discount");

    if (term.discount() >= 100.0)
	return error("Discount too large");

    return true;
}
