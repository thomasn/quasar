// $Id: sequence_number.cpp,v 1.20 2005/03/29 22:30:18 bpepers Exp $
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

#include "sequence_number.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "id_edit.h"

#include <qapplication.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qscrollview.h>
#include <qbuttongroup.h>
#include <qstatusbar.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qtimer.h>

SequenceNumber::SequenceNumber(MainWindow* main)
    : QuasarWindow(main, "SequenceNumber")
{
    _helpSource = "seq_number.html";

    QFrame* frame = new QFrame(this);
    QScrollView* sv = new QScrollView(frame);
    _nums = new QButtonGroup(4, Horizontal, tr("Seq Numbers"), sv->viewport());

    new QLabel("Type", _nums);
    new QLabel("Minimum", _nums);
    new QLabel("Maximum", _nums);
    new QLabel("Next", _nums);

    addIdEdit(tr("Data Object:"), "data_object", "object_id");
    addIdEdit(tr("Journal Entry:"), "gltx", "Journal Entry");
    addIdEdit(tr("Ledger Transfer:"), "gltx", "Ledger Transfer");
    addIdEdit(tr("Card Adjustment:"), "gltx", "Card Adjustment");
    addIdEdit(tr("Customer Invoice:"), "gltx", "Customer Invoice");
    addIdEdit(tr("Customer Return:"), "gltx", "Customer Return");
    addIdEdit(tr("Customer Payment:"), "gltx", "Customer Payment");
    addIdEdit(tr("Customer Quote:"), "quote", "number");
    addIdEdit(tr("Vendor Invoice:"), "gltx", "Vendor Invoice");
    addIdEdit(tr("Vendor Claim:"), "gltx", "Vendor Claim");
    addIdEdit(tr("Purchase Order:"), "porder", "number");
    addIdEdit(tr("Packing Slip:"), "slip", "number");
    addIdEdit(tr("Nosale:"), "gltx", "Nosale");
    addIdEdit(tr("Payout:"), "gltx", "Payout");
    addIdEdit(tr("Withdraw:"), "gltx", "Withdraw");
    addIdEdit(tr("Shift:"), "gltx", "Shift");
    addIdEdit(tr("Item Adjustment:"), "gltx", "Item Adjustment");
    addIdEdit(tr("Item Transfer:"), "gltx", "Item Transfer");
    addIdEdit(tr("Physical Count:"), "pcount", "number");
    addIdEdit(tr("Label Batch:"), "label_batch", "number");
    addIdEdit(tr("Price Batch:"), "price_batch", "number");
    addIdEdit(tr("Promo Batch:"), "promo_batch", "number");
    addIdEdit(tr("Company Number:"), "company", "number");
    addIdEdit(tr("Store Number:"), "store", "number");
    addIdEdit(tr("Station Number:"), "station", "number");
    addIdEdit(tr("Tender Count #:"), "tender_count", "number");
    addIdEdit(tr("Tender Menu #:"), "tender", "menu_num");

    QFrame* buttons = new QFrame(frame);
    QPushButton* ok = new QPushButton(tr("&OK"), buttons);
    QPushButton* quit = new QPushButton(tr("&Close"), buttons);

    connect(ok, SIGNAL(clicked()), SLOT(slotOk()));
    connect(quit, SIGNAL(clicked()), SLOT(close()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(3);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(ok, 0, 1);
    buttonGrid->addWidget(quit, 0, 2);

    _nums->resize(_nums->sizeHint());
    sv->setVScrollBarMode(QScrollView::AlwaysOn);
    sv->resizeContents(_nums->width() + 20, _nums->height());

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setRowStretch(0, 1);
    grid->addWidget(sv, 0, 0);
    grid->addWidget(buttons, 1, 0);

    for (unsigned int i = 0; i < _ids.size(); ++i) {
	IdInfo& info = _ids[i];
	_quasar->db()->getSequence(info.seq);
	info.minNum->setFixed(info.seq.minNumber());
	info.maxNum->setFixed(info.seq.maxNumber());
	info.nextNum->setFixed(info.seq.nextNumber());
    }

    statusBar()->hide();
    setCentralWidget(frame);
    setCaption(tr("Sequence Numbers"));
    finalize();

    if (!allowed("View")) {
	QTimer::singleShot(50, this, SLOT(slotNotAllowed()));
	return;
    }
}

SequenceNumber::~SequenceNumber()
{
}

void
SequenceNumber::slotNotAllowed()
{
    qApp->beep();
    QString message = tr("Access to this screen is denied");
    QMessageBox::critical(this, tr("Error"), message);
    delete this;
}

void
SequenceNumber::slotOk()
{
    if (!allowed("Update")) {
	qApp->beep();
	QString message = tr("Changing data is not allowed");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    // Verify all are valid
    for (unsigned int i = 0; i < _ids.size(); ++i) {
	IdInfo& info = _ids[i];
	fixed minNum = info.minNum->getFixed();
	if (minNum < 1) {
	    error(tr("Invalid minimum number"), info.minNum);
	    return;
	}

	fixed maxNum = info.maxNum->getFixed();
	if (maxNum < 1) {
	    error(tr("Invalid maximum number"), info.maxNum);
	    return;
	}

	fixed nextNum = info.nextNum->getFixed();
	if (nextNum < 1) {
	    error(tr("Invalid next number"), info.nextNum);
	    return;
	}

	if (nextNum < minNum || nextNum > maxNum) {
	    error(tr("Invalid next number"), info.nextNum);
	    return;
	}
    }

    // Set the numbers that have changed
    for (unsigned int i = 0; i < _ids.size(); ++i) {
	IdInfo& info = _ids[i];
	fixed minNum = info.minNum->getFixed();
	fixed maxNum = info.maxNum->getFixed();
	fixed nextNum = info.nextNum->getFixed();

	bool changed = false;
	if (minNum != info.seq.minNumber()) changed = true;
	if (maxNum != info.seq.maxNumber()) changed = true;
	if (nextNum != info.seq.nextNumber()) changed = true;

	if (changed) {
	    info.seq.setMinNumber(minNum);
	    info.seq.setMaxNumber(maxNum);
	    info.seq.setNextNumber(nextNum);
	    _quasar->db()->setSequence(info.seq);
	}
    }
    close();
}

void
SequenceNumber::addIdEdit(const QString& label, const QString& table,
			  const QString& attr)
{
    IdInfo info;
    info.seq.setTableName(table);
    info.seq.setAttrName(attr);
    info.seq.setSystemId(_quasar->db()->systemId());

    new QLabel(label, _nums);
    info.minNum = new IdEdit(_nums);
    info.maxNum = new IdEdit(_nums);
    info.nextNum = new IdEdit(_nums);
    _ids.push_back(info);
}
