// $Id: quasar_setup.cpp,v 1.12 2005/04/07 18:41:03 bpepers Exp $
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

#include "setup_main_window.h"
#include "server_config.h"

#include <qapplication.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char** argv)
{
    // Setup for Qt
    QApplication app(argc, argv);

    // Parse command line
    for (int i = 1; i < app.argc(); ++i) {
        QString arg(app.argv()[i]);

	if (arg == "-configDir") {
	    if (i + 1 == app.argc())
		qFatal("Error: missing -configDir argument");
	    ConfigFile::setDefaultConfigDir(app.argv()[++i]);
	} else if (arg == "-help") {
	    printf("Usage: quasar_setup [-configDir dir]\n");
	    exit(0);
	} else {
	    qFatal("Error: unknown command line argument: " + arg);
	}
    }

    // Create main window
    SetupMainWindow* main = new SetupMainWindow();
    main->show();
    qApp->setMainWidget(main);

    return app.exec();
}
