// $Id: column_edit.cpp,v 1.3 2004/12/19 10:40:52 bpepers Exp $
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

#include "column_edit.h"
#include "table_edit.h"
#include "line_edit.h"
#include "multi_line_edit.h"
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

ColumnEdit::ColumnEdit(TableEdit* edit, const ColumnDefn& column)
    : QMainWindow(NULL, "ColumnEdit", WDestructiveClose), _edit(edit)
{
    createWidgets();
    _orig = column;
    _curr = column;
    setWidgets();
}

ColumnEdit::~ColumnEdit()
{
}

void
ColumnEdit::createWidgets()
{
    setCaption("Column Edit");
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

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(3);
    topGrid->setMargin(3);
    topGrid->setRowStretch(2, 1);
    topGrid->setColStretch(1, 1);
    topGrid->addWidget(nameLabel, 0, 0);
    topGrid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(descLabel, 1, 0);
    topGrid->addMultiCellWidget(_desc, 1, 2, 1, 1);

    QLabel* typeLabel = new QLabel("&Type:", mid);
    _type = new ComboBox(mid);
    typeLabel->setBuddy(_type);

    QLabel* lengthLabel = new QLabel("&Length:", mid);
    _length = new LineEdit(mid);
    lengthLabel->setBuddy(_length);

    _manditory = new QCheckBox("Manditory?", mid);
    _unique = new QCheckBox("Unique?", mid);

    QGridLayout* midGrid = new QGridLayout(mid);
    midGrid->setSpacing(3);
    midGrid->setMargin(3);
    midGrid->setColStretch(2, 1);
    midGrid->addColSpacing(2, 20);
    midGrid->addWidget(typeLabel, 0, 0);
    midGrid->addWidget(_type, 0, 1, AlignLeft | AlignVCenter);
    midGrid->addWidget(lengthLabel, 1, 0);
    midGrid->addWidget(_length, 1, 1, AlignLeft | AlignVCenter);
    midGrid->addMultiCellWidget(_manditory, 2, 2, 0, 1,AlignLeft|AlignVCenter);
    midGrid->addMultiCellWidget(_unique, 3, 3, 0, 1,AlignLeft|AlignVCenter);

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
    connect(_type, SIGNAL(activated(int)), this, SLOT(slotTypeChanged()));

    for (int type = 0; type <= ColumnDefn::TYPE_QUANTITY; ++type)
	_type->insertItem(ColumnDefn::typeName(type));

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
ColumnEdit::slotOk()
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
ColumnEdit::slotNext()
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
ColumnEdit::slotReset()
{
    getWidgets();

    if (_curr != _orig) {
	QString msg = QString("Column \"%1\" has been changed. Are you\n"
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
ColumnEdit::slotCancel()
{
    close();
}

void
ColumnEdit::slotTypeChanged()
{
    switch (_type->currentItem()) {
    case ColumnDefn::TYPE_CHAR:
    case ColumnDefn::TYPE_STRING:
	_length->setEnabled(true);
	break;
    default:
	_length->setEnabled(false);
	_length->setText("");
    }
}

void
ColumnEdit::closeEvent(QCloseEvent* e)
{
    if (!checkChanged()) return;
    QWidget::closeEvent(e);
}

bool
ColumnEdit::checkChanged()
{
    getWidgets();

    if (_curr != _orig) {
	QString message = QString("Column \"%1\" has been changed.\n"
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
ColumnEdit::setWidgets()
{
    _name->setText(_curr.name);
    _desc->setText(_curr.description);
    _type->setCurrentItem(_curr.type);
    _manditory->setChecked(_curr.manditory);
    _unique->setChecked(_curr.unique);

    switch (_curr.type) {
    case ColumnDefn::TYPE_CHAR:
    case ColumnDefn::TYPE_STRING:
	_length->setText(QString::number(_curr.size));
	break;
    default:
	_length->setText("");
	_length->setEnabled(false);
    }
}

void
ColumnEdit::getWidgets()
{
    _curr.name = _name->text();
    _curr.description = _desc->text();
    _curr.type = _type->currentItem();
    _curr.manditory = _manditory->isChecked();
    _curr.unique = _unique->isChecked();

    switch (_curr.type) {
    case ColumnDefn::TYPE_CHAR:
    case ColumnDefn::TYPE_STRING:
	_curr.size = _length->text().toInt();
	break;
    default:
	_curr.size = 0;
    }
}
