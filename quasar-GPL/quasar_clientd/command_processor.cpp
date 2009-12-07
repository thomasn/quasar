// $Id: command_processor.cpp,v 1.5 2005/06/07 01:15:09 bpepers Exp $
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

#include "command_processor.h"

CommandProcessor::CommandProcessor()
{
    _worker.start();
    _waiting = false;
}

CommandProcessor::~CommandProcessor()
{
    _worker.shutdown();
}

void
CommandProcessor::processCommand(const QString& line)
{
    if (_waiting) qWarning("New command when already processing one");

    TclObject list(line);
    if (list.llength() == 0) return;

    _command = list[0].toString();
    _args.clear();
    for (int i = 1; i < list.llength(); ++i)
	_args << list[i].toString();

    if (_command == "version")		fastCommand();
    else if (_command == "system")	fastCommand();
    else if (_command == "system_id")	fastCommand();
    else if (_command == "resources")	slowCommand();
    else if (_command == "checksum")	slowCommand();
    else if (_command == "sendfile")	slowCommand();
    else if (_command == "validate")	slowCommand();
    else if (_command == "password")	slowCommand();
    else if (_command == "add_user")	slowCommand();
    else if (_command == "update_user")	slowCommand();
    else if (_command == "delete_user")	slowCommand();
    else if (_command == "set")		fastCommand();
    else if (_command == "commit")	fastCommand();
    else if (_command == "rollback")	fastCommand();
    else if (_command == "exec_cmd")	fastCommand();
    else if (_command == "alloc_stmt")	fastCommand();
    else if (_command == "free_stmt")	fastCommand();
    else if (_command == "set_stmt")	fastCommand();
    else if (_command == "execute")	fastCommand();
    else if (_command == "next")	fastCommand();
    else slowCommand();
}

bool
CommandProcessor::waitingForResults()
{
    return _waiting;
}

bool
CommandProcessor::resultsAvailable()
{
    return _worker.resultsAvailable();
}

void
CommandProcessor::getResults(QString& type, QStringList& results)
{
    _worker.getResults(type, results);
    _waiting = false;
}

void
CommandProcessor::slowCommand()
{
    _worker.queueCommand(_command, _args);
    _waiting = true;
}

void
CommandProcessor::fastCommand()
{
    _worker.processCommand(_command, _args);
    _waiting = true;
}

