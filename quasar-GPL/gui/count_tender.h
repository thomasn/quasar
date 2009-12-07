// $Id: count_tender.h,v 1.10 2005/01/30 04:25:31 bpepers Exp $
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

#ifndef COUNT_TENDER_H
#define COUNT_TENDER_H

#include "tender_count.h"
#include "tender.h"
#include "data_window.h"
#include "variant.h"

class LineEdit;
class DatePopup;
class LookupEdit;
class Table;
class MoneyEdit;

class CountTender: public DataWindow {
    Q_OBJECT
public:
    CountTender(MainWindow* main, Id count_id=INVALID_ID);
    ~CountTender();

    void setStoreId(Id store_id);
    void setStationId(Id station_id);
    void setEmployeeId(Id employee_id);
    void setDate(QDate date);
    void setTender(Id tender_id, fixed amount);

protected slots:
    void cellChanged(int row, int col, Variant old);
    void focusNext(bool& leave, int& newRow, int& newcol, int type);
    void recalculate();

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

    TenderCount _orig;
    TenderCount _curr;

    // Widgets
    LineEdit* _number;
    DatePopup* _date;
    LookupEdit* _station;
    LookupEdit* _employee;
    LookupEdit* _store;
    Table* _tenders;
    MoneyEdit* _total;

    vector<Tender> _tenderList;
};

#endif // COUNT_TENDER_H
