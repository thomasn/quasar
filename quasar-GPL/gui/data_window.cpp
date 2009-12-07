// $Id: data_window.cpp,v 1.48 2004/12/28 09:24:17 bpepers Exp $
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

#include "data_window.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "popup.h"
#include "line_edit.h"
#include "gltx.h"
#include "gltx_select.h"

#include <qobjectlist.h>
#include <qapplication.h>
#include <qmenubar.h>
#include <qstatusbar.h>
#include <qpopupmenu.h>
#include <qvbox.h>
#include <qframe.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <assert.h>

DataWindow::DataWindow(MainWindow* main, const char* name, Id id)
    : EditWindow(main, name), _id(id), _firstField(NULL), _busy(false)
{
    // Create standard buttons
    _ok = new QPushButton(tr("&OK"), _buttons);
    _next = new QPushButton(tr("&Next"), _buttons);
    _cancel = new QPushButton(tr("&Cancel"), _buttons);
    connect(_ok, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(_next, SIGNAL(clicked()), this, SLOT(slotNext()));
    connect(_cancel, SIGNAL(clicked()), this, SLOT(slotCancel()));

    // Inactive checkbox
    _inactive = new QCheckBox(tr("Inactive?"), _buttons);
    _inactive->setFocusPolicy(NoFocus);

    // Create standard menus
    _file->insertItem(tr("&OK"), this, SLOT(slotOk()), ALT+Key_O);
    _file->insertItem(tr("&Next"), this, SLOT(slotNext()), ALT+Key_N);
    _file->insertItem(tr("&Cancel"), this, SLOT(slotCancel()), ALT+Key_C);
    _file->insertSeparator();
    _file->insertItem(tr("&Delete"), this, SLOT(slotDelete()));
    _file->insertItem(tr("Clone"), this, SLOT(slotClone()));
}

DataWindow::~DataWindow()
{
}

void
DataWindow::finalize()
{
    EditWindow::finalize();

    // Check if can view and screen is showing existing data
    bool allowView = allowed("View");
    if (!allowView && _id != INVALID_ID) {
	QTimer::singleShot(50, this, SLOT(slotNotAllowed()));
	return;
    }

    // Check is can create and screen is creating new data
    bool allowCreate = allowed("Create");
    if (!allowCreate && _id == INVALID_ID) {
	QTimer::singleShot(50, this, SLOT(slotNotAllowed()));
	return;
    }

    startEdit();
}

void
DataWindow::startEdit()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    if (_id == INVALID_ID) {
	newItem();
    } else {
	oldItem();
    }

    dataToWidget();
    if (_firstField) {
	_firstField->setFocus();
	if (_firstField->inherits("QLineEdit"))
	    ((QLineEdit*)_firstField)->selectAll();
	if (_firstField->inherits("Popup")) {
	    Popup* popup = (Popup*)_firstField;
	    if (popup->textWidget()->inherits("QLineEdit"))
		((QLineEdit*)popup->textWidget())->selectAll();
	}
    }

    QApplication::restoreOverrideCursor();
}

bool
DataWindow::saveItem(bool ask)
{
    widgetToData();
    if (isChanged()) {
	if (!allowed("Create") && _id == INVALID_ID) {
	    qApp->beep();
	    QString message = tr("Creating data is not allowed");
	    QMessageBox::critical(this, tr("Error"), message);
	    return false;
	}

	if (!allowed("Update") && _id != INVALID_ID) {
	    qApp->beep();
	    QString message = tr("Changing data is not allowed");
	    QMessageBox::critical(this, tr("Error"), message);
	    return false;
	}

	clearErrors();
	if (!dataValidate()) return false;

	if (ask) {
	    QString message = tr("The changes in this screen must be saved\n"
		"before continuing.  Do you wish to save your\n"
		"changes?");
	    int ch = QMessageBox::warning(this, tr("Save Changes?"), message,
					  QMessageBox::Yes, QMessageBox::No);
	    if (ch != QMessageBox::Yes) return false;
	}

	_busy = true;
	QApplication::setOverrideCursor(waitCursor);
	qApp->processEvents();

	bool isUpdate = (_id != INVALID_ID);
	bool ok = fileItem();

	QApplication::restoreOverrideCursor();

	if (ok) {
	    if (isUpdate)
		emit updated(_id);
	    else
		emit created(_id);
	} else {
	    showErrors();
	    _busy = false;
	    return false;
	}

	if (!ask) printItem(true);
	_busy = false;
    } else {
	QApplication::restoreOverrideCursor();
    }

    return true;
}

void
DataWindow::printItem(bool)
{
}

bool
DataWindow::slotOk()
{
    if (_busy) return false;
    if (!saveItem(false)) return false;
    slotCancel();
    return true;
}

bool
DataWindow::slotNext()
{
    if (_busy) return false;
    if (!saveItem(false)) return false;
    _id = INVALID_ID;
    startEdit();
    return true;
}

void
DataWindow::slotCancel()
{
    // TODO: change to make sure doesn't ask in closeEvent
    delete this;
}

void
DataWindow::slotDelete()
{
    if (!allowed("Delete")) {
	qApp->beep();
	QString message = tr("Deleting data is not allowed");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    if (_busy) return;
    int choice = QMessageBox::warning(this, tr("Delete Data"),
			tr("Are you sure you wish to delete this data?"),
			QMessageBox::No, QMessageBox::Yes);
    if (choice != QMessageBox::Yes)
	return;

    if (deleteItem()) {
        emit deleted(_id);
        slotCancel();
    } else {
	qApp->beep();
	QMessageBox::critical(this, tr("Quasar"),
			tr("The delete failed.  This is usually because\n"
			   "the data is still in use or you don't have\n"
			   "permission to delete data.  Please check for\n"
			   "places the data can be used or set the data\n"
			   "to inactive instead."), QMessageBox::Ok, 0);
    }
}

void
DataWindow::slotClone()
{
    if (_busy) return;
    if (!saveItem(true)) return;
    cloneItem();
}

void
DataWindow::slotPrint()
{
    if (_busy) return;
    if (saveItem(true)) {
	printItem();
	dataToWidget();
    }
}

void
DataWindow::closeEvent(QCloseEvent* e)
{
    widgetToData();
    if (isChanged()) {
	if (!isVisible()) show();

	if (!allowed("Create") && _id == INVALID_ID) {
	    QWidget::closeEvent(e);
	    return;
	}
	if (!allowed("Update") && _id != INVALID_ID) {
	    QWidget::closeEvent(e);
	    return;
	}

	int ch = QMessageBox::warning(this, tr("Save Changes?"),
				      tr("The data in this window has been "
					 "changed.\n\n  Do you wish to save "
					 "your changes?"),
				      QMessageBox::Yes, QMessageBox::No,
				      QMessageBox::Cancel);
	if (ch == QMessageBox::Yes) {
	    if (!dataValidate())
		return;

	    _busy = true;
	    QApplication::setOverrideCursor(waitCursor);
	    qApp->processEvents();

	    bool isUpdate = (_id != INVALID_ID);
	    bool ok = fileItem();

	    QApplication::restoreOverrideCursor();

	    if (ok) {
		if (isUpdate)
		    emit updated(_id);
		else
		    emit created(_id);
	    } else {
		showErrors();
		return;
	    }
	} else if (ch == QMessageBox::Cancel)
	    return;
    }

    QWidget::closeEvent(e);
}

void
DataWindow::clearErrors()
{
    _quasar->db()->clearErrors();
    statusBar()->clear();
}

void
DataWindow::showErrors()
{
    QString* error;
    while ((error = _quasar->db()->nextError()) != NULL) {
	statusBar()->message(*error);
	delete error;
    }
    QApplication::beep();
}

bool
DataWindow::dataValidate()
{
    QObjectList* list = queryList("LineEdit");
    QObjectListIt it(*list);
    LineEdit* edit;
    bool allValid = true;

    while ((edit = (LineEdit*)it.current()) != 0) {
        ++it;
        if (!edit->valid()) {
	    edit->setFocus();
	    statusBar()->message(tr("Invalid data"));
	    QApplication::beep();
	    allValid = false;
	}
    }

    delete list;
    return allValid;
}

void
DataWindow::cloneFrom(Id)
{
    assert(false);
}

void
DataWindow::cloneItem()
{
    assert(false);
}

QString
DataWindow::cloneName()
{
    return caption() + " " + tr("(Clone)");
}

bool
DataWindow::checkGltxUsed(const QString& number, int type, Id id)
{
    vector<Gltx> gltxs;
    GltxSelect conditions;
    conditions.number = number;
    conditions.type = type;
    _quasar->db()->select(gltxs, conditions);

    bool exists = false;
    for (unsigned int i = 0; i < gltxs.size(); ++i) {
	if (gltxs[i].id() == id) continue;
	exists = true;
    }
    if (!exists) return false;

    QString message = tr("This id number is already used for another\n"
	"transaction. Are you sure you want to file\n"
	"this transaction?");

    QApplication::restoreOverrideCursor();
    int choice = QMessageBox::warning(this, tr("Warning"), message,
				      QMessageBox::No, QMessageBox::Yes);
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    return choice != QMessageBox::Yes;
}
