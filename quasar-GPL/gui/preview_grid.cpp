// $Id: preview_grid.cpp,v 1.6 2004/01/31 01:50:30 arandell Exp $
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

#include "preview_grid.h"
#include "main_window.h"
#include "images.h"
#include "frame_view.h"
#include "grid.h"

#include <qlabel.h>
#include <qtextbrowser.h>
#include <qpushbutton.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qdialog.h>
#include <qsimplerichtext.h>
#include <qpaintdevicemetrics.h>
#include <qlayout.h>
#include <qtimer.h>

PreviewGrid::PreviewGrid(MainWindow* main, Grid* showGrid)
    : QMainWindow(NULL, "PreviewGrid", WDestructiveClose)
{
    _main = main;
    _grid = showGrid;

#ifndef _WIN32
    setIcon(QPixmap(window_icon_xpm));
#endif

    QFrame* frame = new QFrame(this);

    _view = new FrameView(frame);
    _view->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QFrame* box = new QFrame(frame);

    QPushButton* print = new QPushButton(tr("&Print"), box);
    connect(print, SIGNAL(clicked()), SLOT(slotPrint()));

    QPushButton* close = new QPushButton(tr("Cl&ose"), box);
    connect(close, SIGNAL(clicked()), SLOT(close()));

    print->setMinimumSize(print->sizeHint());
    close->setMinimumSize(print->sizeHint());

    QGridLayout* grid1 = new QGridLayout(box);
    grid1->setSpacing(6);
    grid1->setMargin(6);
    grid1->setColStretch(0, 1);
    grid1->addWidget(print, 0, 1, AlignRight | AlignVCenter);
    grid1->addWidget(close, 0, 2, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setRowStretch(0, 1);
    grid->setColStretch(0, 1);
    grid->addWidget(_view, 0, 0);
    grid->addWidget(box, 1, 0);

    setCentralWidget(frame);
    setCaption(tr("Printout Preview"));
    setMinimumSize(640, 600);

    QPopupMenu* file = new QPopupMenu(this);
    file->insertItem(tr("&Print"), this, SLOT(slotPrint()), ALT | Key_P);
    file->insertSeparator();
    file->insertItem(tr("&Close"), this, SLOT(close()), ALT | Key_Q );
    file->insertItem(tr("E&xit"), _main, SLOT(slotExit()), ALT | Key_X );

    QPopupMenu* help = new QPopupMenu(this);
    help->insertItem(tr("&Help"), this, SLOT(slotHelp()));
    help->insertSeparator();
    help->insertItem(tr("About Quasar"), _main, SLOT(slotAboutQuasar()));
    help->insertItem(tr("About Qt"), _main, SLOT(slotAboutQt()));

    menuBar()->insertItem(tr("&File"), file);
    menuBar()->insertSeparator();
    menuBar()->insertItem(tr("&Help"), help);
    menuBar()->setSeparator(QMenuBar::InWindowsStyle);

    _view->setFocus();
    _view->setFrame(_grid);
}

PreviewGrid::~PreviewGrid()
{
    delete _grid;
}

void
PreviewGrid::slotHelp()
{
    _main->showHelp("preview_grid.html");
}

void
PreviewGrid::slotPrint()
{
    // Save old size and set printing
    fixed oldWidth = _grid->width();
    fixed oldHeight = _grid->height();
    _view->setPrinting(true);

    // Print to the printer
    _grid->print(this);

    // Set back to pixels and restore size
    _grid->resize(oldWidth, oldHeight);
    _view->setPrinting(false);
}
