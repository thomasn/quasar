// $Id: model_window.cpp,v 1.6 2004/11/29 17:35:58 bpepers Exp $
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

#include "model_window.h"
#include "object_edit.h"
#include "line_edit.h"
#include "multi_line_edit.h"
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

ModelWindow::ModelWindow()
    : QMainWindow(0, "ModelWindow", WType_TopLevel | WDestructiveClose)
{
    createWidgets();
}

ModelWindow::~ModelWindow()
{
}

void
ModelWindow::createWidgets()
{
    setCaption("Model Editor");
    menuBar();

    QFrame* main = new QFrame(this);
    QVBox* body = new QVBox(main);
    QFrame* top = new QFrame(body);
    QVBox* right = new QVBox(main);

    QLabel* versionLabel = new QLabel("&Version:", top);
    _version = new LineEdit(top);
    versionLabel->setBuddy(_version);

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(3);
    topGrid->setMargin(3);
    topGrid->setRowStretch(2, 1);
    topGrid->setColStretch(1, 1);
    topGrid->addWidget(versionLabel, 0, 0);
    topGrid->addWidget(_version, 0, 1, AlignLeft | AlignVCenter);

    _list = new ListView(body);
    _list->setAllColumnsShowFocus(true);
    _list->setShowSortIndicator(true);
    _list->addTextColumn("Object Name", 20);
    _list->addTextColumn("Description", 40);
    _list->addNumberColumn("Number", 8);

    QHBox* buttons = new QHBox(body);

    _add = new QPushButton("&Add Object", buttons);
    _edit = new QPushButton("&Edit Object", buttons);
    _remove = new QPushButton("&Remove Object", buttons);
    _edit->setEnabled(false);
    _remove->setEnabled(false);

    QPushButton* save = new QPushButton("&Save", right);
    QPushButton* exit = new QPushButton("&Exit", right);

    QGridLayout* grid = new QGridLayout(main);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(0, 1);
    grid->addWidget(body, 0, 0);
    grid->addWidget(right, 0, 1, AlignTop | AlignLeft);

    connect(save, SIGNAL(clicked()), this, SLOT(slotSave()));
    connect(exit, SIGNAL(clicked()), this, SLOT(close()));
    connect(_add, SIGNAL(clicked()), this, SLOT(slotAddObject()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(slotEditObject()));
    connect(_remove, SIGNAL(clicked()), this, SLOT(slotRemoveObject()));
    connect(_list, SIGNAL(doubleClicked(QListViewItem*)), this,
	    SLOT(slotEditObject()));
    connect(_list, SIGNAL(currentChanged(QListViewItem*)), this,
	    SLOT(slotCurrentObjectChanged()));

    setCentralWidget(main);

    _file = new QPopupMenu(this);
    connect(_file, SIGNAL(aboutToShow()), SLOT(slotFileMenu()));
    menuBar()->insertItem(tr("&File"), _file);
}

bool
ModelWindow::load(const QString& filename)
{
    DataModel model;
    if (!model.load(filename)) return false;

    _filename = filename;
    _orig = model;
    _curr = model;
    setWidgets();
    reloadObjects();

    return true;
}

void
ModelWindow::add(const BusinessObject& object)
{
    // Validate model with added object
    DataModel model = _curr;
    model.objects.push_back(object);
    //model.validate(false);

    // If valid then add to current
    _curr.objects.push_back(object);
    reloadObjects();
    emit modelAdd(object.name);
}

void
ModelWindow::update(const BusinessObject& orig, const BusinessObject& object)
{
    // Validate model after update
    DataModel model = _curr;
    model.objects.remove(orig);
    model.objects.push_back(object);
    //model.validate(false);

    // If valid then update current
    _curr.objects.remove(orig);
    _curr.objects.push_back(object);
    reloadObjects();
    emit modelUpdate(orig.name);
}

void
ModelWindow::remove(const BusinessObject& object)
{
    // Validate model without object
    DataModel model = _curr;
    model.objects.remove(object);
    //model.validate(false);

    // If valid then remove from current
    _curr.objects.remove(object);
    QListViewItem* lvi = _list->firstChild();
    while (lvi != NULL) {
	if (lvi->text(0) == object.name) {
	    delete lvi;
	    break;
	}
	lvi = lvi->itemBelow();
    }
    emit modelRemove(object.name);
}

void
ModelWindow::slotFileMenu()
{
    QPopupMenu* menu = (QPopupMenu*)sender();
    menu->clear();
    menu->insertItem(tr("&New..."), this, SLOT(slotNew()));
    menu->insertItem(tr("&Open..."), this, SLOT(slotOpen()));
    int id = menu->insertItem(tr("&Save..."), this, SLOT(slotSave()));
    menu->insertItem(tr("Save &As..."), this, SLOT(slotSaveAs()));
    menu->insertSeparator();
    menu->insertItem(tr("E&xit"), this, SLOT(close()), ALT+Key_Q);

    if (_filename.isEmpty())
	menu->setItemEnabled(id, false);
}

void
ModelWindow::slotNew()
{
    if (!checkChanged()) return;

    DataModel blank;
    _curr = blank;
    _orig = blank;
    setWidgets();
    reloadObjects();
}

void
ModelWindow::slotOpen()
{
    if (!checkChanged()) return;

    QString filename = QFileDialog::getOpenFileName(".", "*.xml", this);
    if (filename.isEmpty()) return;

    if (!load(filename)) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), tr("Error loading model"));
    }
}

bool
ModelWindow::slotSave()
{
    if (_filename.isEmpty())
	return slotSaveAs();

    getWidgets();
    if (!_curr.save(_filename)) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), tr("Saving model failed"));
	return false;
    }

    QString message = QString("Model \"%1\" saved").arg(_curr.version);
    QMessageBox::information(this, tr("Information"), message);

    _orig = _curr;
    return true;
}

bool
ModelWindow::slotSaveAs()
{
    QString filename = QFileDialog::getSaveFileName(".", "*.xml", this);
    if (filename.isEmpty()) return false;

    if (filename.right(4) != ".xml" && filename.find('.') == -1)
	filename = filename + ".xml";

    getWidgets();
    if (!_curr.save(filename)) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), tr("Saving model failed"));
	return false;
    }

    QString message = QString("Model \"%1\" saved").arg(_curr.version);
    QMessageBox::information(this, tr("Information"), message);

    _filename = filename;
    _orig = _curr;
    return true;
}

void
ModelWindow::slotAddObject()
{
    ObjectEdit* edit = new ObjectEdit(this);
    edit->show();
}

void
ModelWindow::slotEditObject()
{
    if (currentObject().isEmpty()) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), "No object selected");
	return;
    }

    ObjectEdit* edit = new ObjectEdit(this, currentObject());
    edit->show();
}

void
ModelWindow::slotRemoveObject()
{
    if (currentObject().isEmpty()) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), "No object selected");
	return;
    }

    QString name = currentObject();
    BusinessObject object;
    for (unsigned int i = 0; i < _curr.objects.size(); ++i)
	if (_curr.objects[i].name == name)
	    object = _curr.objects[i];

    QString message = "Are you sure you want to remove \"" + name + "\"";
    int result = QMessageBox::warning(this, "Warning", message,
				      QMessageBox::No, QMessageBox::Yes);
    if (result != QMessageBox::Yes) return;

    // Change list focus to item after or before removed
    QListViewItem* current = _list->currentItem();
    QListViewItem* next = current->itemBelow();
    if (next == NULL) next = current->itemAbove();

    remove(object);
    _list->setCurrentItem(next);
    _list->setSelected(next, true);
}

void
ModelWindow::slotCurrentObjectChanged()
{
    QListViewItem* current = _list->currentItem();
    if (current != NULL) _list->setSelected(current, true);
    _edit->setEnabled(current != NULL);
    _remove->setEnabled(current != NULL);
}

void
ModelWindow::closeEvent(QCloseEvent* e)
{
    if (!checkChanged()) return;
    QWidget::closeEvent(e);
}

bool
ModelWindow::checkChanged()
{
    getWidgets();

    if (_curr != _orig) {
	QString message = QString("Model \"%1\" has been changed.\n"
				  "Save changes?").arg(_curr.version);
	int result = QMessageBox::warning(this, "Warning", message,
					  QMessageBox::Yes, QMessageBox::No,
					  QMessageBox::Cancel);
	if (result == QMessageBox::Yes)
	    if (!slotSave()) return false;
	if (result == QMessageBox::Cancel)
	    return false;
    }

    return true;
}

QString
ModelWindow::currentObject()
{
    QString name;
    if (_list->currentItem() != NULL)
	name = _list->currentItem()->text(0);
    return name;
}

void
ModelWindow::setWidgets()
{
    _version->setText(_curr.version);
}

void
ModelWindow::getWidgets()
{
    _curr.version = _version->text();
}

void
ModelWindow::reloadObjects()
{
    QString oldName = currentObject();
    QListViewItem* current = NULL;

    _list->clear();
    for (unsigned int i = 0; i < _curr.objects.size(); ++i) {
	const BusinessObject& object = _curr.objects[i];

	ListViewItem* lvi = new ListViewItem(_list);
	lvi->setValue(0, object.name);
	lvi->setValue(1, object.description);
	lvi->setValue(2, object.objectNumber);

	if (object.name == oldName)
	    current = lvi;
    }
    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
    slotCurrentObjectChanged();
}
