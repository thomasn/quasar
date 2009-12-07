// $Id: firebird_config_dialog.h,v 1.6 2005/03/03 00:35:32 bpepers Exp $
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

#ifndef FIREBIRD_CONFIG_DIALOG_H
#define FIREBIRD_CONFIG_DIALOG_H

#include <qdialog.h>
#include "firebird_config.h"

class LineEdit;
class IntegerEdit;

class FirebirdConfigDialog: public QDialog {
    Q_OBJECT
public:
    FirebirdConfigDialog(QWidget* parent);
    ~FirebirdConfigDialog();

private slots:
    void test();

private:
    // Show a message and if its critical, set _critical to true 
    void warning(const QString& message);
    void critical(const QString& message);
    bool _critical;

    // Initial configuration
    FirebirdConfig _config;

    // Current configuration
    FirebirdConfig getConfig();

    // Over-ride to do validation
    void accept();
    void reject();

    LineEdit* _hostname;
    IntegerEdit* _port;
    LineEdit* _library;
    LineEdit* _installDir;
    LineEdit* _username;
    LineEdit* _password;
    LineEdit* _dbaPassword;
    LineEdit* _databaseDir;
    IntegerEdit* _blockSize;
    LineEdit* _charSet;
};

#endif // FIREBIRD_CONFIG_DIALOG_H
