// $Id: edit_window.cpp,v 1.8 2004/01/31 01:50:30 arandell Exp $
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

#include "edit_window.h"
#include "main_window.h"

#include <qapplication.h>
#include <qmenubar.h>
#include <qstatusbar.h>
#include <qpopupmenu.h>
#include <qvbox.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qwhatsthis.h>

EditWindow::EditWindow(MainWindow* main, const char* name)
    : QuasarWindow(main, name)
{
    // Top level widgets
    _top = new QWidget(this);
    _frame = new QWidget(_top);
    _buttons = new QVBox(_top);
    _buttons->setMargin(4);
    _buttons->setSpacing(4);

    _grid = new QGridLayout(_top, 1, 2);
    _grid->setSpacing(3);
    _grid->setMargin(3);
    _grid->setColStretch(0, 1);
    _grid->addWidget(_frame, 0, 0);
    _grid->addWidget(_buttons, 0, 1, AlignRight | AlignTop);
}

EditWindow::~EditWindow()
{
}

void
EditWindow::finalize()
{
    _grid->activate();
    setCentralWidget(_top);
    QuasarWindow::finalize();
}
