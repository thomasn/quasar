// $Id: user_config.h,v 1.11 2005/02/24 02:56:14 bpepers Exp $
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

#ifndef USER_CONFIG_H
#define USER_CONFIG_H

#include "config_file.h"
#include <qframe.h>

class UserConfig: public ConfigFile {
public:
    // Constructors and Destructor
    UserConfig();
    ~UserConfig();

    // Place to cache server files
    QString cacheDir;

    // Display configuration
    bool changeStyle;
    bool changeColor;
    bool changeFont;
    QString style;
    QString color;
    QString font;
    QString mainPixmap;

    // Locale
    QString locale;

    // Default open company info when starting
    QString defaultServer;
    QString defaultCompany;
    QString defaultUserName;
    QString defaultPassword;

    // Other internal configuration
    QString smtpHost;
    int smtpPort;
    QString handheldDevice;
    bool preferA4;

    // Load/save config in XML format
    bool load(bool showErrors=true);
    bool save(bool overwrite=false);

    // Clear to defaults
    void clear();

    // Operations
    bool operator==(const UserConfig& rhs) const;
    bool operator!=(const UserConfig& rhs) const;

    // Defaults for new UserConfigs
    static QString defaultStyle;
    static QPalette defaultPalette;
    static QFont defaultFont;
};

#endif // USER_CONFIG_H
