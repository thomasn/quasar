// $Id: model_main_window.cpp,v 1.1 2004/11/29 17:38:00 bpepers Exp $
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

#include "model_main_window.h"
#include "server_config.h"
#include "model_edit.h"
#include "list_view_item.h"

#include <qapplication.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qwidgetlist.h>

ModelMainWindow::ModelMainWindow()
    : QMainWindow(0, "ModelMainWindow", WType_TopLevel | WDestructiveClose)
{
    createWidgets();
    slotRefresh();
}

ModelMainWindow::~ModelMainWindow()
{
}

void
ModelMainWindow::createWidgets()
{
    setCaption("Model Editor");
    menuBar();

    QFrame* main = new QFrame(this);
    QVBox* body = new QVBox(main);
    QVBox* right = new QVBox(main);

    _list = new ListView(body);
    _list->setAllColumnsShowFocus(true);
    _list->setShowSortIndicator(true);
    _list->addTextColumn("Version", 12);
    _list->addTextColumn("Description", 40);
    _list->setSorting(0, false);

    QHBox* buttons = new QHBox(body);

    _new = new QPushButton("&New Model", buttons);
    _edit = new QPushButton("&Edit Model", buttons);
    _remove = new QPushButton("&Remove Model", buttons);
    _edit->setEnabled(false);
    _remove->setEnabled(false);

    QPushButton* refresh = new QPushButton("&Refresh", right);
    QPushButton* exit = new QPushButton("&Exit", right);

    QGridLayout* grid = new QGridLayout(main);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(0, 1);
    grid->addWidget(body, 0, 0);
    grid->addWidget(right, 0, 1, AlignTop | AlignLeft);

    connect(refresh, SIGNAL(clicked()), this, SLOT(slotRefresh()));
    connect(exit, SIGNAL(clicked()), this, SLOT(close()));
    connect(_new, SIGNAL(clicked()), this, SLOT(slotNewModel()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(slotEditModel()));
    connect(_remove, SIGNAL(clicked()), this, SLOT(slotRemoveModel()));
    connect(_list, SIGNAL(doubleClicked(QListViewItem*)), this,
	    SLOT(slotEditModel()));
    connect(_list, SIGNAL(currentChanged(QListViewItem*)), this,
	    SLOT(slotCurrentModelChanged()));

    setCentralWidget(main);

    QPopupMenu* file = new QPopupMenu(this);
    file->insertItem(tr("E&xit"), this, SLOT(close()), ALT+Key_Q);
    menuBar()->insertItem(tr("&File"), file);
}

void
ModelMainWindow::slotNewModel()
{
    ServerConfig config;
    config.load();
    QString dataDir = parseDir(config.dataDir);

    DataModel model;
    model.version = QDate::currentDate().toString("yyyy-MM-dd");
    model.fromVersion = _latestVersion;
    model.description = "New Model";

    // Copy objects from previous version
    if (!model.fromVersion.isEmpty()) {
	DataModel from;
	from.load(dataDir + "/models/" + model.fromVersion + ".xml");
	model.objects = from.objects;
    }

    model.save(dataDir + "/models/" + model.version + ".xml");
    _models.push_back(model);
    _latestVersion = model.version;

    ListViewItem* lvi = new ListViewItem(_list);
    lvi->setValue(0, model.version);
    lvi->setValue(1, model.description);
    _list->setCurrentItem(lvi);
    _list->setSelected(lvi, true);

    slotCurrentModelChanged();
    slotEditModel();
}

void
ModelMainWindow::slotEditModel()
{
    if (currentModel().isEmpty()) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), "No model selected");
	return;
    }

    ModelEdit* edit = new ModelEdit(this, currentModel());
    edit->show();
}

void
ModelMainWindow::slotRemoveModel()
{
    if (currentModel().isEmpty()) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), "No model selected");
	return;
    }

    // Only allow remove of top of the model list
    if (currentModel() != _latestVersion) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), "Can only remove top model");
	return;
    }

    QString version = currentModel();
    QString message = "Are you sure you want to remove \"" + version + "\"";
    int result = QMessageBox::warning(this, "Warning", message,
				      QMessageBox::No, QMessageBox::Yes);
    if (result != QMessageBox::Yes) return;

    // Change list focus to item after or before removed
    QListViewItem* current = _list->currentItem();
    QListViewItem* next = current->itemBelow();
    if (next == NULL) next = current->itemAbove();

    // TODO: actually remove the file and the data from _models and
    // reset _latestVersion

    _list->setCurrentItem(next);
    _list->setSelected(next, true);
}

void
ModelMainWindow::slotCurrentModelChanged()
{
    QListViewItem* current = _list->currentItem();
    if (current != NULL) _list->setSelected(current, true);
    _edit->setEnabled(current != NULL);
    _remove->setEnabled(current != NULL);
}

void
ModelMainWindow::slotRefresh()
{
    QString oldVersion = currentModel();
    QListViewItem* current = NULL;

    ServerConfig config;
    config.load();

    _models.clear();
    _latestVersion = "";

    QString dataDir = parseDir(config.dataDir);
    QDir dir(dataDir + "/models", "*.xml");
    QStringList entries = dir.entryList();
    for (unsigned int i = 0; i < entries.size(); ++i) {
	QString filePath = dir.filePath(entries[i]);

	DataModel model;
	if (model.load(filePath))
	    _models.push_back(model);
    }

    _list->clear();
    for (unsigned int i = 0; i < _models.size(); ++i) {
	const DataModel& model = _models[i];

	ListViewItem* lvi = new ListViewItem(_list);
	lvi->setValue(0, model.version);
	lvi->setValue(1, model.description);

	if (model.version == oldVersion)
	    current = lvi;
	if (model.version > _latestVersion)
	    _latestVersion = model.version;
    }
    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
    slotCurrentModelChanged();
}

void
ModelMainWindow::closeEvent(QCloseEvent* e)
{
    // Try to close all ModelEdit windows
    QWidgetList* list = QApplication::topLevelWidgets();
    QWidgetListIt it(*list);
    QWidget* widget;
    while ((widget = it.current()) != NULL) {
        ++it;
	if (widget->className() != "ModelEdit") continue;
	if (!widget->close()) return;
    }
    delete list;

    QWidget::closeEvent(e);
}

QString
ModelMainWindow::currentModel()
{
    QString version;
    if (_list->currentItem() != NULL)
	version = _list->currentItem()->text(0);
    return version;
}
