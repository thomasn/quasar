// $Id: employee_db.cpp,v 1.28 2005/03/01 19:59:42 bpepers Exp $
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

#include "employee.h"
#include "employee_select.h"

// Create an Employee
bool
QuasarDB::create(Employee& employee)
{
    if (!validate(employee)) return false;
    if (!create((Card&)employee)) return false;

    QString cmd = "insert into card_employee (employee_id,login_name,"
	"passwd,store_id,pos_level) values (?,?,?,?,?)";
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, employee.id());
    stmtSetString(stmt, employee.loginName());
    stmtSetString(stmt, employee.password());
    stmtSetId(stmt, employee.defaultStore());
    stmtSetInt(stmt, employee.posLevel());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Insert, employee);
    return true;
}

// Delete an Employee
bool
QuasarDB::remove(const Employee& employee)
{
    if (employee.id() == INVALID_ID) return false;
    if (!remove((Card&)employee)) return false;

    commit();
    dataSignal(DataEvent::Delete, employee);
    return true;
}

// Update an Employee
bool
QuasarDB::update(const Employee& orig, Employee& employee)
{
    if (orig.id() == INVALID_ID || employee.id() == INVALID_ID) return false;
    if (!validate(employee)) return false;
    if (!update(orig, (Card&)employee)) return false;

    // Update the employee tables
    QString cmd = "update card_employee set login_name=?,"
	"passwd=?,store_id=?,pos_level=? where employee_id=?";
    Stmt stmt(_connection, cmd);

    stmtSetString(stmt, employee.loginName());
    stmtSetString(stmt, employee.password());
    stmtSetId(stmt, employee.defaultStore());
    stmtSetInt(stmt, employee.posLevel());
    stmtSetId(stmt, employee.id());

    if (!execute(stmt)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup an Employee.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id employee_id, Employee& employee)
{
    if (employee_id == INVALID_ID) return false;
    EmployeeSelect conditions;
    vector<Employee> employees;

    conditions.id = employee_id;
    if (!select(employees, conditions)) return false;
    if (employees.size() != 1) return false;

    employee = employees[0];
    return true;
}

// Lookup an Employee by its number
bool
QuasarDB::lookup(const QString& number, Employee& employee)
{
    if (number.isEmpty()) return false;
    EmployeeSelect conditions;
    vector<Employee> employees;

    conditions.number = number;
    if (!select(employees, conditions)) return false;
    if (employees.size() != 1) return false;

    employee = employees[0];
    return true;
}

// Returns a vector of Employees.  Returns employees sorted by name.
bool
QuasarDB::select(vector<Employee>& employees, const EmployeeSelect& conditions)
{
    employees.clear();

    QString table = "card_employee join card on card.card_id = "
	"card_employee.employee_id";

    QString cmd = selectCmd(table, "card_id", "company,first_name,"
			    "last_name,number,street,street2,city,"
			    "province,country,postal,phone_num,phone2_num,"
			    "fax_num,email,web_page,contact,comments,"
			    "patgroup_id,login_name,passwd,store_id,"
			    "pos_level", conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Employee employee;
	int next = 1;
	selectData(employee, stmt, next);
	selectCard(employee, stmt, next);
	employee.setLoginName(stmtGetString(stmt, next++));
	employee.setPassword(stmtGetString(stmt, next++));
	employee.setDefaultStore(stmtGetId(stmt, next++));
	employee.setPosLevel(stmtGetInt(stmt, next++));
	employees.push_back(employee);
    }

    QString cmd1 = "select group_id from card_group where card_id = ? "
	"order by seq_num";
    Stmt stmt1(_connection, cmd1);

    QString cmd2 = "select extra_id,data_value from card_extra "
	"where card_id = ? order by seq_num";
    Stmt stmt2(_connection, cmd2);

    for (unsigned int i = 0; i < employees.size(); ++i) {
	Id card_id = employees[i].id();

	stmtSetId(stmt1, card_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    Id group_id = stmtGetId(stmt1, 1);
	    employees[i].groups().push_back(group_id);
	}

	stmtSetId(stmt2, card_id);
	if (!execute(stmt2)) return false;
	while (stmt2.next()) {
	    Id extra_id = stmtGetId(stmt2, 1);
	    QString value = stmtGetString(stmt2, 2);
	    employees[i].setValue(extra_id, value);
	}
    }

    commit();
    return true;
}

bool
QuasarDB::validate(const Employee& employee)
{
    if (!validate((Card&)employee)) return false;

    return true;
}
