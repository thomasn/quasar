// $Id: quasard.cpp,v 1.63 2004/12/21 09:03:47 bpepers Exp $
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

#include "quasar_server.h"
#include "quasar_misc.h"
#include "server_config.h"
#include "message_log.h"

#include <qapplication.h>
#include <qfile.h>
#include <qdir.h>

#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int
main(int argc, char** argv)
{
    int port = 0;
    bool daemon = false;
    bool debug = false;
    QString logFile;

    // Parse command line
    for (int i = 1; i < argc; ++i) {
        QString arg(argv[i]);

	if (arg == "-port")
	    port = QString(argv[++i]).toInt();
	else if (arg == "-daemon")
	    daemon = true;
	else if (arg == "-debug")
	    debug = true;
	else if (arg == "-log")
	    logFile = argv[++i];
	else if (arg == "-configDir") {
	    if (i + 1 == argc) qFatal("Error: missing -configDir argument");
	    ConfigFile::setDefaultConfigDir(argv[++i]);
	} else {
	    qFatal("Error: unknown command line argument: " + arg);
	}
    }

    // Get default port
    if (port == 0) {
	ServerConfig config;
	if (config.load())
	    port = config.port;
    }

    // Decide on a log filename
    if (logFile.isEmpty()) {
	QFile file("/var/log/quasar/quasard.log");
	if (file.open(IO_WriteOnly | IO_Append))
	    logFile = file.name();
    }
    if (logFile.isEmpty()) {
	QFile file(programDir() + "/../log/quasard.log");
	if (file.open(IO_WriteOnly | IO_Append))
	    logFile = file.name();
    }
    if (logFile.isEmpty()) {
	QFile file(programDir() + "/../quasard.log");
	if (file.open(IO_WriteOnly | IO_Append))
	    logFile = file.name();
	else
	    logFile = "/tmp/quasard.log";
    }

    // Become a daemon if requested
    if (daemon) {
	pid_t pid = fork();
	if (pid < 0)
	    return -1;
	if (pid != 0)
	    exit(0);

	// Detach from controlling tty
	int tty = open("/dev/tty", O_RDWR);
	if (tty >= 0) {
	    ioctl(tty, TIOCNOTTY, (char*)0);
	    close(tty);
	}

	// Close all files
	for (int i = 0; i < 255; ++i)
	    close(i);

	// Other cleanup
	setpgrp();
	setsid();
	chdir("/");
	umask(0);
    }

    // Initialize logging
    logInitialize(logFile, daemon);

    QApplication app(argc, argv, false);

    QuasarServer* server = new QuasarServer(port, debug);
    if (!server->ok()) {
	logError("Can't start server");
	exit(1);
    }

    int result = qApp->exec();
    delete server;

    return result;
}
