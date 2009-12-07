// $Id: output_window.cpp,v 1.1 2005/03/29 22:29:06 bpepers Exp $
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

#include "output_window.h"
#include "list_view_item.h"
#include "grid.h"

#include <qapplication.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qlayout.h>

OutputWindow::OutputWindow(QWidget* parent)
    : QMainWindow(parent, "OutputWindow")
{
    QVBox* main = new QVBox(this);

    _list = new ListView(main);
    _list->addNumberColumn("#", 4);
    _list->addTextColumn("Type", 10);
    _list->addTextColumn("Message", 40);
    _list->setAllColumnsShowFocus(true);
    _list->setShowSortIndicator(true);
    _list->setSorting(0);

    QFrame* buttons = new QFrame(main);

    QPushButton* print = new QPushButton(tr("&Print"), buttons);
    connect(print, SIGNAL(clicked()), SLOT(slotPrint()));

    QPushButton* close = new QPushButton(tr("&Close"), buttons);
    connect(close, SIGNAL(clicked()), SLOT(close()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(print, 0, 1);
    buttonGrid->addWidget(close, 0, 2);

    setCentralWidget(main);
    setCaption(tr("Output Window"));
}

OutputWindow::~OutputWindow()
{
}

void
OutputWindow::info(const QString& message)
{
    ListViewItem* item = new ListViewItem(_list);
    item->setValue(0, _list->childCount());
    item->setValue(1, tr("Info"));
    item->setValue(2, message);
}

void
OutputWindow::warning(const QString& message)
{
    ListViewItem* item = new ListViewItem(_list);
    item->setValue(0, _list->childCount());
    item->setValue(1, tr("Warning"));
    item->setValue(2, message);
}

void
OutputWindow::error(const QString& message)
{
    ListViewItem* item = new ListViewItem(_list);
    item->setValue(0, _list->childCount());
    item->setValue(1, tr("Error"));
    item->setValue(2, message);
}

void
OutputWindow::reset()
{
    _list->clear();
}

void
OutputWindow::slotPrint()
{
    Grid* grid = Grid::buildGrid(_list, "Output Window");
    grid->print(this);
    delete grid;
}
