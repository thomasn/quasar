// $Id: model_edit.cpp,v 1.2 2004/11/30 03:52:56 bpepers Exp $
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

#include "model_edit.h"
#include "model_main_window.h"
#include "server_config.h"
#include "object_edit.h"
#include "line_edit.h"
#include "multi_line_edit.h"
#include "list_view_item.h"

#include <qapplication.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qmessagebox.h>
#include <qfile.h>

ModelEdit::ModelEdit(ModelMainWindow* main, const QString& version)
    : QMainWindow(NULL, "ModelEdit", WDestructiveClose), _main(main)
{
    createWidgets();

    ServerConfig config;
    config.load();
    QString dataDir = parseDir(config.dataDir);
    _orig.load(dataDir + "/models/" + version + ".xml");

    _curr = _orig;
    setWidgets();
}

ModelEdit::~ModelEdit()
{
}

void
ModelEdit::createWidgets()
{
    setCaption("Model Edit");
    menuBar();

    QFrame* main = new QFrame(this);
    QVBox* body = new QVBox(main);
    QFrame* top = new QFrame(body);
    QVBox* right = new QVBox(main);

    QLabel* versionLabel = new QLabel("&Version:", top);
    _version = new LineEdit(top);
    versionLabel->setBuddy(_version);

    QLabel* descLabel = new QLabel("&Description:", top);
    _desc = new MultiLineEdit(top);
    descLabel->setBuddy(_desc);

    QLabel* fromLabel = new QLabel("&From:", top);
    _from = new LineEdit(top);
    fromLabel->setBuddy(_from);

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(3);
    topGrid->setMargin(3);
    topGrid->setRowStretch(2, 1);
    topGrid->setColStretch(1, 1);
    topGrid->addWidget(versionLabel, 0, 0);
    topGrid->addWidget(_version, 0, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(descLabel, 1, 0);
    topGrid->addMultiCellWidget(_desc, 1, 2, 1, 1);
    topGrid->addWidget(fromLabel, 3, 0);
    topGrid->addWidget(_from, 3, 1, AlignLeft | AlignVCenter);

    QTabWidget* tabs = new QTabWidget(body);
    QVBox* objects = new QVBox(tabs);
    QVBox* updates = new QVBox(tabs);
    tabs->addTab(objects, tr("Objects"));
    tabs->addTab(updates, tr("Updates"));

    _objects = new ListView(objects);
    _objects->setAllColumnsShowFocus(true);
    _objects->setShowSortIndicator(true);
    _objects->addTextColumn("Name", 20);
    _objects->addTextColumn("Number", 10);
    _objects->addTextColumn("Description", 40);

    QHBox* objectCmds = new QHBox(objects);
    QPushButton* addObject = new QPushButton("Add Object", objectCmds);
    QPushButton* editObject = new QPushButton("Edit Object", objectCmds);
    QPushButton* removeObject = new QPushButton("Remove Object", objectCmds);

    _updates = new ListView(updates);
    _updates->setAllColumnsShowFocus(true);
    _updates->setShowSortIndicator(true);
    _updates->addTextColumn("Number", 10);
    _updates->addTextColumn("Description", 60);

    QHBox* updateCmds = new QHBox(updates);
    QPushButton* addUpdate = new QPushButton("Add Update", updateCmds);
    QPushButton* editUpdate = new QPushButton("Edit Update", updateCmds);
    QPushButton* removeUpdate = new QPushButton("Remove Update", updateCmds);

    QPushButton* ok = new QPushButton("&Ok", right);
    QPushButton* reset = new QPushButton("&Reset", right);
    QPushButton* cancel = new QPushButton("&Cancel", right);

    QGridLayout* grid = new QGridLayout(main);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(0, 1);
    grid->addWidget(body, 0, 0);
    grid->addWidget(right, 0, 1, AlignTop | AlignLeft);

    connect(ok, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(reset, SIGNAL(clicked()), this, SLOT(slotReset()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(slotCancel()));
    connect(addObject, SIGNAL(clicked()), this, SLOT(slotAddObject()));
    connect(editObject, SIGNAL(clicked()), this, SLOT(slotEditObject()));
    connect(removeObject, SIGNAL(clicked()), this, SLOT(slotRemoveObject()));
    connect(_objects, SIGNAL(doubleClicked(QListViewItem*)), this,
	    SLOT(slotEditObject()));
    connect(addUpdate, SIGNAL(clicked()), this, SLOT(slotAddObject()));
    connect(editUpdate, SIGNAL(clicked()), this, SLOT(slotEditObject()));
    connect(removeUpdate, SIGNAL(clicked()), this, SLOT(slotRemoveObject()));
    connect(_updates, SIGNAL(doubleClicked(QListViewItem*)), this,
	    SLOT(slotEditUpdate()));

    setCentralWidget(main);

    QPopupMenu* file = new QPopupMenu(this);
    file->insertItem(tr("&Ok"), this, SLOT(slotOk()));
    file->insertItem(tr("&Reset"), this, SLOT(slotReset()));
    file->insertSeparator();
    file->insertItem(tr("&Cancel"), this, SLOT(slotCancel()), ALT+Key_Q);
    menuBar()->insertItem(tr("&File"), file);
}

void
ModelEdit::insert(const BusinessObject& object)
{
    _curr.objects.push_back(object);

    // Update list
    ListViewItem* lvi = new ListViewItem(_objects);
    lvi->setText(0, object.name);
    if (object.number != -1)
	lvi->setValue(1, object.number);
    lvi->setText(2, object.description);
    _objects->setCurrentItem(lvi);
    _objects->setSelected(lvi, true);
}

void
ModelEdit::update(const BusinessObject& orig, const BusinessObject& object)
{
    // Update object
    for (unsigned int i = 0; i < _curr.objects.size(); ++i) {
	if (_curr.objects[i].name == orig.name) {
	    _curr.objects[i] = object;
	    break;
	}
    }

    // Update list
    ListViewItem* lvi = _objects->firstChild();
    while (lvi != NULL) {
	if (lvi->text(0) == orig.name) {
	    lvi->setText(0, object.name);
	    if (object.number == -1)
		lvi->setText(1, "");
	    else
		lvi->setValue(1, object.number);
	    lvi->setText(2, object.description);
	    break;
	}
	lvi = lvi->itemBelow();
    }
}

void
ModelEdit::remove(const BusinessObject& object)
{
    // Remove from current
    _curr.objects.remove(object);

    // Remove from list
    QListViewItem* lvi = _objects->firstChild();
    while (lvi != NULL) {
	if (lvi->text(0) == object.name) {
	    delete lvi;
	    break;
	}
	lvi = lvi->itemBelow();
    }
}

bool
ModelEdit::slotOk()
{
    getWidgets();

    if (_curr != _orig) {
	ServerConfig config;
	config.load();
	QString dataDir = parseDir(config.dataDir);
	_curr.save(dataDir + "/models/" + _curr.version + ".xml");

	if (_curr.version != _orig.version)
	    QFile::remove(dataDir + "/models/" + _orig.version + ".xml");

	// TODO: inform model_main_window of changes
    }

    _orig = _curr;
    close();
    return true;
}

void
ModelEdit::slotReset()
{
    getWidgets();

    if (_curr != _orig) {
	QString msg = QString("Model \"%1\" has been changed. Are you\n"
			      "sure you want to reset?").arg(_orig.version);
	int result = QMessageBox::warning(this, "Warning", msg,
					  QMessageBox::No, QMessageBox::Yes);
	if (result != QMessageBox::Yes)
	    return;
    }

    _curr = _orig;
    setWidgets();
}

void
ModelEdit::slotCancel()
{
    close();
}

void
ModelEdit::slotAddObject()
{
    ObjectEdit* edit = new ObjectEdit(this, BusinessObject());
    edit->show();
    _objectEdits.push_back(edit);
}

void
ModelEdit::slotEditObject()
{
    BusinessObject* object = currentObject();
    if (object == NULL) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), "No object selected");
	return;
    }

    ObjectEdit* edit = new ObjectEdit(this, *object);
    edit->show();
    _objectEdits.push_back(edit);
}

void
ModelEdit::slotRemoveObject()
{
    BusinessObject* object = currentObject();
    if (object == NULL) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), "No object selected");
	return;
    }

    QString name = object->name;
    QString message = "Are you sure you want to remove \"" + name + "\"";
    int result = QMessageBox::warning(this, "Warning", message,
				      QMessageBox::No, QMessageBox::Yes);
    if (result != QMessageBox::Yes) return;

    remove(*object);
}

void
ModelEdit::slotAddUpdate()
{
    // TODO: add new update
}

void
ModelEdit::slotEditUpdate()
{
    // TODO: edit existing update
}

void
ModelEdit::slotRemoveUpdate()
{
    // TODO: remove existing update
}

void
ModelEdit::closeEvent(QCloseEvent* e)
{
    // Ask all children to close
    for (unsigned int i = 0; i < _objectEdits.size(); ++i) {
	ObjectEdit* edit = _objectEdits[i];
	if (edit == NULL) continue;
	if (!edit->close()) return;
    }

    if (!checkChanged()) return;
    QWidget::closeEvent(e);
}

bool
ModelEdit::checkChanged()
{
    getWidgets();

    if (_curr != _orig) {
	QString message = QString("Model \"%1\" has been changed.\n"
				  "Save changes?").arg(_curr.version);
	int result = QMessageBox::warning(this, "Warning", message,
					  QMessageBox::Yes, QMessageBox::No,
					  QMessageBox::Cancel);
	if (result == QMessageBox::Yes)
	    if (!slotOk()) return false;
	if (result == QMessageBox::Cancel)
	    return false;
    }

    return true;
}

void
ModelEdit::setWidgets()
{
    _version->setText(_curr.version);
    _desc->setText(_curr.description);
    _from->setText(_curr.fromVersion);
    refreshObjects();
    refreshUpdates();
}

void
ModelEdit::getWidgets()
{
    _curr.version = _version->text();
    _curr.description = _desc->text();
    _curr.fromVersion = _from->text();
}

void
ModelEdit::refreshObjects()
{
    BusinessObject* oldObject = currentObject();
    QListViewItem* current = NULL;

    _objects->clear();
    for (unsigned int i = 0; i < _curr.objects.size(); ++i) {
	const BusinessObject& object = _curr.objects[i];

	ListViewItem* lvi = new ListViewItem(_objects);
	lvi->setValue(0, object.name);
	if (object.number != -1)
	    lvi->setValue(1, object.number);
	lvi->setValue(2, object.description);

	if (oldObject != NULL && object.name == oldObject->name)
	    current = lvi;
    }

    if (current == NULL) current = _objects->firstChild();
    _objects->setCurrentItem(current);
    _objects->setSelected(current, true);
}

void
ModelEdit::refreshUpdates()
{
    UpdateDefn* oldUpdate = currentUpdate();
    QListViewItem* current = NULL;

    _updates->clear();
    for (unsigned int i = 0; i < _curr.updates.size(); ++i) {
	const UpdateDefn& update = _curr.updates[i];

	ListViewItem* lvi = new ListViewItem(_updates);
	lvi->setValue(0, update.number);
	lvi->setValue(1, update.description);

	if (oldUpdate != NULL && update.number == oldUpdate->number)
	    current = lvi;
    }

    if (current == NULL) current = _updates->firstChild();
    _updates->setCurrentItem(current);
    _updates->setSelected(current, true);
}

BusinessObject*
ModelEdit::currentObject()
{
    if (_objects->currentItem() == NULL)
	return NULL;

    QString name = _objects->currentItem()->text(0);
    for (unsigned int i = 0; i < _curr.objects.size(); ++i)
	if (_curr.objects[i].name == name)
	    return &_curr.objects[i];

    return NULL;
}

UpdateDefn*
ModelEdit::currentUpdate()
{
    if (_updates->currentItem() == NULL)
	return NULL;

    QString number = _updates->currentItem()->text(0);
    for (unsigned int i = 0; i < _curr.updates.size(); ++i)
	if (_curr.updates[i].number == number)
	    return &_curr.updates[i];

    return NULL;
}
