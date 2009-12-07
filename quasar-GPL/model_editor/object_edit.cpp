// $Id: object_edit.cpp,v 1.12 2004/11/30 03:52:56 bpepers Exp $
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

#include "object_edit.h"
#include "model_edit.h"
#include "table_edit.h"
#include "integer_edit.h"
#include "multi_line_edit.h"
#include "list_view_item.h"

#include <qapplication.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qmessagebox.h>

ObjectEdit::ObjectEdit(ModelEdit* edit, const BusinessObject& object)
    : QMainWindow(NULL, "ObjectEdit", WDestructiveClose), _edit(edit)
{
    createWidgets();
    _orig = object;
    _curr = object;
    setWidgets();
}

ObjectEdit::~ObjectEdit()
{
}

void
ObjectEdit::createWidgets()
{
    setCaption("Object Edit");
    menuBar();

    QFrame* main = new QFrame(this);
    QVBox* body = new QVBox(main);
    QFrame* top = new QFrame(body);
    QVBox* right = new QVBox(main);

    QLabel* nameLabel = new QLabel("&Name:", top);
    _name = new LineEdit(top);
    nameLabel->setBuddy(_name);

    QLabel* objectNumLabel = new QLabel("N&umber:", top);
    _objectNum = new IntegerEdit(top);
    objectNumLabel->setBuddy(_objectNum);

    QLabel* descLabel = new QLabel("&Description:", top);
    _desc = new MultiLineEdit(top);
    descLabel->setBuddy(_desc);

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(3);
    topGrid->setMargin(3);
    topGrid->setRowStretch(2, 1);
    topGrid->setColStretch(2, 1);
    topGrid->addWidget(nameLabel, 0, 0);
    topGrid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(objectNumLabel, 0, 3);
    topGrid->addWidget(_objectNum, 0, 4, AlignLeft | AlignVCenter);
    topGrid->addWidget(descLabel, 1, 0);
    topGrid->addMultiCellWidget(_desc, 1, 2, 1, 4);

    _list = new ListView(body);
    _list->setAllColumnsShowFocus(true);
    _list->setShowSortIndicator(true);
    _list->addTextColumn("Table Name", 20);
    _list->addTextColumn("Description", 40);

    QHBox* buttons = new QHBox(body);
    QPushButton* add = new QPushButton("&Add Table", buttons);
    QPushButton* edit = new QPushButton("&Edit Table", buttons);
    QPushButton* remove = new QPushButton("&Remove Table", buttons);

    QPushButton* ok = new QPushButton("&Ok", right);
    QPushButton* next = new QPushButton("&Next", right);
    QPushButton* reset = new QPushButton("&Reset", right);
    QPushButton* cancel = new QPushButton("&Cancel", right);

    QGridLayout* grid = new QGridLayout(main);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(0, 1);
    grid->addWidget(body, 0, 0);
    grid->addWidget(right, 0, 1, AlignTop | AlignLeft);

    connect(ok, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(next, SIGNAL(clicked()), this, SLOT(slotNext()));
    connect(reset, SIGNAL(clicked()), this, SLOT(slotReset()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(slotCancel()));
    connect(add, SIGNAL(clicked()), this, SLOT(slotAddTable()));
    connect(edit, SIGNAL(clicked()), this, SLOT(slotEditTable()));
    connect(remove, SIGNAL(clicked()), this, SLOT(slotRemoveTable()));
    connect(_list, SIGNAL(doubleClicked(QListViewItem*)), this,
	    SLOT(slotEditTable()));

    setCentralWidget(main);

    QPopupMenu* file = new QPopupMenu(this);
    file->insertItem(tr("&Ok"), this, SLOT(slotOk()));
    file->insertItem(tr("&Next"), this, SLOT(slotNext()));
    file->insertItem(tr("&Reset"), this, SLOT(slotReset()));
    file->insertSeparator();
    file->insertItem(tr("&Cancel"), this, SLOT(slotCancel()), ALT+Key_Q);
    menuBar()->insertItem(tr("&File"), file);
}

void
ObjectEdit::insert(const TableDefn& table)
{
    _curr.tables.push_back(table);

    // Update list
    ListViewItem* lvi = new ListViewItem(_list);
    lvi->setText(0, table.name);
    lvi->setText(1, table.description);
    _list->setCurrentItem(lvi);
    _list->setSelected(lvi, true);
}

void
ObjectEdit::update(const TableDefn& orig, const TableDefn& table)
{
    // Update table
    for (unsigned int i = 0; i < _curr.tables.size(); ++i) {
	if (_curr.tables[i].name == orig.name) {
	    _curr.tables[i] = table;
	    break;
	}
    }

    // Update list
    QListViewItem* lvi = _list->firstChild();
    while (lvi != NULL) {
	if (lvi->text(0) == orig.name) {
	    lvi->setText(0, table.name);
	    lvi->setText(1, table.description);
	    break;
	}
	lvi = lvi->itemBelow();
    }
}

void
ObjectEdit::remove(const TableDefn& table)
{
    // Remove from current
    _curr.tables.remove(table);

    // Remove from list
    QListViewItem* lvi = _list->firstChild();
    while (lvi != NULL) {
	if (lvi->text(0) == table.name) {
	    delete lvi;
	    break;
	}
	lvi = lvi->itemBelow();
    }
}

bool
ObjectEdit::slotOk()
{
    getWidgets();

    if (_curr != _orig) {
	if (_orig.name.isEmpty())
	    _edit->insert(_curr);
	else
	    _edit->update(_orig, _curr);
    }

    _orig = _curr;
    close();
    return true;
}

void
ObjectEdit::slotNext()
{
    getWidgets();

    if (_curr != _orig) {
	if (_orig.name.isEmpty())
	    _edit->insert(_curr);
	else
	    _edit->update(_orig, _curr);
    }

    _curr.clear();
    _orig.clear();

    setWidgets();
    _name->setFocus();
}

void
ObjectEdit::slotReset()
{
    getWidgets();

    if (_curr != _orig) {
	QString message = QString("Object \"%1\" has been changed. Are you\n"
				  "sure you want to reset?").arg(_orig.name);
	int result = QMessageBox::warning(this, "Warning", message,
					  QMessageBox::No, QMessageBox::Yes);
	if (result != QMessageBox::Yes)
	    return;
    }

    _curr = _orig;
    setWidgets();
}

void
ObjectEdit::slotCancel()
{
    close();
}

void
ObjectEdit::slotAddTable()
{
    TableEdit* edit = new TableEdit(this, TableDefn());
    edit->show();
    _tableEdits.push_back(edit);
}

void
ObjectEdit::slotEditTable()
{
    TableDefn* table = currentTable();
    if (table == NULL) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), "No table selected");
	return;
    }

    TableEdit* edit = new TableEdit(this, *table);
    edit->show();
    _tableEdits.push_back(edit);
}

void
ObjectEdit::slotRemoveTable()
{
    TableDefn* table = currentTable();
    if (table == NULL) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), "No table selected");
	return;
    }

    QString name = table->name;
    QString message = "Are you sure you want to remove \"" + name + "\"";
    int result = QMessageBox::warning(this, "Warning", message,
				      QMessageBox::No, QMessageBox::Yes);
    if (result != QMessageBox::Yes) return;

    remove(*table);
}

void
ObjectEdit::closeEvent(QCloseEvent* e)
{
    // Ask all children to close
    for (unsigned int i = 0; i < _tableEdits.size(); ++i) {
	TableEdit* edit = _tableEdits[i];
	if (edit == NULL) continue;
	if (!edit->close()) return;
    }

    if (!checkChanged()) return;
    QWidget::closeEvent(e);
}

bool
ObjectEdit::checkChanged()
{
    getWidgets();

    if (_curr != _orig) {
	QString message = QString("Object \"%1\" has been changed.\n"
				  "Save changes?").arg(_curr.name);
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
ObjectEdit::setWidgets()
{
    _name->setText(_curr.name);
    if (_curr.number == -1)
	_objectNum->setText("");
    else
	_objectNum->setInt(_curr.number);
    _desc->setText(_curr.description);
    refreshTables();
}

void
ObjectEdit::getWidgets()
{
    _curr.name = _name->text();
    if (_objectNum->text().isEmpty())
	_curr.number = -1;
    else
	_curr.number = _objectNum->getInt();
    _curr.description = _desc->text();
}

void
ObjectEdit::refreshTables()
{
    TableDefn* oldTable = currentTable();
    QListViewItem* current = NULL;

    _list->clear();
    for (unsigned int i = 0; i < _curr.tables.size(); ++i) {
	const TableDefn& table = _curr.tables[i];

	ListViewItem* lvi = new ListViewItem(_list);
	lvi->setText(0, table.name);
	lvi->setText(1, table.description);

	if (oldTable != NULL && table.name == oldTable->name)
	    current = lvi;
    }

    if (current == NULL) current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

TableDefn*
ObjectEdit::currentTable()
{
    if (_list->currentItem() == NULL)
	return NULL;

    QString name = _list->currentItem()->text(0);
    for (unsigned int i = 0; i < _curr.tables.size(); ++i)
	if (_curr.tables[i].name == name)
	    return &_curr.tables[i];

    return NULL;
}
