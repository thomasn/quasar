// $Id: edit_window.h,v 1.6 2004/01/31 01:50:31 arandell Exp $
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

#ifndef EDIT_WINDOW_H
#define EDIT_WINDOW_H

#include "quasar_window.h"

class QFrame;
class QVBox;
class QPushButton;
class QGridLayout;

class EditWindow: public QuasarWindow {
    Q_OBJECT
public:
    EditWindow(MainWindow* main, const char* name);
    ~EditWindow();

protected:
    virtual void finalize();

    // Widgets
    QWidget* _top;		// central widget for QMainWindow
    QWidget* _frame;		// frame for the contents
    QVBox* _buttons;		// box for the buttons
    QGridLayout* _grid;		// grid that does layout for _top
};

#endif // EDIT_WINDOW_H
