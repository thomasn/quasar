// $Id: restore_screen.h,v 1.6 2005/03/01 19:53:02 bpepers Exp $
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

#ifndef RESTORE_SCREEN_H
#define RESTORE_SCREEN_H

#include <qmainwindow.h>
#include "quasar_db.h"
#include "company_defn.h"
#include "server_config.h"
#include "data_model.h"
#include "zlib.h"

class LineEdit;
class ComboBox;
class QLabel;
class QProgressBar;
class QPushButton;

class RestoreScreen: public QMainWindow {
    Q_OBJECT
public:
    RestoreScreen();
    ~RestoreScreen();

protected slots:
    void slotOpenFile();
    void slotRestore();
    void slotClose();

protected:
    CompanyDefn _company;
    Driver* _driver;
    Connection* _connection;
    QuasarDB* _db;
    ServerConfig _config;
    time_t _start;
    bool _abort;
    gzFile _restoreFile;
    DataModel _model;
    QString _version;

    bool createCompany();
    bool createTables();
    bool createConstraints();
    bool createIndexes();
    bool restoreData();
    void updateTime();

    LineEdit* _name;
    ComboBox* _type;
    LineEdit* _filePath;
    LineEdit* _started;
    LineEdit* _elapsed;
    QLabel* _currTable;
    QProgressBar* _tableProgress;
    QLabel* _currRecord;
    QProgressBar* _recordProgress;
    QPushButton* _restore;
    QPushButton* _close;
};

#endif // RESTORE_SCREEN_H
