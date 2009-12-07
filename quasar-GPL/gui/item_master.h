// $Id: item_master.h,v 1.38 2005/02/02 12:07:28 bpepers Exp $
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

#ifndef ITEM_MASTER_H
#define ITEM_MASTER_H

#include "item.h"
#include "company.h"
#include "data_window.h"
#include "variant.h"

class LineEdit;
class NumberEdit;
class PriceEdit;
class LookupEdit;
class DatePopup;
class SubdeptLookup;
class AccountLookup;
class Table;
class QCheckBox;
class QComboBox;
class QLabel;
class QGroupBox;
class QTabWidget;

class ItemMaster: public DataWindow {
    Q_OBJECT
public:
    ItemMaster(MainWindow* main, Id item_id=INVALID_ID);
    ~ItemMaster();

    void setNumber(const QString& number);

protected slots:
    void slotPrices();
    void slotCosts();
    void slotStockStatus();
    void slotMargin();
    void slotBuild();
    void slotCreateData();
    void slotDeleteData();
    void slotRenameData();
    void numberChanged();
    void deptChanged();
    void subdeptChanged();
    void typeChanged();
    void storeChanged();
    void setStoreInfo();
    void recalculateMargin();
    void purchaseSizeChanged();
    void purchaseQtyChanged();
    void costChanged();
    void vendorChanged();
    void vendorNumChanged();
    void sellSizeChanged();
    void sellQtyChanged();
    void priceChanged();
    void marginChanged();
    void profitChanged();
    void targetChanged();
    void allowedChanged();
    void openSizeChanged();
    void openQtyChanged();
    void sizesChanged();
    void groupCellChanged(int row, int col, Variant old);
    void sizeCellChanged(int row, int col, Variant old);
    void sizeFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void sizeRowDeleted(int row);
    void numberCellChanged(int row, int col, Variant old);
    void numberFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void numberRowDeleted(int row);
    void numberRowInserted(int row);
    void vendorCellChanged(int row, int col, Variant old);
    void vendorFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void vendorRowDeleted(int row);
    void vendorRowInserted(int row);
    void storesCellChanged(int row, int col, Variant old);
    void storesFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void storesRowInserted(int row);
    void storesRowDeleted(int row);
    void costsCellChanged(int row, int col, Variant old);
    void costsFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void costsRowInserted(int row);
    void costsRowDeleted(int row);
    void buildCellChanged(int row, int col, Variant old);
    void buildFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void extraFocusNext(bool& leave, int& newRow, int& newcol, int type);

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
    virtual bool dataValidate();

    Item _orig;
    Item _curr;

    int findStoreRow(const QString& size);
    void addSize(const QString& size, fixed qty);
    void addCostRows(const QString& size);
    bool sizeExists(const QString& size, fixed& qty);
    bool sizeExists(const QString& size);
    bool _ignoreSubdept;
    bool _skipUpdates;
    int _purch_row;
    int _sell_row;
    Company _company;

    // Widgets
    LineEdit* _number;
    LineEdit* _desc;
    LookupEdit* _dept;
    SubdeptLookup* _subLookup;
    LookupEdit* _subdept;
    QCheckBox* _purchased;
    QCheckBox* _sold;
    QCheckBox* _inventoried;
    QCheckBox* _open_dept;
    QLabel* _expenseLabel;
    AccountLookup* _expenseLookup;
    LookupEdit* _expense_acct;
    LookupEdit* _income_acct;
    LookupEdit* _asset_acct;
    Table* _sizes;
    Table* _numbers;
    QComboBox* _number_size;
    Table* _stores;
    Table* _costs;
    Table* _vendors;
    QComboBox* _ordernum_size;
    Table* _groups;
    QTabWidget* _tabs;
    QFrame* _vendorsTab;
    QFrame* _buildTab;
    LineEdit* _purch_size;
    NumberEdit* _purch_qty;
    PriceEdit* _cost;
    LookupEdit* _purch_tax;
    LookupEdit* _vendor;
    LineEdit* _vendor_num;
    LineEdit* _sell_size;
    NumberEdit* _sell_qty;
    PriceEdit* _price;
    NumberEdit* _margin;
    NumberEdit* _profit;
    NumberEdit* _target_gm;
    NumberEdit* _allowed_var;
    LookupEdit* _sell_tax;
    QLabel* _depositLabel;
    NumberEdit* _deposit;
    QCheckBox* _cost_inc_tax;
    QCheckBox* _price_inc_tax;
    QCheckBox* _cost_inc_dep;
    QCheckBox* _price_inc_dep;
    QCheckBox* _discountable;
    QCheckBox* _qty_enforced;
    QCheckBox* _qty_decimals;
    QCheckBox* _qty_price;
    QCheckBox* _weighed;
    NumberEdit* _costDisc;
    NumberEdit* _build_qty;
    NumberEdit* _build_cost;
    Table* _build;
    Table* _extra;
    LookupEdit* _store;
    LookupEdit* _location;
    NumberEdit* _minQty;
    NumberEdit* _maxQty;
    QCheckBox* _discontinued;
    QCheckBox* _stocked;
    NumberEdit* _onHand;
    NumberEdit* _onOrder;
    QGroupBox* _open_inv;
    QLabel* _asOfLabel;
    DatePopup* _as_of;
    QComboBox* _open_size;
    NumberEdit* _open_qty;
    NumberEdit* _open_cost;
};

#endif // ITEM_MASTER_H
