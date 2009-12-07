// $Id: quasar_server.h,v 1.45 2004/12/19 19:26:01 bpepers Exp $
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

#ifndef QUASAR_SERVER_H
#define QUASAR_SERVER_H

#include <qserversocket.h>
#include <qvaluelist.h>
#include <qmutex.h>
#include <unistd.h>

class QuasarClient;
class POS_Poster;

class QuasarServer: public QServerSocket {
    Q_OBJECT
public:
    QuasarServer(int port, bool debug);
    ~QuasarServer();

    // Accept new connection
    void newConnection(int socket);

protected slots:
    void slotCheckClients();

protected:
    int _port;				// Port for server socket
    bool _debug;			// Debugging?
    QSocketDevice* _socket;		// Server socket
    QValueList<pid_t> _clients;		// Clients
};

#endif
