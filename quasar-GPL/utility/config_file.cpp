// $Id: config_file.cpp,v 1.27 2005/04/12 07:38:33 bpepers Exp $
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

#include "config_file.h"
#include "quasar_misc.h"

#include <qdom.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <stdlib.h>

QString ConfigFile::_defaultConfigDir;

ConfigFile::ConfigFile(const QString& fileName)
    : _fileName(fileName), _configDir(_defaultConfigDir)
{
}

ConfigFile::ConfigFile(const QString& fileName, const QString& configDir)
    : _fileName(fileName), _configDir(configDir)
{
}

ConfigFile::~ConfigFile()
{
}

QString
ConfigFile::filePath()
{
    return QDir(configDir()).filePath(_fileName);
}

QStringList
ConfigFile::configDirs() const
{
    // Use environment variable if supplied
    if (getenv("QUASAR_CONFIG_DIRS"))
	return QStringList::split(':', getenv("QUASAR_CONFIG_DIRS"));

    // Config files found near the program are likely the next choice
    QStringList dirs;
    QDir dir(programDir());
    dirs << dir.absPath();
    if (dir.cd("config")) {
	dirs << dir.absPath();
	dir.cdUp();
    }
    if (dir.cdUp()) {
	dirs << dir.absPath();
	if (dir.cd("config"))
	    dirs << dir.absPath();
    }

    // Add on some other standard places to look
    dirs << QDir::currentDirPath();
    dirs << QDir::homeDirPath() + "/Quasar";
#ifdef WIN32
    dirs << "c:/Program Files/Quasar";
#else
    dirs << "/etc/quasar";
#endif

    return dirs;
}

QString
ConfigFile::platformFileName() const
{
    QFileInfo info(_fileName);
#ifdef WIN32
    return info.baseName() + "_win." + info.extension();
#else
    return info.baseName() + "_unix." + info.extension();
#endif
}

QString
ConfigFile::searchDirs(const QString& fileName) const
{
    QStringList dirs = configDirs();
    for (unsigned int i = 0; i < dirs.size(); ++i) {
	QDir dir(dirs[i]);
	if (!dir.exists())
	    continue;
	if (dir.exists(fileName))
	    return dirs[i];
    }
    return "";
}

QString
ConfigFile::configDir()
{
    // Return cached result if set
    if (!_configDir.isEmpty())
	return _configDir;

    // Check for environment variable
    _configDir = getenv("QUASAR_CONFIG_DIR");
    if (!_configDir.isEmpty())
	return _configDir;

    // First try with platform specific file name
    QStringList dirs = configDirs();
    _configDir = searchDirs(platformFileName());

    // If not found, look using generic file name
    if (_configDir.isEmpty())
	_configDir = searchDirs(_fileName);

    return _configDir;
}

bool
ConfigFile::canSave()
{
    QFileInfo info(filePath());
    if (info.exists())
	return info.isWritable();

    QFile file(filePath());
    if (file.open(IO_ReadWrite)) {
	file.remove();
	return true;
    }

    return false;
}

void
ConfigFile::setPermissions(int mode, const char* owner, const char* group)
{
    ::setPermissions(filePath(), mode, owner, group);
}

void
ConfigFile::setDefaultConfigDir(const QString& dir)
{
    _defaultConfigDir = dir;
}

void
ConfigFile::addNode(QDomDocument& doc, const QString& tag, const QString& text)
{
    QDomElement e = doc.createElement(tag);
    e.appendChild(doc.createTextNode(text));
    doc.documentElement().appendChild(e);
}

bool
ConfigFile::error(const QString& error)
{
    qWarning("Error: " + error);
    _lastError = error;
    return false;
}

QString
parseDir(const QString& dirPath)
{
    if (dirPath.left(2) == "~/")
	return QDir::home().filePath(dirPath.mid(2));
    return dirPath;
}
