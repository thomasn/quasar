// $Id: report_viewer.cpp,v 1.24 2005/02/05 11:54:24 bpepers Exp $
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
//

#include "report_viewer.h"
#include "report_output.h"
#include "report_page_view.h"
#include "param_dialog.h"
#include "line_edit.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qinputdialog.h>

ReportViewer::ReportViewer(ReportDefn& report, ParamMap& params,
			   ReportOutput* output)
    : QMainWindow(0, "MainWindow", WType_TopLevel | WDestructiveClose),
      _report(report), _params(params), _output(output)
{
    QFrame* frame = new QFrame(this);

    QLabel* nameLabel = new QLabel(tr("Report:"), frame);
    _name = new LineEdit(frame);
    _name->setFocusPolicy(NoFocus);
    nameLabel->setBuddy(_name);

    QPushButton* gen = new QPushButton(tr("&Regenerate"), frame);

    _page = new ReportPageView(frame);
    _page->setFocusPolicy(StrongFocus);
    _page->setOutput(_output);
    _page->setZoom(1.2);

    QFrame* buttons = new QFrame(frame);
    QPushButton* first = new QPushButton(tr("First"), buttons);
    QPushButton* prev = new QPushButton(tr("Prev"), buttons);
    QPushButton* next = new QPushButton(tr("Next"), buttons);
    QPushButton* last = new QPushButton(tr("Last"), buttons);
    QPushButton* pick = new QPushButton(tr("Pick"), buttons);
    _pageLabel = new QLabel(buttons);
    QPushButton* print = new QPushButton(tr("&Print"), buttons);
    QPushButton* close = new QPushButton(tr("&Close"), buttons);

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(3);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(5, 1);
    buttonGrid->setColStretch(7, 1);
    buttonGrid->addWidget(first, 0, 0);
    buttonGrid->addWidget(prev, 0, 1);
    buttonGrid->addWidget(next, 0, 2);
    buttonGrid->addWidget(last, 0, 3);
    buttonGrid->addWidget(pick, 0, 4);
    buttonGrid->addWidget(_pageLabel, 0, 6);
    buttonGrid->addWidget(print, 0, 8);
    buttonGrid->addWidget(close, 0, 9);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(1, 1);
    grid->setColStretch(1, 1);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1);
    grid->addWidget(gen, 0, 2);
    grid->addMultiCellWidget(_page, 1, 1, 0, 2);
    grid->addMultiCellWidget(buttons, 2, 2, 0, 2);

    connect(gen, SIGNAL(clicked()), SLOT(regenerate()));
    connect(first, SIGNAL(clicked()), SLOT(firstPage()));
    connect(prev, SIGNAL(clicked()), SLOT(prevPage()));
    connect(next, SIGNAL(clicked()), SLOT(nextPage()));
    connect(last, SIGNAL(clicked()), SLOT(lastPage()));
    connect(pick, SIGNAL(clicked()), SLOT(pickPage()));
    connect(print, SIGNAL(clicked()), SLOT(print()));
    connect(close, SIGNAL(clicked()), SLOT(close()));

    _name->setText(report.name);
    _page->setFocus();
    setPageLabel();

    setCaption("Report Viewer");
    setCentralWidget(frame);
}

ReportViewer::~ReportViewer()
{
    delete _output;
}

void
ReportViewer::setPageLabel()
{
    int page = _page->pageNumber();
    int pageCount = _output == NULL ? 1 : _output->pages.size();
    if (pageCount == 0) page = 0;
    _pageLabel->setText(tr("Page %1 of %2").arg(page).arg(pageCount));
}

void
ReportViewer::firstPage()
{
    _page->firstPage();
    setPageLabel();
}

void
ReportViewer::prevPage()
{
    _page->prevPage();
    setPageLabel();
}

void
ReportViewer::nextPage()
{
    _page->nextPage();
    setPageLabel();
}

void
ReportViewer::lastPage()
{
    _page->lastPage();
    setPageLabel();
}

void
ReportViewer::pickPage()
{
    if (_output == NULL) return;

    QString caption = tr("Pick a page");
    QString label = tr("Enter the page number:");
    int current = _page->pageNumber();
    int pageCount = _output->pages.size();
    bool ok;
    int page = QInputDialog::getInteger(caption, label, current, 1,
					pageCount, 1, &ok, this);
    if (!ok) return;

    _page->setPageNumber(page);
    _page->setContentsPos(0, 0);
    setPageLabel();
}

void
ReportViewer::keyPressEvent(QKeyEvent* e)
{
    switch (e->key()) {
    case Key_Home:	firstPage(); break;
    case Key_PageUp:	pageUp(); break;
    case Key_PageDown:	pageDown(); break;
    case Key_End:	lastPage(); break;
    case Key_Up:	lineUp(); break;
    case Key_Down:	lineDown(); break;
    case Key_Plus:	zoomOut(); break;
    case Key_Minus:	zoomIn(); break;
    default:
	QMainWindow::keyPressEvent(e);
    }
}

void
ReportViewer::pageUp()
{
    if (_output == NULL) return;

    int contentsHeight = _page->contentsHeight();
    int contentsX = _page->contentsX();
    int contentsY = _page->contentsY();
    int viewportHeight = _page->viewport()->height();

    if (contentsY == 0) {
	if (_page->pageNumber() > 1) {
	    _page->prevPage();
	    setPageLabel();

	    contentsY = contentsHeight - viewportHeight;
	    if (contentsY < 0) contentsY = 0;
	    _page->setContentsPos(contentsX, contentsY);
	}
    } else {
	contentsY -= viewportHeight - 20;
	if (contentsY < 0) contentsY = 0;
	_page->setContentsPos(contentsX, contentsY);
    }
}

void
ReportViewer::pageDown()
{
    if (_output == NULL) return;

    int contentsHeight = _page->contentsHeight();
    int contentsX = _page->contentsX();
    int contentsY = _page->contentsY();
    int viewportHeight = _page->viewport()->height();

    if (contentsY + viewportHeight >= contentsHeight) {
	if (_page->pageNumber() < int(_output->pages.size())) {
	    _page->nextPage();
	    setPageLabel();
	    _page->setContentsPos(contentsX, 0);
	}
    } else {
	contentsY += viewportHeight - 20;
	if (contentsY + viewportHeight > contentsHeight)
	    contentsY = contentsHeight - viewportHeight;
	_page->setContentsPos(contentsX, contentsY);
    }
}

void
ReportViewer::lineUp()
{
    if (_output == NULL) return;

    int contentsHeight = _page->contentsHeight();
    int contentsX = _page->contentsX();
    int contentsY = _page->contentsY();
    int viewportHeight = _page->viewport()->height();

    if (contentsY == 0) {
	if (_page->pageNumber() > 1) {
	    _page->prevPage();
	    setPageLabel();

	    contentsY = contentsHeight - viewportHeight;
	    if (contentsY < 0) contentsY = 0;
	    _page->setContentsPos(contentsX, contentsY);
	}
    } else {
	contentsY -= 20;
	if (contentsY < 0) contentsY = 0;
	_page->setContentsPos(contentsX, contentsY);
    }
}

void
ReportViewer::lineDown()
{
    if (_output == NULL) return;

    int contentsHeight = _page->contentsHeight();
    int contentsX = _page->contentsX();
    int contentsY = _page->contentsY();
    int viewportHeight = _page->viewport()->height();

    if (contentsY + viewportHeight >= contentsHeight) {
	if (_page->pageNumber() < int(_output->pages.size())) {
	    _page->nextPage();
	    setPageLabel();
	    _page->setContentsPos(contentsX, 0);
	}
    } else {
	contentsY += 20;
	if (contentsY + viewportHeight > contentsHeight)
	    contentsY = contentsHeight - viewportHeight;
	_page->setContentsPos(contentsX, contentsY);
    }
}

void
ReportViewer::zoomIn()
{
    _page->setZoom(_page->zoom() + .2);
}

void
ReportViewer::zoomOut()
{
    _page->setZoom(_page->zoom() - .2);
}

void
ReportViewer::regenerate()
{
    if (!_report.getParams(this, _params))
	return;

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    ReportOutput* output = new ReportOutput();
    if (!_report.generate(_params, *output)) {
	QApplication::restoreOverrideCursor();
	delete output;
	QString message = tr("Report generate failed");
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QApplication::restoreOverrideCursor();

    delete _output;
    _output = output;
    _page->setOutput(_output);
    setPageLabel();
}

void
ReportViewer::print()
{
    if (_output != NULL)
	_output->print(this);
}
