// $Id: item_margin.h,v 1.4 2004/01/31 01:50:31 arandell Exp $
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

#ifndef ITEM_MARGIN_H
#define ITEM_MARGIN_H

#include "quasar_window.h"
#include "item.h"
#include "tax.h"

class ItemLookup;
class LookupEdit;
class LineEdit;
class PriceEdit;
class NumberEdit;
class ComboBox;

class ItemMargin: public QuasarWindow {
    Q_OBJECT
public:
    ItemMargin(MainWindow* main);
    ~ItemMargin();

    void setStoreId(Id store_id);
    void setItem(Id item_id, const QString& number);

protected slots:
    void slotItemChanged();
    void slotStoreChanged();
    void slotPriceSizeChanged();
    void slotPriceChanged();
    void slotPriceBaseChanged();
    void slotCostSizeChanged();
    void slotCostChanged();
    void slotCostBaseChanged();
    void slotMarginSizeChanged();
    void slotMarginPriceChanged();
    void slotRepCostChanged();
    void slotRepProfitChanged();
    void slotRepMarginChanged();
    void slotLastProfitChanged();
    void slotLastMarginChanged();
    void slotAvgProfitChanged();
    void slotAvgMarginChanged();
    void slotRefresh();
    void slotSave();
    void slotClose();

protected:
    bool checkForChanges();
    void newItem();
    void recalculateMargins();

    ItemLookup* _lookup;
    LookupEdit* _item;
    LineEdit* _desc;
    LookupEdit* _store;
    ComboBox* _priceSize;
    PriceEdit* _price;
    NumberEdit* _priceTax;
    NumberEdit* _priceDeposit;
    NumberEdit* _priceBase;
    ComboBox* _costSize;
    PriceEdit* _cost;
    NumberEdit* _costTax;
    NumberEdit* _costDeposit;
    NumberEdit* _costBase;
    ComboBox* _marginSize;
    NumberEdit* _marginPrice;
    NumberEdit* _targetGM;
    NumberEdit* _repCost;
    NumberEdit* _repProfit;
    NumberEdit* _repMargin;
    NumberEdit* _lastCost;
    NumberEdit* _lastProfit;
    NumberEdit* _lastMargin;
    NumberEdit* _avgCost;
    NumberEdit* _avgProfit;
    NumberEdit* _avgMargin;

    Item _curr;
    Id _currStoreId;
    QString _currPriceSize;
    QString _currCostSize;
    QString _currMarginSize;
    Tax _sellTax;
    Tax _purchaseTax;
    fixed _currAvgCost;
    fixed _currLastCost;
};

#endif // ITEM_MARGIN_H
