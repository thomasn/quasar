// $Id: constraint_edit.cpp,v 1.1 2004/12/19 09:12:29 bpepers Exp $
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

#include "constraint_edit.h"
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

ConstraintEdit::ConstraintEdit(TableEdit* edit, const ConstraintDefn& cnst)
    : QMainWindow(NULL, "ConstraintEdit", WDestructiveClose), _edit(edit)
{
    createWidgets();
    _orig = cnst;
    _curr = cnst;
    setWidgets();
}

ConstraintEdit::~ConstraintEdit()
{
}

void
ConstraintEdit::createWidgets()
{
    setCaption("Constraint Edit");
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

    QGridLayout* midGrid = new QGridLayout(mid);
    midGrid->setSpacing(3);
    midGrid->setMargin(3);
    midGrid->setColStretch(2, 1);
    midGrid->addColSpacing(2, 20);
    midGrid->addWidget(typeLabel, 0, 0);
    midGrid->addWidget(_type, 0, 1, AlignLeft | AlignVCenter);

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

    for (int type = 0; type <= ConstraintDefn::TYPE_FOREIGN_KEY; ++type)
	_type->insertItem(ConstraintDefn::typeName(type));

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
ConstraintEdit::slotOk()
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
ConstraintEdit::slotNext()
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
ConstraintEdit::slotReset()
{
    getWidgets();

    if (_curr != _orig) {
	QString msg = QString("Constraint \"%1\" has been changed. Are you\n"
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
ConstraintEdit::slotCancel()
{
    close();
}

void
ConstraintEdit::slotTypeChanged()
{
}

void
ConstraintEdit::closeEvent(QCloseEvent* e)
{
    if (!checkChanged()) return;
    QWidget::closeEvent(e);
}

bool
ConstraintEdit::checkChanged()
{
    getWidgets();

    if (_curr != _orig) {
	QString message = QString("Constraint \"%1\" has been changed.\n"
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
ConstraintEdit::setWidgets()
{
    _name->setText(_curr.name);
    _desc->setText(_curr.description);
}

void
ConstraintEdit::getWidgets()
{
    _curr.name = _name->text();
    _curr.description = _desc->text();
}
