// $Id: import_screen.cpp,v 1.4 2004/11/11 00:20:12 bpepers Exp $
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

#include "import_screen.h"
#include "data_import.h"
#include "server_config.h"
#include "list_view_item.h"
#include "date_valcon.h"
#include "line_edit.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qfile.h>

ImportScreen::ImportScreen(const CompanyDefn& company)
    : QMainWindow(0, "ImportScreen", WType_TopLevel | WDestructiveClose)
{
    _import = new DataImport(company, this);
    connect(_import, SIGNAL(message(int,QString,QString)),
	    SLOT(slotMessage(int,QString,QString)));

    QFrame* frame = new QFrame(this);
    QFrame* file = new QFrame(frame);

    QLabel* fileLabel = new QLabel(tr("Import File:"), file);
    _filePath = new LineEdit(file);
    _filePath->addPopup(Key_F9, tr("browse"));
    fileLabel->setBuddy(_filePath);
    connect(_filePath, SIGNAL(doPopup(QKeySequence)), SLOT(slotOpenFile()));

    QPushButton* browse = new QPushButton("...", file);
    browse->setFocusPolicy(ClickFocus);
    connect(browse, SIGNAL(clicked()), SLOT(slotOpenFile()));

    QGridLayout* fileGrid = new QGridLayout(file);
    fileGrid->setMargin(6);
    fileGrid->setSpacing(6);
    fileGrid->setColStretch(1, 1);
    fileGrid->addWidget(fileLabel, 0, 0);
    fileGrid->addWidget(_filePath, 0, 1);
    fileGrid->addWidget(browse, 0, 2);

    // TODO: add data types

    _log = new ListView(frame);
    _log->addNumberColumn(tr("Count"), 8);
    _log->addTextColumn(tr("Severity"), 12);
    _log->addTextColumn(tr("Data Type"), 16);
    _log->addTextColumn(tr("Data Name"), 20);
    _log->addTextColumn(tr("Message"), 30);
    _log->setAllColumnsShowFocus(true);
    _log->setShowSortIndicator(true);

    QFrame* buttons = new QFrame(frame);
    QPushButton* import = new QPushButton(tr("&Import"), buttons);
    QPushButton* close = new QPushButton(tr("&Close"), buttons);

    connect(import, SIGNAL(clicked()), SLOT(slotImport()));
    connect(close, SIGNAL(clicked()), SLOT(close()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(import, 0, 1);
    buttonGrid->addWidget(close, 0, 2);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setRowStretch(1, 1);
    grid->addWidget(file, 0, 0);
    grid->addWidget(_log, 1, 0);
    grid->addWidget(buttons, 2, 0);

    _filePath->setFocus();

    setCentralWidget(frame);
    setCaption(tr("Data Import: " + company.name()));
}

ImportScreen::~ImportScreen()
{
}

void
ImportScreen::slotMessage(int count, QString severity, QString message)
{
    ListViewItem* item = new ListViewItem(_log);
    item->setValue(0, count);
    item->setValue(1, severity);
    item->setValue(2, _import->currentType());
    item->setValue(3, _import->currentName());
    item->setValue(4, message);
}

void
ImportScreen::slotOpenFile()
{
    QString start = _filePath->text();
    QString filter = "Import Files (*.xml)";

    if (start.isEmpty()) {
	ServerConfig config;
	config.load();
	start = parseDir(config.importDir);
    }

    QString file = QFileDialog::getOpenFileName(start, filter, this);
    if (file.isEmpty()) return;

    _filePath->setText(file);
}

void
ImportScreen::slotImport()
{
    QString filePath = _filePath->text();
    if (filePath.isEmpty()) {
	QString message = tr("An import file is required");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QFile file(filePath);
    if (!file.open(IO_ReadOnly)) {
	QString message = tr("Can't open import file: %1").arg(filePath);
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    _log->clear();
    bool result = _import->processFile(filePath);

    QApplication::restoreOverrideCursor();
    qApp->beep();

    if (!result) {
	int errorCount = _import->errorCount();
	QString message;
	if (errorCount == 1)
	    message = tr("Import completed with %1 error").arg(errorCount);
	else
	    message = tr("Import completed with %1 errors").arg(errorCount);
	QMessageBox::critical(this, tr("Status"), message);
    } else {
	QString message = tr("Import completed successfully");
	QMessageBox::information(this, tr("Status"), message);
    }
}
