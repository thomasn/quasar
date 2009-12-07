// $Id: client_config.h,v 1.5 2004/04/23 07:40:36 bpepers Exp $
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

#ifndef CLIENT_CONFIG_H
#define CLIENT_CONFIG_H

#include "config_file.h"

class ClientConfig: public ConfigFile
{
public:
    // Constructors
    ClientConfig();
    ClientConfig(const QString& configDir);
    virtual ~ClientConfig();

    // Public information
    QString localeDir;		// Location for locales
    QString stationNumber;	// Station number

    // Load/save config in XML format
    bool load(bool showErrors=true);
    bool save(bool overwrite=false);

    // Clear to defaults
    void clear();

    // Operations
    bool operator==(const ClientConfig& rhs) const;
    bool operator!=(const ClientConfig& rhs) const;
};

#endif // CLIENT_CONFIG_H
