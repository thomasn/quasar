// $Id: quasar_clientd.cpp,v 1.8 2005/03/01 19:47:04 bpepers Exp $
//
// Copyright (C) 1998-2004 Linux Canada Inc.  All rights reserved.
//
// This file is part of Quasar Accounting
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

#include "quasar_misc.h"
#include "server_config.h"
#include "command_processor.h"
#include "message_log.h"
#include "tcl_object.h"

#include <qapplication.h>
#include <qfile.h>
#include <qdir.h>
#include <qsocketdevice.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

void
handleSignal(int signum)
{
    signal(signum, handleSignal);
    logStatus("Exit on signal %d", signum);
    exit(3);
}

void
send(QSocketDevice& socket, const QString& type, const QStringList& args,
     bool debug)
{
    TclObject list;
    list.lappend(type);
    for (unsigned int i = 0; i < args.size(); ++i)
	list.lappend(args[i]);

    if (debug) logDebug("send: " + list.toString());

    QString message = list.toString();
    if (message.contains('\n'))
	message = "\002" + message + "\003";
    else
	message += "\n";

    QCString data = message.utf8();
    int len = data.length();
    int pos = 0;
    while (pos < len) {
	int count = socket.writeBlock(data.data() + pos, len - pos);
	socket.flush();

	if (count == -1) {
	    logError("writeBlock error: %d", errno);
	    exit(4);
	}

	pos += count;
    }
}

void
mainLoop(QSocketDevice& socket, bool debug)
{
    CommandProcessor processor;
    QString data;

    bool done = false;
    while (!done) {
	int timeout = -1;
	if (processor.waitingForResults())
	    timeout = 50;
	// TODO: different timeout if using POS and not ping

	bool wasTimeout = false;
	int result = socket.waitForMore(timeout, &wasTimeout);
	if (socket.atEnd() && result == 0 && !wasTimeout)
	    break;

	if (result == -1) {
	    logError("waitForMore error: %d", errno);
	    break;
	}

	if (processor.waitingForResults() && processor.resultsAvailable()) {
	    QString type;
	    QStringList results;
	    processor.getResults(type, results);
	    send(socket, type, results, debug);
	}

	if (wasTimeout) continue;

	char buffer[result + 1];
	result = socket.readBlock(buffer, result);
	if (result == -1) {
	    logError("readBlock error: %d", errno);
	    break;
	}

	buffer[result] = 0;
	data += QString::fromUtf8(buffer);

	while (true) {
	    QString line;
	    if (data.left(1) == "\002") {
		int index = data.find('\003');
		if (index == -1) break;

		line = data.mid(1, index - 1);
		data = data.mid(index + 1);
	    } else {
		int index = data.find('\n');
		if (index == -1) break;

		line = data.left(index);
		data = data.mid(index + 1);

		while (line.right(1) == "\r")
		    line = line.left(line.length() - 1);
	    }

	    if (line.isEmpty()) continue;
	    if (debug) logDebug("recv: " + line);

	    if (line == "ping") {
		send(socket, "ping:", "pong", debug);
	    } else if (line == "quit" || line == "exit") {
		done = true;
	    } else {
		processor.processCommand(line);
		if (processor.resultsAvailable()) {
		    QString type;
		    QStringList results;
		    processor.getResults(type, results);
		    send(socket, type, results, debug);
		}
	    }
	}
    }
}

int
main(int argc, char** argv)
{
    bool debug = false;
    QString logFile;

    // Parse command line
    for (int i = 1; i < argc; ++i) {
        QString arg(argv[i]);

	if (arg == "-debug")
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

    // Use fd 10 for socket
    if (dup2(0, 10) != 10) exit(2);
    for (int fd = 0; fd < 10; ++fd)
	close(fd);
    QSocketDevice socket(10, QSocketDevice::Stream);

    // Decide on a log filename
    QString fileName = socket.peerAddress().toString() + ".log";
    if (logFile.isEmpty()) {
	QFile file("/var/log/quasar/" + fileName);
	if (file.open(IO_WriteOnly | IO_Append))
	    logFile = file.name();
    }
    if (logFile.isEmpty()) {
	QFile file(programDir() + "/../log/" + fileName);
	if (file.open(IO_WriteOnly | IO_Append))
	    logFile = file.name();
    }
    if (logFile.isEmpty()) {
	QFile file(programDir() + "/../" + fileName);
	if (file.open(IO_WriteOnly | IO_Append))
	    logFile = file.name();
	else
	    logFile = "/tmp/" + fileName;
    }

    // Initialize logging
    logInitialize(logFile);
    logStatus("Connection started (pid %d)", getpid());
    dup2(0, 1);
    dup2(0, 2);

    // Signal handling
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, handleSignal);
    signal(SIGQUIT, handleSignal);
    signal(SIGTERM, handleSignal);

    QApplication app(argc, argv, false);

    // Command processing loop
    mainLoop(socket, debug);

    logStatus("Connection closed (pid %d)", getpid());
    return 0;
}
