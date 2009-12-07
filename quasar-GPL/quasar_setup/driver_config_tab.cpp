// $Id: driver_config_tab.cpp,v 1.7 2005/03/01 20:12:41 bpepers Exp $
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

#include "driver_config_tab.h"
#include "db_driver.h"
#include "line_edit.h"
#include "list_view_item.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>

DriverConfigTab::DriverConfigTab(QWidget* parent)
    : ConfigTab(parent, "DriverConfig")
{
    _drivers = new ListView(this);
    _drivers->setAllColumnsShowFocus(true);
    _drivers->setShowSortIndicator(true);
    _drivers->addTextColumn(tr("Driver Name"), 50);

    QFrame* funcs = new QFrame(this);
    QPushButton* config = new QPushButton(tr("Configure"), funcs);

    QGridLayout* funcGrid = new QGridLayout(funcs);
    funcGrid->setSpacing(2);
    funcGrid->setMargin(2);
    funcGrid->setRowStretch(1, 1);
    funcGrid->addWidget(config, 0, 0);

    QGridLayout* grid = new QGridLayout(this);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(0, 1);
    grid->addWidget(_drivers, 0, 0);
    grid->addWidget(funcs, 0, 1);

    connect(config, SIGNAL(clicked()), SLOT(slotConfigure()));
}

DriverConfigTab::~DriverConfigTab()
{
}

void
DriverConfigTab::loadData()
{
    QListViewItem* item = _drivers->currentItem();
    QString currentName;
    if (item != NULL) currentName = item->text(0);

    QStringList drivers = Driver::types();

    _drivers->clear();
    QListViewItem* current = NULL;
    for (unsigned int i = 0; i < drivers.size(); ++i) {
	const QString& driver = drivers[i];

	ListViewItem* lvi = new ListViewItem(_drivers);
	lvi->setText(0, driver);

	if (driver == currentName)
	    current = lvi;
    }

    if (current == NULL) current = _drivers->firstChild();
    _drivers->setCurrentItem(current);
    _drivers->setSelected(current, true);
}

bool
DriverConfigTab::saveChanges()
{
    return true;
}

void
DriverConfigTab::slotConfigure()
{
    QListViewItem* current = _drivers->currentItem();
    if (current == NULL) {
	QApplication::beep();
	QString message = tr("No driver selected");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    Driver* driver = Driver::getDriver(current->text(0));
    if (driver == NULL) {
	QApplication::beep();
	QString message = tr("Loading driver failed");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QDialog* dialog = driver->configureDialog(this);
    if (dialog != NULL) dialog->exec();
}
