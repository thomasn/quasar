// $Id: config_file.h,v 1.18 2005/04/05 19:00:01 bpepers Exp $
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

#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

// ConfigFile is the base class of Quasar configuration files.  It
// provides default methods for finding the location of configuration
// files and requires certain methods to be implemented in the child
// classes.

#include <qstring.h>
#include <qstringlist.h>

class QDomDocument;

class ConfigFile
{
public:
    // Constructors
    ConfigFile(const QString& fileName);
    ConfigFile(const QString& fileName, const QString& configDir);
    virtual ~ConfigFile();

    // Get the name of the configuration file
    QString fileName() const { return _fileName; }
    QString filePath();

    // Get the list of directories to check
    QStringList configDirs() const;

    // Get the directory this config file was found
    QString configDir();

    // Load/save config in XML format
    virtual bool load(bool showErrors=true)=0;
    virtual bool save(bool overwrite=false)=0;
    bool canSave();

    // Set permissions and owner/group
    void setPermissions(int mode, const char* owner, const char* group);

    // Clear to defaults
    virtual void clear()=0;

    // Set default config directory
    static void setDefaultConfigDir(const QString& dir);

    // Error handling
    QString lastError() const { return _lastError; }

protected:
    QString platformFileName() const;
    QString searchDirs(const QString& fileName) const;
    void addNode(QDomDocument& doc, const QString& tag, const QString& text);
    bool error(const QString& message);

    // Directory used by default for config files
    static QString _defaultConfigDir;

    QString _fileName;
    QString _configDir;
    QString _lastError;
};

// Parse a directory name replacing special characters like ~
extern QString parseDir(const QString& dir);

#endif // CONFIG_FILE_H
