// $Id: send_email.cpp,v 1.7 2004/01/31 01:50:31 arandell Exp $
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

#include "send_email.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "user_config.h"
#include "smtp.h"
#include "company.h"
#include "integer_edit.h"
#include "list_view_item.h"
#include "card.h"
#include "card_select.h"

#include <qapplication.h>
#include <qstatusbar.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qfiledialog.h>
#include <qfile.h>
#include <qtextstream.h>
#include <assert.h>

SendEmail::SendEmail(MainWindow* main)
    : QuasarWindow(main, "SendEmail"), _smtp(NULL)
{
    _helpSource = "send_email.html";

    QFrame* frame = new QFrame(this);
    _tabs = new QTabWidget(frame);

    _customer = new ListView(_tabs);
    _tabs->addTab(_customer, tr("Customers"));
    _customer->addTextColumn(tr("Name"), 30);
    _customer->addTextColumn(tr("Email Address"), 30);
    _customer->setSorting(0);
    _customer->setSelectionMode(QListView::Multi);
    _customer->setAllColumnsShowFocus(true);
    _customer->setShowSortIndicator(true);

    _vendor = new ListView(_tabs);
    _tabs->addTab(_vendor, tr("Vendors"));
    _vendor->addTextColumn(tr("Name"), 30);
    _vendor->addTextColumn(tr("Email Address"), 30);
    _vendor->setSorting(0);
    _vendor->setSelectionMode(QListView::Multi);
    _vendor->setAllColumnsShowFocus(true);
    _vendor->setShowSortIndicator(true);

    _employee = new ListView(_tabs);
    _tabs->addTab(_employee, tr("Employees"));
    _employee->addTextColumn(tr("Name"), 30);
    _employee->addTextColumn(tr("Email Address"), 30);
    _employee->setSorting(0);
    _employee->setSelectionMode(QListView::Multi);
    _employee->setAllColumnsShowFocus(true);
    _employee->setShowSortIndicator(true);

    _personal = new ListView(_tabs);
    _tabs->addTab(_personal, tr("Personal"));
    _personal->addTextColumn(tr("Name"), 30);
    _personal->addTextColumn(tr("Email Address"), 30);
    _personal->setSorting(0);
    _personal->setSelectionMode(QListView::Multi);
    _personal->setAllColumnsShowFocus(true);
    _personal->setShowSortIndicator(true);

    _manual = new QFrame(_tabs);
    _tabs->addTab(_manual, tr("Manual"));

    QLabel* toLabel = new QLabel(tr("To:"), _manual);
    _to = new LineEdit(_manual);
    toLabel->setBuddy(_to);

    QGridLayout* toGrid = new QGridLayout(_manual);
    toGrid->setSpacing(3);
    toGrid->setMargin(6);
    toGrid->setRowStretch(1, 1);
    toGrid->setColStretch(1, 1);
    toGrid->addWidget(toLabel, 0, 0);
    toGrid->addWidget(_to, 0, 1);

    QLabel* fromLabel = new QLabel(tr("From:"), frame);
    _from = new LineEdit(frame);
    fromLabel->setBuddy(_from);

    QLabel* subjectLabel = new QLabel(tr("Subject:"), frame);
    _subject = new LineEdit(frame);
    subjectLabel->setBuddy(_subject);

    _message = new QTextEdit(frame);

    QFrame* buttons = new QFrame(frame);

    QPushButton* config = new QPushButton(tr("Configure..."), buttons);
    config->setFocusPolicy(ClickFocus);
    connect(config, SIGNAL(clicked()), SLOT(slotConfigure()));

    QPushButton* insert = new QPushButton(tr("Insert File..."), buttons);
    insert->setFocusPolicy(ClickFocus);
    connect(insert, SIGNAL(clicked()), SLOT(slotInsertFile()));

    _send = new QPushButton(tr("&Send"), buttons);
    connect(_send, SIGNAL(clicked()), SLOT(slotSend()));

    QPushButton* cancel = new QPushButton(tr("&Cancel"), buttons);
    connect(cancel, SIGNAL(clicked()), SLOT(close()));

    _send->setMinimumSize(cancel->sizeHint());
    cancel->setMinimumSize(cancel->sizeHint());

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(1, 1);
    buttonGrid->addWidget(config, 0, 0, AlignLeft | AlignVCenter);
    buttonGrid->addWidget(insert, 0, 1, AlignLeft | AlignVCenter);
    buttonGrid->addWidget(_send, 0, 2, AlignRight | AlignVCenter);
    buttonGrid->addWidget(cancel, 0, 3, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setRowStretch(3, 1);
    grid->setColStretch(1, 1);
    grid->addMultiCellWidget(_tabs, 0, 0, 0, 1);
    grid->addWidget(fromLabel, 1, 0);
    grid->addWidget(_from, 1, 1);
    grid->addWidget(subjectLabel, 2, 0);
    grid->addWidget(_subject, 2, 1);
    grid->addMultiCellWidget(_message, 3, 3, 0, 1);
    grid->addMultiCellWidget(buttons, 4, 4, 0, 1);

    UserConfig userConfig;
    userConfig.load();
    _host = userConfig.smtpHost;
    _port = userConfig.smtpPort;

    Company company;
    _quasar->db()->lookup(company);
    _from->setText(company.email());

    _tabs->setFocus();
    setCentralWidget(frame);
    setCaption(tr("Send Email"));
    statusBar();
    finalize();

    slotRefresh();
    //QTimer::singleShot(0, this, SLOT(slotRefresh()));
}

SendEmail::~SendEmail()
{
    delete _smtp;
}

void
SendEmail::setTo(const QString& address)
{
    _tabs->showPage(_manual);
    _to->setText(address);
}

void
SendEmail::setFrom(const QString& address)
{
    _from->setText(address);
    if (!address.isEmpty())
	_subject->setFocus();
}

void
SendEmail::setSubject(const QString& text)
{
    _subject->setText(text);
    if (!text.isEmpty())
	_message->setFocus();
}

void
SendEmail::setCard(Id card_id)
{
    Card card;
    if (!_quasar->db()->lookup(card_id, card))
	return;

    ListViewItem* child;

    switch (card.dataType()) {
    case DataObject::CUSTOMER:
	_tabs->showPage(_customer);
	child = _customer->firstChild();
	while (child != NULL) {
	    if (child->id == card.id()) {
		_customer->setSelected(child, true);
		break;
	    }
	    child = child->nextSibling();
	}
	break;
    case DataObject::VENDOR:
	_tabs->showPage(_vendor);
	child = _vendor->firstChild();
	while (child != NULL) {
	    if (child->id == card.id()) {
		_vendor->setSelected(child, true);
		break;
	    }
	    child = child->nextSibling();
	}
	break;
    case DataObject::EMPLOYEE:
	_tabs->showPage(_employee);
	child = _employee->firstChild();
	while (child != NULL) {
	    if (child->id == card.id()) {
		_employee->setSelected(child, true);
		break;
	    }
	    child = child->nextSibling();
	}
	break;
    case DataObject::PERSONAL:
	_tabs->showPage(_personal);
	child = _personal->firstChild();
	while (child != NULL) {
	    if (child->id == card.id()) {
		_personal->setSelected(child, true);
		break;
	    }
	    child = child->nextSibling();
	}
	break;
    default:
	assert(false);
    }

    _subject->setFocus();
}

void
SendEmail::slotConfigure()
{
    QDialog* dialog = new QDialog(this, "Config", true);
    dialog->setCaption("Email Configuration");

    QLabel* hostLabel = new QLabel(tr("Mail Hostname:"), dialog);
    LineEdit* host = new LineEdit(30, dialog);
    hostLabel->setBuddy(host);

    QLabel* portLabel = new QLabel(tr("Port Number:"), dialog);
    IntegerEdit* port = new IntegerEdit(dialog);
    port->setLength(6);
    portLabel->setBuddy(port);

    QFrame* buttons = new QFrame(dialog);
    QPushButton* ok = new QPushButton(tr("Ok"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);

    ok->connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    cancel->connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
    ok->setDefault(true);

    QGridLayout* grid1 = new QGridLayout(buttons);
    grid1->setSpacing(6);
    grid1->setMargin(6);
    grid1->setColStretch(0, 1);
    grid1->addWidget(ok, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
    grid1->addWidget(cancel, 0, 1, Qt::AlignRight | Qt::AlignVCenter);

    QGridLayout* grid = new QGridLayout(dialog);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setColStretch(1, 1);
    grid->addWidget(hostLabel, 0, 0);
    grid->addWidget(host, 0, 1);
    grid->addWidget(portLabel, 1, 0);
    grid->addWidget(port, 1, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(buttons, 2, 2, 0, 1);

    host->setText(_host);
    port->setInt(_port);
    host->setFocus();

    if (dialog->exec()) {
	UserConfig config;
	config.load();
	config.smtpHost = host->text();
	config.smtpPort = port->getInt();
	config.save();

	_host = config.smtpHost;
	_port = config.smtpPort;
    }
    delete dialog;
}

void
SendEmail::slotInsertFile()
{
    QFileDialog* dialog = new QFileDialog(this, NULL, true);
    dialog->setCaption(tr("Insert File"));
    dialog->setMode(QFileDialog::ExistingFile);
    dialog->setFilters(tr("All Files (*)"));
    if (dialog->exec() != QDialog::Accepted) return;
    if (dialog->selectedFile().isEmpty()) return;
    QString filepath = dialog->selectedFile();
    delete dialog;

    QFile file(filepath);
    if (!file.open(IO_ReadOnly)) return;

    QTextStream stream(&file);
    _message->setText(stream.read());
}

void
SendEmail::slotSend()
{
    if (_host.isEmpty()) {
	QString message = tr("You must configure a mail server\n"
			     "before you can send any email.");
	QMessageBox::warning(this, "Warning", message);
	slotConfigure();
	if (_host.isEmpty()) return;
    }

    _toList.clear();
    _toPos = 0;

    QListViewItem* child = _customer->firstChild();
    while (child != NULL) {
	if (child->isSelected())
	    _toList << child->text(1);
	child = child->nextSibling();
    }
    child = _vendor->firstChild();
    while (child != NULL) {
	if (child->isSelected())
	    _toList << child->text(1);
	child = child->nextSibling();
    }
    child = _employee->firstChild();
    while (child != NULL) {
	if (child->isSelected())
	    _toList << child->text(1);
	child = child->nextSibling();
    }
    child = _personal->firstChild();
    while (child != NULL) {
	if (child->isSelected())
	    _toList << child->text(1);
	child = child->nextSibling();
    }
    if (!_to->text().isEmpty())
	_toList << _to->text();

    // Check thats something was selected
    if (_toList.count() == 0) {
	QMessageBox::critical(this, tr("Error"), tr("No cards selected"));
	return;
    }

    _send->setEnabled(false);
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    _smtp = new Smtp(_host, _port);
    _smtp->setFrom(_from->text());
    _smtp->setTo(_toList[_toPos++]);
    _smtp->setSubject(_subject->text());
    _smtp->setBody(_message->text());
    connect(_smtp, SIGNAL(finished(bool)), SLOT(slotSendDone(bool)));
    connect(_smtp, SIGNAL(status(const QString&)),
	    SLOT(slotStatus(const QString&)));
    _smtp->sendMail();
}

void
SendEmail::slotSendDone(bool result)
{
    if (_toPos < _toList.count() && result) {
	_smtp->setTo(_toList[_toPos++]);
	_smtp->sendMail();
	return;
    }

    delete _smtp;
    _smtp = NULL;
    _send->setEnabled(true);
    QApplication::restoreOverrideCursor();

    if (!result) {
	QMessageBox::critical(this, tr("Error"), tr("Sending mail failed"));
	statusBar()->clear();
    } else {
	close();
    }
}

void
SendEmail::slotStatus(const QString& message)
{
    statusBar()->message(message);
    qDebug(message);
}

void
SendEmail::slotRefresh()
{
    _customer->clear();
    _vendor->clear();
    _employee->clear();
    _personal->clear();

    vector<Card> cards;
    vector<Id> card_ids;
    _quasar->db()->select(cards, CardSelect());

    for (unsigned int i = 0; i < cards.size(); ++i) {
	const Card& card = cards[i];
	if (card.email().isEmpty()) continue;

	ListViewItem* item;

	switch (card.dataType()) {
	case DataObject::CUSTOMER:
	    item = new ListViewItem(_customer, card.id());
	    break;
	case DataObject::VENDOR:
	    item = new ListViewItem(_vendor, card.id());
	    break;
	case DataObject::EMPLOYEE:
	    item = new ListViewItem(_employee, card.id());
	    break;
	case DataObject::PERSONAL:
	    item = new ListViewItem(_personal, card.id());
	    break;
	default:
	    assert(0);
	    break;
	}

	item->setValue(0, card.name());
	item->setValue(1, card.email());
    }
}
