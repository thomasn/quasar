// $Id: change_password.cpp,v 1.7 2005/02/27 18:30:47 bpepers Exp $
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

#include "change_password.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "line_edit.h"
#include "company.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qmessagebox.h>

ChangePassword::ChangePassword(MainWindow* main)
    : QuasarWindow(main, "ChangePassword")
{
    _helpSource = "change_password.html";

    QFrame* frame = new QFrame(this);

    Company company;
    _db->lookup(company);

    QLabel* message = new QLabel(frame);
    message->setText(tr("Changing password for user: \"%1\"\n"
			"In the company: \"%2\"\n\n"
			"Please type in the current password and then a new\n"
			"password.  Re-type the new password to verify it's\n"
			"been typed correctly.\n").arg(_quasar->username())
		     .arg(company.name()));

    QLabel* currentLabel = new QLabel(tr("Current Password:"), frame);
    _current = new LineEdit(16, frame);
    _current->setEchoMode(QLineEdit::Password);
    currentLabel->setBuddy(_current);

    QLabel* passwordLabel = new QLabel(tr("New Password:"), frame);
    _password = new LineEdit(16, frame);
    _password->setEchoMode(QLineEdit::Password);
    passwordLabel->setBuddy(_password);

    QLabel* verifyLabel = new QLabel(tr("Retype to Verify:"), frame);
    _verify = new LineEdit(16, frame);
    _verify->setEchoMode(QLineEdit::Password);
    verifyLabel->setBuddy(_verify);

    QFrame* buttons = new QFrame(frame);
    QPushButton* ok = new QPushButton(tr("OK"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);

    connect(ok, SIGNAL(clicked()), SLOT(slotChangePassword()));
    connect(cancel, SIGNAL(clicked()), SLOT(close()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(ok, 0, 1);
    buttonGrid->addWidget(cancel, 0, 2);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setColStretch(1, 1);
    grid->addMultiCellWidget(message, 0, 0, 0, 1);
    grid->addWidget(currentLabel, 1, 0);
    grid->addWidget(_current, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(passwordLabel, 2, 0);
    grid->addWidget(_password, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(verifyLabel, 3, 0);
    grid->addWidget(_verify, 3, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(buttons, 4, 4, 0, 1);

    _current->setFocus();
    setCaption(tr("Change Password"));
    setCentralWidget(frame);
    finalize();
}

ChangePassword::~ChangePassword()
{
}

void
ChangePassword::slotChangePassword()
{
    QString current = _current->text();
    QString password = _password->text();
    QString verify = _verify->text();

    if (password != verify) {
	QString message = tr("The new password entered doesn't\n"
			     "match the verify.  The means one\n"
			     "or the other of them has been typed\n"
			     "incorrectly.  Please re-enter them.");
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), message);

	_password->setFocus();
	_password->clear();
	_verify->clear();
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    bool result = _quasar->changePassword(current, password);
    QApplication::restoreOverrideCursor();

    if (result) {
	QString message = tr("The password for \"%1\"\n"
			     "has been changed").arg(_quasar->username());
	qApp->beep();
	QMessageBox::information(this, tr("Success"), message);

	close();
    } else {
	QString message = tr("Changing the password has failed.  This\n"
			     "usually means the current password was\n"
			     "entered incorrectly.");
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), message);

	_current->setFocus();
	_current->selectAll();
    }
}
