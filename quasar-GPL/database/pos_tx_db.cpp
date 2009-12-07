// $Id: pos_tx_db.cpp,v 1.31 2005/03/01 19:59:42 bpepers Exp $
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

#include "pos_tx.h"
#include "pos_tx_select.h"

// Create a new pos_tx
bool
QuasarDB::create(const PosTx& tx)
{
    QString cmd = "insert into pos_tx (tx_type,tx_id,tx_data) "
	"values (?,?,?)";
    Stmt stmt(_connection, cmd);

    stmtSetString(stmt, tx.type());
    stmtSetString(stmt, tx.id());
    stmtSetString(stmt, tx.data());

    if (!execute(stmt)) return false;

    commit();
    return true;
}

// Remove a pos_tx
bool
QuasarDB::remove(const PosTx& tx)
{
    QString cmd = "delete from pos_tx where tx_type=? and tx_id=?";
    Stmt stmt(_connection, cmd);

    stmtSetString(stmt, tx.type());
    stmtSetString(stmt, tx.id());
    if (!execute(stmt)) return false;

    commit();
    return true;
}

// Update the pos_tx information
bool
QuasarDB::update(const PosTx& tx)
{
    QString cmd = "update pos_tx set tx_data=? where tx_type=? "
	"and tx_id=?";
    Stmt stmt(_connection, cmd);

    stmtSetString(stmt, tx.data());
    stmtSetString(stmt, tx.type());
    stmtSetString(stmt, tx.id());

    if (!execute(stmt)) return false;

    commit();
    return true;
}

// Read in a pos_tx record
bool
QuasarDB::lookup(const QString& type, const QString& id, PosTx& tx)
{
    bool result = false;

    QString cmd = "select tx_data from pos_tx where tx_type=? "
	"and tx_id=?";
    Stmt stmt(_connection, cmd);
    stmtSetString(stmt, type);
    stmtSetString(stmt, id);
    if (!execute(stmt)) return false;
    if (stmt.next()) {
	tx.setType(type);
	tx.setId(id);
	tx.setData(stmtGetString(stmt, 1));
	result = true;
    }

    commit();
    return result;
}

// Returns a vector of pos_tx
bool
QuasarDB::select(vector<PosTx>& txs, const PosTxSelect& conditions)
{
    txs.clear();

    QString cmd = "select tx_type,tx_id,tx_data from pos_tx";
    cmd += conditions.where();
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	PosTx tx;
	int next = 1;
	tx.setType(stmtGetString(stmt, next++));
	tx.setId(stmtGetString(stmt, next++));
	tx.setData(stmtGetString(stmt, next++));
	txs.push_back(tx);
    }

    commit();
    return true;
}
