// $Id: shared_state.h,v 1.1 2004/12/21 08:55:04 bpepers Exp $
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

#ifndef SHARED_STATE_H
#define SHARED_STATE_H

#include <unistd.h>
#include <qstringlist.h>

class SharedState
{
public:
    SharedState();
    ~SharedState();

    // Lock access to the shared memory
    bool lock();

    // Get/set the current quasar_posd pid
    pid_t getPosPid();
    void setPosPid(pid_t pid);

    // Get the list of clients and companies
    bool getClients(QStringList& clients, QStringList& companies);

    // Add a client to the list
    bool addClient(pid_t pid, const QString& addr, const QString& company);

    // Remove a client from the list
    bool removeClient(pid_t pid);

    // Unlock access when done
    bool release();

protected:
    bool initialize();

    int _semid;
    void* _memory;
};

#endif // SHARED_STATE_H
