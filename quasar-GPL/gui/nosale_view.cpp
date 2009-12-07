// $Id: nosale_view.cpp,v 1.5 2004/01/31 01:50:30 arandell Exp $
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

#include "nosale_view.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "gltx_frame.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>

NosaleView::NosaleView(MainWindow* main, Id tx_id)
    : QuasarWindow(main, "NosaleView")
{
    _helpSource = "nosale_view.html";

    QFrame* frame = new QFrame(this);

    _gltxFrame = new GltxFrame(main, tr("Nosale No."), frame);
    _gltxFrame->hideMemo();

    QFrame* buttons = new QFrame(frame);
    QPushButton* close = new QPushButton(tr("Close"), buttons);
    connect(close, SIGNAL(clicked()), SLOT(close()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(close, 0, 0, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(0, 1);
    grid->setRowStretch(0, 1);
    grid->addWidget(_gltxFrame, 0, 0);
    grid->addWidget(buttons, 1, 0);

    // Load data
    Nosale nosale;
    _quasar->db()->lookup(tx_id, nosale);
    _gltxFrame->setData(nosale);

    setCaption(tr("Nosale View"));
    setCentralWidget(frame);
    close->setFocus();
    finalize();
}

NosaleView::~NosaleView()
{
}
