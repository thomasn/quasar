// $Id: company_config_tab.cpp,v 1.9 2005/03/01 19:53:02 bpepers Exp $
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

#include "company_config_tab.h"
#include "quasar_version.h"
#include "db_driver.h"
#include "new_company.h"
#include "update_screen.h"
#include "import_screen.h"
#include "backup_screen.h"
#include "restore_screen.h"
#include "list_view_item.h"
#include "conn.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qdir.h>

CompanyConfigTab::CompanyConfigTab(QWidget* parent)
    : ConfigTab(parent, "CompanyConfig")
{
    _companies = new ListView(this);
    _companies->setAllColumnsShowFocus(true);
    _companies->setShowSortIndicator(true);
    _companies->addTextColumn(tr("Company Name"), 40);
    _companies->addTextColumn(tr("Version"), 12);
    _companies->addTextColumn(tr("Database"), 12);

    QFrame* buttons = new QFrame(this);
    QPushButton* create = new QPushButton(tr("New Company"), buttons);
    QPushButton* restore = new QPushButton(tr("Restore Backup"), buttons);

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(1, 1);
    buttonGrid->addWidget(create, 0, 0);
    buttonGrid->addWidget(restore, 0, 2);

    QFrame* funcs = new QFrame(this);
    QPushButton* update = new QPushButton(tr("Update"), funcs);
    QPushButton* import = new QPushButton(tr("Import"), funcs);
    QPushButton* script = new QPushButton(tr("Run Script"), funcs);
    QPushButton* backup = new QPushButton(tr("Backup"), funcs);
    QPushButton* remove = new QPushButton(tr("Delete"), funcs);

    QGridLayout* funcGrid = new QGridLayout(funcs);
    funcGrid->setSpacing(2);
    funcGrid->setMargin(2);
    funcGrid->setRowStretch(4, 1);
    funcGrid->addWidget(update, 0, 0);
    funcGrid->addWidget(import, 1, 0);
    funcGrid->addWidget(script, 2, 0);
    funcGrid->addWidget(backup, 3, 0);
    funcGrid->addWidget(remove, 5, 0);

    QGridLayout* grid = new QGridLayout(this);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(0, 1);
    grid->setColStretch(0, 1);
    grid->addWidget(_companies, 0, 0);
    grid->addWidget(buttons, 1, 0);
    grid->addWidget(funcs, 0, 1);

    connect(create, SIGNAL(clicked()), SLOT(slotNewCompany()));
    connect(restore, SIGNAL(clicked()), SLOT(slotRestoreCompany()));
    connect(update, SIGNAL(clicked()), SLOT(slotUpdate()));
    connect(import, SIGNAL(clicked()), SLOT(slotImport()));
    connect(script, SIGNAL(clicked()), SLOT(slotRunScript()));
    connect(backup, SIGNAL(clicked()), SLOT(slotBackup()));
    connect(remove, SIGNAL(clicked()), SLOT(slotDelete()));
}

CompanyConfigTab::~CompanyConfigTab()
{
}

void
CompanyConfigTab::loadData()
{
    QListViewItem* item = _companies->currentItem();
    QString currentName;
    if (item != NULL) currentName = item->text(0);

    QValueList<CompanyDefn> companies;
    companyList(companies);

    _companies->clear();
    QListViewItem* current = NULL;
    for (unsigned int i = 0; i < companies.size(); ++i) {
	const CompanyDefn& company = companies[i];

	ListViewItem* lvi = new ListViewItem(_companies);
	lvi->setText(0, company.name());
	lvi->setText(1, company.version());
	lvi->setText(2, company.dbType());

	if (company.name() == currentName)
	    current = lvi;
    }

    if (current == NULL) current = _companies->firstChild();
    _companies->setCurrentItem(current);
    _companies->setSelected(current, true);
}

void
CompanyConfigTab::slotNewCompany()
{
    NewCompany* window = new NewCompany();
    window->show();
    connect(window, SIGNAL(destroyed()), SLOT(loadData()));
}

void
CompanyConfigTab::slotRestoreCompany()
{
    RestoreScreen* window = new RestoreScreen();
    window->show();
    connect(window, SIGNAL(destroyed()), SLOT(loadData()));
}

void
CompanyConfigTab::slotUpdate()
{
    CompanyDefn company;
    if (!currentCompany(company)) return;

    UpdateScreen* window = new UpdateScreen(company);
    window->show();
    connect(window, SIGNAL(destroyed()), SLOT(loadData()));
}

void
CompanyConfigTab::slotImport()
{
    CompanyDefn company;
    if (!currentCompany(company)) return;

    if (company.version() != QUASAR_DB_VERSION) {
	QString message = tr("Can't import into company with diffent version");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    ImportScreen* window = new ImportScreen(company);
    window->show();
}

void
CompanyConfigTab::slotRunScript()
{
    CompanyDefn company;
    if (!currentCompany(company)) return;

    QString filter = "Script Files (*.sql)";
    QString filePath = QFileDialog::getOpenFileName("", filter, this);
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(IO_ReadOnly)) {
	QApplication::beep();
	QMessageBox::critical(this, tr("Error"), tr("Can't open script file"));
	return;
    }

    Driver* driver = Driver::getDriver(company.dbType());
    if (driver == NULL) {
	QApplication::beep();
	QString message = tr("Get driver failed");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    Conn connection(driver);
    if (!connection.dbaConnect(company.database())) {
	QApplication::beep();
	QString msg=tr("Open company failed: %1").arg(connection.lastError());
	QMessageBox::critical(this, tr("Error"), msg);
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Stmt stmt(connection);
    QTextStream stream(&file);
    int lineNumber = 0;
    QString command;
    while (true) {
	QString line = stream.readLine();
	if (line.isNull()) break;
	++lineNumber;

	// Skip blank lines and comments
	if (line.isEmpty()) continue;
	if (line.left(1) == "#") continue;

	// Append line to command and see if we are done
	command += line;
	if (command.right(1) != ";") {
	    command += "\n";
	    continue;
	}

	// Check for commit/rollback or execute
	if (command == "commit;")
	    connection.commit();
	else if (strcmp(command, "rollback;") == 0)
	    connection.rollback();
	else {
	    stmt.setCommand(command);
	    if (!stmt.execute()) {
		qWarning("Execute failed: " + command);
		qWarning(stmt.lastError());
	    }
	}
	command = "";
    }
    connection.commit();

    QApplication::restoreOverrideCursor();
    QMessageBox::information(this, tr("Finished"), tr("Script has finished"));
}

void
CompanyConfigTab::slotBackup()
{
    CompanyDefn company;
    if (!currentCompany(company)) return;

    BackupScreen* window = new BackupScreen(company);
    window->show();
}

void
CompanyConfigTab::slotDelete()
{
    CompanyDefn company;
    if (!currentCompany(company)) return;

    QString message = tr("Are you sure you want to\ndelete the company?");
    int choice = QMessageBox::warning(topLevelWidget(), tr("Warning"), message,
				      QMessageBox::No, QMessageBox::Yes);
    if (choice != QMessageBox::Yes) return;

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Driver* driver = Driver::getDriver(company.dbType());
    if (driver == NULL) {
	QApplication::restoreOverrideCursor();
	critical("Failed setting driver: " + company.dbType());
	return;
    }

    if (!driver->remove(company)) {
	QApplication::restoreOverrideCursor();
	critical("Failed deleting company: " + driver->lastError());
	return;
    }

    QFile::remove(company.filePath());

    loadData();
    QApplication::restoreOverrideCursor();
}

void
CompanyConfigTab::companyList(QValueList<CompanyDefn>& companies)
{
    companies.clear();

    ServerConfig config;
    config.load();

    QString dirPath = parseDir(config.dataDir) + "/companies";
    if (dirPath.isEmpty()) return;

    QDir dir(dirPath, "*.xml");
    QStringList entries = dir.entryList();
    for (unsigned int i = 0; i < entries.size(); ++i) {
	QString filePath = dir.filePath(entries[i]);

	CompanyDefn company;
	if (company.load(filePath)) {
	    companies.push_back(company);
	} else {
	    qWarning("Warning: failed loading \"" + filePath + "\"");
	}
    }
}

bool
CompanyConfigTab::findCompany(const QString& name, CompanyDefn& company)
{
    QValueList<CompanyDefn> list;
    companyList(list);
    for (unsigned int i = 0; i < list.size(); ++i) {
	if (list[i].name() == name) {
	    company = list[i];
	    return true;
	}
    }
    return false;
}

bool
CompanyConfigTab::currentCompany(CompanyDefn& company)
{
    QListViewItem* current = _companies->currentItem();
    if (current == NULL) {
	critical(tr("No company selected"));
	return false;
    }

    QString name = current->text(0);
    if (!findCompany(name, company)) {
	critical(tr("Company \"%1\" no longer exists").arg(name));
	loadData();
	return false;
    }

    return true;
}

void
CompanyConfigTab::critical(const QString& message)
{
    qApp->beep();
    QMessageBox::critical(this, tr("Error"), message);
}
