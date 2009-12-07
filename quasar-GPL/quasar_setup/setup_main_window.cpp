// $Id: setup_main_window.cpp,v 1.26 2005/03/03 22:09:50 bpepers Exp $
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
#include "company_config_tab.h"
#include "driver_config_tab.h"
#include "server_config_tab.h"
#include "client_config_tab.h"

#include <qapplication.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <stdlib.h>

SetupMainWindow::SetupMainWindow()
    : QMainWindow(0, "SetupMainWindow", WType_TopLevel | WDestructiveClose)
{
    setCaption(tr("Quasar Setup"));
    menuBar();

    QPopupMenu* file = new QPopupMenu(this);
    file->insertItem(tr("E&xit"), this, SLOT(close()), ALT+Key_Q);
    menuBar()->insertItem(tr("&File"), file);

    _tabs = new QTabWidget(this);
    setCentralWidget(_tabs);

    ServerConfig serverConfig;
    if (serverConfig.load(false)) {
	_tabs->addTab(new CompanyConfigTab(_tabs), tr("Companies"));
	_tabs->addTab(new DriverConfigTab(_tabs), tr("Drivers"));
	_tabs->addTab(new ServerConfigTab(_tabs), tr("Server"));
    }

    ClientConfig clientConfig;
    if (clientConfig.load(false)) {
	_tabs->addTab(new ClientConfigTab(_tabs), tr("Client"));
    }

    if (_tabs->count() == 0) {
	QString message = "You do not have permission to setup Quasar. You\n"
	    "must run quasar_setup with permission to read and\n"
	    "write the configuration files.";
	QMessageBox::critical(NULL, "Error", message);
	exit(1);
    }

    connect(_tabs, SIGNAL(currentChanged(QWidget*)), SLOT(slotTabChange()));
}

SetupMainWindow::~SetupMainWindow()
{
    slotTabChange();
}

void
SetupMainWindow::slotTabChange()
{
    static ConfigTab* lastTab = NULL;
    if (lastTab != NULL)
	lastTab->saveChanges();

    ConfigTab* tab = (ConfigTab*)_tabs->currentPage();
    tab->loadData();

    lastTab = tab;
}
