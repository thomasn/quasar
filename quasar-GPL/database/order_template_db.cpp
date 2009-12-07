// $Id: order_template_db.cpp,v 1.9 2005/03/01 19:59:42 bpepers Exp $
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

#include "order_template.h"
#include "order_template_select.h"

// Create an OrderTemplate
bool
QuasarDB::create(OrderTemplate& temp)
{
    if (!validate(temp)) return false;

    QString cmd = insertCmd("order_template", "template_id", "name,"
			    "vendor_id");
    Stmt stmt(_connection, cmd);

    insertData(temp, stmt);
    stmtSetString(stmt, temp.name());
    stmtSetId(stmt, temp.vendorId());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(temp)) return false;

    commit();
    dataSignal(DataEvent::Insert, temp);
    return true;
}

// Delete an OrderTemplate
bool
QuasarDB::remove(const OrderTemplate& temp)
{
    if (temp.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(temp)) return false;
    if (!removeData(temp, "order_template", "template_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, temp);
    return true;
}

// Update an OrderTemplate.
bool
QuasarDB::update(const OrderTemplate& orig, OrderTemplate& temp)
{
    if (orig.id() == INVALID_ID || temp.id() == INVALID_ID) return false;
    if (!validate(temp)) return false;

    // Update the order_template table
    QString cmd = updateCmd("order_template", "template_id", "name,"
			    "vendor_id");
    Stmt stmt(_connection, cmd);

    updateData(orig, temp, stmt);
    stmtSetString(stmt, temp.name());
    stmtSetId(stmt, temp.vendorId());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");
    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(temp)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup an OrderTemplate.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id template_id, OrderTemplate& temp)
{
    if (template_id == INVALID_ID) return false;
    OrderTemplateSelect conditions;
    vector<OrderTemplate> templates;

    conditions.id = template_id;
    if (!select(templates, conditions)) return false;
    if (templates.size() != 1) return false;

    temp = templates[0];
    return true;
}

// Returns a vector of OrderTemplates.  Returns OrderTemplates sorted by name.
bool
QuasarDB::select(vector<OrderTemplate>& temps,const OrderTemplateSelect& conds)
{
    temps.clear();

    QString cmd = selectCmd("order_template", "template_id", "name,"
			    "vendor_id", conds, "name");
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	OrderTemplate temp;
	int next = 1;
	selectData(temp, stmt, next);
	temp.setName(stmtGetString(stmt, next++));
	temp.setVendorId(stmtGetId(stmt, next++));
	temps.push_back(temp);
    }

    QString cmd1 = "select item_id,number,size_name from "
	"template_item where template_id = ? order by seq_num";
    Stmt stmt1(_connection, cmd1);

    QString cmd2 = "select charge_id,tax_id,amount,internal from "
	"template_charge where template_id = ? order by seq_num";
    Stmt stmt2(_connection, cmd2);

    for (unsigned int i = 0; i < temps.size(); ++i) {
	Id template_id = temps[i].id();

	stmtSetId(stmt1, template_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    TemplateItem line;
	    int next = 1;
	    line.item_id = stmtGetId(stmt1, next++);
	    line.number = stmtGetString(stmt1, next++);
	    line.size = stmtGetString(stmt1, next++);
	    temps[i].items().push_back(line);
	}

	stmtSetId(stmt2, template_id);
	if (!execute(stmt2)) return false;
	while (stmt2.next()) {
	    TemplateCharge line;
	    line.charge_id = stmtGetId(stmt2, 1);
	    line.tax_id = stmtGetId(stmt2, 2);
	    line.amount = stmtGetFixed(stmt2, 3);
	    line.internal = stmtGetBool(stmt2, 4);
	    temps[i].charges().push_back(line);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const OrderTemplate& temp)
{
    if (!validate((DataObject&)temp)) return false;

    if (temp.name().stripWhiteSpace().isEmpty())
	return error("Blank template name");

    if (temp.vendorId() == INVALID_ID)
	return error("Missing vendor");

    return true;
}

bool
QuasarDB::sqlCreateLines(const OrderTemplate& temp)
{
    QString cmd = insertText("template_item", "template_id", "seq_num,"
			     "item_id,number,size_name");
    Stmt stmt(_connection, cmd);

    unsigned int i;
    for (i = 0; i < temp.items().size(); ++i) {
	const TemplateItem& line = temp.items()[i];

	stmtSetId(stmt, temp.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.item_id);
	stmtSetString(stmt, line.number);
	stmtSetString(stmt, line.size);
	if (!execute(stmt)) return false;
    }

    cmd = insertText("template_charge", "template_id", "seq_num,charge_id,"
		     "tax_id,amount,internal");
    stmt.setCommand(cmd);

    for (i = 0; i < temp.charges().size(); ++i) {
	const TemplateCharge& line = temp.charges()[i];

	stmtSetId(stmt, temp.id());
	stmtSetInt(stmt, i);
	stmtSetId(stmt, line.charge_id);
	stmtSetId(stmt, line.tax_id);
	stmtSetFixed(stmt, line.amount);
	stmtSetBool(stmt, line.internal);
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const OrderTemplate& temp)
{
    if (!remove(temp, "template_item", "template_id")) return false;
    if (!remove(temp, "template_charge", "template_id")) return false;
    return true;
}
