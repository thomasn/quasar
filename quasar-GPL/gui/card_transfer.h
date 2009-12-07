// $Id: card_transfer.h,v 1.5 2004/06/15 03:07:07 bpepers Exp $
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

#ifndef CARD_TRANSFER_H
#define CARD_TRANSFER_H

#include "card_adjust.h"
#include "data_window.h"
#include "variant.h"

class GltxFrame;
class LineEdit;
class LookupEdit;
class MoneyEdit;

class CardTransfer: public DataWindow {
    Q_OBJECT
public:
    CardTransfer(MainWindow* main, Id transfer_id=INVALID_ID);
    ~CardTransfer();

protected slots:
    void slotFromChanged();
    void slotToChanged();

protected:
    virtual void oldItem();
    virtual void newItem();
    virtual void cloneFrom(Id id);
    virtual bool fileItem();
    virtual bool deleteItem();
    virtual void restoreItem();
    virtual void cloneItem();
    virtual bool isChanged();
    virtual void dataToWidget();
    virtual void widgetToData();

    CardAdjust _orig;
    CardAdjust _curr;
    CardAdjust _origLink;
    CardAdjust _link;

    // Widgets
    GltxFrame* _gltxFrame;
    LookupEdit* _from;
    MoneyEdit* _fromBalance;
    LookupEdit* _to;
    MoneyEdit* _toBalance;
    MoneyEdit* _amount;
    LineEdit* _memo;
    LineEdit* _toNumber;
    LookupEdit* _toShift;
    LookupEdit* _account;
};

#endif // CARD_TRANSFER_H
