// $Id: item_price.h,v 1.15 2005/01/30 00:51:13 bpepers Exp $
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

#ifndef ITEM_PRICE_H
#define ITEM_PRICE_H

#include "data_object.h"
#include "price.h"

class ItemPrice: public DataObject {
public:
    // Constructors and Destructor
    ItemPrice();
    ~ItemPrice();

    enum { PRICE, COST_PLUS, PERCENT_OFF, DOLLAR_OFF };

    // Get methods
    bool isCost()			const { return _is_cost; }
    Id itemId()				const { return _item_id; }
    QString number()			const { return _number; }
    Id itemGroup()			const { return _item_group; }
    Id deptId()				const { return _dept_id; }
    Id subdeptId()			const { return _subdept_id; }
    QString size()			const { return _size; }
    Id cardId()				const { return _card_id; }
    Id cardGroup()			const { return _card_group; }
    Id storeId()			const { return _store_id; }
    bool isPromotion()			const { return _promotion; }
    bool isDiscountable()		const { return _discountable; }
    QDate startDate()			const { return _start_date; }
    QDate stopDate()			const { return _stop_date; }
    fixed qtyLimit()			const { return _qty_limit; }
    fixed minimumQty()			const { return _min_qty; }
    bool dayOfWeek(int day)		const { return _day_of_week[day]; }
    int method()			const { return _method; }
    Price price()			const { return _price; }
    fixed percentDiscount()		const { return _percent; }
    fixed dollarDiscount()		const { return _dollar; }

    // Set methods
    void setIsCost(bool flag)			{ _is_cost = flag; }
    void setItemId(Id item_id)			{ _item_id = item_id; }
    void setNumber(const QString& number)	{ _number = number; }
    void setItemGroup(Id item_group)		{ _item_group = item_group; }
    void setDeptId(Id dept_id)			{ _dept_id = dept_id; }
    void setSubdeptId(Id subdept_id)		{ _subdept_id = subdept_id; }
    void setSize(const QString& size)		{ _size = size; }
    void setCardId(Id card_id)			{ _card_id = card_id; }
    void setCardGroup(Id card_group)		{ _card_group = card_group; }
    void setStoreId(Id store_id)		{ _store_id = store_id; }
    void setPromotion(bool flag)		{ _promotion = flag; }
    void setDiscountable(bool flag)		{ _discountable = flag; }
    void setStartDate(QDate date)		{ _start_date = date; }
    void setStopDate(QDate date)		{ _stop_date = date; }
    void setQtyLimit(fixed limit)		{ _qty_limit = limit; }
    void setMinimumQty(fixed qty)		{ _min_qty = qty; }
    void setDayOfWeek(int day, bool flag)	{ _day_of_week[day] = flag; }
    void setMethod(int method)			{ _method = method; }
    void setPrice(Price price)			{ _price = price; }
    void setPercentDiscount(fixed percent)	{ _percent = percent; }
    void setDollarDiscount(fixed dollar)	{ _dollar = dollar; }

    // Check if null price
    bool isNull() const				{ return _price.isNull(); }
    void setNull()				{ _price.setNull(); }

    // Day of week as string of 7 Y/N characters
    QString dayOfWeekString() const;
    void setDayOfWeek(const QString& days);

    // Calculate price for a quantity of items
    fixed calculate(fixed qty) const	{ return _price.calculate(qty); }

    // Operations
    bool operator==(const ItemPrice& rhs) const;
    bool operator!=(const ItemPrice& rhs) const;
    bool operator< (const ItemPrice& rhs) const;
    bool operator<=(const ItemPrice& rhs) const;
    bool operator> (const ItemPrice& rhs) const;
    bool operator>=(const ItemPrice& rhs) const;

protected:
    bool _is_cost;		// True if this is a cost
    Id _item_id;		// Link to specific item
    QString _number;		// Item number
    Id _item_group;		// Link to group of items
    Id _dept_id;		// Link to items in department
    Id _subdept_id;		// Link to items in subdepartment
    QString _size;		// Size of item
    Id _card_id;		// Link to specific customer or vendor
    Id _card_group;		// Link to customer or vendor group
    Id _store_id;		// Store price is for
    bool _promotion;		// Is price a promotion?
    bool _discountable;		// Is price further discountable?
    QDate _start_date;		// First date price is valid
    QDate _stop_date;		// Last date price is valid
    fixed _qty_limit;		// Maximum quantity can sell at price
    fixed _min_qty;		// Minimum qty of item in sale for price
    bool _day_of_week[7];	// Days of week price is valid
    int _method;		// Method of price calculation
    Price _price;		// Price for PRICE method
    fixed _percent;		// Percent for COST_PLUS and PERCENT_OFF
    fixed _dollar;		// Amount for DOLLAR_OFF
};

#endif // ITEM_PRICE_H
