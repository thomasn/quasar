// $Id: quote_master.h,v 1.8 2005/01/11 08:35:16 bpepers Exp $
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

#ifndef QUOTE_MASTER_H
#define QUOTE_MASTER_H

#include "quote.h"
#include "customer.h"
#include "item.h"
#include "account.h"
#include "tax.h"
#include "item_price.h"
#include "discount.h"
#include "data_window.h"
#include "variant.h"
#include "stock_status.h"
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

class QuoteMaster: public DataWindow {
    Q_OBJECT
public:
    QuoteMaster(MainWindow* main, Id id=INVALID_ID);
    ~QuoteMaster();

protected slots:
    void itemCellMoved(int row, int col);
    void itemCellChanged(int row, int col, Variant old);
    void itemFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void itemRowInserted(int row);
    void itemRowDeleted(int row);
    void discCellChanged(int row, int col, Variant old);
    void discFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void discRowInserted(int row);
    void discRowDeleted(int row);

protected slots:
    void slotCustomerChanged();
    void slotStoreChanged();
    void slotShipToChanged();
    void slotTypeChanged();
    void slotInvoice();
    void slotStockStatus();
    void slotIncTaxChanged();
    void slotIncDepChanged();
    void slotDiscableChanged();
    void slotDepositChanged();
    void slotDiscountChanged();
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
    void recalculatePrice(int row);
    void addTax(fixed taxable, vector<Id>& ids, vector<fixed>& amts);
    void addTaxInc(fixed taxable, vector<Id>& ids, vector<fixed>& amts);
    bool findItem(Id item_id, Item& item);
    bool findPrices(Id item_id, vector<ItemPrice>& prices);
    bool findTax(Id tax_id, Tax& tax);
    bool findDiscount(Id discoun_id, Discount& discount);

    Quote _orig;
    Quote _curr;

    // Widgets
    LookupEdit* _cust_id;
    MultiLineEdit* _cust_addr;
    LineEdit* _number;
    QComboBox* _type;
    LineEdit* _reference;
    LineEdit* _invoice;
    DatePopup* _date;
    LineEdit* _ship_via;
    LookupEdit* _term_id;
    DatePopup* _expiry;
    LookupEdit* _employee;
    LookupEdit* _store;
    ItemLookup* _lookup;
    Table* _items;
    QComboBox* _size;
    NumberEdit* _item_amt;
    NumberEdit* _ldisc_amt;
    NumberEdit* _tdisc_amt;
    NumberEdit* _tax_amt;
    NumberEdit* _deposit_amt;
    MultiLineEdit* _comment;
    Table* _taxes;
    LookupEdit* _ship;
    MultiLineEdit* _ship_addr;
    QCheckBox* _inc_tax;
    QCheckBox* _inc_dep;
    QCheckBox* _discountable;
    LookupEdit* _line_disc;
    QRadioButton* _percent;
    NumberEdit* _percentAmt;
    QRadioButton* _dollar;
    NumberEdit* _dollarAmt;
    NumberEdit* _deposit;
    NumberEdit* _discount;
    NumberEdit* _line_total;
    NumberEdit* _tx_disc;
    Table* _discs;
    NumberEdit* _total_amt;

    // Internal list of discounts
    struct DiscountLine {
	DiscountLine();

	Id discount_id;
	int method;
	fixed amount;
	fixed total_amt;
	Id account_id;
    };

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
	Id price_id;
	fixed price_disc;
	Price price;
	fixed ext_price;
	fixed ext_base;
	fixed ext_deposit;
	fixed ext_tax;
	Id tax_id;
	Id item_tax_id;
	fixed inv_cost;
	bool include_tax;
	bool include_deposit;
	bool discountable;
	bool open_dept;
	DiscountLine line_disc;
	vector<int> tdisc_nums;
	vector<fixed> tdisc_amts;
    };

    struct ItemCache {
	Item item;
	vector<ItemPrice> prices;
    };

    int _old_type;
    int _size_width;
    QGuardedPtr<StockStatus> _stock;
    Customer _customer;
    vector<DiscountLine> _discounts;
    vector<ItemLine> _lines;
    vector<TaxLine> _tax_info;
    vector<ItemCache> _item_cache;
    vector<Tax> _tax_cache;
    vector<Discount> _discount_cache;
    int _item_row;
};

#endif // QUOTE_MASTER_H
