// $Id: open_company.h,v 1.26 2004/09/20 17:35:55 bpepers Exp $
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

#ifndef OPEN_COMPANY_H
#define OPEN_COMPANY_H

#include <qmainwindow.h>
#include "company_defn.h"
#include "screen_defn.h"

class MainWindow;
class QuasarClient;
class QTimer;
class QFrame;
class LineEdit;
class ListView;
class QLabel;
class QPushButton;
class QProgressDialog;

class OpenCompany: public QMainWindow {
    Q_OBJECT
public:
    OpenCompany(MainWindow* main=NULL);
    virtual ~OpenCompany();

    void setInfo(const QString& server, const QString& company,
		 const QString& username, const QString& password);

    QuasarClient* getQuasarClient();
    ScreenDefn getScreenDefn();

protected slots:
    void slotServerChanged();
    void slotHostFound();
    void slotConnected();
    void slotConnectionError(int errorNum);
    void slotLogin();
    void slotAbort();
    void slotTimeout();
    void slotDisconnected();
    void slotEnsureVisible();
    bool eventFilter(QObject* o, QEvent* e);

protected:
    void setDisconnected();
    void setProcessing(const QString& message);
    void setConnected();

    void setStatus(const QString& message);
    void clearStatus();

    MainWindow* _main;
    QuasarClient* _quasar;
    ScreenDefn _screen;
    QTimer* _timer;
    QString _defaultCompany;

    QFrame* _top;
    LineEdit* _server;
    QFrame* _mid;
    ListView* _companies;
    LineEdit* _username;
    LineEdit* _password;
    QPushButton* _login;
    QPushButton* _cancel;
    QDialog* _status;
    QLabel* _statusText;
};

#endif // OPEN_COMPANY_H
