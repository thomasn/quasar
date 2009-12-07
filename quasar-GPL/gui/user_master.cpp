// $Id: user_master.cpp,v 1.23 2004/12/12 11:02:09 bpepers Exp $
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

#include "user_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "user_select.h"
#include "lookup_edit.h"
#include "store_lookup.h"
#include "employee_lookup.h"
#include "security_type_lookup.h"
#include "combo_box.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qinputdialog.h>

UserMaster::UserMaster(MainWindow* main, Id user_id)
    : DataWindow(main, "UserMaster", user_id)
{
    _helpSource = "user_master.html";

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("User Name:"), _frame);
    _name = new LineEdit(16, _frame);
    nameLabel->setBuddy(_name);

    _newPassword = new QCheckBox(tr("Change Password?"), _frame);
    connect(_newPassword, SIGNAL(toggled(bool)), SLOT(slotToggled(bool)));

    QLabel* passwordLabel = new QLabel(tr("Password:"), _frame);
    _password = new LineEdit(16, _frame);
    _password->setEchoMode(QLineEdit::Password);
    passwordLabel->setBuddy(_password);

    QLabel* confirmLabel = new QLabel(tr("Confirm:"), _frame);
    _confirm = new LineEdit(16, _frame);
    _confirm->setEchoMode(QLineEdit::Password);
    confirmLabel->setBuddy(_confirm);

    QLabel* storeLabel = new QLabel(tr("Default Store:"), _frame);
    _store = new LookupEdit(new StoreLookup(_main, this), _frame);
    storeLabel->setBuddy(_store);

    QLabel* employeeLabel = new QLabel(tr("Default Employee:"), _frame);
    _employee = new LookupEdit(new EmployeeLookup(_main, this), _frame);
    employeeLabel->setBuddy(_employee);

    QLabel* securityLabel = new QLabel(tr("Security Type:"), _frame);
    _security = new LookupEdit(new SecurityTypeLookup(_main, this), _frame);
    securityLabel->setBuddy(_security);

    QLabel* screenLabel = new QLabel(tr("Screen:"), _frame);
    _screen = new ComboBox(_frame);
    screenLabel->setBuddy(_screen);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_newPassword, 1, 1, 0, 1, AlignLeft|AlignVCenter);
    grid->addWidget(passwordLabel, 2, 0);
    grid->addWidget(_password, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(confirmLabel, 3, 0);
    grid->addWidget(_confirm, 3, 1, AlignLeft | AlignVCenter);
    grid->addWidget(storeLabel, 4, 0);
    grid->addWidget(_store, 4, 1, AlignLeft | AlignVCenter);
    grid->addWidget(employeeLabel, 5, 0);
    grid->addWidget(_employee, 5, 1, AlignLeft | AlignVCenter);
    grid->addWidget(securityLabel, 6, 0);
    grid->addWidget(_security, 6, 1, AlignLeft | AlignVCenter);
    grid->addWidget(screenLabel, 7, 0);
    grid->addWidget(_screen, 7, 1, AlignLeft | AlignVCenter);

    // Get screen filenames
    QStringList fileNames;
    _quasar->resourceList("screens", fileNames);
    _screen->insertItem("");
    _screen->insertStringList(fileNames);

    if (user_id == INVALID_ID)
	_newPassword->hide();

    setCaption(tr("User Master"));
    finalize();
}

UserMaster::~UserMaster()
{
}

void
UserMaster::slotToggled(bool flag)
{
    _password->setEnabled(flag);
    _confirm->setEnabled(flag);
}

void
UserMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _newPassword->setChecked(false);
    _newPassword->show();
    slotToggled(_newPassword->isChecked());

    _curr = _orig;
    _firstField = _name;
}

void
UserMaster::newItem()
{
    User blank;
    _orig = blank;

    _newPassword->setChecked(true);
    _newPassword->hide();
    slotToggled(_newPassword->isChecked());

    _curr = _orig;
    _firstField = _name;
}

void
UserMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _name->selectAll();
}

bool
UserMaster::fileItem()
{
    if (_password->text() != _confirm->text()) {
	QString message = tr("Password doesn't match confirmation");
	QMessageBox::critical(this, tr("Error"), message);
	return false;
    }

    if (_orig.id() == INVALID_ID) {
	if (!_quasar->addUser(_curr)) return false;
    } else {
	if (!_quasar->updateUser(_orig, _curr)) return false;
    }

    _orig = _curr;
    _id = _curr.id();

    return true;
}

bool
UserMaster::deleteItem()
{
    UserSelect conditions;
    vector<User> users;
    _db->select(users, conditions);

    if (users.size() == 1) {
	QString message = tr("Can't delete last user");
	QMessageBox::critical(this, tr("Error"), message);
	return false;
    }

    if (_orig.id() == _quasar->user().id()) {
	QString message = tr("Can't delete current user");
	QMessageBox::critical(this, tr("Error"), message);
	return false;
    }

    return _quasar->deleteUser(_orig);
}

void
UserMaster::restoreItem()
{
    _curr = _orig;
}

void
UserMaster::cloneItem()
{
    UserMaster* clone = new UserMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
UserMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
UserMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _password->clear();
    _confirm->clear();
    _store->setId(_curr.defaultStore());
    _employee->setId(_curr.defaultEmployee());
    _security->setId(_curr.securityType());
    _screen->setCurrentItem(_curr.screen());
    _inactive->setChecked(!_curr.isActive());
}

// Set the data object from the widgets.
void
UserMaster::widgetToData()
{
    _curr.setName(_name->text());
    _curr.setDefaultStore(_store->getId());
    _curr.setDefaultEmployee(_employee->getId());
    _curr.setSecurityType(_security->getId());
    _curr.setScreen(_screen->currentText());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    if (_newPassword->isChecked())
	_curr.setPassword(_password->text());
    else
	_curr.setPassword("<<<no change>>>");
}
