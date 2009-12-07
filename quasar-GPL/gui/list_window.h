// $Id: list_window.h,v 1.28 2004/10/01 23:42:28 bpepers Exp $
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

#ifndef LIST_WINDOW_H
#define LIST_WINDOW_H

#include <qmainwindow.h>
#include "list_view.h"
#include "list_view_item.h"
#include "data_object.h"

class MainWindow;
class QuasarClient;
class QuasarDB;
class DataEvent;
class QFrame;
class QTabWidget;
class QPushButton;

class ListWindow: public QMainWindow {
    Q_OBJECT
public:
    ListWindow(MainWindow* main, const char* name, bool tabs=false);
    ~ListWindow();

    const QString& helpSource() const;
    void setHelpSource(const QString& source);

public slots:
    void slotHelp();
    void slotWhatsThis();
    void slotRefresh();

protected slots:
    void slotPrint();
    void slotClose();
    void slotExit();
    void slotNew();
    void slotEdit();
    void slotPopup(QListViewItem* item, const QPoint& pos, int index);
    virtual void slotActivities();
    void slotRealRefresh();
    void slotDoubleClick();

protected:
    virtual void finalize(bool refresh=true);
    virtual void addToPopup(QPopupMenu* menu);

    virtual void performRefresh();
    virtual void performPrint();
    virtual void performNew();
    virtual void performEdit();

    MainWindow* _main;
    QuasarClient* _quasar;
    QuasarDB* _db;
    QString _helpSource;
    bool _refreshPending;

    // Widgets
    QFrame* _center;
    QFrame* _search;
    QTabWidget* _tabs;
    ListView* _list;
    QFrame* _extra;
    QFrame* _buttons;
    QPushButton* _new;
    QPushButton* _edit;
    QPushButton* _refresh;
    QPushButton* _print;
    QPushButton* _close;
    QWidget* _focus;

    // Menus
    QPopupMenu* _file;
    QPopupMenu* _activities;
    QPopupMenu* _help;
    int _fileRefresh, _filePrint, _fileClose, _fileExit;
    int _helpHelp, _helpWhatsThis, _helpAboutQuasar, _helpAboutQt;

    void connectList(ListView* list);
    virtual void clearLists();
    virtual ListView* currentList();
    ListViewItem* currentItem();

    ListViewItem* findId(Id id);
    Id currentId();
    Id currentId(ListView* list);
};

#endif // LIST_WINDOW_H
