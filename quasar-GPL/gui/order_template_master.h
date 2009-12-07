// $Id: order_template_master.h,v 1.6 2004/12/13 09:15:57 bpepers Exp $
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

#ifndef ORDER_TEMPLATE_MASTER_H
#define ORDER_TEMPLATE_MASTER_H

#include "order_template.h"
#include "data_window.h"
#include "charge.h"
#include "variant.h"

class LineEdit;
class LookupEdit;
class Table;
class QComboBox;

class OrderTemplateMaster: public DataWindow {
    Q_OBJECT
public:
    OrderTemplateMaster(MainWindow* main, Id temp_id=INVALID_ID);
    ~OrderTemplateMaster();

protected slots:
    void slotSort();
    void slotSearch();
    void itemCellMoved(int row, int col);
    void itemCellChanged(int row, int col, Variant old);
    void itemFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void ichargeCellChanged(int row, int col, Variant old);
    void ichargeFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void ichargeRowInserted(int row);
    void ichargeRowDeleted(int row);
    void echargeCellChanged(int row, int col, Variant old);
    void echargeFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void echargeRowInserted(int row);
    void echargeRowDeleted(int row);

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

    OrderTemplate _orig;
    OrderTemplate _curr;

    // Widgets
    LineEdit* _name;
    LookupEdit* _vendor;
    Table* _items;
    QComboBox* _size;
    Table* _icharges;
    Table* _echarges;

    // Internal list of charges
    struct ChargeLine {
	ChargeLine();

	Charge charge;
	Id tax_id;
	fixed amount;
    };
    vector<ChargeLine> _icharge_lines;
    vector<ChargeLine> _echarge_lines;

    bool findCharge(Id charge_id, Charge& charge);
    vector<Charge> _charge_cache;
};

#endif // ORDER_TEMPLATE_MASTER_H
