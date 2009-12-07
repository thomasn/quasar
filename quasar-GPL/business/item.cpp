// $Id: item.cpp,v 1.48 2005/02/02 10:48:54 bpepers Exp $
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

#include "item.h"

Item::Item()
    : _description(""), _dept_id(INVALID_ID), _subdept_id(INVALID_ID),
      _purchased(true), _sold(true), _inventoried(true), _sell_size(""),
      _purchase_size(""), _expense_acct(INVALID_ID), _income_acct(INVALID_ID),
      _asset_acct(INVALID_ID), _sell_tax(INVALID_ID),
      _purchase_tax(INVALID_ID), _qty_enforced(false), _qty_decimals(false),
      _weighed(false), _discountable(true), _qty_price(false), _deposit(0.0),
      _cost_inc_tax(false), _cost_inc_dep(false), _price_inc_tax(false),
      _price_inc_dep(false), _build_qty(0.0), _discontinued(false),
      _open_dept(false), _cost_disc(0.0)
{
    _data_type = ITEM;
}

Item::~Item()
{
}

bool
Item::operator==(const Item& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._description != _description) return false;
    if (rhs._dept_id != _dept_id) return false;
    if (rhs._subdept_id != _subdept_id) return false;
    if (rhs._purchased != _purchased) return false;
    if (rhs._sold != _sold) return false;
    if (rhs._inventoried != _inventoried) return false;
    if (rhs._sell_size != _sell_size) return false;
    if (rhs._purchase_size != _purchase_size) return false;
    if (rhs._expense_acct != _expense_acct) return false;
    if (rhs._income_acct != _income_acct) return false;
    if (rhs._asset_acct != _asset_acct) return false;
    if (rhs._sell_tax != _sell_tax) return false;
    if (rhs._purchase_tax != _purchase_tax) return false;
    if (rhs._qty_enforced != _qty_enforced) return false;
    if (rhs._qty_decimals != _qty_decimals) return false;
    if (rhs._weighed != _weighed) return false;
    if (rhs._discountable != _discountable) return false;
    if (rhs._qty_price != _qty_price) return false;
    if (rhs._deposit != _deposit) return false;
    if (rhs._cost_inc_tax != _cost_inc_tax) return false;
    if (rhs._cost_inc_dep != _cost_inc_dep) return false;
    if (rhs._price_inc_tax != _price_inc_tax) return false;
    if (rhs._price_inc_dep != _price_inc_dep) return false;
    if (rhs._build_qty != _build_qty) return false;
    if (rhs._discontinued != _discontinued) return false;
    if (rhs._open_dept != _open_dept) return false;
    if (rhs._cost_disc != _cost_disc) return false;
    if (rhs._descs != _descs) return false;
    if (rhs._sizes != _sizes) return false;
    if (rhs._numbers != _numbers) return false;
    if (rhs._vendors != _vendors) return false;
    if (rhs._groups != _groups) return false;
    if (rhs._stores != _stores) return false;
    if (rhs._costs != _costs) return false;
    if (rhs._items != _items) return false;
    return true;
}

bool
Item::operator!=(const Item& rhs) const
{
    return !(*this == rhs);
}

QString
Item::number() const
{
    QString number;
    if (_numbers.size() > 0)
	number = _numbers[0].number;
    return number;
}

QString
Item::number(const QString& size) const
{
    for (unsigned int i = 0; i < _numbers.size(); ++i)
	if (_numbers[i].size == size)
	    return _numbers[i].number;
    return number();
}

QString
Item::numberSize(const QString& number) const
{
    unsigned int i;
    for (i = 0; i < _numbers.size(); ++i)
	if (_numbers[i].number == number)
	    return _numbers[i].size;
    for (i = 0; i < _vendors.size(); ++i)
	if (_vendors[i].number == number)
	    return _vendors[i].size;
    return "";
}

ItemDesc::ItemDesc()
    : description(""), card_id(INVALID_ID), card_group(INVALID_ID)
{
}

ItemDesc::ItemDesc(const QString& _desc, Id _id, Id _group)
    : description(_desc), card_id(_id), card_group(_group)
{
}

bool
ItemDesc::operator==(const ItemDesc& rhs) const
{
    if (rhs.description != description) return false;
    if (rhs.card_id != card_id) return false;
    if (rhs.card_group != card_group) return false;
    return true;
}

bool
ItemDesc::operator!=(const ItemDesc& rhs) const
{
    return !(*this == rhs);
}

ItemSize::ItemSize()
    : qty(0.0), weight(0.0), multiple(1.0)
{
}

ItemSize::ItemSize(const QString& _name, fixed _qty, fixed _weight)
    : name(_name), qty(_qty), weight(_weight)
{
}

bool
ItemSize::operator==(const ItemSize& rhs) const
{
    if (rhs.name != name) return false;
    if (rhs.qty != qty) return false;
    if (rhs.weight != weight) return false;
    if (rhs.multiple != multiple) return false;
    return true;
}

bool
ItemSize::operator!=(const ItemSize& rhs) const
{
    return !(*this == rhs);
}

Id
Item::defaultVendor() const
{
    if (_vendors.size() == 0)
	return INVALID_ID;
    return _vendors[0].vendor_id;
}

QString
Item::orderNumber(Id vendor_id, const QString& size) const
{
    for (unsigned int i = 0; i < _vendors.size(); ++i) {
	if (_vendors[i].vendor_id != vendor_id) continue;
	if (_vendors[i].number.isEmpty()) continue;

	QString number = _vendors[i].number;
	QString numSize = numberSize(number);
	if (!size.isEmpty() && !numSize.isEmpty() && numSize != size)
	    continue;
	return number;
    }

    if (!number(size).isEmpty())
	return number(size);

    return number();
}

fixed
Item::sizeQty(const QString& size) const
{
    for (unsigned int i = 0; i < _sizes.size(); ++i)
	if (_sizes[i].name == size)
	    return _sizes[i].qty;
    return 1.0;
}

fixed
Item::weight(const QString& size) const
{
    for (unsigned int i = 0; i < _sizes.size(); ++i)
	if (_sizes[i].name == size)
	    return _sizes[i].weight;
    return 0.0;
}

fixed
Item::orderMultiple(const QString& size) const
{
    for (unsigned int i = 0; i < _sizes.size(); ++i)
	if (_sizes[i].name == size)
	    return _sizes[i].multiple;
    return 1.0;
}

bool
Item::stocked(Id store_id) const
{
    for (unsigned int i = 0; i < _stores.size(); ++i) {
	if (_stores[i].store_id != store_id) continue;
	return _stores[i].stocked;
    }
    return true;
}

Id
Item::locationId(Id store_id) const
{
    for (unsigned int i = 0; i < _stores.size(); ++i) {
	if (_stores[i].store_id != store_id) continue;
	return _stores[i].location_id;
    }
    return INVALID_ID;
}

fixed
Item::minQty(Id store_id) const
{
    for (unsigned int i = 0; i < _stores.size(); ++i) {
	if (_stores[i].store_id != store_id) continue;
	return _stores[i].min_qty;
    }
    return 0.0;
}

fixed
Item::maxQty(Id store_id) const
{
    for (unsigned int i = 0; i < _stores.size(); ++i) {
	if (_stores[i].store_id != store_id) continue;
	return _stores[i].max_qty;
    }
    return 0.0;
}

Price
Item::cost(Id store_id, const QString& size) const
{
    for (unsigned int i = 0; i < _costs.size(); ++i) {
	if (_costs[i].store_id != store_id) continue;
	if (_costs[i].size != size) continue;
	return _costs[i].cost;
    }
    return Price();
}

fixed
Item::cost(Id store_id, const QString& size, fixed qty) const
{
    for (unsigned int i = 0; i < _costs.size(); ++i) {
	if (_costs[i].store_id != store_id) continue;
	if (_costs[i].size != size) continue;
	if (_costs[i].cost.isNull()) continue;
	return _costs[i].cost.calculate(qty);
    }

    if (size != _purchase_size) {
	fixed caseCost = cost(store_id, _purchase_size, qty);
	return caseCost * sizeQty(size) / sizeQty(_purchase_size);
    }

    return 0.0;
}

void
Item::setCost(Id store_id, const QString& size, Price cost)
{
    for (unsigned int i = 0; i < _costs.size(); ++i) {
	if (_costs[i].store_id != store_id) continue;
	if (_costs[i].size != size) continue;
	_costs[i].cost = cost;
	return;
    }

    ItemCost info;
    info.store_id = store_id;
    info.size = size;
    info.cost = cost;
    _costs.push_back(info);
}

Price
Item::price(Id store_id, const QString& size) const
{
    for (unsigned int i = 0; i < _costs.size(); ++i) {
	if (_costs[i].store_id != store_id) continue;
	if (_costs[i].size != size) continue;
	return _costs[i].price;
    }
    return Price();
}

void
Item::setPrice(Id store_id, const QString& size, Price price)
{
    for (unsigned int i = 0; i < _costs.size(); ++i) {
	if (_costs[i].store_id != store_id) continue;
	if (_costs[i].size != size) continue;
	_costs[i].price = price;
	return;
    }

    ItemCost info;
    info.store_id = store_id;
    info.size = size;
    info.price = price;
    _costs.push_back(info);
}

fixed
Item::targetGM(Id store_id, const QString& size) const
{
    for (unsigned int i = 0; i < _costs.size(); ++i) {
	if (_costs[i].store_id != store_id) continue;
	if (_costs[i].size != size) continue;
	return _costs[i].target_gm;
    }
    return 0.0;
}

fixed
Item::allowedVariance(Id store_id, const QString& size) const
{
    for (unsigned int i = 0; i < _costs.size(); ++i) {
	if (_costs[i].store_id != store_id) continue;
	if (_costs[i].size != size) continue;
	return _costs[i].allowed_var;
    }
    return 0.0;
}

ItemPlu::ItemPlu()
    : number(""), size("")
{
}

ItemPlu::ItemPlu(const QString& _number, const QString& _size)
    : number(_number), size(_size)
{
}

bool
ItemPlu::operator==(const ItemPlu& rhs) const
{
    if (rhs.number != number) return false;
    if (rhs.size != size) return false;
    return true;
}

bool
ItemPlu::operator!=(const ItemPlu& rhs) const
{
    return !(*this == rhs);
}

ItemVendor::ItemVendor()
{
}

ItemVendor::ItemVendor(Id _vendor_id, const QString& _number,
		       const QString& _size)
    : vendor_id(_vendor_id), number(_number), size(_size)
{
}

bool
ItemVendor::operator==(const ItemVendor& rhs) const
{
    if (rhs.vendor_id != vendor_id) return false;
    if (rhs.number != number) return false;
    if (rhs.size != size) return false;
    return true;
}

bool
ItemVendor::operator!=(const ItemVendor& rhs) const
{
    return !(*this == rhs);
}

ItemStore::ItemStore()
    : store_id(INVALID_ID), stocked(false), location_id(INVALID_ID),
      min_qty(0.0), max_qty(0.0)
{
}

bool
ItemStore::operator==(const ItemStore& rhs) const
{
    if (rhs.store_id != store_id) return false;
    if (rhs.stocked != stocked) return false;
    if (rhs.location_id != location_id) return false;
    if (rhs.min_qty != min_qty) return false;
    if (rhs.max_qty != max_qty) return false;
    return true;
}

bool
ItemStore::operator!=(const ItemStore& rhs) const
{
    return !(*this == rhs);
}

ItemCost::ItemCost()
    : store_id(INVALID_ID), size(""), target_gm(0.0), allowed_var(0.0)
{
}

bool
ItemCost::operator==(const ItemCost& rhs) const
{
    if (rhs.store_id != store_id) return false;
    if (rhs.size != size) return false;
    if (rhs.cost != cost) return false;
    if (rhs.price != price) return false;
    if (rhs.target_gm != target_gm) return false;
    if (rhs.allowed_var != allowed_var) return false;
    return true;
}

bool
ItemCost::operator!=(const ItemCost& rhs) const
{
    return !(*this == rhs);
}

Component::Component()
    : item_id(INVALID_ID), number(""), size(""), qty(0.0)
{
}

Component::Component(Id _item_id, const QString& _number,
		     const QString& _size, fixed _qty)
    : item_id(_item_id), number(_number), size(_size), qty(_qty)
{
}

bool
Component::operator==(const Component& rhs) const
{
    if (rhs.item_id != item_id) return false;
    if (rhs.number != number) return false;
    if (rhs.size != size) return false;
    if (rhs.qty != qty) return false;
    return true;
}

bool
Component::operator!=(const Component& rhs) const
{
    return !(*this == rhs);
}

void
Item::setFromSubdept(const Subdept& subdept)
{
    setDeptId(subdept.deptId());
    setSubdeptId(subdept.id());
    setPurchased(subdept.isPurchased());
    setSold(subdept.isSold());
    setInventoried(subdept.isInventoried());
    setExpenseAccount(subdept.expenseAccount());
    setIncomeAccount(subdept.incomeAccount());
    setAssetAccount(subdept.assetAccount());
    setSellTax(subdept.sellTax());
    setPurchaseTax(subdept.purchaseTax());
    setDiscountable(subdept.isDiscountable());
}
