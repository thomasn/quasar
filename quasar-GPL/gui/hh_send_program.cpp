// $Id: hh_send_program.cpp,v 1.20 2005/03/16 15:39:19 bpepers Exp $
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

#include "hh_send_program.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "quasar_misc.h"
#include "user_config.h"
#include "serial_port.h"

#include <qapplication.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qstatusbar.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include <qfile.h>

#define BUF_SIZE 80

HandheldSendProgram::HandheldSendProgram(MainWindow* main)
    : QuasarWindow(main, "HandheldSendProgram")
{
    _helpSource = "hh_send_program.html";

    QVBox* top = new QVBox(this);
    top->setMargin(3);
    top->setSpacing(3);

    QString message = tr("This program downloads your inventory program\n"
	"to the handheld unit.  Please place the handheld\n"
	"into the cradle, select \"Receive Program\" on the\n"
	"handheld, and click the \"Send Program\" button.\n");
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
    QPushButton* send = new QPushButton(tr("&Send Program"), buttons);
    QPushButton* quit = new QPushButton(tr("&Close"), buttons);

    connect(send, SIGNAL(clicked()), SLOT(slotSendProgram()));
    connect(quit, SIGNAL(clicked()), SLOT(close()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(3);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(send, 0, 1);
    buttonGrid->addWidget(quit, 0, 2);

    statusBar()->hide();
    setCentralWidget(top);
    setCaption(tr("Send Handheld Program"));
    finalize();
}

HandheldSendProgram::~HandheldSendProgram()
{
}

void
HandheldSendProgram::slotSendProgram()
{
    QString filePath;
    if (!_quasar->resourceFetch("handheld", "handheld.hex", filePath)) {
	QString message = tr("Missing handheld program");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QFile file(filePath);
    if (!file.open(IO_ReadOnly)) {
	QString message = tr("Error opening handheld.hex file");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QString device = _device->currentText();
    if (device.isEmpty()) {
	QString message = tr("A serial device is required");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    SerialPort port(device);
    if (!port.open(IO_WriteOnly)) {
	QString message = tr("Failed to open %1 for writing").arg(device);
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    UserConfig config;
    if (config.load()) {
	config.handheldDevice = device;
	config.save(true);
    }

    // Set line and raise DTR
    port.setSerial(38400, 7, SerialPort::PARITY_ODD, 1);
    port.setHandshake(SerialPort::HS_XON_XOFF);
    port.setStatus(SerialPort::DTR, true);

    // Wait for DSR or DCD
    long count = file.size() + 1;
    QString message = tr("Waiting for handheld...");
    QProgressDialog* progress = new QProgressDialog(message, tr("Cancel"),
						    count, this, "foo", true);
    progress->setMinimumDuration(0);
    progress->setCaption(tr("Progress"));

    progress->setProgress(0);
    while (true) {
	qApp->processEvents();
	if (progress->wasCancelled()) {
	    QString message = tr("Sending program cancelled");
	    QMessageBox::information(this, tr("Cancelled"), message);
	    port.setStatus(SerialPort::DTR, false);
	    return;
	}
	if (port.getStatus(SerialPort::DSR)) break;
	if (port.getStatus(SerialPort::DCD)) break;
    }

    progress->setLabelText(tr("Sending handheld program..."));
    qApp->processEvents();
    qsleep(1);

    long pos = 0;
    char buf[BUF_SIZE];
    while (1) {
	progress->setProgress(pos);
	qApp->processEvents();
	if (progress->wasCancelled()) {
	    QString message = tr("Sending program cancelled");
	    QMessageBox::information(this, tr("Cancelled"), message);
	    port.setStatus(SerialPort::DTR, false);
	    return;
	}

	int bytes = file.readBlock(buf, BUF_SIZE);
	if (bytes <= 0) break;

	if (port.writeBlock(buf, bytes) != bytes) {
	    QString message = tr("Write to serial port failed");
	    QMessageBox::critical(this, tr("Error"), message);
	    port.setStatus(SerialPort::DTR, false);
	    return;
	}

	pos += bytes;
    }

    port.drain();
    qsleep(2);
    port.setStatus(SerialPort::DTR, false);

    progress->setProgress(count);
    delete progress;
    qApp->beep();

    QMessageBox::information(this, tr("Complete"),tr("Program has been sent"));
    close();
}
