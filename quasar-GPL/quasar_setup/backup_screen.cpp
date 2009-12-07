// $Id: backup_screen.cpp,v 1.15 2005/03/01 19:53:02 bpepers Exp $
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

#include "backup_screen.h"
#include "quasar_version.h"
#include "server_config.h"
#include "data_model.h"
#include "line_edit.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qfile.h>
#include <qdir.h>
#include <qregexp.h>

#ifdef WIN32
#include <stdarg.h>
int
gzprintf(gzFile file, const char *format, ...)
{
    char buf[4096];
    va_list va;
    int len;

    va_start(va, format);
    //(void)vsnprintf(buf, sizeof(buf), format, va);
    (void)vsprintf(buf, format, va);
    va_end(va);

    len = strlen(buf);
    if (len <= 0) return 0;

    return gzwrite(file, buf, (unsigned)len);
}
#endif

BackupScreen::BackupScreen(const CompanyDefn& company)
    : QMainWindow(0, "BackupScreen", WType_TopLevel | WDestructiveClose),
      _connection(NULL)
{
    QFrame* frame = new QFrame(this);

    QLabel* fileLabel = new QLabel(tr("Backup File:"), frame);
    _filePath = new LineEdit(frame);
    _filePath->setMinimumWidth(50 * fontMetrics().width('x'));
    _filePath->addPopup(Key_F9, tr("browse"));
    fileLabel->setBuddy(_filePath);
    connect(_filePath, SIGNAL(doPopup(QKeySequence)), SLOT(slotOpenFile()));

    QPushButton* browse = new QPushButton("...", frame);
    browse->setFocusPolicy(ClickFocus);
    connect(browse, SIGNAL(clicked()), SLOT(slotOpenFile()));

    QLabel* startedLabel = new QLabel(tr("Started:"), frame);
    _started = new LineEdit(frame);
    _started->setMinimumWidth(30 * fontMetrics().width('x'));
    _started->setFocusPolicy(NoFocus);

    QLabel* elapsedLabel = new QLabel(tr("Elapsed:"), frame);
    _elapsed = new LineEdit(frame);
    _elapsed->setMinimumWidth(30 * fontMetrics().width('x'));
    _elapsed->setFocusPolicy(NoFocus);

    _currTable = new QLabel(frame);
    _tableProgress = new QProgressBar(frame);

    _currRecord = new QLabel(frame);
    _recordProgress = new QProgressBar(frame);

    QFrame* buttons = new QFrame(frame);
    _backup = new QPushButton(tr("&Backup"), buttons);
    _close = new QPushButton(tr("&Close"), buttons);

    connect(_backup, SIGNAL(clicked()), SLOT(slotBackup()));
    connect(_close, SIGNAL(clicked()), SLOT(slotClose()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(_backup, 0, 1);
    buttonGrid->addWidget(_close, 0, 2);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(1, 1);
    grid->addRowSpacing(1, 10);
    grid->addRowSpacing(4, 10);
    grid->addWidget(fileLabel, 0, 0);
    grid->addWidget(_filePath, 0, 1);
    grid->addWidget(browse, 0, 2);
    grid->addWidget(startedLabel, 2, 0);
    grid->addWidget(_started, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(elapsedLabel, 3, 0);
    grid->addWidget(_elapsed, 3, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_currTable, 5, 5, 0, 2, AlignLeft|AlignVCenter);
    grid->addMultiCellWidget(_tableProgress, 6, 6, 0, 2);
    grid->addMultiCellWidget(_currRecord, 7, 7, 0, 2, AlignLeft|AlignVCenter);
    grid->addMultiCellWidget(_recordProgress, 8, 8, 0, 2);
    grid->addMultiCellWidget(buttons, 9, 9, 0, 2);

    ServerConfig config;
    config.load();

    QString baseName = QFileInfo(company.database()).baseName() + ".bak";
    QString filePath = QDir(parseDir(config.backupDir)).filePath(baseName);
    _filePath->setText(filePath);
    _filePath->setFocus();

    _company = company;

    setCentralWidget(frame);
    setCaption(tr("Data Backup: %1").arg(company.name()));
}

BackupScreen::~BackupScreen()
{
    delete _connection;
}

void
BackupScreen::slotOpenFile()
{
    if (_connection != NULL) return;
    QFileInfo info(_filePath->text());

    QFileDialog* dialog = new QFileDialog(this, NULL, true);
    dialog->setCaption(tr("Backup To File"));
    dialog->setFilters(QString(tr("Backup Files (*.bak);;All Files (*)")));
    dialog->setDir(info.dirPath());
    dialog->setSelection(info.fileName());
    dialog->setMode(QFileDialog::AnyFile);

    if (dialog->exec() == QDialog::Accepted) {
	QString filePath = dialog->selectedFile();
	if (QFileInfo(filePath).extension().isEmpty())
	    filePath += ".bak";
	_filePath->setText(filePath);
    }

    delete dialog;
}

void
BackupScreen::slotBackup()
{
    QString filePath = _filePath->text();
    if (filePath.isEmpty()) {
	qApp->beep();
	QString message = tr("A backup file is required");
	QMessageBox::critical(this, tr("Error"), message);
	_filePath->setFocus();
	return;
    }

    // Check if file already exists in directory
    if (QFile::exists(filePath)) {
	qApp->beep();
	QString message = tr("The file %1\nalready exists.  "
	    "Overwrite it?").arg(filePath);
	int ch = QMessageBox::warning(this, tr("Error"), message,
				      QMessageBox::Yes, QMessageBox::No);
	if (ch != QMessageBox::Yes) return;
    }

    // Open backup file for writing
    _backupFile = gzopen(filePath, "wb9");
    if (_backupFile == NULL) {
	qApp->beep();
	QString message = tr("Failed to open '%1' for writing").arg(filePath);
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    // Connect to company database
    Driver* driver = Driver::getDriver(_company.dbType());
    if (driver == NULL) {
	gzclose(_backupFile);
	qApp->beep();

	QString message = tr("Get driver failed: %1").arg(_company.dbType());
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    _connection = driver->allocConnection();
    if (!_connection->dbaConnect(_company.database())) {
	delete _connection;
	_connection = NULL;
	gzclose(_backupFile);
	qApp->beep();

	QString message = tr("Open company failed: %1")
	    .arg(_connection->lastError());
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    // Increase concurrency level
    if (!_connection->setIsolationLevel(SERIALIZABLE)) {
	delete _connection;
	_connection = NULL;
	gzclose(_backupFile);
	qApp->beep();

	QString message = tr("Setting isolation level failed");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    ServerConfig config;
    config.load();

    // Get the data model for the company version
    QString dataDir = parseDir(config.dataDir);
    filePath = dataDir + "/models/" + _company.version() + ".xml";
    DataModel model;
    if (!model.load(filePath)) {
	delete _connection;
	_connection = NULL;
	gzclose(_backupFile);
	qApp->beep();

	QString message = tr("Data model not found");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    // Get tables and sort by dependencies
    QValueList<TableDefn> tables = model.tables();

    // Initialize
    _start = time(NULL);
    _started->setText(ctime(&_start));
    _backup->hide();
    _close->setText(tr("Abort"));
    _abort = false;

    gzprintf(_backupFile, "# name: " + _company.name() + "\n");
    gzprintf(_backupFile, "# version: " + _company.version() + "\n");
    gzprintf(_backupFile, "# date: " + QDate::currentDate().toString() + "\n");
    gzprintf(_backupFile, "# time: " + QTime::currentTime().toString() + "\n");
    gzprintf(_backupFile, "# tables: " + QString::number(tables.size())+"\n");
    gzprintf(_backupFile, "# end headers\n");

    _tableProgress->setTotalSteps(tables.size());
    for (unsigned int i = 0; i < tables.size() && !_abort; ++i) {
	const TableDefn& table = tables[i];
	_tableProgress->setProgress(i);

	if (!dumpTable(table.name)) {
	    qApp->beep();
	    QString message = tr("Error backing up table: %1").arg(table.name);
	    QMessageBox::critical(this, tr("Error"), message);
	    _abort = true;
	}
    }
    _tableProgress->setProgress(tables.size());

    delete _connection;
    _connection = NULL;
    gzclose(_backupFile);
    qApp->beep();

    if (_abort) {
	_started->setText("");
	_elapsed->setText("");
	_currTable->setText("");
	_tableProgress->reset();
	_currRecord->setText("");
	_recordProgress->reset();
	_backup->show();
	_close->setText(tr("&Close"));

	QMessageBox::critical(this, tr("Status"), tr("Backup aborted"));
	return;
    }

    QMessageBox::information(this, tr("Status"), tr("Backup completed"));
    close();
}

void
BackupScreen::slotClose()
{
    if (_connection != NULL) {
	_abort = true;
	return;
    }

    close();
}

static QString
quoteString(const QString& text)
{
    QString sql = text;
    sql.replace(QRegExp("'"), "''");
    sql.replace(QRegExp("%"), "%%");
    return "'" + sql + "'";
}

bool
BackupScreen::dumpTable(const QString& table)
{
    _currTable->setText(tr("Current Table: %1").arg(table));
    _currRecord->setText(tr("Counting..."));
    _recordProgress->reset();
    updateTime();

    QString cmd = "select count(*) from " + table;
    Stmt stmt(_connection, cmd);
    if (!stmt.execute()) {
	qWarning(stmt.lastError());
	return false;
    }

    if (!stmt.next()) {
	qWarning("no data selected");
	return false;
    }

    long count = stmt.getLong(1);
    QString comment = QString("# table: %1 %2").arg(table).arg(count);
    gzprintf(_backupFile, comment + "\n");

    // Calculate modulo of when to update screen.  Try to update every
    // one percent but don't allow the value to be too large or too small.
    long modulo = count / 100;
    if (modulo <= 0) modulo = 1;
    if (modulo > 1000) modulo = 1000;

    _currRecord->setText(tr("Selecting..."));
    _recordProgress->setTotalSteps(count);
    updateTime();
    if (_abort) return true;

    stmt.setCommand("select * from " + table);
    if (!stmt.execute()) {
	qWarning(stmt.lastError());
	return false;
    }

    QString columns = "";
    for (int col = 1; col <= stmt.columnCount(); ++col) {
	if (col > 1) columns += ",";
	columns += stmt.columnName(col);
    }

    long counter = 0;
    while (stmt.next() && !_abort) {
	if (counter % modulo == 0) {
	    _recordProgress->setProgress(counter);
	    QString msg = tr("Record %1 of %2").arg(counter).arg(count);
	    _currRecord->setText(msg);
	    updateTime();
	}
	++counter;

	QString values = "";
	for (int col = 1; col <= stmt.columnCount(); ++col) {
	    if (col > 1) values += ",";

	    if (stmt.isNull(col)) {
		values += "NULL";
		continue;
	    }

	    Variant value = stmt.getValue(col);
	    QString text;
	    switch (value.type()) {
	    case Variant::STRING:
	    case Variant::DATE:
	    case Variant::TIME:
		text = quoteString(value.toString());
		break;
	    case Variant::BOOL:
		text = quoteString(value.toBool() ? "Y" : "N");
		break;
	    default:
		text = value.toString();
	    }

	    text = text.replace(QRegExp("\n"), "\\n");
	    values += text;
	}

	QString line = "insert into " + table + " (" + columns +
	    ") values (" + values + ");\n";
	gzprintf(_backupFile, line.utf8());
    }
    _recordProgress->setProgress(count);

    return true;
}

void
BackupScreen::updateTime()
{
    time_t elapsed = time(NULL) - _start;

    int hours = elapsed / 3600;
    int minutes = (elapsed - hours * 3600) / 60;
    int seconds = elapsed % 60;

    QString message = tr("%1 seconds").arg(seconds);
    if (hours > 0) {
	if (hours == 1)
	    message = tr("%1 hour").arg(hours);
	else
	    message = tr("%1 hours").arg(hours);

	if (minutes == 1)
	    message += ", " + tr("%1 minute").arg(minutes);
	else if (minutes != 0)
	    message += ", " + tr("%1 minutes").arg(minutes);
    } else if (minutes > 0) {
	if (minutes == 1)
	    message = tr("%1 minute").arg(minutes);
	else
	    message = tr("%1 minutes").arg(minutes);
    }

    _elapsed->setText(message);
    qApp->processEvents();
}
