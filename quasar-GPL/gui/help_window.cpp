// $Id: help_window.cpp,v 1.11 2005/02/17 09:55:03 bpepers Exp $
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

#include "help_window.h"
#include "images.h"

#include <qtextbrowser.h>
#include <qstatusbar.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qiconset.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstylesheet.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qlineedit.h>
#include <qobjectlist.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qsimplerichtext.h>
#include <qpaintdevicemetrics.h>

#include <ctype.h>

HelpWindow::HelpWindow(const QString& home, const QString& path,
		       QWidget* parent, const char* name)
    : QMainWindow(parent, name, WDestructiveClose)
{
    setCaption(tr("Quasar Help"));

    _browser = new QTextBrowser(this);
    _browser->mimeSourceFactory()->setFilePath(path);
    _browser->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    setCentralWidget(_browser);

    if (!home.isEmpty())
	_browser->setSource(home);

    connect(_browser, SIGNAL(highlighted(const QString&)),
	    statusBar(), SLOT(message(const QString&)));

    resize(800, 700);

    QPopupMenu* file = new QPopupMenu(this);
    file->insertItem(tr("&Print"), this, SLOT(print()), ALT | Key_P);
    file->insertSeparator();
    file->insertItem(tr("&Close"), this, SLOT(close()), ALT | Key_Q );

    QPopupMenu* go = new QPopupMenu(this);
    _backwardId = go->insertItem(QPixmap(back_image),
				 tr("&Backward"), _browser, SLOT(backward()),
				 ALT | Key_Left);
    _forwardId = go->insertItem(QPixmap(forward_image),
				tr("&Forward"), _browser, SLOT(forward()),
				ALT | Key_Right);
    _homeId = go->insertItem(QPixmap(home_image),
			     tr("&Home"), _browser, SLOT(home()));

    menuBar()->insertItem(tr("&File"), file);
    menuBar()->insertItem(tr("&Go"), go);

    menuBar()->setItemEnabled(_forwardId, FALSE);
    menuBar()->setItemEnabled(_backwardId, FALSE);
    connect(_browser, SIGNAL(backwardAvailable(bool)),
	    this, SLOT(setBackwardAvailable(bool)));
    connect(_browser, SIGNAL(forwardAvailable(bool)),
	    this, SLOT(setForwardAvailable(bool)));

    QToolBar* toolbar = new QToolBar(this);

    QToolButton* button;
    button = new QToolButton(QPixmap(back_image), tr("Backward"), "",
			     _browser, SLOT(backward()), toolbar);
    connect(_browser, SIGNAL(backwardAvailable(bool)), button,
	    SLOT(setEnabled(bool)));
    button->setEnabled(FALSE);

    button = new QToolButton(QPixmap(forward_image), tr("Forward"), "",
			     _browser, SLOT(forward()), toolbar);
    connect(_browser, SIGNAL(forwardAvailable(bool)), button,
	    SLOT(setEnabled(bool)));
    button->setEnabled(FALSE);

    button = new QToolButton(QPixmap(home_image), tr("Home"), "",
			     _browser, SLOT(home()), toolbar);
    toolbar->addSeparator();

    _browser->setFocus();
}

HelpWindow::~HelpWindow()
{
}

void
HelpWindow::setBackwardAvailable(bool b)
{
    menuBar()->setItemEnabled(_backwardId, b);
}

void
HelpWindow::setForwardAvailable(bool b)
{
    menuBar()->setItemEnabled(_forwardId, b);
}

void
HelpWindow::print()
{
    QPrinter printer;
    printer.setFullPage(TRUE);

    if (!printer.setup())
	return;

    QPainter p(&printer);
    QPaintDeviceMetrics metrics(p.device());
    int dpix = metrics.logicalDpiX();
    int dpiy = metrics.logicalDpiY();
    const int margin = 72; // pt
    QRect body(margin*dpix/72, margin*dpiy/72,
	       metrics.width()-margin*dpix/72*2,
	       metrics.height()-margin*dpiy/72*2);
    QFont font("times", 10);
    QSimpleRichText richText(_browser->text(), font, _browser->context(),
			     _browser->styleSheet(),
			     _browser->mimeSourceFactory(), body.height());
    richText.setWidth(&p, body.width());
    QRect view(body);
    int page = 1;
    do {
	richText.draw(&p, body.left(), body.top(), view, colorGroup());
	view.moveBy(0, body.height());
	p.translate(0 , -body.height());
	p.setFont(font);
	p.drawText(view.right() - p.fontMetrics().width(QString::number(page)),
		   view.bottom() + p.fontMetrics().ascent() + 5,
		   QString::number(page));
	if (view.top() >= richText.height())
	    break;
	printer.newPage();
	page++;
    } while (TRUE);
    p.end();
}

void
HelpWindow::pathSelected(const QString& path)
{
    _browser->setSource(path);
}
