// $Id: index_edit.cpp,v 1.1 2004/12/04 21:23:57 bpepers Exp $
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

#include "index_edit.h"
#include "table_edit.h"
#include "line_edit.h"
#include "multi_line_edit.h"
#include "list_view_item.h"
#include "combo_box.h"

#include <qapplication.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qmessagebox.h>
#include <qregexp.h>

IndexEdit::IndexEdit(TableEdit* edit, const IndexDefn& index)
    : QMainWindow(NULL, "IndexEdit", WDestructiveClose), _edit(edit)
{
    createWidgets();
    _orig = index;
    _curr = index;
    setWidgets();
}

IndexEdit::~IndexEdit()
{
}

void
IndexEdit::createWidgets()
{
    setCaption("Index Edit");
    menuBar();

    QFrame* main = new QFrame(this);
    QVBox* body = new QVBox(main);
    QFrame* top = new QFrame(body);
    QFrame* mid = new QFrame(body);
    QVBox* right = new QVBox(main);

    QLabel* nameLabel = new QLabel("&Name:", top);
    _name = new LineEdit(top);
    nameLabel->setBuddy(_name);

    QLabel* descLabel = new QLabel("&Description:", top);
    _desc = new MultiLineEdit(top);
    descLabel->setBuddy(_desc);

    _unique = new QCheckBox("Unique?", top);

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(3);
    topGrid->setMargin(3);
    topGrid->setRowStretch(2, 1);
    topGrid->setColStretch(1, 1);
    topGrid->addWidget(nameLabel, 0, 0);
    topGrid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(descLabel, 1, 0);
    topGrid->addMultiCellWidget(_desc, 1, 2, 1, 1);
    topGrid->addMultiCellWidget(_unique, 3, 3, 0, 1, AlignLeft | AlignVCenter);

    _columns = new ListView(body);
    _columns->setAllColumnsShowFocus(true);
    _columns->setShowSortIndicator(true);
    _columns->addTextColumn("Name", 30);
    _columns->addTextColumn("Ascending?", 14);

    QHBox* columnButtons = new QHBox(body);
    QPushButton* columnAdd = new QPushButton("&Add Column", columnButtons);
    QPushButton* columnEdit = new QPushButton("&Edit Column", columnButtons);
    QPushButton* columnDel = new QPushButton("&Remove Column", columnButtons);

    QGridLayout* midGrid = new QGridLayout(mid);
    midGrid->setSpacing(3);
    midGrid->setMargin(3);
    midGrid->setColStretch(2, 1);
    midGrid->addColSpacing(2, 20);

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

    setCentralWidget(main);

    QPopupMenu* file = new QPopupMenu(this);
    file->insertItem(tr("&Ok"), this, SLOT(slotOk()));
    file->insertItem(tr("&Next"), this, SLOT(slotNext()));
    file->insertItem(tr("&Reset"), this, SLOT(slotReset()));
    file->insertSeparator();
    file->insertItem(tr("&Cancel"), this, SLOT(slotCancel()), ALT+Key_Q);
    menuBar()->insertItem(tr("&File"), file);
}

bool
IndexEdit::slotOk()
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
IndexEdit::slotNext()
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
IndexEdit::slotReset()
{
    getWidgets();

    if (_curr != _orig) {
	QString msg = QString("Index \"%1\" has been changed. Are you\n"
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
IndexEdit::slotCancel()
{
    close();
}

void
IndexEdit::slotAddColumn()
{
    QDialog* dialog = new QDialog(this, "AddColumn", true);
    dialog->setCaption(tr("Add Column"));

    QLabel* columnLabel = new QLabel(tr("Column:"), dialog);
    ComboBox* column = new ComboBox(dialog);
    columnLabel->setBuddy(column);
    QCheckBox* ascending = new QCheckBox(tr("Ascending?"), dialog);

    const TableDefn& table = _edit->table();
    for (unsigned int i = 0; i < table.columns.size(); ++i)
	column->insertItem(table.columns[i].name);

    QFrame* buttons = new QFrame(dialog);
    QPushButton* ok = new QPushButton(tr("&OK"), buttons);
    QPushButton* cancel = new QPushButton(tr("&Cancel"), buttons);
    ok->setDefault(true);

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setMargin(6);
    buttonGrid->setSpacing(10);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(ok, 0, 1);
    buttonGrid->addWidget(cancel, 0, 2);

    QGridLayout* grid = new QGridLayout(dialog);
    grid->setMargin(6);
    grid->setSpacing(10);
    grid->addWidget(columnLabel, 0, 0);
    grid->addWidget(column, 0, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(ascending, 1, 1, 0, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(buttons, 2, 2, 0, 1);

    connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

    int result = dialog->exec();
    QString name = column->currentText();
    bool asc = ascending->isChecked();
    delete dialog;
    if (result != QDialog::Accepted) return;

    IndexColumn info;
    info.name = name;
    info.ascending = asc;
    _curr.columns.push_back(info);
    refreshColumns();
}

void
IndexEdit::slotEditColumn()
{
    IndexColumn* info = currentColumn();
    if (info == NULL) {
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), "No column selected");
	return;
    }

    QDialog* dialog = new QDialog(this, "EditColumn", true);
    dialog->setCaption(tr("Edit Column"));

    QLabel* columnLabel = new QLabel(tr("Column:"), dialog);
    ComboBox* column = new ComboBox(dialog);
    columnLabel->setBuddy(column);
    QCheckBox* ascending = new QCheckBox(tr("Ascending?"), dialog);

    const TableDefn& table = _edit->table();
    for (unsigned int i = 0; i < table.columns.size(); ++i)
	column->insertItem(table.columns[i].name);

    column->setCurrentItem(info->name);
    ascending->setChecked(info->ascending);

    QFrame* buttons = new QFrame(dialog);
    QPushButton* ok = new QPushButton(tr("&OK"), buttons);
    QPushButton* cancel = new QPushButton(tr("&Cancel"), buttons);
    ok->setDefault(true);

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setMargin(6);
    buttonGrid->setSpacing(10);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(ok, 0, 1);
    buttonGrid->addWidget(cancel, 0, 2);

    QGridLayout* grid = new QGridLayout(dialog);
    grid->setMargin(6);
    grid->setSpacing(10);
    grid->addWidget(columnLabel, 0, 0);
    grid->addWidget(column, 0, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(ascending, 1, 1, 0, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(buttons, 2, 2, 0, 1);

    connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

    int result = dialog->exec();
    QString name = column->currentText();
    bool asc = ascending->isChecked();
    delete dialog;
    if (result != QDialog::Accepted) return;

    info->name = name;
    info->ascending = asc;
    refreshColumns();
}

void
IndexEdit::slotRemoveColumn()
{
    IndexColumn* column = currentColumn();
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

    // Remove from current
    _curr.columns.remove(*column);

    // Remove from list
    QListViewItem* lvi = _columns->firstChild();
    while (lvi != NULL) {
	if (lvi->text(0) == column->name) {
	    delete lvi;
	    break;
	}
	lvi = lvi->itemBelow();
    }
}

void
IndexEdit::closeEvent(QCloseEvent* e)
{
    if (!checkChanged()) return;
    QWidget::closeEvent(e);
}

bool
IndexEdit::checkChanged()
{
    getWidgets();

    if (_curr != _orig) {
	QString message = QString("Index \"%1\" has been changed.\n"
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
IndexEdit::setWidgets()
{
    _name->setText(_curr.name);
    _desc->setText(_curr.description);
    _unique->setChecked(_curr.unique);
    refreshColumns();
}

void
IndexEdit::getWidgets()
{
    _curr.name = _name->text();
    _curr.description = _desc->text();
    _curr.unique = _unique->isChecked();
}

void
IndexEdit::refreshColumns()
{
    IndexColumn* old = currentColumn();
    QListViewItem* current = NULL;

    _columns->clear();
    for (unsigned int i = 0; i < _curr.columns.size(); ++i) {
	const IndexColumn& column = _curr.columns[i];

	ListViewItem* lvi = new ListViewItem(_columns);
	lvi->setText(0, column.name);
	lvi->setText(1, column.ascending ? "Yes" : "No");

	if (old != NULL && column.name == old->name)
	    current = lvi;
    }

    if (current == NULL) current = _columns->firstChild();
    _columns->setCurrentItem(current);
    _columns->setSelected(current, true);
}

IndexColumn*
IndexEdit::currentColumn()
{
    if (_columns->currentItem() == NULL)
	return NULL;

    QString name = _columns->currentItem()->text(0);
    for (unsigned int i = 0; i < _curr.columns.size(); ++i)
	if (_curr.columns[i].name == name)
	    return &_curr.columns[i];

    return NULL;
}
