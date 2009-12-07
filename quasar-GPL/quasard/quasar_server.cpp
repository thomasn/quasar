// $Id: quasar_server.cpp,v 1.85 2004/12/19 19:26:01 bpepers Exp $
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
#include <qsocketdevice.h>
#include <qtimer.h>
#include <qvaluevector.h>

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>

void
handleSignal(int signum)
{
    signal(signum, handleSignal);
    logStatus("Exit on signal %d", signum);
    qApp->exit(1);
}

QuasarServer::QuasarServer(int port, bool debug)
    : QServerSocket(), _port(port), _debug(debug), _socket(NULL)
{
    // Setup server socket
    _socket = new QSocketDevice();
    if (_socket == NULL) {
	logError("Failed creating QSocketDevice");
	return;
    }
    _socket->setAddressReusable(true);
    _socket->setBlocking(false);

    // Get server port
    if (_port == 0) {
	struct servent* ent = getservbyname("quasar-server", "tcp");
	if (ent == NULL) ent = getservbyname("quasar", "tcp");
	_port = (ent == NULL) ? DEFAULT_QUASARD_PORT : ntohs(ent->s_port);
    }

    // Bind to address
    QHostAddress addr;
    if (!_socket->bind(addr, _port)) {
	logError("Failed binding QSocketDevice");
	return;
    }

    // Setup socket
    if (!_socket->listen(10)) {
	logError("Failed calling listen");
	return;
    }
    setSocket(_socket->socket());

    // Signal handling
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, handleSignal);
    signal(SIGQUIT, handleSignal);
    signal(SIGTERM, handleSignal);

    QTimer::singleShot(2000, this, SLOT(slotCheckClients()));
    logStatus("Quasar server started on port %d", _port);
}

QuasarServer::~QuasarServer()
{
    for (unsigned int i = 0; i < _clients.size(); ++i)
	kill(_clients[i], 15);
    delete _socket;

    logStatus("Quasar server on port %d stopped", _port);
}

void
QuasarServer::newConnection(int fd)
{
    pid_t pid = fork();
    if (pid == -1) {
	logError("Error: fork failed");
	close(fd);
	return;
    }

    // Parent
    if (pid != 0) {
	_clients.push_back(pid);
	logStatus("Quasar client started (pid %d)", pid);
	close(fd);
	return;
    }

    // Duplicate fd for stdin and stdout
    dup2(fd, 0);
    dup2(fd, 1);

    // Close other files
    for (int i = 3; i < 255; ++i)
	close(i);

    // Detach from controlling tty
    int tty = open("/dev/tty", O_RDWR);
    if (tty >= 0) {
	ioctl(tty, TIOCNOTTY, (char*)0);
	close(tty);
    }

    // Other cleanup
    chdir("/");
    setpgrp();

    // Build command line args
    QValueVector<QString> args;
    args.push_back(programDir() + "/quasar_clientd");
    if (_debug) args.push_back("-debug");

    // Run client daemon
    char* argv[args.size() + 1];
    for (unsigned int i = 0; i < args.size(); ++i)
	argv[i] = strdup(args[i]);
    argv[args.size()] = NULL;
    execv(args[0], argv);

    // Should never get here
    qWarning("Error: execv failed");
}

void
QuasarServer::slotCheckClients()
{
    while (true) {
	int status = 0;
	pid_t pid = waitpid(-1, &status, WNOHANG);
	if (pid == 0) break;
	if (pid == -1 && errno == ECHILD) break;

	if (pid == -1) {
	    logError("Error from waitpid: %d", errno);
	    continue;
	}

	for (unsigned int i = 0; i < _clients.size(); ++i) {
	    if (_clients[i] == pid) {
		if (WIFEXITED(status)) {
		    int result = WEXITSTATUS(status);
		    if (result == 0)
			logStatus("Quasar client stopped (pid %d)", pid);
		    else
			logStatus("Quasar client error (pid %d, result %d)",
				  pid, result);
		} else if (WIFSIGNALED(status)) {
		    logStatus("Quasar client killed (pid %d, signal %d)",
			      pid, WTERMSIG(status));
		} else {
		    logStatus("Quasar client unknown exit (pid %d, status %d)",
			      pid, status);
		}

		_clients.remove(_clients[i]);
		break;
	    }
	}
    }
    QTimer::singleShot(2000, this, SLOT(slotCheckClients()));
}
