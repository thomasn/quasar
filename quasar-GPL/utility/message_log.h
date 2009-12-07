// $Id: message_log.h,v 1.4 2004/08/02 00:31:35 bpepers Exp $
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

#ifndef MESSAGE_LOG_H
#define MESSAGE_LOG_H

#include <qstring.h>

// Log message types
enum { LOG_MSG_STATUS, LOG_MSG_ERROR, LOG_MSG_WARNING, LOG_MSG_DEBUG };

// Initialize the logging code
extern void logInitialize(const QString& filePath, bool daemon=false);

// Log a message of a given type
extern void logMessage(int type, const char* text, ...);

// Shortcuts for specific types of message
extern void logStatus(const char* text, ...);
extern void logError(const char* text, ...);
extern void logWarning(const char* text, ...);
extern void logDebug(const char* text, ...);

// Get the current log filename
extern QString logFileName();

#endif // MESSAGE_LOG_H
