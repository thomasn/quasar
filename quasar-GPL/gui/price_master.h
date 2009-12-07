// $Id: price_master.h,v 1.11 2004/01/31 01:50:31 arandell Exp $
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

#ifndef PRICE_MASTER_H
#define PRICE_MASTER_H

#include "item_price.h"
#include "data_window.h"

class LookupEdit;
class LineEdit;
class DatePopup;
class NumberEdit;
class MoneyEdit;
class PercentEdit;
class PriceEdit;
class QWidgetStack;
class QRadioButton;
class QCheckBox;
class QComboBox;

class PriceMaster: public DataWindow {
    Q_OBJECT
public:
    PriceMaster(MainWindow* main, Id price_id=INVALID_ID, bool isCost=false);
    ~PriceMaster();

protected slots:
    void slotItemChanged();
    void slotItemIdChanged();
    void slotCardChanged();
    void slotPriceChanged();

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

    bool _cost;
    ItemPrice _orig;
    ItemPrice _curr;

    QString currentSize();

    // Widgets
    QRadioButton* _itemIdLabel;
    LookupEdit* _itemId;
    QRadioButton* _itemGroupLabel;
    LookupEdit* _itemGroup;
    QRadioButton* _subdeptIdLabel;
    LookupEdit* _subdeptId;
    QWidgetStack* _sizes;
    QComboBox* _comboSize;
    LineEdit* _editSize;
    LookupEdit* _cardId;
    LookupEdit* _cardGroup;
    QRadioButton* _cardIdLabel;
    QRadioButton* _cardGroupLabel;
    LookupEdit* _store;
    QCheckBox* _promotion;
    QCheckBox* _discountable;
    DatePopup* _startDate;
    DatePopup* _stopDate;
    LineEdit* _days;
    NumberEdit* _qtyLimit;
    NumberEdit* _minQty;
    MoneyEdit* _minValue;
    PriceEdit* _price;
    PercentEdit* _costPlus;
    PercentEdit* _percentOff;
    MoneyEdit* _dollarOff;
    QRadioButton* _priceLabel;
    QRadioButton* _costLabel;
    QRadioButton* _percentLabel;
    QRadioButton* _dollarLabel;
};

#endif // PRICE_MASTER_H
