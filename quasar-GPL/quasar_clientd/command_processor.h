// $Id: command_processor.h,v 1.1 2004/12/19 08:10:47 bpepers Exp $
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

#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include "worker_thread.h"

class CommandProcessor {
public:
    CommandProcessor();
    ~CommandProcessor();

    // Process a command line
    void processCommand(const QString& line);

    // Status of results
    bool waitingForResults();
    bool resultsAvailable();

    // Return the results
    void getResults(QString& type, QStringList& results);

protected:
    // Slow commands are ones that can take a while to run so we will
    // queue the command to the worker thread and then poll for results
    void slowCommand();

    // Fast commands are ones that will always return results very
    // quickly so we just use the worker class to get the results
    void fastCommand();

    WorkerThread _worker;	// Worker thread handling most commands
    QString _command;		// Current command worker is processing
    QStringList _args;		// Current command arguments
    bool _waiting;		// Waiting for results?
};

#endif
