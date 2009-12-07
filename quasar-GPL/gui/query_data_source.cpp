// $Id: query_data_source.cpp,v 1.7 2005/03/01 20:08:24 bpepers Exp $
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

#include "query_data_source.h"
#include "report_interp.h"
#include "db_driver.h"

Connection* QueryDataSource::connection;

QueryDataSource::QueryDataSource()
    : query(""), queryExpr(""), queryCode(""), stmt(NULL)
{
}

QueryDataSource::~QueryDataSource()
{
    delete stmt;
}

QueryDataSource*
QueryDataSource::clone() const
{
    QueryDataSource* src = new QueryDataSource();
    src->columns = columns;
    src->query = query;
    src->queryExpr = queryExpr;
    src->queryCode = queryCode;
    src->stmt = NULL;
    return src;
}

bool
QueryDataSource::fromXML(QDomElement e)
{
    QString typeName = e.attribute("type").lower();
    if (typeName != "query") return false;

    QDomNodeList nodes = e.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "text" || tag == "query")
	    query = text;
	else if (tag == "expr" || tag == "queryExpr")
	    queryExpr = text;
	else if (tag == "code")
	    queryCode = text;
	else if (tag == "column")
	    columns.push_back(text);
	else
	    qWarning("Unknown query data source tag: " + tag);
    }

    return true;
}

bool
QueryDataSource::toXML(QDomElement&) const
{
    // TODO: convert to XML
    return false;
}

bool
QueryDataSource::initialize(ReportInterp* interp)
{
    delete stmt;
    stmt = connection->allocStatement();

    QString command = query;
    if (command.isEmpty() && !queryExpr.isEmpty())
	command = interp->evaluate(queryExpr).toString();
    if (command.isEmpty() && !queryCode.isEmpty())
	command = interp->execute(queryCode).toString();
    if (command.isEmpty())
	return false;

    stmt->setCommand(command);
    if (!stmt->execute()) {
	qWarning("Error: " + stmt->lastError());
	return false;
    }

    return true;
}

bool
QueryDataSource::next(ReportInterp*)
{
    if (stmt == NULL) return false;
    return stmt->next();
}

QVariant
QueryDataSource::get(ReportInterp*, int column)
{
    if (stmt == NULL) return "";
    // TODO: this should return a proper QVariant
    return stmt->getString(column + 1);
}

DataSource*
createQueryDataSource()
{
    return new QueryDataSource();
}
