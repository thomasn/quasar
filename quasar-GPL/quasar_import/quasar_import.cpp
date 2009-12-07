// $Id: quasar_import.cpp,v 1.8 2004/06/30 21:35:40 bpepers Exp $
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
#include <qapplication.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <stdlib.h>

void
usage()
{
    qWarning("Usage: quasar_import [options]");

    qWarning("    Options:");
    qWarning("    -h               Show this usage message");
    qWarning("    -company <file>  Full file path to company definition");
    qWarning("    -file <file>     Path of file to import");
    qWarning("    -dir <dir>       Path of dir containing file to import");
    exit(1);
}

int
main(int argc, char** argv)
{
    // Setup for Qt
    QApplication app(argc, argv, false);
    if (app.argc() == 1) usage();

    // Process command line args
    QString companyFile, importFile, importDir;
    for (int i = 1; i < app.argc(); i++) {
	QString cmd = app.argv()[i];
	if (cmd.lower() == "-h") usage();
	else if (cmd.lower() == "-company") companyFile = app.argv()[++i];
	else if (cmd.lower() == "-file") importFile = app.argv()[++i];
	else if (cmd.lower() == "-dir") importDir = app.argv()[++i];
	else {
	    qWarning("Error: unknown command line option: " + cmd);
	    usage();
	}
    }

    // Test company file is set and exists
    if (companyFile.isEmpty()) usage();
    if (!QFileInfo(companyFile).exists())
	qFatal("Error: company file '" + companyFile + "' doesn't exist");
    if (!QFileInfo(companyFile).isFile())
	qFatal("Error: company file '" + companyFile + "' isn't a file");

    // Test import file is set and exists
    if (importFile.isEmpty() && importDir.isEmpty()) usage();
    if (!importFile.isEmpty()) {
	if (!QFileInfo(importFile).exists())
	    qFatal("Error: import file '" + importFile + "' doesn't exist");
	if (!QFileInfo(importFile).isFile())
	    qFatal("Error: import file '" + importFile + "' isn't a file");
    }
    if (!importDir.isEmpty()) {
	if (!QFileInfo(importDir).exists())
	    qFatal("Error: import dir '" + importDir + "' doesn't exist");
	if (!QFileInfo(importDir).isDir())
	    qFatal("Error: import dir '" + importDir + "' isn't a dir");
    }

    // Load company definition
    CompanyDefn company;
    if (!company.load(companyFile))
	qFatal("Error: failed loading company '" + companyFile + "'");

    // Get list of files to import
    QStringList files;
    if (!importFile.isEmpty()) {
	files.push_back(importFile);
    } else {
	QDir dir(importDir);
	QStringList entries = dir.entryList();
	for (unsigned int i = 0; i < entries.size(); ++i) {
	    QString filePath = dir.filePath(entries[i]);
	    if (filePath.right(4) != ".xml") continue;
	    files.push_back(filePath);
	}
    }

    // Import files
    Importer import(company);
    for (unsigned int i = 0; i < files.size(); ++i) {
	QString filePath = files[i];
	import.processFile(filePath);
	printf("Import of '" + filePath + "' completed\n");
    }

    return 0;
}
