// $Id: main_window.h,v 1.42 2004/09/20 17:35:55 bpepers Exp $
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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <qmainwindow.h>
#include "data_object.h"
#include "screen_defn.h"

class QuasarClient;
class QuasarDB;
class Navigator;
class QPopupMenu;
class QLabel;
class QComboBox;
class QHBox;

class MainWindow: public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QuasarClient* quasar, ScreenDefn& screen);
    ~MainWindow();

    void showHelp(const QString& source);
    QWidget* findWindow(const QString& name);

    QuasarClient* quasar()	{ return _quasar; }
    QuasarDB* db()		{ return _db; }
    Navigator* navigator()	{ return _navigator; }

    void setDefaultStore(Id store_id);
    Id defaultStore();

public slots:
    void slotMenuItem(int id);
    void slotToolButton();
    void slotPanelButton();
    void slotShortcut();
    void slotFunction(QString type, QString args);
    void slotReport(const QString& fileName);
    void slotWindowShow();
    void slotCascade();
    void slotTile();
    void slotCloseAll();
    void slotChangeCompany();
    void slotExit();
    void slotHelp();
    void slotAboutQuasar();
    void slotAboutQt();

protected slots:
    bool eventFilter(QObject* o, QEvent* e);

private:
    void buildMenu(MenuItem& menu);

    QuasarClient* _quasar;
    QuasarDB* _db;
    ScreenDefn _screen;
    Navigator* _navigator;

    // Menus
    QPopupMenu* _file;
    QPopupMenu* _edit;
    QPopupMenu* _nav;
    QPopupMenu* _reports;
    QPopupMenu* _setup;
    QPopupMenu* _window;
    QPopupMenu* _help;
    QPopupMenu* _cardAct;
    QPopupMenu* _ledgerAct;
    QPopupMenu* _chequeAct;
    QPopupMenu* _invAct;
    QPopupMenu* _salesAct;
    QPopupMenu* _purchAct;
    QPopupMenu* _systemSetup;
    QPopupMenu* _cardSetup;
    QPopupMenu* _invSetup;
    QPopupMenu* _salesSetup;
    QPopupMenu* _purchSetup;
    QPopupMenu* _hhSetup;

    // Status bar
    QLabel* _store;
    QLabel* _station;
    QLabel* _employee;
};

#endif // MAIN_WINDOW_H
