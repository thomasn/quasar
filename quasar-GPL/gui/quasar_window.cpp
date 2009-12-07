// $Id: quasar_window.cpp,v 1.14 2004/12/28 09:18:14 bpepers Exp $
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

#include "quasar_window.h"
#include "quasar_client.h"
#include "main_window.h"
#include "images.h"
#include "line_edit.h"
#include "multi_line_edit.h"

#include <qapplication.h>
#include <qmenubar.h>
#include <qstatusbar.h>
#include <qpopupmenu.h>
#include <qvbox.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qwhatsthis.h>

QuasarWindow::QuasarWindow(MainWindow* main, const char* name)
    : QMainWindow(NULL, name, WDestructiveClose)
{
    // Set links to _main and _quasar
    _main = main;
    _quasar = main->quasar();
    _db = _quasar->db();

#ifndef _WIN32
    setIcon(QPixmap(window_icon_xpm));
#endif

    // Create standard menus
    _file = new QPopupMenu(this);
    _edit = new QPopupMenu(this);
    _help = new QPopupMenu(this);

    menuBar()->insertItem(tr("&File"), _file);
    menuBar()->insertItem(tr("&Edit"), _edit);
    menuBar()->insertSeparator();
    menuBar()->insertItem(tr("&Help"), _help);
    menuBar()->setSeparator(QMenuBar::InWindowsStyle);

    // Set foreground color for status bar text to red
    QPalette palette = statusBar()->palette();
    palette.setColor(QPalette::Active, QColorGroup::Text,
		     QColor(255, 10, 10));
    statusBar()->setPalette(palette);
}

QuasarWindow::~QuasarWindow()
{
}

void
QuasarWindow::finalize()
{
    _file->insertSeparator();
    _file->insertItem(tr("Close"), this, SLOT(close()));
    _file->insertItem(tr("E&xit"), _main, SLOT(slotExit()));

    _edit->insertItem(tr("C&ut"), this, SLOT(slotEditCut()), CTRL + Key_X);
    _edit->insertItem(tr("&Copy"), this, SLOT(slotEditCopy()), CTRL + Key_C);
    _edit->insertItem(tr("&Paste"), this, SLOT(slotEditPaste()), CTRL + Key_V);
    _edit->insertSeparator();
    _edit->insertItem(tr("Select &All"), this, SLOT(slotEditSelectAll()));
    _edit->insertItem(tr("C&lear"), this, SLOT(slotEditClear()));

    _help->insertItem(tr("&Help"), this, SLOT(slotHelp()), Key_F1);
    _help->insertSeparator();
    _help->insertItem(tr("About Quasar"), _main, SLOT(slotAboutQuasar()));
    _help->insertItem(tr("About Qt"), _main, SLOT(slotAboutQt()));
}

bool
QuasarWindow::error(const QString& msg, QWidget* widget)
{
    statusBar()->message(msg);
    QApplication::beep();
    if (widget) widget->setFocus();
    return false;
}

bool
QuasarWindow::errorInvalid(QWidget* widget)
{
    if (widget->name(NULL) == NULL)
	return error(tr("Invalid field"), widget);
    return error(tr("Invalid field: ") + widget->name(), widget);
}

bool
QuasarWindow::errorManditory(QWidget* widget)
{
    if (widget->name(NULL) == NULL)
	return error(tr("Manditory field"), widget);
    return error(tr("Manditory field: ") + widget->name(), widget);
}

bool
QuasarWindow::allowed(const QString& operation)
{
    return _quasar->securityCheck(name(), operation);
}

void
QuasarWindow::slotNotAllowed()
{
    qApp->beep();
    QString message = tr("Access to this screen is denied");
    QMessageBox::critical(this, tr("Error"), message);
    delete this;
}

void
QuasarWindow::slotEditCut()
{
    QWidget* widget = focusWidget();
    if (widget->inherits("QLineEdit")) {
	QLineEdit* edit = (QLineEdit*)widget;
	edit->cut();
    } else if (widget->inherits("MultiLineEdit")) {
	MultiLineEdit* edit = (MultiLineEdit*)widget;
	edit->cut();
    }
}

void
QuasarWindow::slotEditCopy()
{
    QWidget* widget = focusWidget();
    if (widget->inherits("QLineEdit")) {
	QLineEdit* edit = (QLineEdit*)widget;
	edit->copy();
    }
    if (widget->inherits("MultiLineEdit")) {
	MultiLineEdit* edit = (MultiLineEdit*)widget;
	edit->copy();
    }
}

void
QuasarWindow::slotEditPaste()
{
    QWidget* widget = focusWidget();
    if (widget->inherits("QLineEdit")) {
	QLineEdit* edit = (QLineEdit*)widget;
	edit->paste();
    }
    if (widget->inherits("MultiLineEdit")) {
	MultiLineEdit* edit = (MultiLineEdit*)widget;
	edit->paste();
    }
}

void
QuasarWindow::slotEditSelectAll()
{
    QWidget* widget = focusWidget();
    if (widget->inherits("QLineEdit")) {
	QLineEdit* edit = (QLineEdit*)widget;
	edit->selectAll();
    }
    if (widget->inherits("MultiLineEdit")) {
	MultiLineEdit* edit = (MultiLineEdit*)widget;
	edit->selectAll();
    }
}

void
QuasarWindow::slotEditClear()
{
    QWidget* widget = focusWidget();
    if (widget->inherits("QLineEdit")) {
	QLineEdit* edit = (QLineEdit*)widget;
	edit->clear();
    }
    if (widget->inherits("MultiLineEdit")) {
	MultiLineEdit* edit = (MultiLineEdit*)widget;
	edit->clear();
    }
}

void
QuasarWindow::slotHelp()
{
    _main->showHelp(_helpSource);
}

void
QuasarWindow::slotWhatsThis()
{
    QWhatsThis::enterWhatsThisMode();
}
