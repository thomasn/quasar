// $Id: report_list.cpp,v 1.30 2005/02/17 09:55:55 bpepers Exp $
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

#include "report_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "param_dialog.h"
#include "report_viewer.h"
#include "report_defn.h"
#include "report_page_view.h"
#include "list_view_item.h"

#include <qapplication.h>
#include <qvbox.h>
#include <qsplitter.h>
#include <qwidgetstack.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qheader.h>
#include <qmessagebox.h>

ReportList::ReportList(MainWindow* main)
    : QuasarWindow(main, "ReportList")
{
    _helpSource = "report_list.html";

    QVBox* frame = new QVBox(this);
    QSplitter* split = new QSplitter(frame);
    QFrame* buttons = new QFrame(frame);

    _module = new ListView(split);
    _module->addTextColumn(tr("Module"), 20);
    _module->setSorting(-1);
    connect(_module, SIGNAL(selectionChanged()), SLOT(slotModuleChange()));

    _stack = new QWidgetStack(split);

    QPushButton* refresh = new QPushButton(tr("Refresh"), buttons);
    QPushButton* view = new QPushButton(tr("View"), buttons);
    QPushButton* close = new QPushButton(tr("Close"), buttons);

    connect(refresh, SIGNAL(clicked()), SLOT(slotRefresh()));
    connect(view, SIGNAL(clicked()), SLOT(slotView()));
    connect(close, SIGNAL(clicked()), SLOT(close()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setMargin(3);
    buttonGrid->setSpacing(3);
    buttonGrid->setColStretch(1, 1);
    buttonGrid->addWidget(refresh, 0, 0);
    buttonGrid->addWidget(view, 0, 2);
    buttonGrid->addWidget(close, 0, 3);

    setCaption(tr("Report List"));
    setCentralWidget(frame);

    clearData();
    finalize();

    slotRefresh();
}

ReportList::~ReportList()
{
}

void
ReportList::clearData()
{
    // Delete existing lists of reports
    for (unsigned int i = 0; i < _views.size(); ++i) {
	_stack->removeWidget(_views[i]);
	delete _views[i];
    }
    _views.clear();

    // Clear module list
    _module->clear();

    // Add modules in standard order
    findModule(tr("All"));
    findModule(tr("Card File"));
    findModule(tr("Ledger"));
    findModule(tr("Cheques"));
    findModule(tr("Inventory"));
    findModule(tr("Sales"));
    findModule(tr("Purchases"));

    // Clear info on reports
    _files.clear();
    _names.clear();
    _modules.clear();
}

void
ReportList::slotModuleChange()
{
    QListViewItem* item = _module->currentItem();
    if (item == NULL) return;

    QString name = item->text(0);
    for (unsigned int i = 0; i < _views.size(); ++i) {
	if (_views[i]->name() == name) {
	    _stack->raiseWidget(_views[i]);
	    _views[i]->setFocus();
	    return;
	}
    }
}

void
ReportList::slotRefresh()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    clearData();
    _quasar->resourceList("reports", ".name", ".module", _files, _names,
			  _modules);

    for (unsigned int i = 0; i < _names.size(); ++i) {
	ListView* list = findModule(_modules[i]);
	ListViewItem* item = new ListViewItem(list);
	item->extra.push_back(_files[i]);
	item->setValue(0, _names[i]);

	if (_modules[i] != tr("All")) {
	    list = findModule(tr("All"));
	    item = new ListViewItem(list);
	    item->extra.push_back(_files[i]);
	    item->setValue(0, _names[i]);
	    item->setValue(1, _modules[i]);
	}
    }

    if (_module->firstChild() != NULL) {
	_module->setSelected(_module->firstChild(), true);
	_module->setCurrentItem(_module->firstChild());
    }
    slotModuleChange();

    QApplication::restoreOverrideCursor();
}

void
ReportList::slotView()
{
    ListView* list = (ListView*)_stack->visibleWidget();
    if (list == NULL) return;
    ListViewItem* item = list->currentItem();
    if (item == NULL) return;

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    QString fileName = item->extra[0].toString();
    QString filePath;
    if (!_quasar->resourceFetch("reports", fileName, filePath)) {
	QApplication::restoreOverrideCursor();

	QString message = tr("Error fetching report '%1'").arg(fileName);
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    ReportDefn report;
    if (!report.load(filePath)) {
	QApplication::restoreOverrideCursor();

	QString message = tr("Report '%1' is invalid").arg(fileName);
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QApplication::restoreOverrideCursor();

    ParamMap params;
    if (!report.getParams(this, params))
	return;

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    ReportOutput* output = new ReportOutput();
    if (!report.generate(params, *output)) {
	QApplication::restoreOverrideCursor();
	delete output;
	QString message = tr("Report '%1' failed").arg(fileName);
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QApplication::restoreOverrideCursor();
    ReportViewer* view = new ReportViewer(report, params, output);
    view->show();
}

ListView*
ReportList::findModule(const QString& name)
{
    // Check if already exists
    for (unsigned int i = 0; i < _views.size(); ++i) {
	if (_views[i]->name() == name)
	    return _views[i];
    }

    // Add to modules list
    if (_module->lastItem() == NULL) {
	ListViewItem* item = new ListViewItem(_module);
	item->setText(0, name);
    } else {
	ListViewItem* last = (ListViewItem*)_module->lastItem();
	ListViewItem* item = new ListViewItem(_module, last);
	item->setText(0, name);
    }

    // Create new ListView
    ListView* list = new ListView(_stack, name);
    list->setAllColumnsShowFocus(true);
    list->setShowSortIndicator(true);
    connect(list, SIGNAL(doubleClicked(QListViewItem*)), SLOT(slotView()));

    // Set columns depending on whether its the all tab or not
    if (name == tr("All")) {
	list->addTextColumn(tr("Name"), 60);
	list->addTextColumn(tr("Module"), 17);
    } else {
	list->addTextColumn(tr("Name"), 80);
    }
    list->setSorting(0);

    _views.push_back(list);
    _stack->addWidget(list);

    return list;
}
