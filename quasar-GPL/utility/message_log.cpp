// $Id: message_log.cpp,v 1.7 2004/08/10 20:44:46 bpepers Exp $
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

#include "message_log.h"

#include <qfile.h>
#include <qdatetime.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef LINUX
#include <sys/file.h>
#endif

#ifdef WIN32
#define vsnprintf _vsnprintf
#endif

static QFile logFile;
static bool isDaemon;

static void
qtMessageHandler(QtMsgType type, const char* msg)
{
    switch (type) {
    case QtDebugMsg:
	logDebug(msg);
	break;
    case QtWarningMsg:
	logWarning(msg);
	break;
    case QtFatalMsg:
	logError(msg);
	abort();
    }
}

void
logInitialize(const QString& filePath, bool daemon)
{
    isDaemon = daemon;
    logFile.setName(filePath);
    if (!logFile.open(IO_WriteOnly | IO_Append))
	qFatal("Error: failed to open log file: " + logFile.name());

    qInstallMsgHandler(qtMessageHandler);
}

QString
logFileName()
{
    return logFile.name();
}

static void
logMessage(int type, const char* text, va_list args)
{
    int maxSize = strlen(text) + 4096;
    char* buffer = (char*)malloc(maxSize);
    if (buffer == NULL) return;

    int n = vsnprintf(buffer, maxSize, text, args);
    if (n == -1) strcpy(buffer, "Error: vsprintf failed");

    QDateTime now = QDateTime::currentDateTime();
    QString date = now.toString("yyyy-MM-dd hh:mm:ss");

    QString typeString;
    switch (type) {
    case LOG_MSG_STATUS:	typeString = " Status: "; break;
    case LOG_MSG_ERROR:		typeString = " Error: "; break;
    case LOG_MSG_WARNING:	typeString = " Warning: "; break;
    case LOG_MSG_DEBUG:		typeString = " Debug: "; break;
    }

    if (logFile.isOpen()) {
#ifndef WIN32
	flock(logFile.handle(), LOCK_EX);
#endif
	logFile.writeBlock(date, date.length());
	logFile.writeBlock(typeString, typeString.length());
	logFile.writeBlock(buffer, strlen(buffer));
	logFile.writeBlock("\n", 1);
	logFile.flush();
#ifndef WIN32
	flock(logFile.handle(), LOCK_UN);
#endif
    }

    if (!isDaemon)
	printf("%s%s%s\n", date.latin1(), typeString.latin1(), buffer);

    free(buffer);
}

void
logMessage(int type, const char* text, ...)
{
    va_list args;
    va_start(args, text);
    logMessage(type, text, args);
    va_end(args);
}

void
logStatus(const char* text, ...)
{
    va_list args;
    va_start(args, text);
    logMessage(LOG_MSG_STATUS, text, args);
    va_end(args);
}

void
logError(const char* text, ...)
{
    va_list args;
    va_start(args, text);
    logMessage(LOG_MSG_ERROR, text, args);
    va_end(args);
}

void
logWarning(const char* text, ...)
{
    va_list args;
    va_start(args, text);
    logMessage(LOG_MSG_WARNING, text, args);
    va_end(args);
}

void
logDebug(const char* text, ...)
{
    va_list args;
    va_start(args, text);
    logMessage(LOG_MSG_DEBUG, text, args);
    va_end(args);
}
