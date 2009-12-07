// $Id: card_allocate.h,v 1.1 2004/10/09 00:35:20 bpepers Exp $
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
// See http://www.linuxcanada.com or email card@linuxcanada.com for
// information about Quasar Accounting support and maintenance options.
//
// Contact card@linuxcanada.com if any conditions of this licensing are
// not clear to you.

#ifndef CARD_ALLOCATE_H
#define CARD_ALLOCATE_H

#include "quasar_window.h"
#include "gltx.h"

class LookupEdit;
class ListView;
class NumberEdit;
class QTabWidget;

class CardAllocate: public QuasarWindow {
    Q_OBJECT
public:
    CardAllocate(MainWindow* main);
    ~CardAllocate();

    void setCardId(Id card_id);

protected slots:
    void slotCardChanged();
    void slotPickLine();
    void slotAutoAllocate();
    void slotPost();
    void slotCancel();

protected:
    vector<Gltx> _credits;
    vector<Gltx> _debits;

    LookupEdit* _card;
    NumberEdit* _balance;
    QTabWidget* _tabs;
    QFrame* _creditFrame;
    ListView* _creditList;
    NumberEdit* _creditTotal;
    NumberEdit* _creditAlloc;
    QFrame* _debitFrame;
    ListView* _debitList;
    NumberEdit* _debitTotal;
    NumberEdit* _debitAlloc;
};

#endif // CARD_ALLOCATE_H
