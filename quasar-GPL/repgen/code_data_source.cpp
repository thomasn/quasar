// $Id: code_data_source.cpp,v 1.6 2004/10/21 03:03:14 bpepers Exp $
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

#include "code_data_source.h"
#include "report_interp.h"

CodeDataSource::CodeDataSource()
    : initCode(""), nextCode(""), getCode("")
{
}

CodeDataSource::~CodeDataSource()
{
}

CodeDataSource*
CodeDataSource::clone() const
{
    CodeDataSource* src = new CodeDataSource();
    src->columns = columns;
    src->initCode = initCode;
    src->nextCode = nextCode;
    src->getCode = getCode;
    return src;
}

bool
CodeDataSource::fromXML(QDomElement e)
{
    QString typeName = e.attribute("type").lower();
    if (typeName != "code") return false;

    QDomNodeList nodes = e.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "initialize")
	    initCode = text;
	else if (tag == "next")
	    nextCode = text;
	else if (tag == "get")
	    getCode = text;
	else if (tag == "column")
	    columns.push_back(text);
	else
	    qWarning("Unknown code data source tag: " + tag);
    }

    return true;
}

bool
CodeDataSource::toXML(QDomElement&) const
{
    // TODO: convert to XML
    return false;
}

bool
CodeDataSource::initialize(ReportInterp* interp)
{
    currentRow = 0;
    interp->execute(initCode);
    return true;
}

bool
CodeDataSource::next(ReportInterp* interp)
{
    interp->setVar("current_row", QString::number(currentRow++));
    return interp->evalBoolean(nextCode);
}

QVariant
CodeDataSource::get(ReportInterp* interp, int column)
{
    interp->setVar("index", QString::number(column));
    return interp->execute(getCode);
}
