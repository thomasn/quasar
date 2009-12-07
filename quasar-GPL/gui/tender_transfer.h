// $Id: tender_transfer.h,v 1.7 2005/01/30 04:25:31 bpepers Exp $
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

#ifndef TENDER_TRANSFER_H
#define TENDER_TRANSFER_H

#include "tender_adjust.h"
#include "company.h"
#include "data_window.h"
#include "variant.h"

class GltxFrame;
class Table;
class LineEdit;
class LookupEdit;
class NumberEdit;

class TenderTransfer: public DataWindow {
    Q_OBJECT
public:
    TenderTransfer(MainWindow* main, Id transfer_id=INVALID_ID);
    ~TenderTransfer();

    void setStore(Id store_id);
    void setStation(Id station_id);
    void setEmployee(Id employee_id);
    void setDate(QDate date);

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

    TenderAdjust _orig;
    TenderAdjust _curr;
    TenderAdjust _origLink;
    TenderAdjust _link;

    // Widgets
    GltxFrame* _gltxFrame;
    Table* _tenders;
    LineEdit* _toNumber;
    LookupEdit* _toShift;
    LookupEdit* _toStore;
    LookupEdit* _toStation;
    LookupEdit* _toEmployee;
    LookupEdit* _account;
    NumberEdit* _total;

    Company _company;
};

#endif // TENDER_TRANSFER_H
