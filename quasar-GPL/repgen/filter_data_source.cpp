// $Id: filter_data_source.cpp,v 1.8 2004/09/20 09:04:49 bpepers Exp $
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

#include "filter_data_source.h"
#include "report_interp.h"
#include "data_source_factory.h"

FilterDataSource::FilterDataSource()
    : filterExpr(""), source(NULL)
{
}

FilterDataSource::~FilterDataSource()
{
    delete source;
}

FilterDataSource*
FilterDataSource::clone() const
{
    FilterDataSource* src = new FilterDataSource();
    src->columns = columns;
    src->filterExpr = filterExpr;
    src->source = source->clone();
    return src;
}

bool
FilterDataSource::fromXML(QDomElement e)
{
    QString typeName = e.attribute("type").lower();
    if (typeName != "filter") return false;

    QDomNodeList nodes = e.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "expr")
	    filterExpr = text;
	else if (tag == "dataSource") {
	    QString type = e.attribute("type");
	    source = DataSourceFactory::create(type);
	    if (source != NULL)
		source->fromXML(e);
	    else
		qWarning("Invalid data source type: " + type);
	} else
	    qWarning("Unknown filter data source tag: " + tag);
    }

    if (source != NULL) columns = source->columns;

    return true;
}

bool
FilterDataSource::toXML(QDomElement&) const
{
    // TODO: convert to XML
    return false;
}

bool
FilterDataSource::initialize(ReportInterp* interp)
{
    if (source == NULL) return true;
    return source->initialize(interp);
}

bool
FilterDataSource::next(ReportInterp* interp)
{
    if (source == NULL) return false;

    // Save column values
    QValueVector<QVariant> saved;
    for (unsigned int i = 0; i < source->columns.size(); ++i) {
	QString column = source->columns[i];
	QVariant value = interp->getVar("column", column);
	saved.push_back(value);
    }

    while (true) {
	if (!source->next(interp)) return false;

	// Set column variables for filter expr
	for (unsigned int i = 0; i < source->columns.size(); ++i) {
	    QString column = source->columns[i];
	    QVariant value = source->get(interp, i);
	    interp->setVar("column", column, value);
	}

	if (interp->evalBoolean(filterExpr, true)) break;
    }

    // Restore column values
    for (unsigned int i = 0; i < source->columns.size(); ++i) {
	QString column = source->columns[i];
	const QVariant& value = saved[i];
	interp->setVar("column", column, value);
    }

    return true;
}

QVariant
FilterDataSource::get(ReportInterp* interp, int column)
{
    if (source == NULL) return "";
    return source->get(interp, column);
}
