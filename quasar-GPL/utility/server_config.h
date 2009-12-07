// $Id: server_config.h,v 1.22 2005/03/03 21:35:33 bpepers Exp $
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

#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

#include "config_file.h"

#define DEFAULT_QUASARD_PORT 3599

class ServerConfig: public ConfigFile
{
public:
    // Constructors
    ServerConfig();
    ServerConfig(const QString& configDir);
    virtual ~ServerConfig();

    // Public information
    int port;			// Port for server (default to use services)
    int systemId;		// System id for replication
    QString dataDir;		// Location for companies, screens, labels
    QString driverDir;		// Location for database drivers
    QString backupDir;		// Location for backup files
    QString importDir;		// Location for import files
    QString adminPassword;	// Password used by admin client login

    // Load/save config in XML format
    bool load(bool showError=true);
    bool save(bool overwrite=false);

    // Clear to defaults
    void clear();

    // Operations
    bool operator==(const ServerConfig& rhs) const;
    bool operator!=(const ServerConfig& rhs) const;
};

#endif // SERVER_CONFIG_H
