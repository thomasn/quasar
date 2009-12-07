// $Id: quasar_misc.cpp,v 1.10 2005/04/12 07:39:03 bpepers Exp $
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

#include "quasar_misc.h"
#include "sha1.h"

#include <qapplication.h>
#include <qfileinfo.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#else
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

QString
programDir()
{
    static QString dir;
    if (!dir.isEmpty()) return dir;

#ifdef WIN32
    char filename[1024];
    if (GetModuleFileName(NULL, filename, 1024) > 0) {
	QFileInfo info(filename);
	if (info.exists()) {
	    dir = info.dirPath(true);
	    return dir;
	}
    }
#else
    // *NOTE*  this is very Linux specific.  Perhaps other things should
    // be tried like looking at argv[0] and if its not absolute then
    // checking the PATH environment variable.

    QFileInfo info("/proc/self/exe");
    if (info.exists() && info.isSymLink()) {
	info.setFile(info.readLink());
	if (info.exists()) {
	    dir = info.dirPath(true);
	    return dir;
	}
    }

    QString program = qApp->argv()[0];
    info.setFile(program);

    // If absolute path given or program exists relative to the
    // current directory then use that
    if (!info.isRelative()) {
	dir = info.dirPath();
	return dir;
    }
    if (info.exists() && info.isFile() && info.isExecutable()) {
	dir = info.dirPath(true);
	return dir;
    }

    // Otherwise its time to search the PATH environment variable
    if (getenv("PATH") != NULL) {
	QString path = getenv("PATH");
	QStringList dirs = QStringList::split(":", path);
	for (unsigned int i = 0; i < dirs.size(); ++i) {
	    QFileInfo info(dirs[i] + "/" + program);
	    if (info.exists() && info.isFile() && info.isExecutable()) {
		dir = info.dirPath(true);
		return dir;
	    }
	}
    }
#endif

    qWarning("Can't get directory program was run from");
    return "";
}

QString
sha1Crypt(const QString& password)
{
    SHA1_CTX context;
    uint8_t digest[SHA1_DIGEST_SIZE];
    QCString utf8 = password.utf8();
    size_t utf8_len = utf8.length();

    uint8_t* buffer = (uint8_t*)malloc(utf8_len);
    if (buffer == NULL) return "";
    memcpy(buffer, utf8, utf8_len);

    SHA1Init(&context);
    SHA1Update(&context, buffer, utf8_len);
    SHA1Final(&context, digest);

    QString crypted;
    for (unsigned int i = 0; i < SHA1_DIGEST_SIZE; ++i) {
	QString value;
	value.sprintf("%02x", (int)digest[i]);
	crypted += value;
    }

    free(buffer);
    return crypted;
}

void
qsleep(int seconds)
{
#ifdef WIN32
    Sleep(seconds * 1000);
#else
    sleep(seconds);
#endif
}

void
setPermissions(const char* file, int mode, const char* owner, const char* group)
{
#ifndef WIN32
    if (mode != -1)
	chmod(file, mode);

    if (owner != NULL) {
	struct passwd* userInfo = getpwnam(owner);
	if (userInfo != NULL)
	    chown(file, userInfo->pw_uid, gid_t(-1));
    }

    if (group != NULL) {
	struct group* groupInfo = getgrnam(group);
	if (groupInfo != NULL)
	    chown(file, uid_t(-1), groupInfo->gr_gid);
    }
#endif
}
