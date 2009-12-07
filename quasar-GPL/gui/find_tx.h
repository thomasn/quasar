// $Id: find_tx.h,v 1.12 2004/01/31 01:50:31 arandell Exp $
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

#ifndef FIND_TX_H
#define FIND_TX_H

#include "quasar_window.h"
#include "data_object.h"

class ComboBox;
class QLabel;
class LineEdit;
class NumberEdit;
class CardLookup;
class LookupEdit;
class DateRange;
class DatePopup;
class ListView;
class Grid;

class FindTx: public QuasarWindow {
    Q_OBJECT
public:
    FindTx(MainWindow* main);
    ~FindTx();

    void setType(int type);
    void setStoreId(Id store_id);
    void setCardId(Id card_id);

protected slots:
    void slotTypeChanged();
    void slotFind();
    void slotPrint();
    void slotEdit();

protected:
    ComboBox* _type;
    QLabel* _numberLabel;
    LineEdit* _number;
    LineEdit* _reference;
    DateRange* _range;
    DatePopup* _from;
    DatePopup* _to;
    LookupEdit* _store;
    LookupEdit* _station;
    LookupEdit* _employee;
    LookupEdit* _shift;
    QLabel* _cardLabel;
    CardLookup* _cardLookup;
    LookupEdit* _card;
    NumberEdit* _total;
    ListView* _list;

    QString buildTitle(const QString& name);
};

#endif // FIND_TX_H
