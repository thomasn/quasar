// $Id: quasar_window.h,v 1.9 2004/12/28 09:18:14 bpepers Exp $
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

#ifndef QUASAR_WINDOW_H
#define QUASAR_WINDOW_H

#include <qmainwindow.h>
#include "screen_decl.h"

class MainWindow;
class QuasarClient;
class QuasarDB;
class QPopupMenu;

class QuasarWindow: public QMainWindow {
    Q_OBJECT
public:
    QuasarWindow(MainWindow* main, const char* name=0);
    ~QuasarWindow();

    const QString& helpSource() const;
    void setHelpSource(const QString& source);

    bool error(const QString& msg, QWidget* widget=0);
    bool errorInvalid(QWidget* widget);
    bool errorManditory(QWidget* widget);

    bool allowed(const QString& operation);

public slots:
    void slotNotAllowed();
    virtual void slotEditCut();
    virtual void slotEditCopy();
    virtual void slotEditPaste();
    virtual void slotEditSelectAll();
    virtual void slotEditClear();
    virtual void slotHelp();
    virtual void slotWhatsThis();

protected:
    virtual void finalize();

    MainWindow* _main;
    QuasarClient* _quasar;
    QuasarDB* _db;
    QString _helpSource;

    // Menus
    QPopupMenu* _file;
    QPopupMenu* _edit;
    QPopupMenu* _help;
};

#endif // QUASAR_WINDOW_H
