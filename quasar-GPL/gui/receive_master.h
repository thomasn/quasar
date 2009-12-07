// $Id: receive_master.h,v 1.37 2005/05/13 22:50:53 bpepers Exp $
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

#ifndef RECEIVE_MASTER_H
#define RECEIVE_MASTER_H

#include "receive.h"
#include "vendor.h"
#include "item.h"
#include "account.h"
#include "tax.h"
#include "charge.h"
#include "item_price.h"
#include "data_window.h"
#include "variant.h"
#include "sales_history.h"
#include <qguardedptr.h>
#include <object_cache.h>

class GltxFrame;
class LookupEdit;
class DatePopup;
class NumberEdit;
class LineEdit;
class ItemLookup;
class Table;
class OrderLookup;
class MultiLineEdit;
class QLabel;
class QTabWidget;
class QRadioButton;
class QComboBox;
class QCheckBox;

class ReceiveMaster: public DataWindow {
    Q_OBJECT
public:
    ReceiveMaster(MainWindow* main, Id receive_id=INVALID_ID);
    ~ReceiveMaster();

protected slots:
    void itemCellMoved(int row, int col);
    void itemCellChanged(int row, int col, Variant old);
    void itemFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void itemRowInserted(int row);
    void itemRowDeleted(int row);
    void orderCellChanged(int row, int col, Variant old);
    void orderFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void ichargeCellChanged(int row, int col, Variant old);
    void ichargeFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void ichargeRowInserted(int row);
    void ichargeRowDeleted(int row);
    void echargeCellChanged(int row, int col, Variant old);
    void echargeFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void echargeRowInserted(int row);
    void echargeRowDeleted(int row);

protected slots:
    void slotSearch();
    void slotRecurring();
    void slotClaimChanged();
    void slotDateChanged();
    void slotVendorChanged();
    void slotStoreChanged();
    void slotShipToChanged();
    void slotTypeChanged();
    void slotPayment();
    void slotSalesHistory();
    void slotIncTaxChanged();
    void slotIncDepChanged();
    void slotDepositChanged();
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

    bool clearItems(bool ask=true);
    void updateItemInfo(int row);
    void recalculateCost(int row);
    void addTax(fixed taxable, vector<Id>& ids, vector<fixed>& amts);
    void addTaxInc(fixed taxable, vector<Id>& ids, vector<fixed>& amts);
    bool findCosts(Id item_id, vector<ItemPrice>& costs);
    void importOrder(Id order_id);

    Receive _orig;
    Receive _curr;

    // Widgets
    QPushButton* _payment;
    GltxFrame* _gltxFrame;
    LookupEdit* _vendor_id;
    MultiLineEdit* _vendor_addr;
    LookupEdit* _term_id;
    LineEdit* _ship_via;
    DatePopup* _inv_date;
    QComboBox* _type;
    ItemLookup* _lookup;
    Table* _items;
    QComboBox* _size;
    QTabWidget* _tabs;
    QFrame* _ordersTab;
    NumberEdit* _item_amt;
    NumberEdit* _charge_amt;
    NumberEdit* _tax_amt;
    NumberEdit* _deposit_amt;
    NumberEdit* _item_qty;
    NumberEdit* _total_amt;
    NumberEdit* _paid_amt;
    NumberEdit* _due_amt;
    MultiLineEdit* _comment;
    Table* _taxes;
    Table* _icharges;
    Table* _echarges;
    LookupEdit* _ship;
    MultiLineEdit* _ship_addr;
    NumberEdit* _ordered;
    QCheckBox* _inc_tax;
    QCheckBox* _inc_dep;
    NumberEdit* _deposit;
    NumberEdit* _line_cost;
    NumberEdit* _line_icharge;
    NumberEdit* _line_echarge;
    OrderLookup* _orderLookup;
    Table* _orders;
    QCheckBox* _claim;

    // Internal list of items
    struct ItemLine {
	ItemLine();

	Item item;
	Account account;
	QString number;
	QString description;
	QString size;
	fixed size_qty;
	fixed qty;
	fixed ordered;
	fixed received;
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
	fixed inv_cost;
	bool include_tax;
	bool include_deposit;
	bool open_dept;
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
	Id item_id;
	vector<ItemPrice> costs;
    };

    ObjectCache _cache;
    int _old_type;
    int _size_width;
    QGuardedPtr<SalesHistory> _hist;
    Vendor _vendor;
    vector<ItemLine> _lines;
    vector<TaxLine> _tax_info;
    vector<ChargeLine> _icharge_lines;
    vector<ChargeLine> _echarge_lines;
    vector<ItemCache> _item_cache;
    int _item_row;
    bool _skip_recalc;
};

#endif // RECEIVE_MASTER_H
