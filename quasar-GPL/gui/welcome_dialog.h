// $Id: welcome_dialog.h,v 1.3 2005/02/24 18:17:58 bpepers Exp $
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

#ifndef WELCOME_DIALOG_H
#define WELCOME_DIALOG_H

#include <qdialog.h>
#include <qstringlist.h>
#include <unicode/locid.h>

class QuasarClient;
class QTimer;
class QGroupBox;
class QLabel;
class ComboBox;
class LineEdit;
class QPushButton;

class WelcomeDialog: public QDialog {
    Q_OBJECT
public:
    WelcomeDialog();
    virtual ~WelcomeDialog();

    QString locale() const;
    QString serverName() const;

protected slots:
    void accept();
    void slotLocaleChanged();
    void slotHostFound();
    void slotConnected();
    void slotConnectionError(int errorNum);
    void slotTimeout();

protected:
    QuasarClient* _quasar;
    QTimer* _timer;
    const Locale* _locales;
    int32_t _localesCnt;

    QGroupBox* _localeBox;
    QLabel* _localeMsg;
    QLabel* _localeLabel;
    ComboBox* _locale;
    QGroupBox* _serverBox;
    QLabel* _serverMsg;
    QLabel* _serverLabel;
    LineEdit* _server;
    QLabel* _status;
    QPushButton* _ok;
    QPushButton* _cancel;
};

#endif // WELCOME_DIALOG_H
