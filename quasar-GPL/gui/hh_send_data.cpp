// $Id: hh_send_data.cpp,v 1.6 2005/06/22 06:01:40 bpepers Exp $
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

#include "hh_send_data.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "quasar_misc.h"
#include "user_config.h"
#include "item.h"
#include "item_select.h"
#include "adjust_reason.h"
#include "adjust_reason_select.h"
#include "send_xmodem.h"

#include <qapplication.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qstatusbar.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include <qdir.h>
#include <qfile.h>
#include <qregexp.h>

HandheldSendData::HandheldSendData(MainWindow* main)
    : QuasarWindow(main, "HandheldSendData")
{
    _helpSource = "hh_send_data.html";

    QVBox* top = new QVBox(this);
    top->setMargin(3);
    top->setSpacing(3);

    QString message = tr("This program downloads your inventory items\n"
			 "and adjustment reasons to the handheld unit.\n"
			 "Please select a serial device and then click\n"
			 "on \"Send Data\" to start the process.\n");
    new QLabel(message, top);

    QFrame* device = new QFrame(top);
    QLabel* deviceLabel = new QLabel(tr("Device:"), device);
    _device = new QComboBox(true, device);

    // TODO: it would be nice to try and figure out the valid serial
    // port devices and only list those here
    _device->insertItem("");
#ifdef WIN32
    _device->insertItem("COM1");
    _device->insertItem("COM2");
    _device->insertItem("COM3");
    _device->insertItem("COM4");
#else
    _device->insertItem("/dev/handheld");
    _device->insertItem("/dev/ttyS0");
    _device->insertItem("/dev/ttyS1");
    _device->insertItem("/dev/ttyUSB0");
    _device->insertItem("/dev/ttyUSB1");
#endif

    UserConfig config;
    if (config.load())
	_device->setCurrentText(config.handheldDevice);

    QGridLayout* deviceGrid = new QGridLayout(device);
    deviceGrid->setSpacing(3);
    deviceGrid->setMargin(3);
    deviceGrid->setColStretch(1, 1);
    deviceGrid->addWidget(deviceLabel, 0, 0);
    deviceGrid->addWidget(_device, 0, 1);

    QFrame* buttons = new QFrame(top);
    QPushButton* send = new QPushButton(tr("&Send"), buttons);
    QPushButton* quit = new QPushButton(tr("&Close"), buttons);

    connect(send, SIGNAL(clicked()), SLOT(slotSend()));
    connect(quit, SIGNAL(clicked()), SLOT(close()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(3);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(send, 0, 1);
    buttonGrid->addWidget(quit, 0, 2);

    statusBar()->hide();
    setCentralWidget(top);
    setCaption(tr("Send Data"));
    finalize();
}

HandheldSendData::~HandheldSendData()
{
}

void
HandheldSendData::slotSend()
{
    QString device = _device->currentText();
    if (device.isEmpty()) {
	QString message = tr("A serial device is required");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    SerialPort port(device);
    if (!port.open(IO_ReadWrite)) {
	QString message = tr("Failed to open %1 for read/write").arg(device);
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QFile file("data.dat");
    if (!file.open(IO_ReadWrite | IO_Truncate)) {
	QString message = tr("Failed to open 'data.dat' for writing");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    UserConfig config;
    if (config.load()) {
	config.handheldDevice = device;
	config.save(true);
    }

    Id store_id = _quasar->defaultStore();

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    vector<Item> items;
    ItemSelect conditions;
    conditions.activeOnly = true;
    conditions.inventoriedOnly = true;
    conditions.stockedOnly = true;
    conditions.store_id = store_id;
    _quasar->db()->select(items, conditions);

    vector<AdjustReason> reasons;
    AdjustReasonSelect cond;
    cond.activeOnly = true;
    _quasar->db()->select(reasons, cond);

    QApplication::restoreOverrideCursor();

    long count = items.size() + reasons.size();
    QString message = tr("Building data list...");
    QProgressDialog* progress = new QProgressDialog(message, tr("Cancel"),
						    count, this, "foo", true);
    progress->setMinimumDuration(0);
    progress->setCaption(tr("Progress"));

    QRegExp numExpr("[0-9]*");
    for (unsigned int i = 0; i < items.size(); ++i) {
	progress->setProgress(i);
	qApp->processEvents();
	if (progress->wasCancelled()) {
	    QString message = tr("Sending data cancelled");
	    QMessageBox::information(this, tr("Cancelled"), message);
	    file.remove();
	    return;
	}

	const Item& item = items[i];
	QString desc = item.description().left(16);

	for (unsigned int j = 0; j < item.numbers().size(); ++j) {
	    QString number = item.numbers()[j].number;
	    QString size = item.numbers()[j].size;
	    if (size.isEmpty()) size = item.sellSize();
	    fixed price = item.price(store_id, size).price() * 100.0;
	    price.roundTo(0);

	    if (number.length() > 14) continue;
	    if (!numExpr.exactMatch(number)) continue;
	    desc = desc.left(16);
	    if (price > 9999999) continue;

	    QString line;
	    line.sprintf("I%14s%-16s%7s\r\n", number.latin1(), desc.latin1(),
			 price.toString().latin1());
	    file.writeBlock(line, line.length());
	}
    }

    for (unsigned int i = 0; i < reasons.size(); ++i) {
	progress->setProgress(items.size() + i);
	qApp->processEvents();
	if (progress->wasCancelled()) {
	    QString message = tr("Sending data cancelled");
	    QMessageBox::information(this, tr("Cancelled"), message);
	    file.remove();
	    return;
	}

	const AdjustReason& reason = reasons[i];
	QString number = reason.number().left(5);
	QString name = reason.name().left(16);

	QString line;
	line.sprintf("R%5s%-16s%16s\r\n", number.latin1(), name.latin1(), "");
	file.writeBlock(line, line.length());
    }
    file.flush();

    progress->setProgress(count);
    delete progress;

    message = tr("The data is now ready to send.  Select\n"
		 "\"Load Data\" from the top menu on the\n"
		 "handheld and then click OK to start sending.");
    QMessageBox::information(this, tr("Ready To Send"), message);

    // Set line and raise DTR
    port.setSerial(38400, 8, SerialPort::PARITY_NONE, 1);
    port.setHandshake(SerialPort::HS_RTS_CTS);
    port.setStatus(SerialPort::DTR, true);

    // Wait for DSR or DCD
    int blocks = (file.size() + 127) / 128;
    count = blocks * 128 + 1;
    message = tr("Waiting for handheld...");
    progress = new QProgressDialog(message, tr("Cancel"), count, this, "foo",
				   true);
    progress->setMinimumDuration(0);
    progress->setCaption(tr("Progress"));

    progress->setProgress(0);
    while (true) {
	qApp->processEvents();
	if (progress->wasCancelled()) {
	    QString message = tr("Sending data cancelled");
	    QMessageBox::information(this, tr("Cancelled"), message);
	    port.setStatus(SerialPort::DTR, false);
	    file.remove();
	    return;
	}
	if (port.getStatus(SerialPort::DSR)) break;
	if (port.getStatus(SerialPort::DCD)) break;
    }

    progress->setLabelText(tr("Sending data records..."));
    qApp->processEvents();

    int result = sendXModem(file, port, progress);

    port.setStatus(SerialPort::DTR, false);
    file.remove();

    progress->setProgress(count);
    delete progress;
    qApp->beep();

    if (result == 0) {
	QMessageBox::information(this, tr("Complete"),
				 tr("All data has been sent"));
	delete this;
    } else {
	QMessageBox::critical(this, tr("Error"),
			      tr("Sending data failed"));
    }
}
