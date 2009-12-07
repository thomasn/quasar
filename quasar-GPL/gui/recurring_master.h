// $Id: recurring_master.h,v 1.5 2004/01/31 01:50:31 arandell Exp $
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

#ifndef RECURRING_MASTER_H
#define RECURRING_MASTER_H

#include "recurring.h"
#include "data_window.h"

class Gltx;
class LineEdit;
class ComboBox;
class LookupEdit;
class GroupLookup;
class IntegerEdit;
class DatePopup;
class QRadioButton;

class RecurringMaster: public DataWindow {
    Q_OBJECT
public:
    RecurringMaster(MainWindow*, Id recurring_id=INVALID_ID);
    ~RecurringMaster();

    void setGltx(Id gltx_id);

protected slots:
    void slotTypeChanged();
    void slotRefresh();
    void slotPost();

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

    bool postOne(const Gltx& gltx, Id card_id);

    Recurring _orig;
    Recurring _curr;

    // Widgets
    LineEdit* _type;
    LineEdit* _store;
    LineEdit* _number;
    LineEdit* _date;
    LineEdit* _desc;
    QRadioButton* _cardLabel;
    LineEdit* _card;
    QRadioButton* _groupLabel;
    GroupLookup* _groupLookup;
    LookupEdit* _group;
    ComboBox* _freq;
    IntegerEdit* _maxPost;
    IntegerEdit* _day1;
    IntegerEdit* _day2;
    DatePopup* _lastPost;
    LineEdit* _nextDue;
    IntegerEdit* _postCnt;
    IntegerEdit* _overdue;
};

#endif // RECURRING_MASTER_H
