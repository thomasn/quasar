// $Id: table_edit.cpp,v 1.4 2004/12/19 09:12:29 bpepers Exp $
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

#include "table_edit.h"
#include "object_edit.h"
#include "column_edit.h"
#include "constraint_edit.h"
#include "index_edit.h"
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

TableEdit::TableEdit(ObjectEdit* edit, const TableDefn& table)
    : QMainWindow(NULL, "TableEdit", WDestructiveClose), _edit(edit)
{
    createWidgets();
    _orig = table;
    _curr = table;
    setWidgets();
}

TableEdit::~TableEdit()
{
}

void
TableEdit::createWidgets()
{
    setCaption("Table Edit");
    menuBar();

    QFrame* main = new QFrame(this);
    QVBox* body = new QVBox(main);
    QFrame* top = new QFrame(body);
    QVBox* right = new QVBox(main);

    QLabel* nameLabel = new QLabel("&Name:", top);
    _name = new LineEdit(top);
    nameLabel->setBuddy(_name);

    QLabel* descLabel = new QLabel("&Description:", top);
    _desc = new MultiLineEdit(top);
    descLabel->setBuddy(_desc);

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(3);
    topGrid->setMargin(3);
    topGrid->setRowStretch(2, 1);
    topGrid->setColStretch(1, 1);
    topGrid->addWidget(nameLabel, 0, 0);
    topGrid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(descLabel, 1, 0);
    topGrid->addMultiCellWidget(_desc, 1, 2, 1, 1);

    QTabWidget* tabs = new QTabWidget(body);
    QVBox* columns = new QVBox(tabs);
    QVBox* constraints = new QVBox(tabs);
    QVBox* indexes = new QVBox(tabs);
    tabs->addTab(columns, tr("Columns"));
    tabs->addTab(constraints, tr("Constraints"));
    tabs->addTab(indexes, tr("Indexes"));

    _columns = new ListView(columns);
    _columns->setAllColumnsShowFocus(true);
    _columns->setShowSortIndicator(true);
    _columns->addTextColumn("Name", 20);
    _columns->addTextColumn("Description", 40);

    QHBox* columnButtons = new QHBox(columns);
    QPushButton* columnAdd = new QPushButton("&Add Column", columnButtons);
    QPushButton* columnEdit = new QPushButton("&Edit Column", columnButtons);
    QPushButton* columnDel = new QPushButton("&Remove Column", columnButtons);

    _constraints = new ListView(constraints);
    _constraints->setAllColumnsShowFocus(true);
    _constraints->setShowSortIndicator(true);
    _constraints->addTextColumn("Name", 20);
    _constraints->addTextColumn("Description", 40);

    QHBox* constButtons = new QHBox(constraints);
    QPushButton* constAdd = new QPushButton("&Add Constraint", constButtons);
    QPushButton* constEdit = new QPushButton("&Edit Constraint", constButtons);
    QPushButton* constDel = new QPushButton("&Remove Constraint",constButtons);

    _indexes = new ListView(indexes);
    _indexes->setAllColumnsShowFocus(true);
    _indexes->setShowSortIndicator(true);
    _indexes->addTextColumn("Name", 20);
    _indexes->addTextColumn("Description", 40);

    QHBox* indexButtons = new QHBox(indexes);
    QPushButton* indexAdd = new QPushButton("&Add Index", indexButtons);
    QPushButton* indexEdit = new QPushButton("&Edit Index", indexButtons);
    QPushButton* indexDel = new QPushButton("&Remove Index", indexButtons);

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
    connect(columnAdd, SIGNAL(clicked()), this, SLOT(slotAddColumn()));
    connect(columnEdit, SIGNAL(clicked()), this, SLOT(slotEditColumn()));
    connect(columnDel, SIGNAL(clicked()), this, SLOT(slotRemoveColumn()));
    connect(_columns, SIGNAL(doubleClicked(QListViewItem*)), this,
	    SLOT(slotEditColumn()));
    connect(constAdd, SIGNAL(clicked()), this, SLOT(slotAddConstraint()));
    connect(constEdit, SIGNAL(clicked()), this, SLOT(slotEditConstraint()));
    connect(constDel, SIGNAL(clicked()), this, SLOT(slotRemoveConstraint()));
    connect(_constraints, SIGNAL(doubleClicked(QListViewItem*)), this,
	    SLOT(slotEditConstraint()));
    connect(indexAdd, SIGNAL(clicked()), this, SLOT(slotAddIndex()));
    connect(indexEdit, SIGNAL(clicked()), this, SLOT(slotEditIndex()));
    connect(indexDel, SIGNAL(clicked()), this, SLOT(slotRemoveIndex()));
    connect(_indexes, SIGNAL(doubleClicked(QListViewItem*)), this,
	    SLOT(slotEditIndex()));

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
TableEdit::insert(const ColumnDefn& column)
{
    _curr.columns.push_back(column);

    // Update list
    ListViewItem* lvi = new ListViewItem(_columns);
    lvi->setText(0, column.name);
    lvi->setText(1, column.description);
    _columns->setCurrentItem(lvi);
    _columns->setSelected(lvi, true);
}

void
TableEdit::update(const ColumnDefn& orig, const ColumnDefn& column)
{
    // Update column
    for (unsigned int i = 0; i < _curr.columns.size(); ++i) {
	if (_curr.columns[i].name == orig.name) {
	    _curr.columns[i] = column;
	    break;
	}
    }

    // Update list
    QListViewItem* lvi = _columns->firstChild();
    while (lvi != NULL) {
	if (lvi->text(0) == orig.name) {
	    lvi->setText(0, column.name);
	    lvi->setText(1, column.description);
	    break;
	}
	lvi = lvi->itemBelow();
    }
}

void
TableEdit::remove(const ColumnDefn& column)
{
    // Remove from current
    _curr.columns.remove(column);

    // Remove from list
    QListViewItem* lvi = _columns->firstChild();
    while (lvi != NULL) {
	if (lvi->text(0) == column.name) {
	    delete lvi;
	    break;
	}
	lvi = lvi->itemBelow();
    }
}

void
TableEdit::insert(const ConstraintDefn& constraint)
{
    _curr.constraints.push_back(constraint);

    // Update list
    ListViewItem* lvi = new ListViewItem(_constraints);
    lvi->setText(0, constraint.name);
    lvi->setText(1, constraint.description);
    _constraints->setCurrentItem(lvi);
    _constraints->setSelected(lvi, true);
}

void
TableEdit::update(const ConstraintDefn& orig, const ConstraintDefn& constraint)
{
    // Update constraint
    for (unsigned int i = 0; i < _curr.constraints.size(); ++i) {
	if (_curr.constraints[i].name == orig.name) {
	    _curr.constraints[i] = constraint;
	    break;
	}
    }

    // Update list
    QListViewItem* lvi = _constraints->firstChild();
    while (lvi != NULL) {
	if (lvi->text(0) == orig.name) {
	    lvi->setText(0, constraint.name);
	    lvi->setText(1, constraint.description);
	    break;
	}
	lvi = lvi->itemBelow();
    }
}

void
TableEdit::remove(const ConstraintDefn& constraint)
{
    // Remove from current
    _curr.constraints.remove(constraint);

    // Remove from list
    QListViewItem* lvi = _constraints->firstChild();
    while (lvi != NULL) {
	if (lvi->text(0) == constraint.name) {
	    delete lvi;
	    break;
	}
	lvi = lvi->itemBelow();
    }
}

void
TableEdit::insert(const IndexDefn& index)
{
    _curr.indexes.push_back(index);

    // Update list
    ListViewItem* lvi = new ListViewItem(_indexes);
    lvi->setText(0, index.name);
    lvi->setText(1, index.description);
    _indexes->setCurrentItem(lvi);
    _indexes->setSelected(lvi, true);
}

void
TableEdit::update(const IndexDefn& orig, const IndexDefn& index)
{
    // Update index
    for (unsigned int i = 0; i < _curr.indexes.size(); ++i) {
	if (_curr.indexes[i].name == orig.name) {
	    _curr.indexes[i] = index;
	    break;
	}
    }

    // Update list
    QListViewItem* lvi = _indexes->firstChild();
    while (lvi != NULL) {
	if (lvi->text(0) == orig.name) {
	    lvi->setText(0, index.name);
	    lvi->setText(1, index.description);
	    break;
	}
	lvi = lvi->itemBelow();
    }
}

void
TableEdit::remove(const IndexDefn& index)
{
    // Remove from current
    _curr.indexes.remove(index);

    // Remove from list
    QListViewItem* lvi = _indexes->firstChild();
    while (lvi != NULL) {
	if (lvi->text(0) == index.name) {
	    delete lvi;
	    break;
	}
	lvi = lvi->itemBelow();
    }
}

bool
TableEdit::slotOk()
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
TableEdit::slotNext()
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
TableEdit::slotReset()
{
    getWidgets();

    if (_curr != _orig) {
	QString msg = QString("Table \"%1\" has been changed. Are you\n"
			      "sure you want to reset?").arg(_orig.name);
	int result = QMessageBox::warning(this, "Warning", msg,
					  QMessageBox::No, QMessageBox::Yes);
	if (result != QMessageBox::Yes)
	    return;
    }

    _curr = _orig;
    setWidgets();
}

void
TableEdit::slotCancel()
{
    close();
}

void
TableEdit::slotAddColumn()
{
    ColumnEdit* edit = new ColumnEdit(this, ColumnDefn());
    edit->show();
    _columnEdits.push_back(edit);
}

void
TableEdit::slotEditColumn()
{
    ColumnDefn* column = currentColumn();
    if (column == NULL) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), "No column selected");
	return;
    }

    ColumnEdit* edit = new ColumnEdit(this, *column);
    edit->show();
    _columnEdits.push_back(edit);
}

void
TableEdit::slotRemoveColumn()
{
    ColumnDefn* column = currentColumn();
    if (column == NULL) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), "No column selected");
	return;
    }

    QString name = column->name;
    QString message = "Are you sure you want to remove \"" + name + "\"";
    int result = QMessageBox::warning(this, "Warning", message,
				      QMessageBox::No, QMessageBox::Yes);
    if (result != QMessageBox::Yes) return;

    remove(*column);
}

void
TableEdit::slotAddConstraint()
{
    ConstraintEdit* edit = new ConstraintEdit(this, ConstraintDefn());
    edit->show();
    _constEdits.push_back(edit);
}

void
TableEdit::slotEditConstraint()
{
    ConstraintDefn* constraint = currentConstraint();
    if (constraint == NULL) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), "No constraint selected");
	return;
    }

    ConstraintEdit* edit = new ConstraintEdit(this, *constraint);
    edit->show();
    _constEdits.push_back(edit);
}

void
TableEdit::slotRemoveConstraint()
{
    ConstraintDefn* constraint = currentConstraint();
    if (constraint == NULL) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), "No constraint selected");
	return;
    }

    QString name = constraint->name;
    QString message = "Are you sure you want to remove \"" + name + "\"";
    int result = QMessageBox::warning(this, "Warning", message,
				      QMessageBox::No, QMessageBox::Yes);
    if (result != QMessageBox::Yes) return;

    remove(*constraint);
}

void
TableEdit::slotAddIndex()
{
    IndexEdit* edit = new IndexEdit(this, IndexDefn());
    edit->show();
    _indexEdits.push_back(edit);
}

void
TableEdit::slotEditIndex()
{
    IndexDefn* index = currentIndex();
    if (index == NULL) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), "No index selected");
	return;
    }

    IndexEdit* edit = new IndexEdit(this, *index);
    edit->show();
    _indexEdits.push_back(edit);
}

void
TableEdit::slotRemoveIndex()
{
    IndexDefn* index = currentIndex();
    if (index == NULL) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), "No index selected");
	return;
    }

    QString name = index->name;
    QString message = "Are you sure you want to remove \"" + name + "\"";
    int result = QMessageBox::warning(this, "Warning", message,
				      QMessageBox::No, QMessageBox::Yes);
    if (result != QMessageBox::Yes) return;

    remove(*index);
}

void
TableEdit::closeEvent(QCloseEvent* e)
{
    // Ask all children to close
    for (unsigned int i = 0; i < _columnEdits.size(); ++i) {
	ColumnEdit* edit = _columnEdits[i];
	if (edit == NULL) continue;
	if (!edit->close()) return;
    }
    for (unsigned int i = 0; i < _constEdits.size(); ++i) {
	ConstraintEdit* edit = _constEdits[i];
	if (edit == NULL) continue;
	if (!edit->close()) return;
    }
    for (unsigned int i = 0; i < _indexEdits.size(); ++i) {
	IndexEdit* edit = _indexEdits[i];
	if (edit == NULL) continue;
	if (!edit->close()) return;
    }

    if (!checkChanged()) return;
    QWidget::closeEvent(e);
}

bool
TableEdit::checkChanged()
{
    getWidgets();

    if (_curr != _orig) {
	QString message = QString("Table \"%1\" has been changed.\n"
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
TableEdit::setWidgets()
{
    _name->setText(_curr.name);
    _desc->setText(_curr.description);
    refreshColumns();
    refreshConstraints();
    refreshIndexes();
}

void
TableEdit::getWidgets()
{
    _curr.name = _name->text();
    _curr.description = _desc->text();
}

void
TableEdit::refreshColumns()
{
    ColumnDefn* old = currentColumn();
    QListViewItem* current = NULL;

    _columns->clear();
    for (unsigned int i = 0; i < _curr.columns.size(); ++i) {
	const ColumnDefn& column = _curr.columns[i];

	ListViewItem* lvi = new ListViewItem(_columns);
	lvi->setText(0, column.name);
	lvi->setText(1, column.description);

	if (old != NULL && column.name == old->name)
	    current = lvi;
    }

    if (current == NULL) current = _columns->firstChild();
    _columns->setCurrentItem(current);
    _columns->setSelected(current, true);
}

void
TableEdit::refreshConstraints()
{
    ConstraintDefn* old = currentConstraint();
    QListViewItem* current = NULL;

    _constraints->clear();
    for (unsigned int i = 0; i < _curr.constraints.size(); ++i) {
	const ConstraintDefn& constraint = _curr.constraints[i];

	ListViewItem* lvi = new ListViewItem(_constraints);
	lvi->setText(0, constraint.name);
	lvi->setText(1, constraint.description);

	if (old != NULL && constraint.name == old->name)
	    current = lvi;
    }

    if (current == NULL) current = _constraints->firstChild();
    _constraints->setCurrentItem(current);
    _constraints->setSelected(current, true);
}

void
TableEdit::refreshIndexes()
{
    IndexDefn* old = currentIndex();
    QListViewItem* current = NULL;

    _indexes->clear();
    for (unsigned int i = 0; i < _curr.indexes.size(); ++i) {
	const IndexDefn& index = _curr.indexes[i];

	ListViewItem* lvi = new ListViewItem(_indexes);
	lvi->setText(0, index.name);
	lvi->setText(1, index.description);

	if (old != NULL && index.name == old->name)
	    current = lvi;
    }

    if (current == NULL) current = _indexes->firstChild();
    _indexes->setCurrentItem(current);
    _indexes->setSelected(current, true);
}

ColumnDefn*
TableEdit::currentColumn()
{
    if (_columns->currentItem() == NULL)
	return NULL;

    QString name = _columns->currentItem()->text(0);
    for (unsigned int i = 0; i < _curr.columns.size(); ++i)
	if (_curr.columns[i].name == name)
	    return &_curr.columns[i];

    return NULL;
}

ConstraintDefn*
TableEdit::currentConstraint()
{
    if (_constraints->currentItem() == NULL)
	return NULL;

    QString name = _constraints->currentItem()->text(0);
    for (unsigned int i = 0; i < _curr.constraints.size(); ++i)
	if (_curr.constraints[i].name == name)
	    return &_curr.constraints[i];

    return NULL;
}

IndexDefn*
TableEdit::currentIndex()
{
    if (_indexes->currentItem() == NULL)
	return NULL;

    QString name = _indexes->currentItem()->text(0);
    for (unsigned int i = 0; i < _curr.indexes.size(); ++i)
	if (_curr.indexes[i].name == name)
	    return &_curr.indexes[i];

    return NULL;
}
