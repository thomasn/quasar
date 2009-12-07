// $Id: merge_tr.cpp,v 1.2 2004/02/02 22:42:03 arandell Exp $
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

#include "messages.h"

#include <qapplication.h>
#include <qfileinfo.h>
#include <qdir.h>

int
main(int argc, char** argv)
{
    QApplication app(argc, argv, false);

    // First collect all messages.ts in code subdirs
    Messages messages;
    QStringList subdirs = QDir::current().entryList(QDir::Dirs);
    for (unsigned int i = 0; i < subdirs.count(); ++i) {
	QDir subdir(subdirs[i]);
	if (!subdir.exists("messages.ts")) continue;

	QString filePath = subdir.filePath("messages.ts");
	if (!messages.load(filePath))
	    qWarning("Error: failed loading " + filePath);
    }

    // Save in top level directory
    messages.save("messages.ts");

    // Merge with locale messages
    QDir locales("locales");
    subdirs = locales.entryList(QDir::Dirs);
    for (unsigned int i = 0; i < subdirs.count(); ++i) {
	QDir subdir("locales");
	subdir.cd(subdirs[i]);
	if (!subdir.exists("messages.ts")) continue;

	Messages localeMsg = messages;
	QString filePath = subdir.filePath("messages.ts");
	if (!localeMsg.merge(filePath))
	    qWarning("Error: failed merging " + filePath);
	localeMsg.save(filePath);
    }

    return 0;
}
