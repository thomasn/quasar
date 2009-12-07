// $Id: table_defn.cpp,v 1.9 2004/12/11 01:30:32 bpepers Exp $
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

#include "table_defn.h"

TableDefn::TableDefn()
{
    clear();
}

TableDefn::TableDefn(QDomElement e)
{
    fromXML(e);
}

QStringList
TableDefn::dependencies() const
{
    QStringList depends;

    for (unsigned int i = 0; i < columns.size(); ++i) {
	const ColumnDefn& column = columns[i];
	if (column.type != ColumnDefn::TYPE_ID) continue;
	if (column.table.isEmpty()) continue;
	if (depends.contains(column.table)) continue;
	depends.push_back(column.table);
    }

    if (primaryKey.size() > 0)
	depends.push_back(name);

    for (unsigned int i = 0; i < constraints.size(); ++i) {
	const ConstraintDefn& constraint = constraints[i];
	if (constraint.type != ConstraintDefn::TYPE_FOREIGN_KEY) continue;
	if (depends.contains(constraint.referenceTable)) continue;
	depends.push_back(constraint.referenceTable);
    }

    return depends;
}

void
TableDefn::addStringColumn(const QString& name, int length, bool manditory,
			   bool unique)
{
    ColumnDefn column;
    column.name = name;
    column.type = ColumnDefn::TYPE_STRING;
    column.size = length;
    column.manditory = manditory;
    column.unique = unique;
    columns.push_back(column);
}

void
TableDefn::addDateColumn(const QString& name, bool manditory)
{
    ColumnDefn column;
    column.name = name;
    column.type = ColumnDefn::TYPE_DATE;
    column.manditory = manditory;
    columns.push_back(column);
}

void
TableDefn::addTimeColumn(const QString& name, bool manditory)
{
    ColumnDefn column;
    column.name = name;
    column.type = ColumnDefn::TYPE_TIME;
    column.manditory = manditory;
    columns.push_back(column);
}

void
TableDefn::addBooleanColumn(const QString& name)
{
    ColumnDefn column;
    column.name = name;
    column.type = ColumnDefn::TYPE_BOOL;
    columns.push_back(column);
}

void
TableDefn::addIntegerColumn(const QString& name)
{
    ColumnDefn column;
    column.name = name;
    column.type = ColumnDefn::TYPE_INT;
    columns.push_back(column);
}

void
TableDefn::addBigIntColumn(const QString& name, bool manditory)
{
    ColumnDefn column;
    column.name = name;
    column.type = ColumnDefn::TYPE_BIGINT;
    column.manditory = manditory;
    columns.push_back(column);
}

void
TableDefn::addNumericColumn(const QString& name)
{
    ColumnDefn column;
    column.name = name;
    column.type = ColumnDefn::TYPE_NUMERIC;
    columns.push_back(column);
}

void
TableDefn::addMoneyColumn(const QString& name)
{
    ColumnDefn column;
    column.name = name;
    column.type = ColumnDefn::TYPE_MONEY;
    columns.push_back(column);
}

void
TableDefn::addPercentColumn(const QString& name)
{
    ColumnDefn column;
    column.name = name;
    column.type = ColumnDefn::TYPE_PERCENT;
    columns.push_back(column);
}

void
TableDefn::addQuantityColumn(const QString& name)
{
    ColumnDefn column;
    column.name = name;
    column.type = ColumnDefn::TYPE_QUANTITY;
    columns.push_back(column);
}

void
TableDefn::addIdColumn(const QString& name, const QString& table,
		       bool manditory)
{
    ColumnDefn column;
    column.name = name;
    column.type = ColumnDefn::TYPE_ID;
    column.manditory = manditory;
    column.table = table;
    columns.push_back(column);
}

void
TableDefn::addPrimaryKey(const QString& columns)
{
    primaryKey = QStringList::split(",", columns);
}

void
TableDefn::addUnique(const QString& columns)
{
    ConstraintDefn constraint;
    constraint.name = name + "_c" + QString::number(constraints.size());
    constraint.table = name;
    constraint.type = ConstraintDefn::TYPE_UNIQUE;
    constraint.columns = QStringList::split(",", columns);
    constraints.push_back(constraint);
}

void
TableDefn::addCheck(const QString& check)
{
    ConstraintDefn constraint;
    constraint.name = name + "_c" + QString::number(constraints.size());
    constraint.table = name;
    constraint.type = ConstraintDefn::TYPE_CHECK;
    constraint.check = check;
    constraints.push_back(constraint);
}

void
TableDefn::addReference(const QString& columns, const QString& referenceTable,
			const QString& referenceColumns, int flags)
{
    ConstraintDefn constraint;
    constraint.name = name + "_c" + QString::number(constraints.size());
    constraint.table = name;
    constraint.type = ConstraintDefn::TYPE_FOREIGN_KEY;
    constraint.columns = QStringList::split(",", columns);
    constraint.referenceTable = referenceTable;
    constraint.referenceColumns = QStringList::split(",", referenceColumns);
    constraint.referenceFlags = flags;
    constraints.push_back(constraint);
}

void
TableDefn::addReference(const QString& columns, const QString& referenceTable,
			int flags)
{
    addReference(columns, referenceTable, columns, flags);
}

void
TableDefn::addIndex(const QString& columns, bool unique)
{
    QStringList names = QStringList::split(",", columns);

    IndexDefn index;
    index.name = name + "_i" + QString::number(indexes.size());
    index.table = name;
    for (unsigned int i = 0; i < names.size(); ++i)
	index.addColumn(names[i]);
    index.unique = unique;
    indexes.push_back(index);
}

void
TableDefn::fromXML(QDomElement e)
{
    clear();

    QDomNodeList nodes = e.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString content = e.text();

	if (tag == "name")
	    name = content;
	else if (tag == "description")
	    description = content;
	else if (tag == "column")
	    columns.push_back(ColumnDefn(e));
	else if (tag == "primaryKey")
	    primaryKey = QStringList::split(",", content);
	else if (tag == "constraint")
	    constraints.push_back(ConstraintDefn(e));
	else if (tag == "index")
	    indexes.push_back(IndexDefn(e));
	else
	    qWarning("Unknown table tag: " + tag);
    }
}

void
TableDefn::toXML(QDomElement& e) const
{
    QDomDocument doc = e.ownerDocument();

    QDomElement n = doc.createElement("name");
    n.appendChild(doc.createTextNode(name));
    e.appendChild(n);

    QDomElement d = doc.createElement("description");
    d.appendChild(doc.createTextNode(description));
    e.appendChild(d);

    for (unsigned int i = 0; i < columns.size(); ++i) {
	QDomElement t = doc.createElement("column");
	columns[i].toXML(t);
	e.appendChild(t);
    }

    QDomElement p = doc.createElement("primaryKey");
    p.appendChild(doc.createTextNode(primaryKey.join(",")));
    e.appendChild(p);

    for (unsigned int i = 0; i < constraints.size(); ++i) {
	QDomElement t = doc.createElement("constraint");
	constraints[i].toXML(t);
	e.appendChild(t);
    }

    for (unsigned int i = 0; i < indexes.size(); ++i) {
	QDomElement t = doc.createElement("index");
	indexes[i].toXML(t);
	e.appendChild(t);
    }
}

void
TableDefn::clear()
{
    name = "";
    description = "";
    columns.clear();
    primaryKey.clear();
    constraints.clear();
    indexes.clear();
}

bool
TableDefn::operator==(const TableDefn& rhs) const
{
    if (rhs.name != name) return false;
    if (rhs.description != description) return false;
    if (rhs.columns != columns) return false;
    if (rhs.primaryKey != primaryKey) return false;
    if (rhs.constraints != constraints) return false;
    if (rhs.indexes != indexes) return false;
    return true;
}

bool
TableDefn::operator!=(const TableDefn& rhs) const
{
    return !(*this == rhs);
}
