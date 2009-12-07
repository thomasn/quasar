// $Id: order_master.h,v 1.25 2005/04/01 22:13:37 bpepers Exp $
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

#ifndef ORDER_MASTER_H
#define ORDER_MASTER_H

#include "order.h"
#include "vendor.h"
#include "item.h"
#include "tax.h"
#include "charge.h"
#include "item_price.h"
#include "data_window.h"
#include "variant.h"
#include "sales_history.h"
#include <qguardedptr.h>

class LookupEdit;
class DatePopup;
class NumberEdit;
class LineEdit;
class ItemLookup;
class Table;
class MultiLineEdit;
class QRadioButton;
class QComboBox;
class QCheckBox;

class OrderMaster: public DataWindow {
    Q_OBJECT
public:
    OrderMaster(MainWindow* main, Id order_id=INVALID_ID);
    ~OrderMaster();

protected slots:
    void slotVendorChanged();
    void slotStoreChanged();
    void slotShipToChanged();
    void slotSearch();
    void slotSalesHistory();
    void slotTemplate();
    void slotIncTaxChanged();
    void slotIncDepChanged();
    void slotDepositChanged();
    void itemCellMoved(int row, int col);
    void itemCellValidate(int row, int col, Variant value, bool& ok);
    void itemCellChanged(int row, int col, Variant old);
    void itemFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void itemRowInserted(int row);
    void itemRowDeleted(int row);
    void ichargeCellChanged(int row, int col, Variant old);
    void ichargeFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void ichargeRowInserted(int row);
    void ichargeRowDeleted(int row);
    void echargeCellChanged(int row, int col, Variant old);
    void echargeFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void echargeRowInserted(int row);
    void echargeRowDeleted(int row);
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
    virtual void printItem(bool ask=false);

    void updateItemInfo(int row);
    void recalculateCost(int row);
    void addTax(fixed taxable, vector<Id>& ids, vector<fixed>& amts);
    void addTaxInc(fixed taxable, vector<Id>& ids, vector<fixed>& amts);
    bool findItem(Id item_id, Item& item);
    bool findCosts(Id item_id, vector<ItemPrice>& costs);
    bool findTax(Id tax_id, Tax& tax);
    bool findCharge(Id charge_id, Charge& charge);

    Order _orig;
    Order _curr;

    // Widgets
    QPushButton* _print;
    LookupEdit* _vend_id;
    MultiLineEdit* _vendor_addr;
    LineEdit* _number;
    LookupEdit* _store;
    DatePopup* _date;
    LineEdit* _ship_via;
    LookupEdit* _term_id;
    ItemLookup* _lookup;
    Table* _items;
    QComboBox* _size;
    NumberEdit* _item_amt;
    NumberEdit* _charge_amt;
    NumberEdit* _tax_amt;
    NumberEdit* _deposit_amt;
    NumberEdit* _total_amt;
    MultiLineEdit* _comment;
    Table* _taxes;
    Table* _icharges;
    Table* _echarges;
    LookupEdit* _ship;
    MultiLineEdit* _ship_addr;
    NumberEdit* _received;
    QCheckBox* _inc_tax;
    NumberEdit* _tax;
    QCheckBox* _inc_dep;
    NumberEdit* _deposit;
    NumberEdit* _line_cost;
    NumberEdit* _line_icharge;
    NumberEdit* _line_echarge;

    // Internal list of items
    struct ItemLine {
	ItemLine();

	Item item;
	QString number;
	QString description;
	QString size;
	fixed size_qty;
	fixed ordered;
	fixed billed;
	Id cost_id;
	fixed cost_disc;
	Price cost;
	fixed ext_cost;
	fixed ext_base;
	fixed ext_deposit;
	fixed ext_tax;
	fixed int_charges;
	fixed ext_charges;
	Id tax_id;
	Id item_tax_id;
	bool include_tax;
	bool include_deposit;
    };

    // Internal list of charges
    struct ChargeLine {
	ChargeLine();

	Charge charge;
	Id tax_id;
	fixed amount;
	fixed base;
    };

    struct ItemCache {
	Item item;
	vector<ItemPrice> costs;
    };

    QGuardedPtr<SalesHistory> _hist;
    Vendor _vendor;
    vector<ItemLine> _lines;
    vector<OrderTax> _tax_info;
    vector<ChargeLine> _icharge_lines;
    vector<ChargeLine> _echarge_lines;
    vector<ItemCache> _item_cache;
    vector<Tax> _tax_cache;
    vector<Charge> _charge_cache;
    int _item_row;
    bool _skip_recalc;
};

#endif // ORDER_MASTER_H
