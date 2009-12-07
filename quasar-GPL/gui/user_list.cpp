// $Id: user_list.cpp,v 1.3 2004/12/12 11:02:09 bpepers Exp $
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

#include "user_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "user_select.h"
#include "user_master.h"
#include "grid.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcheckbox.h>
#include <qinputdialog.h>
#include <qmessagebox.h>

UserList::UserList(MainWindow* main)
    : ActiveList(main, "UserList")
{
    _helpSource = "user_list.html";

    _list->addTextColumn(tr("Name"), 30);
    _list->addTextColumn(tr("Security"), 20);
    _list->setSorting(0);

    setCaption(tr("User List"));
    finalize();
}

UserList::~UserList()
{
}

bool
UserList::isActive(Id user_id)
{
    User user;
    _quasar->db()->lookup(user_id, user);
    return user.isActive();
}

void
UserList::setActive(Id user_id, bool active)
{
    if (user_id == INVALID_ID) return;

    User orig;
    _quasar->db()->lookup(user_id, orig);

    User user = orig;
    _quasar->db()->setActive(user, active);

    if (!_quasar->updateUser(orig, user)) {
	QString message = tr("Updating user failed.");
	QMessageBox::critical(this, tr("Error"), message);
    }
}

void
UserList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id user_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    UserSelect conditions;
    conditions.activeOnly = !showInactive;
    vector<User> users;
    _quasar->db()->select(users, conditions);

    for (unsigned int i = 0; i < users.size(); ++i) {
	const User& user = users[i];

	SecurityType type;
	_db->lookup(user.securityType(), type);

	ListViewItem* lvi = new ListViewItem(_list, user.id());
	lvi->setText(0, user.name());
	lvi->setText(1, type.name());
	if (showInactive) lvi->setValue(2, !user.isActive());
	if (user.id() == user_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
UserList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Users"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
UserList::performNew()
{
    UserMaster* master = new UserMaster(_main);
    master->show();
}

void
UserList::performEdit()
{
    Id user_id = currentId();
    UserMaster* master = new UserMaster(_main, user_id);
    master->show();
}
