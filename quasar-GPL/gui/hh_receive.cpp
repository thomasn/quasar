// $Id: hh_receive.cpp,v 1.28 2005/06/22 06:01:40 bpepers Exp $
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

#include "hh_receive.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "quasar_misc.h"
#include "user_config.h"
#include "item_lookup.h"
#include "count_master.h"
#include "price_batch.h"
#include "price_batch_master.h"
#include "label_batch.h"
#include "label_batch_master.h"
#include "slip_master.h"
#include "item_adjust.h"
#include "item_adjustment.h"
#include "item.h"
#include "count.h"
#include "adjust_reason.h"
#include "item_select.h"
#include "slip_select.h"
#include "recv_xmodem.h"

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
#include <qdir.h>

HandheldReceive::HandheldReceive(MainWindow* main)
    : QuasarWindow(main, "HandheldReceive")
{
    _helpSource = "hh_receive.html";

    QVBox* top = new QVBox(this);
    top->setMargin(3);
    top->setSpacing(3);

    QString message = tr("This program receives data from the handheld\n"
			 "unit and creates labels or prices from the\n"
			 "data.  Please place the handheld into the\n"
			 "cradle, press \"T\" in one of the data files,\n"
			 "and click the \"Receive\" button.\n");
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
    QPushButton* receive = new QPushButton(tr("&Receive"), buttons);
    QPushButton* quit = new QPushButton(tr("&Close"), buttons);

    connect(receive, SIGNAL(clicked()), SLOT(slotReceive()));
    connect(quit, SIGNAL(clicked()), SLOT(close()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(3);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(receive, 0, 1);
    buttonGrid->addWidget(quit, 0, 2);

    statusBar()->hide();
    setCentralWidget(top);
    setCaption(tr("Handheld Receive"));
    finalize();
}

HandheldReceive::~HandheldReceive()
{
}

typedef QPair<Id, fixed> InfoPair;

static void
addInfo(vector<InfoPair>& info, Id id, fixed amount)
{
    if (id == INVALID_ID) return;
    for (unsigned int i = 0; i < info.size(); ++i) {
	if (info[i].first == id) {
	    info[i].second += amount;
	    return;
	}
    }
    info.push_back(InfoPair(id, amount));
}

void
HandheldReceive::slotReceive()
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

    UserConfig config;
    if (config.load()) {
	config.handheldDevice = device;
	config.save(true);
    }

    // Set line, raise DTR, and sleep for a short time for PDT
    port.setSerial(38400, 8, SerialPort::PARITY_NONE, 1);
    port.setHandshake(SerialPort::HS_RTS_CTS);
    port.setStatus(SerialPort::DTR, true);
    qsleep(1);

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    QString saveDir = QDir::currentDirPath();
#ifdef WIN32
    QDir::setCurrent("C:/");
#else
    QDir::setCurrent("/tmp");
#endif

    int result = recvXModem(port);
    if (result != 0) {
	QApplication::restoreOverrideCursor();
	qApp->beep();

	QMessageBox::critical(this, tr("Error"), tr("Error receiving data"));
	QDir::setCurrent(saveDir);
	return;
    }

    ItemSelect conditions;
    conditions.store_id = _quasar->defaultStore();
    conditions.stockedOnly = true;
    conditions.activeOnly = true;
    vector<Item> items;

    if (QFile::exists("label")) {
	QFile file("label");
	if (!file.open(IO_ReadOnly)) {
	    QApplication::restoreOverrideCursor();
	    qApp->beep();

	    QString message = tr("Failed opening label file");
	    QMessageBox::critical(this, tr("Error"), message);
	    QDir::setCurrent(saveDir);
	    return;
	}

	Id store_id = _quasar->defaultStore();

	LabelBatch batch;
	batch.setNumber("#");
	batch.setDescription("From handheld");
	batch.setStoreId(store_id);

	QString line;
	while (file.readLine(line, 100) > 0) {
	    QString number = line.left(14).stripWhiteSpace();
	    QString qty = line.mid(14).stripWhiteSpace();

	    _db->lookup(conditions, number, items);
	    if (items.size() == 0) {
		LabelBatchItem line;
		line.number = number;
		batch.items().push_back(line);
		continue;
	    }

	    Item item = items[0];
	    if (!item.isSold()) {
		QString msg = tr("Item '%1' is not a sold item").arg(number);
		QMessageBox::warning(this, tr("Warning"), msg);
		continue;
	    }

	    QString size = item.numberSize(number);
	    if (size.isEmpty()) size = item.sellSize();
	    Price price = item.price(store_id, size);

	    LabelBatchItem line;
	    line.item_id = item.id();
	    line.number = number;
	    line.size = size;
	    line.price = price;
	    line.count = qty.toInt();
	    batch.items().push_back(line);
	}
	QFile::remove("label");

	if (!_quasar->db()->create(batch)) {
	    QApplication::restoreOverrideCursor();
	    qApp->beep();

	    QString message = tr("Failed creating label batch");
	    QMessageBox::critical(this, tr("Error"), message);
	    QDir::setCurrent(saveDir);
	    return;
	}

	QApplication::restoreOverrideCursor();
	qApp->beep();

	QString message = tr("Label batch #%1 created").arg(batch.number());
	QMessageBox::information(this, tr("Complete"), message);

	LabelBatchMaster* screen = new LabelBatchMaster(_main, batch.id());
	screen->show();
    } else if (QFile::exists("count")) {
	QFile file("count");
	if (!file.open(IO_ReadOnly)) {
	    QApplication::restoreOverrideCursor();
	    qApp->beep();

	    QString message = tr("Failed opening count file");
	    QMessageBox::critical(this, tr("Error"), message);
	    QDir::setCurrent(saveDir);
	    return;
	}

	Count count;
	count.setNumber("#");
	count.setDescription("From handheld");
	count.setDate(QDate::currentDate());
	count.setStoreId(_quasar->defaultStore());

	QString line;
	while (file.readLine(line, 100) > 0) {
	    QString number = line.left(14).stripWhiteSpace();
	    QString qty = line.mid(14).stripWhiteSpace();

	    _db->lookup(conditions, number, items);
	    if (items.size() == 0) {
		CountItem line;
		line.number = number;
		line.quantity = qty.toDouble();
		count.items().push_back(line);
		continue;
	    }

	    Item item = items[0];
	    QString size = item.numberSize(number);
	    if (size.isEmpty()) size = item.sellSize();

	    if (!item.isInventoried()) {
		QString msg = tr("Item '%1' is not inventoried").arg(number);
		QMessageBox::warning(this, tr("Warning"), msg);
		continue;
	    }

	    CountItem line;
	    line.item_id = item.id();
	    line.number = number;
	    line.size = size;
	    line.size_qty = item.sizeQty(size);
	    line.quantity = qty.toDouble();
	    count.items().push_back(line);
	}
	QFile::remove("count");

	if (!_quasar->db()->create(count)) {
	    QApplication::restoreOverrideCursor();
	    qApp->beep();

	    QString message = tr("Failed creating count");
	    QMessageBox::critical(this, tr("Error"), message);
	    QDir::setCurrent(saveDir);
	    return;
	}

	QApplication::restoreOverrideCursor();
	qApp->beep();

	QString message = tr("Count #%1 created").arg(count.number());
	QMessageBox::information(this, tr("Complete"), message);

	CountMaster* screen = new CountMaster(_main, count.id());
	screen->show();
    } else if (QFile::exists("price")) {
	QFile file("price");
	if (!file.open(IO_ReadOnly)) {
	    QApplication::restoreOverrideCursor();
	    qApp->beep();

	    QString message = tr("Failed opening price file");
	    QMessageBox::critical(this, tr("Error"), message);
	    QDir::setCurrent(saveDir);
	    return;
	}

	Id store_id = _quasar->defaultStore();

	PriceBatch batch;
	batch.setNumber("#");
	batch.setDescription("From handheld");
	batch.setStoreId(store_id);

	QString line;
	while (file.readLine(line, 100) > 0) {
	    QString number = line.left(14).stripWhiteSpace();
	    QString new_price = line.mid(14).stripWhiteSpace();

	    _db->lookup(conditions, number, items);
	    if (items.size() == 0) {
		PriceBatchItem line;
		line.number = number;
		line.new_price = Price(new_price.toDouble() / 100.0);
		batch.items().push_back(line);
		continue;
	    }

	    Item item = items[0];
	    if (!item.isSold()) {
		QString msg = tr("Item '%1' is not a sold item").arg(number);
		QMessageBox::warning(this, tr("Warning"), msg);
		continue;
	    }

	    QString size = item.numberSize(number);
	    if (size.isEmpty()) size = item.sellSize();

	    Price price = item.price(store_id, size);
	    Price cost = item.cost(store_id, size);
	    if (cost.isNull()) {
		Price purch_cost = item.cost(store_id, item.purchaseSize());
		fixed purch_qty = item.sizeQty(item.purchaseSize());
		fixed size_qty = item.sizeQty(size);
		cost = Price(purch_cost.calculate(1.0) * size_qty / purch_qty);
	    }

	    PriceBatchItem line;
	    line.item_id = item.id();
	    line.number = number;
	    line.size = size;
	    line.old_cost = cost;
	    line.old_price = price;
	    line.new_price = Price(new_price.toDouble() / 100.0);
	    batch.items().push_back(line);
	}
	QFile::remove("price");

	if (!_quasar->db()->create(batch)) {
	    QApplication::restoreOverrideCursor();
	    qApp->beep();

	    QString message = tr("Failed creating price batch");
	    QMessageBox::critical(this, tr("Error"), message);
	    QDir::setCurrent(saveDir);
	    return;
	}

	QApplication::restoreOverrideCursor();
	qApp->beep();

	QString message = tr("Price batch #%1 created").arg(batch.number());
	QMessageBox::information(this, tr("Complete"), message);

	PriceBatchMaster* screen = new PriceBatchMaster(_main, batch.id());
	screen->show();
    } else if (QFile::exists("receive")) {
	QFile file("receive");
	if (!file.open(IO_ReadOnly)) {
	    QApplication::restoreOverrideCursor();
	    qApp->beep();

	    QString message = tr("Failed opening receive file");
	    QMessageBox::critical(this, tr("Error"), message);
	    QDir::setCurrent(saveDir);
	    return;
	}

	QApplication::restoreOverrideCursor();
	qApp->beep();

	QString line;
	while (file.readLine(line, 100) > 0) {
	    QString number = line.left(14).stripWhiteSpace();

	    SlipSelect conditions;
	    conditions.number = number;
	    conditions.activeOnly = false;
	    vector<Slip> slips;
	    _quasar->db()->select(slips, conditions);

	    if (slips.size() == 0) {
		QString message = tr("Slip not found: %1").arg(number);
		QMessageBox::critical(this, tr("Error"), message);
	    } else if (slips.size() > 1) {
		QString message = tr("Multiple match for slip %1").arg(number);
		QMessageBox::critical(this, tr("Error"), message);
	    } else if (slips[0].isInactive()) {
		QString message = tr("Slip already posted: %1").arg(number);
		QMessageBox::critical(this, tr("Error"), message);
	    } else {
		Slip slip = slips[0];

		bool create;
		if (!SlipMaster::postSlip(_quasar, slip, create)) {
		    QString message = tr("Posting slip %1 failed").arg(number);
		    QMessageBox::critical(this, tr("Error"), message);
		}
	    }
	}
	QFile::remove("receive");

	QString message = tr("Slips posted");
	QMessageBox::information(this, tr("Complete"), message);
    } else if (QFile::exists("adjust")) {
	QFile file("adjust");
	if (!file.open(IO_ReadOnly)) {
	    QApplication::restoreOverrideCursor();
	    qApp->beep();

	    QString message = tr("Failed opening adjust file");
	    QMessageBox::critical(this, tr("Error"), message);
	    QDir::setCurrent(saveDir);
	    return;
	}

	Company company;
	_db->lookup(company);

	vector<ItemAdjust> adjusts;

	QString line;
	while (file.readLine(line, 100) > 0) {
	    QString reasonNum = line.left(6).stripWhiteSpace();
	    QString number = line.mid(6, 14).stripWhiteSpace();
	    fixed qty = line.mid(20).stripWhiteSpace().toDouble() * -1.0;

	    AdjustReason reason;
	    if (!_db->lookup(reasonNum, reason)) {
		continue;
	    }

	    int index = -1;
	    for (unsigned int i = 0; i < adjusts.size(); ++i) {
		if (adjusts[i].reasonId() == reason.id()) {
		    index = i;
		    break;
		}
	    }

	    if (index == -1) {
		ItemAdjust adjust;
		adjust.setNumber("#");
		adjust.setMemo("From handheld");
		adjust.setPostDate(QDate::currentDate());
		adjust.setPostTime(QTime::currentTime());
		adjust.setStoreId(_quasar->defaultStore());
		adjust.setReasonId(reason.id());
		adjust.setAccountId(reason.accountId());

		adjusts.push_back(adjust);
		index = adjusts.size() - 1;
	    }

	    ItemAdjust& adjust = adjusts[index];

	    _db->lookup(conditions, number, items);
	    if (items.size() == 0) {
		ItemLine line;
		line.number = number;
		line.quantity = qty;
		adjust.items().push_back(line);
		continue;
	    }

	    Item item = items[0];
	    QString size = item.numberSize(number);
	    if (size.isEmpty()) size = item.sellSize();

	    if (!item.isInventoried()) {
		QString msg = tr("Item '%1' is not inventoried").arg(number);
		QMessageBox::warning(this, tr("Warning"), msg);
		continue;
	    }

	    // Get cost and deposit
	    fixed ext_cost;
	    _quasar->db()->itemSellingCost(item, size, adjust.storeId(),
					   qty, 0, ext_cost);
	    fixed ext_deposit = item.deposit() * item.sizeQty(size) * qty;

	    ItemLine line;
	    line.item_id = item.id();
	    line.number = number;
	    line.size = size;
	    line.size_qty = item.sizeQty(size);
	    line.quantity = qty;
	    line.inv_cost = ext_cost;
	    line.ext_deposit = ext_deposit;
	    adjust.items().push_back(line);
	}
	QFile::remove("adjust");

	// Add accounts and post the adjustments
	QStringList numbers;
	for (unsigned int i = 0; i < adjusts.size(); ++i) {
	    ItemAdjust& adjust = adjusts[i];

	    vector<InfoPair> asset_info;
	    vector<InfoPair> expense_info;
	    fixed total_amt = 0.0;

	    for (unsigned int j = 0; j < adjust.items().size(); ++j) {
		const ItemLine& line = adjust.items()[j];
		if (line.item_id == INVALID_ID) continue;

		Item item;
		_db->lookup(line.item_id, item);

		addInfo(asset_info, item.assetAccount(), line.inv_cost);
		if (company.depositAccount() != INVALID_ID)
		    addInfo(expense_info, company.depositAccount(),
			    line.ext_deposit);

		total_amt += line.inv_cost + line.ext_deposit;
	    }

	    // Add the expense account
	    Id account_id = adjust.accountId();
	    if (account_id != INVALID_ID && total_amt != 0.0)
		addInfo(expense_info, account_id, -total_amt);

	    // Post to the accounts
	    for (unsigned int j = 0; j < asset_info.size(); ++j) {
		Id account_id = asset_info[j].first;
		fixed amount = asset_info[j].second;
		if (amount == 0.0) continue;
		adjust.accounts().push_back(AccountLine(account_id, amount));
	    }
	    for (unsigned int j = 0; j < expense_info.size(); ++j) {
		Id account_id = expense_info[j].first;
		fixed amount = expense_info[j].second;
		if (amount == 0.0) continue;
		adjust.accounts().push_back(AccountLine(account_id, amount));
	    }

	    if (!_quasar->db()->create(adjust)) {
		QApplication::restoreOverrideCursor();
		qApp->beep();

		QString message = tr("Failed creating adjustment");
		QMessageBox::critical(this, tr("Error"), message);
		QDir::setCurrent(saveDir);
		return;
	    }

	    numbers.push_back(adjust.number());
	}

	QApplication::restoreOverrideCursor();
	qApp->beep();

	QString message;
	if (numbers.size() == 0) {
	    message = tr("No adjustments created");
	} else if (numbers.size() == 1) {
	    QString number = numbers[0];
	    message = tr("Adjustment #%1 created").arg(number);
	} else {
	    message = tr("Adjustments created: %1").arg(numbers.join(","));
	}

	QMessageBox::information(this, tr("Complete"), message);
    } else {
	QApplication::restoreOverrideCursor();
	qApp->beep();

	QString message = tr("Good receive but no data found");
	QMessageBox::critical(this, tr("Error"), message);
	QDir::setCurrent(saveDir);
	return;
    }

    QDir::setCurrent(saveDir);
    close();
}
