// $Id: year_end_final.cpp,v 1.2 2005/03/03 22:28:42 bpepers Exp $
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

#include "year_end_final.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "date_popup.h"
#include "company.h"
#include "store.h"
#include "account.h"
#include "general.h"
#include "account_select.h"
#include "store_select.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qmessagebox.h>

YearEndFinal::YearEndFinal(MainWindow* main)
    : QuasarWindow(main, "YearEndFinal")
{
    _helpSource = "year_end_final.html";

    QFrame* frame = new QFrame(this);

    QString desc = tr("The final year end closes off a year so you\n"
		      "can no longer make adjustments to it.  It is\n"
		      "run after you have done the preliminary year\n"
		      "end and completed any adjustments.");
    QLabel* descLabel = new QLabel(desc, frame);

    QFrame* buttons = new QFrame(frame);
    QPushButton* ok = new QPushButton(tr("OK"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);

    connect(ok, SIGNAL(clicked()), SLOT(slotOk()));
    connect(cancel, SIGNAL(clicked()), SLOT(close()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(ok, 0, 1);
    buttonGrid->addWidget(cancel, 0, 2);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->addWidget(descLabel, 0, 0);
    grid->addWidget(buttons, 1, 0);

    setCentralWidget(frame);
    setCaption(tr("Final Year End"));
    finalize();
}

YearEndFinal::~YearEndFinal()
{
}

void
YearEndFinal::slotOk()
{
    Company company;
    _db->lookup(company);

    if (company.lastYearClosed()) {
	QString message = tr("Last year is already closed");
	QApplication::beep();
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    Company orig = company;
    company.setLastYearClosed(true);
    if (!_db->update(orig, company)) {
	QApplication::restoreOverrideCursor();
	QString message = tr("Failed updating company");
	QApplication::beep();
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QApplication::restoreOverrideCursor();
    QString message = tr("Final year end finished");
    QMessageBox::information(this, tr("Finished"), message);

    close();
}
