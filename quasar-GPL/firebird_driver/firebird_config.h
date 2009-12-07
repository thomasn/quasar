// $Id: firebird_config.h,v 1.6 2005/03/03 00:35:32 bpepers Exp $
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

#ifndef FIREBIRD_CONFIG_H
#define FIREBIRD_CONFIG_H

#include "config_file.h"

class FirebirdConfig: public ConfigFile
{
public:
    // Constructors
    FirebirdConfig();
    FirebirdConfig(const QString& configDir);
    virtual ~FirebirdConfig();

    // How to connect to database server
    QString hostname;		// Host where Firebird server is running
    int port;			// Port for Firebird socket on server
    QString library;		// Library to load for Firebird API
    QString installDir;		// Path to where Firebird was installed

    // Username/password for connections
    QString dbaPassword;	// Password for sysdba user
    QString username;		// Username for non-dba user
    QString password;		// Password for non-dba user

    // Database creation
    QString databaseDir;	// Directory to create databases
    int blockSize;		// Block size when creating databases
    QString charSet;		// Character set of companies

    // Load/save config in XML format
    bool load(bool showErrors=true);
    bool save(bool overwrite=false);

    // Clear to defaults
    void clear();

    // Operations
    bool operator==(const FirebirdConfig& rhs) const;
    bool operator!=(const FirebirdConfig& rhs) const;
};

#endif // FIREBIRD_CONFIG_H
