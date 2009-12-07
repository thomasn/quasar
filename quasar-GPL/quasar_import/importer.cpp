// $Id: importer.cpp,v 1.4 2004/04/15 07:25:33 bpepers Exp $
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

#include "importer.h"
#include "data_import.h"

Importer::Importer(const CompanyDefn& company)
    : QObject(NULL, "Importer")
{
    _import = new DataImport(company);
    connect(_import, SIGNAL(message(int,QString,QString)),
	    SLOT(slotMessage(int,QString,QString)));
}

Importer::~Importer()
{
    delete _import;
}

void
Importer::processFile(const QString& filePath)
{
    _errors.clear();
    _warnings.clear();
    _infos.clear();

    printf("Importing: " + filePath + "\n");
    _import->processFile(filePath);

    if (_errors.size() > 0)
	printf("    %d errors\n", _errors.size());
    if (_warnings.size() > 0)
	printf("    %d warnings\n", _warnings.size());
    printf("    %d info msgs\n", _infos.size());

    if (_errors.size() > 0) {
	printf("\n    Errors:\n");
	for (unsigned int i = 0; i < _errors.size(); ++i) {
	    const ImportMsg& msg = _errors[i];
	    printf("        " + msg.type + "," + msg.name + ": " +
		   msg.text + "\n");
	}
    }

    if (_warnings.size() > 0) {
	printf("\n    Warnings:\n");
	for (unsigned int i = 0; i < _warnings.size(); ++i) {
	    const ImportMsg& msg = _warnings[i];
	    printf("        " + msg.type + "," + msg.name + ": " +
		   msg.text + "\n");
	}
    }

    if (_infos.size() > 0) {
	printf("\n    Information:\n");
	for (unsigned int i = 0; i < _infos.size(); ++i) {
	    const ImportMsg& msg = _infos[i];
	    printf("        " + msg.type + "," + msg.name + ": " +
		   msg.text + "\n");
	}
    }
}

void
Importer::slotMessage(int, QString severity, QString message)
{
    ImportMsg msg;
    msg.type = _import->currentType();
    msg.name = _import->currentName();
    msg.text = message;

    if (severity == "Error") _errors.push_back(msg);
    else if (severity == "Warning") _warnings.push_back(msg);
    else _infos.push_back(msg);
}
