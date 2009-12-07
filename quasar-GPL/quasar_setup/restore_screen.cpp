// $Id: restore_screen.cpp,v 1.13 2005/04/04 07:31:33 bpepers Exp $
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

#include "restore_screen.h"
#include "quasar_version.h"
#include "server_config.h"
#include "data_model.h"
#include "line_edit.h"
#include "combo_box.h"

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

RestoreScreen::RestoreScreen()
    : QMainWindow(0, "RestoreScreen", WType_TopLevel | WDestructiveClose),
      _driver(NULL), _connection(NULL), _abort(false), _restoreFile(NULL)
{
    QFrame* frame = new QFrame(this);

    QLabel* nameLabel = new QLabel(tr("&Name:"), frame);
    _name = new LineEdit(60, frame);
    nameLabel->setBuddy(_name);

    QLabel* typeLabel = new QLabel(tr("&Database:"), frame);
    _type = new ComboBox(frame);
    typeLabel->setBuddy(_type);
    _type->insertStringList(Driver::types());

    QLabel* fileLabel = new QLabel(tr("&Restore File:"), frame);
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
    _restore = new QPushButton(tr("&Restore"), buttons);
    _close = new QPushButton(tr("&Close"), buttons);

    connect(_restore, SIGNAL(clicked()), SLOT(slotRestore()));
    connect(_close, SIGNAL(clicked()), SLOT(slotClose()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(_restore, 0, 1);
    buttonGrid->addWidget(_close, 0, 2);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(1, 1);
    grid->addRowSpacing(3, 16);
    grid->addRowSpacing(6, 16);
    grid->addWidget(nameLabel, 0, 0);
    grid->addMultiCellWidget(_name, 0, 0, 1, 2);
    grid->addWidget(typeLabel, 1, 0);
    grid->addWidget(_type, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(fileLabel, 2, 0);
    grid->addWidget(_filePath, 2, 1);
    grid->addWidget(browse, 2, 2);
    grid->addWidget(startedLabel, 4, 0);
    grid->addWidget(_started, 4, 1, AlignLeft | AlignVCenter);
    grid->addWidget(elapsedLabel, 5, 0);
    grid->addWidget(_elapsed, 5, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_currTable, 7, 7, 0, 2, AlignLeft|AlignVCenter);
    grid->addMultiCellWidget(_tableProgress, 8, 8, 0, 2);
    grid->addMultiCellWidget(_currRecord, 9, 9, 0, 2, AlignLeft|AlignVCenter);
    grid->addMultiCellWidget(_recordProgress, 10, 10, 0, 2);
    grid->addMultiCellWidget(buttons, 11, 11, 0, 2);

    _config.load();

    _filePath->setText(parseDir(_config.backupDir));
    _name->setFocus();

    setCentralWidget(frame);
    setCaption(tr("Data Restore"));
}

RestoreScreen::~RestoreScreen()
{
    delete _connection;
}

void
RestoreScreen::slotOpenFile()
{
    if (_connection != NULL) return;
    QFileInfo info(_filePath->text());

    QFileDialog* dialog = new QFileDialog(this, NULL, true);
    dialog->setCaption(tr("Restore From File"));
    dialog->setMode(QFileDialog::ExistingFile);
    dialog->setFilters(QString(tr("Backup Files (*.bak);;All Files (*)")));
    dialog->setDir(info.dirPath());
    dialog->setSelection(info.fileName());

    if (dialog->exec() == QDialog::Accepted)
	_filePath->setText(dialog->selectedFile());

    delete dialog;
}

void
RestoreScreen::slotRestore()
{
    QString companyName = _name->text();
    if (companyName.isEmpty()) {
	qApp->beep();
	QString message = tr("A company name is required");
	QMessageBox::critical(this, tr("Error"), message);
	_name->setFocus();
	return;
    }
    // TODO: check for duplicate company names

    QString databaseType = _type->currentText();
    if (databaseType.isEmpty()) {
	qApp->beep();
	QString message = tr("A database type is required");
	QMessageBox::critical(this, tr("Error"), message);
	_type->setFocus();
	return;
    }

    _driver = Driver::getDriver(databaseType);
    if (_driver == NULL) {
	qApp->beep();
	QString message = tr("Failed getting driver: %1").arg(databaseType);
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QString filePath = _filePath->text();
    if (filePath.isEmpty()) {
	qApp->beep();
	QString message = tr("A restore file is required");
	QMessageBox::critical(this, tr("Error"), message);
	_filePath->setFocus();
	return;
    }
    if (!QFile::exists(filePath)) {
	qApp->beep();
	QString message = tr("The file '%1'\ndoesn't exist").arg(filePath);
	QMessageBox::critical(this, tr("Error"), message);
	_filePath->setFocus();
	return;
    }

    // Open restore file for reading
    _restoreFile = gzopen(filePath, "rb");
    if (_restoreFile == NULL) {
	qApp->beep();
	QString message = tr("Failed to open '%1'\nfor reading").arg(filePath);
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    // Read in header tags (name, version, date, time, ...)
    QMap<QString,QString> tags;
    do {
	char buffer[16384];
	if (gzgets(_restoreFile, buffer, 16384) == NULL) break;

	QString line(buffer);
	line = line.left(line.length() - 1);
	if (line == "# end headers") break;

	if (line.left(2) != "# ") {
	    qApp->beep();
	    QString message = tr("Error reading backup file header");
	    QMessageBox::critical(this, tr("Error"), message);
	    return;
	}

	int pos = line.find(':');
	if (pos == -1) {
	    qApp->beep();
	    QString message = tr("Invalid header line: %1").arg(line);
	    QMessageBox::critical(this, tr("Error"), message);
	    return;
	}

	QString name = line.mid(2, pos - 2);
	QString text = line.mid(pos + 2);
	tags[name] = text;
    } while (!gzeof(_restoreFile));

    QString version = tags["version"];
    QString name = tags["name"];
    QString backupDate = tags["date"];
    QString backupTime = tags["time"];
    int tableCount = tags["tables"].toInt();

    ServerConfig config;
    config.load();

    // Get the data model for the company version
    QString dataDir = parseDir(config.dataDir);
    if (!_model.load(dataDir + "/models/" + version + ".xml")) {
	qApp->beep();
	QString message = tr("Failed loading data model");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    _company.setName(companyName);
    _company.setVersion(version);

    // TODO: show user information and ask if sure

    setCaption(tr("Data Restore: %1").arg(name));
    _start = time(NULL);
    _started->setText(ctime(&_start));
    _restore->hide();
    _close->setText(tr("Abort"));
    _abort = false;
    _tableProgress->setTotalSteps(tableCount + 4);

    if (!createCompany()) _abort = true;
    if (!_abort && !createTables()) _abort = true;
    if (!_abort && !restoreData()) _abort = true;
    if (!_abort && !createConstraints()) _abort = true;
    if (!_abort && !createIndexes()) _abort = true;

    delete _connection;
    _connection = NULL;
    gzclose(_restoreFile);
    qApp->beep();

    if (_abort) {
	_started->setText("");
	_elapsed->setText("");
	_currTable->setText("");
	_tableProgress->reset();
	_currRecord->setText("");
	_recordProgress->reset();
	_restore->show();
	_close->setText(tr("&Close"));
	_driver->remove(_company);

	QMessageBox::critical(this, tr("Status"), tr("Restore aborted"));
	return;
    }

    // Save company definition
    QString file = parseDir(_config.dataDir) + "/companies/" +
	companyName + ".xml";
    if (!_company.save(file)) {
	QString error = _company.lastError();
	_driver->remove(_company);

	qApp->beep();
	QMessageBox::critical(this, tr("Error"), error);
	return;
    }

    QMessageBox::information(this, tr("Status"), tr("Restore completed"));
    close();
}

void
RestoreScreen::slotClose()
{
    if (_connection != NULL) {
	_abort = true;
	return;
    }

    close();
}

bool
RestoreScreen::createCompany()
{
    _currTable->setText("Create database");
    _tableProgress->setProgress(0);
    updateTime();

    // Create the company database
    if (!_driver->create(_company)) {
	qApp->beep();
	QString message = tr("Create company failed: %1")
	    .arg(_driver->lastError());
	QMessageBox::critical(this, tr("Error"), message);
	return false;
    }

    // Connect to the company database
    _connection = _driver->allocConnection();
    if (!_connection->dbaConnect(_company.database())) {
	qApp->beep();
	QString message = tr("Create company failed: %1")
	    .arg(_connection->lastError());
	QMessageBox::critical(this, tr("Error"), message);
	return false;
    }

    // Sybase init
    if (_driver->name() == "Sybase") {
	QString cmd = "set temporary option wait_for_commit = 'on'";
	_connection->execute(cmd);
    }

    return true;
}

bool
RestoreScreen::createTables()
{
    int count = _model.tables().size();
    _currTable->setText("Create tables");
    _tableProgress->setProgress(1);
    _recordProgress->reset();
    _recordProgress->setTotalSteps(count);
    updateTime();

    QValueList<TableDefn> tables = _model.tables();
    for (unsigned int i = 0; i < tables.size(); ++i) {
	_recordProgress->setProgress(i);
	QString msg = tr("Record %1 of %2").arg(i + 1).arg(count);
	_currRecord->setText(msg);
	updateTime();

	const TableDefn& table = tables[i];
	if (!_connection->create(table)) {
	    qApp->beep();
	    QString message = tr("Create tables failed: %1")
		.arg(_connection->lastError());
	    QMessageBox::critical(this, tr("Error"), message);
	    return false;
	}
    }

    return _connection->commit();
}

bool
RestoreScreen::createConstraints()
{
    int count = _model.tables().size();
    _currTable->setText("Create constraints");
    _tableProgress->setProgress(_tableProgress->progress() + 1);
    _recordProgress->reset();
    _recordProgress->setTotalSteps(count);
    updateTime();

    QValueList<TableDefn> tables = _model.tables();
    for (unsigned int i = 0; i < tables.size(); ++i) {
	_recordProgress->setProgress(i);
	QString msg = tr("Record %1 of %2").arg(i + 1).arg(count);
	_currRecord->setText(msg);
	updateTime();

	const TableDefn& table = tables[i];
	for (unsigned int j = 0; j < table.constraints.size(); ++j) {
	    const ConstraintDefn& constraint = table.constraints[j];
	    if (!_connection->create(constraint)) {
		qApp->beep();
		QString message = tr("Create constraint failed: %1")
		    .arg(_connection->lastError());
		QMessageBox::critical(this, tr("Error"), message);
		return false;
	    }
	}
    }

    return _connection->commit();
}

bool
RestoreScreen::createIndexes()
{
    int count = _model.tables().size();
    _currTable->setText("Create indexes");
    _tableProgress->setProgress(_tableProgress->progress() + 1);
    _recordProgress->reset();
    _recordProgress->setTotalSteps(count);
    updateTime();

    QValueList<TableDefn> tables = _model.tables();
    for (unsigned int i = 0; i < tables.size(); ++i) {
	_recordProgress->setProgress(i);
	QString msg = tr("Record %1 of %2").arg(i + 1).arg(count);
	_currRecord->setText(msg);
	updateTime();

	const TableDefn& table = tables[i];
	for (unsigned int j = 0; j < table.indexes.size(); ++j) {
	    const IndexDefn& index = table.indexes[j];
	    if (!_connection->create(index)) {
		qApp->beep();
		QString message = tr("Create index failed: %1")
		    .arg(_connection->lastError());
		QMessageBox::critical(this, tr("Error"), message);
		return false;
	    }
	}
    }

    return _connection->commit();
}

bool
RestoreScreen::restoreData()
{
    QString table = "";
    int tableNum = 2;
    long count = 0;
    long counter = 0;
    long modulo = 1;

    int line = 0;
    Stmt stmt(_connection);
    do {
	char buffer[16384];
	if (gzgets(_restoreFile, buffer, 16384) == NULL) {
	    qWarning("Reading backup file failed");
	    return false;
	}
	++line;

	QString cmd = QString::fromUtf8(buffer);
	cmd = cmd.replace(QRegExp("\\\\n"), "\n");
	cmd = cmd.left(cmd.length() - 1);
	if (cmd.left(1) == "#") {
	    if (cmd.left(9) == "# table: ") {
		QStringList data = QStringList::split(" ", cmd.mid(9));
		table = data[0];
		count = data[1].toLong();
		counter = 0;
		_tableProgress->setProgress(tableNum++);
	    } else if (cmd.left(9) == "# extra: ") {
		table += " extra";
		count = cmd.mid(9).toLong();
		counter = 0;
	    }

	    modulo = count / 100;
	    if (modulo <= 0) modulo = 1;
	    if (modulo > 1000) modulo = 1000;

	    _currTable->setText(tr("Current Table: %1").arg(table));
	    QString msg = tr("Record %1 of %2").arg(counter).arg(count);
	    _currRecord->setText(msg);
	    _recordProgress->reset();
	    _recordProgress->setTotalSteps(count);
	    updateTime();
	    continue;
	}

	if (counter % modulo == 0) {
	    _recordProgress->setProgress(counter);
	    QString msg = tr("Record %1 of %2").arg(counter).arg(count);
	    _currRecord->setText(msg);
	    updateTime();
	}
	++counter;

	stmt.setCommand(cmd);
	if (!stmt.execute()) {
	    qApp->beep();
	    QString message = tr("Restore failed: %1").arg(stmt.lastError());
	    QMessageBox::critical(this, tr("Error"), message);
	    return false;
	}

	if ((line % 1000) == 0)
	    _connection->commit();
    } while (!gzeof(_restoreFile) && !_abort);

    if (!_connection->commit()) {
	qApp->beep();
	QString message = tr("Restore failed: %1")
	    .arg(_connection->lastError());
	QMessageBox::critical(this, tr("Error"), message);
	return false;
    }

    // Database cleanup
    if (_driver->name() == "Sybase") {
	QString cmd = "set temporary option wait_for_commit = 'off'";
	_connection->execute(cmd);
    } else if (_driver->name() == "PostgreSQL") {
	QString cmd = "vacuum full analyze";
	_connection->execute(cmd);
    }

    return true;
}

void
RestoreScreen::updateTime()
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
