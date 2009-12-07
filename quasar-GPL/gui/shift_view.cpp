// $Id: shift_view.cpp,v 1.14 2005/01/30 04:25:31 bpepers Exp $
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

#include "shift_view.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "report_tx.h"
#include "gltx_frame.h"
#include "lookup_edit.h"
#include "gltx_lookup.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>

ShiftView::ShiftView(MainWindow* main, Id shift_id)
    : QuasarWindow(main, "ShiftView")
{
    _helpSource = "shift_view.html";

    QFrame* frame = new QFrame(this);

    _gltxFrame = new GltxFrame(main, tr("Shift No."), frame);
    _gltxFrame->hideMemo();

    QLabel* adjustLabel = new QLabel(tr("Over/Short Adjustment:"), frame);
    _adjust = new LookupEdit(new GltxLookup(_main, this,
					    DataObject::TEND_ADJUST), frame);
    _adjust->setLength(10);
    adjustLabel->setBuddy(_adjust);

    QLabel* transferLabel = new QLabel(tr("Safe Transfer:"), frame);
    _transfer = new LookupEdit(new GltxLookup(_main, this,
					    DataObject::TEND_ADJUST), frame);
    _transfer->setLength(10);
    transferLabel->setBuddy(_transfer);

    QFrame* buttons = new QFrame(frame);
    QPushButton* summary = new QPushButton(tr("Summary"), buttons);
    _reopen = new QPushButton(tr("Re-open"), buttons);
    _delete = new QPushButton(tr("Delete"), buttons);
    QPushButton* close = new QPushButton(tr("Close"), buttons);

    connect(close, SIGNAL(clicked()), SLOT(close()));
    connect(summary, SIGNAL(clicked()), SLOT(slotSummary()));
    connect(_reopen, SIGNAL(clicked()), SLOT(slotReopen()));
    connect(_delete, SIGNAL(clicked()), SLOT(slotDelete()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(3, 1);
    buttonGrid->addWidget(summary, 0, 0);
    buttonGrid->addWidget(_reopen, 0, 1);
    buttonGrid->addWidget(_delete, 0, 2);
    buttonGrid->addWidget(close, 0, 4);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(2, 1);
    grid->addMultiCellWidget(_gltxFrame, 0, 0, 0, 2);
    grid->addWidget(adjustLabel, 1, 0);
    grid->addWidget(_adjust, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(transferLabel, 2, 0);
    grid->addWidget(_transfer, 2, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(buttons, 3, 3, 0, 2);

    // Load data
    _quasar->db()->lookup(shift_id, _shift);
    _gltxFrame->setData(_shift);
    _adjust->setId(_shift.adjustmentId());
    _transfer->setId(_shift.transferId());
    _reopen->setEnabled(_shift.shiftId() != INVALID_ID);
    _delete->setEnabled(_shift.transferId() == INVALID_ID);

    setCaption(tr("Shift View"));
    setCentralWidget(frame);
    close->setFocus();
    finalize();
}

ShiftView::~ShiftView()
{
}

void
ShiftView::slotSummary()
{
    ReportTx* master = new ReportTx(_main);
    master->setShiftId(_shift.id());
    master->setDates(QDate(), QDate());
    master->refresh();
    master->show();
}

void
ShiftView::slotReopen()
{
    QString message = tr("Are you sure you want to re-open\n"
			 "this shift?");
    int ch = QMessageBox::warning(this, tr("Warning"), message,
				  QMessageBox::Yes, QMessageBox::No);
    if (ch != QMessageBox::Yes) return;

    if (!_quasar->db()->shiftOpen(_shift)) {
	QMessageBox::critical(this, tr("Error"), tr("Re-open failed"));
	return;
    }

    message = tr("The shift has been opened");
    QMessageBox::information(this, tr("Information"), message);

    // Reload
    _quasar->db()->lookup(_shift.id(), _shift);
    _gltxFrame->setData(_shift);
    _adjust->setId(_shift.adjustmentId());
    _transfer->setId(_shift.transferId());
    _reopen->setEnabled(_shift.transferId() != INVALID_ID);
    _delete->setEnabled(_shift.transferId() == INVALID_ID);
}

void
ShiftView::slotDelete()
{
    QString message = tr("Are you sure you want to delete\n"
			 "this shift?");
    int ch = QMessageBox::warning(this, tr("Warning"), message,
				  QMessageBox::Yes, QMessageBox::No);
    if (ch != QMessageBox::Yes) return;

    if (!_quasar->db()->remove(_shift)) {
	QMessageBox::critical(this, tr("Error"), tr("Delete failed"));
	return;
    }

    message = tr("The shift has been deleted");
    QMessageBox::information(this, tr("Information"), message);
    close();
}
