// $Id: list_window.cpp,v 1.40 2004/10/01 23:42:28 bpepers Exp $
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

#include "list_window.h"
#include "quasar_client.h"
#include "main_window.h"
#include "images.h"
#include "list_view.h"
#include "list_view_item.h"

#include <qapplication.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qheader.h>
#include <qtimer.h>
#include <qsemimodal.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <assert.h>

ListWindow::ListWindow(MainWindow* main, const char* name, bool tabs)
    : QMainWindow(NULL, name, WDestructiveClose)
{
#ifndef _WIN32
    setIcon(QPixmap(window_icon_xpm));
#endif

    _main = main;
    _quasar = main->quasar();
    _db = _quasar->db();

    _center = new QFrame(this);
    _search = new QFrame(_center);
    if (tabs) {
	_tabs = new QTabWidget(_center);
	_list = new ListView(_tabs);
	_tabs->addTab(_list, tr("All"));
    } else {
	_tabs = NULL;
	_list = new ListView(_center);
    }
    _extra = new QFrame(_center);
    _buttons = new QFrame(_center);

    _list->setAllColumnsShowFocus(true);
    _list->setRootIsDecorated(false);
    _list->setShowSortIndicator(true);

    QGridLayout* grid2 = new QGridLayout(_center);
    grid2->setMargin(4);
    grid2->setSpacing(4);
    grid2->setRowStretch(1, 1);
    grid2->setColStretch(0, 1);
    grid2->addWidget(_search, 0, 0);
    if (_tabs == NULL)
	grid2->addWidget(_list, 1, 0);
    else
	grid2->addWidget(_tabs, 1, 0);
    grid2->addWidget(_extra, 2, 0);
    grid2->addWidget(_buttons, 3, 0);

    _file = new QPopupMenu(this);
    _fileRefresh = _file->insertItem(tr("&Refresh"), this, SLOT(slotRefresh()),
				     ALT+Key_R);
    _file->insertSeparator();
    _filePrint = _file->insertItem(tr("&Print"), this, SLOT(slotPrint()),
				   ALT+Key_P);
    _file->insertSeparator();
    _fileClose = _file->insertItem(tr("&Close"), this, SLOT(slotClose()));
    _fileExit = _file->insertItem(tr("E&xit"), this, SLOT(slotExit()));

    _activities = new QPopupMenu(this);
    connect(_activities, SIGNAL(aboutToShow()), SLOT(slotActivities()));

    _help = new QPopupMenu(this);
    _helpHelp = _help->insertItem(tr("&Help"), this, SLOT(slotHelp()), Key_F1);
    _helpWhatsThis = _help->insertItem(tr("&Whats This?"), this,
				       SLOT(slotWhatsThis()));
    _help->insertSeparator();
    _helpAboutQuasar = _help->insertItem(tr("About Quasar"), _main,
					 SLOT(slotAboutQuasar()));
    _helpAboutQt = _help->insertItem(tr("About Qt"), _main,
				     SLOT(slotAboutQt()));

    menuBar()->insertItem(tr("&File"), _file);
    menuBar()->insertItem(tr("&Activities"), _activities);
    menuBar()->insertSeparator();
    menuBar()->insertItem(tr("&Help"), _help);
    menuBar()->setSeparator(QMenuBar::InWindowsStyle);
    statusBar();

    connectList(_list);
}

ListWindow::~ListWindow()
{
}

void
ListWindow::finalize(bool refresh)
{
    _new = new QPushButton(tr("&New"), _buttons);
    _edit = new QPushButton(tr("&Edit"), _buttons);
    _refresh = new QPushButton(tr("&Refresh"), _buttons);
    _print = new QPushButton(tr("&Print"), _buttons);
    _close = new QPushButton(tr("&Close"), _buttons);

    connect(_new, SIGNAL(clicked()), this, SLOT(slotNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(slotEdit()));
    connect(_refresh, SIGNAL(clicked()), this, SLOT(slotRefresh()));
    connect(_print, SIGNAL(clicked()), this, SLOT(slotPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(slotClose()));

    QGridLayout* grid1 = new QGridLayout(_buttons);
    grid1->setMargin(4);
    grid1->setSpacing(4);
    grid1->setColStretch(7, 1);
    grid1->addWidget(_new, 0, 0, AlignLeft | AlignVCenter);
    grid1->addWidget(_edit, 0, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(_refresh, 0, 7, AlignRight | AlignVCenter);
    grid1->addWidget(_print, 0, 8, AlignRight | AlignVCenter);
    grid1->addWidget(_close, 0, 9, AlignRight | AlignVCenter);

    setCentralWidget(_center);
    setMinimumSize(sizeHint());
    _list->setFocus();
    _refreshPending = false;

    if (refresh) slotRefresh();
}

void
ListWindow::connectList(ListView* list)
{
    connect(list, SIGNAL(doubleClicked(QListViewItem*)), this,
	    SLOT(slotDoubleClick()));
    connect(list, SIGNAL(returnPressed(QListViewItem*)), this,
	    SLOT(slotEdit()));
    connect(list, SIGNAL(rightButtonPressed(QListViewItem*,const QPoint&,\
					    int)), this,
	    SLOT(slotPopup(QListViewItem*,const QPoint&,int)));
}

void
ListWindow::slotActivities()
{
}

void
ListWindow::slotHelp()
{
    _main->showHelp(_helpSource);
}

void
ListWindow::slotWhatsThis()
{
    QWhatsThis::enterWhatsThisMode();
}

void
ListWindow::slotRefresh()
{
    if (_refreshPending) return;
    QTimer::singleShot(0, this, SLOT(slotRealRefresh()));
    _refreshPending = true;
}

void
ListWindow::slotRealRefresh()
{
    _focus = focusWidget();
    setEnabled(false);
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    performRefresh();
    _refreshPending = false;

    setEnabled(true);
    QApplication::restoreOverrideCursor();
    if (_focus != NULL) _focus->setFocus();
    _focus = NULL;
}

void
ListWindow::performRefresh()
{
    // This need to be implemented by subclasses
    assert(0);
}

void
ListWindow::slotPrint()
{
    performPrint();
}

void
ListWindow::performPrint()
{
    // This need to be implemented by subclasses
    assert(0);
}

void
ListWindow::slotClose()
{
    delete this;
}

void
ListWindow::slotExit()
{
    _main->slotExit();
}

void
ListWindow::slotNew()
{
    performNew();
}

void
ListWindow::performNew()
{
    // This need to be implemented by subclasses
    assert(0);
}

void
ListWindow::slotEdit()
{
    ListViewItem* item = currentItem();
    if (item == NULL) return;
    performEdit();
}

void
ListWindow::slotDoubleClick()
{
    ListViewItem* item = currentItem();
    if (item == NULL) return;
    item->setOpen(!item->isOpen());
    performEdit();
}

void
ListWindow::performEdit()
{
    // This need to be implemented by subclasses
    assert(0);
}

void
ListWindow::addToPopup(QPopupMenu*)
{
}

void
ListWindow::slotPopup(QListViewItem* item, const QPoint& pos, int)
{
    QListView* list = currentList();

    QPopupMenu* edit = new QPopupMenu(this);
    edit->insertItem(tr("New"), this, SLOT(slotNew()));
    if (item != NULL) {
	list->setCurrentItem(item);
	list->setSelected(item, true);
    }
    if (list->currentItem() != NULL) {
	edit->insertItem(tr("Edit"), this, SLOT(slotEdit()));
    }

    addToPopup(edit);
    edit->popup(pos, 0);
}

ListViewItem*
ListWindow::findId(Id id)
{
    QListViewItemIterator it(_list);
    for (; it.current(); ++it) {
	ListViewItem* item = (ListViewItem*)it.current();
	if (item->id == id) return item;
    }
    return NULL;
}

ListView*
ListWindow::currentList()
{
    if (_tabs == NULL) return _list;
    return (ListView*)_tabs->currentPage();
}

ListViewItem*
ListWindow::currentItem()
{
    ListView* list = currentList();
    return (ListViewItem*)list->currentItem();
}

Id
ListWindow::currentId()
{
    ListViewItem* item = currentItem();
    if (item == NULL) return INVALID_ID;
    return item->id;
}

Id
ListWindow::currentId(ListView* list)
{
    if (list == NULL) return INVALID_ID;
    ListViewItem* item = (ListViewItem*)list->currentItem();
    if (item == NULL) return INVALID_ID;
    return item->id;
}

void
ListWindow::clearLists()
{
    _list->clear();
}
