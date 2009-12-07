// $Id: item.h,v 1.43 2005/02/02 10:48:54 bpepers Exp $
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
//

#ifndef ITEM_H
#define ITEM_H

#include "data_object.h"
#include "price.h"
#include "subdept.h"

// ItemDesc is for item descriptions.  Each item can have more than one
// description and a particular description can be associated with a
// specific card or a card group.  The item itself always has a default
// description.  The purpose of this is to allow for different languages.
struct ItemDesc {
    ItemDesc();
    ItemDesc(const QString& desc, Id card_id, Id card_group);

    QString description;
    Id card_id;
    Id card_group;

    bool operator==(const ItemDesc& rhs) const;
    bool operator!=(const ItemDesc& rhs) const;
};

// ItemSize is for keeping track of the sizes an item is sold purchased in.
// There must always be at least one size defined.  The qty is how many
// individual items are in the container size.
struct ItemSize {
    ItemSize();
    ItemSize(const QString& name, fixed qty, fixed weight);

    QString name;
    fixed qty;
    fixed weight;
    fixed multiple;

    bool operator==(const ItemSize& rhs) const;
    bool operator!=(const ItemSize& rhs) const;
};

// ItemPlu is for keeping track of item numbers.  Each ItemPlu has a number
// which can be used for looking up the item.  It can optionally also have
// a size that the number implies.  If the size is blank, the selling or
// purchasing default size will be used instead based on the type of screen.
struct ItemPlu {
    ItemPlu();
    ItemPlu(const QString& number, const QString& size);

    QString number;
    QString size;

    bool operator==(const ItemPlu& rhs) const;
    bool operator!=(const ItemPlu& rhs) const;
};

// ItemVendor is for keeping track of vendors and vendor order numbers.
// The first vendor in the list is considered the primary vendor for the
// item.  The order numbers must exist in the ItemPlu list already.  The
// same vendor may appear in the list multiple times with different numbers
// as long as the size is different for each number.
struct ItemVendor {
    ItemVendor();
    ItemVendor(Id vendor_id, const QString& number, const QString& size);

    Id vendor_id;
    QString number;
    QString size;

    bool operator==(const ItemVendor& rhs) const;
    bool operator!=(const ItemVendor& rhs) const;
};

// ItemStore is for keeping track of the item data that is different in each
// store.  This is the location where the item can be found in the store and
// a flag on whether a given store stocks an item.
struct ItemStore {
    ItemStore();

    Id store_id;
    bool stocked;
    Id location_id;
    fixed min_qty;
    fixed max_qty;

    bool operator==(const ItemStore& rhs) const;
    bool operator!=(const ItemStore& rhs) const;
};

// ItemCost is for keeping track of the regular cost and price.  It is by
// store and size and also contains the target margin and allowed variance.
struct ItemCost {
    ItemCost();

    Id store_id;
    QString size;
    Price cost;
    Price price;
    fixed target_gm;
    fixed allowed_var;

    bool operator==(const ItemCost& rhs) const;
    bool operator!=(const ItemCost& rhs) const;
};

// Component is used to hold the list of items used to build from a kit.
struct Component {
    Component();
    Component(Id item_id, const QString& num, const QString& size, fixed qty);

    Id item_id;
    QString number;
    QString size;
    fixed qty;

    bool operator==(const Component& rhs) const;
    bool operator!=(const Component& rhs) const;
};

class Item: public DataObject {
public:
    // Constructors and Destructor
    Item();
    ~Item();

    // Get methods
    QString description()		const { return _description; }
    Id deptId()				const { return _dept_id; }
    Id subdeptId()			const { return _subdept_id; }
    bool isPurchased()			const { return _purchased; }
    bool isSold()			const { return _sold; }
    bool isInventoried()		const { return _inventoried; }
    QString sellSize()			const { return _sell_size; }
    QString purchaseSize()		const { return _purchase_size; }
    Id expenseAccount()			const { return _expense_acct; }
    Id incomeAccount()			const { return _income_acct; }
    Id assetAccount()			const { return _asset_acct; }
    Id sellTax()			const { return _sell_tax; }
    Id purchaseTax()			const { return _purchase_tax; }
    bool isQuantityEnforced()		const { return _qty_enforced; }
    bool isQuantityDecimals()		const { return _qty_decimals; }
    bool isWeighed()			const { return _weighed; }
    bool isDiscountable()		const { return _discountable; }
    bool isQtyFromPrice()		const { return _qty_price; }
    fixed deposit()			const { return _deposit; }
    bool costIncludesTax()		const { return _cost_inc_tax; }
    bool costIncludesDeposit()		const { return _cost_inc_dep; }
    bool priceIncludesTax()		const { return _price_inc_tax; }
    bool priceIncludesDeposit()		const { return _price_inc_dep; }
    fixed buildQty()			const { return _build_qty; }
    bool isDiscontinued()		const { return _discontinued; }
    bool isOpenDept()			const { return _open_dept; }
    fixed costDiscount()		const { return _cost_disc; }

    // Set methods
    void setDescription(const QString& desc)	{ _description = desc; }
    void setDeptId(Id dept_id)			{ _dept_id = dept_id; }
    void setSubdeptId(Id subdept_id)		{ _subdept_id = subdept_id; }
    void setPurchased(bool flag)		{ _purchased = flag; }
    void setSold(bool flag)			{ _sold = flag; }
    void setInventoried(bool flag)		{ _inventoried = flag; }
    void setSellSize(const QString& size)	{ _sell_size = size; }
    void setPurchaseSize(const QString& size)	{ _purchase_size = size; }
    void setExpenseAccount(Id account_id)	{ _expense_acct = account_id; }
    void setIncomeAccount(Id account_id)	{ _income_acct = account_id; }
    void setAssetAccount(Id account_id)		{ _asset_acct = account_id; }
    void setSellTax(Id tax_id)			{ _sell_tax = tax_id; }
    void setPurchaseTax(Id tax_id)		{ _purchase_tax = tax_id; }
    void setQuantityEnforced(bool flag)		{ _qty_enforced = flag; }
    void setQuantityDecimals(bool flag)		{ _qty_decimals = flag; }
    void setWeighed(bool flag)			{ _weighed = flag; }
    void setDiscountable(bool flag)		{ _discountable = flag; }
    void setQtyFromPrice(bool flag)		{ _qty_price = flag; }
    void setDeposit(fixed deposit)		{ _deposit = deposit; }
    void setCostIncludesTax(bool flag)		{ _cost_inc_tax = flag; }
    void setCostIncludesDeposit(bool flag)	{ _cost_inc_dep = flag; }
    void setPriceIncludesTax(bool flag)		{ _price_inc_tax = flag; }
    void setPriceIncludesDeposit(bool flag)	{ _price_inc_dep = flag; }
    void setBuildQty(fixed qty)			{ _build_qty = qty; }
    void setDiscontinued(bool flag)		{ _discontinued = flag; }
    void setOpenDept(bool flag)			{ _open_dept = flag; }
    void setCostDiscount(fixed percent)		{ _cost_disc = percent; }

    // Item sizes
    const vector<ItemSize>& sizes()	const { return _sizes; }
    vector<ItemSize>& sizes()		      { return _sizes; }
    fixed sizeQty(const QString& size) const;
    fixed weight(const QString& size) const;
    fixed orderMultiple(const QString& size) const;

    // Item numbers
    const vector<ItemPlu>& numbers()	const { return _numbers; }
    vector<ItemPlu>& numbers()		      { return _numbers; }
    QString number() const;
    QString number(const QString& size) const;
    QString numberSize(const QString& number) const;

    // Descriptions
    const vector<ItemDesc>& descriptions()	const { return _descs; }
    vector<ItemDesc>& descriptions()		      { return _descs; }

    // Vendors
    const vector<ItemVendor>& vendors()	const { return _vendors; }
    vector<ItemVendor>& vendors()	      { return _vendors; }
    Id defaultVendor() const;
    QString orderNumber(Id vendor_id, const QString& size) const;

    // Item groups
    const vector<Id>& groups()		const { return _groups; }
    vector<Id>& groups()		      { return _groups; }

    // Store info
    const vector<ItemStore>& stores()	const { return _stores; }
    vector<ItemStore>& stores()		      { return _stores; }
    bool stocked(Id store_id) const;
    Id locationId(Id store_id) const;
    fixed minQty(Id store_id) const;
    fixed maxQty(Id store_id) const;

    // Cost info
    const vector<ItemCost>& costs()	const { return _costs; }
    vector<ItemCost>& costs()		      { return _costs; }
    Price cost(Id store_id, const QString& size) const;
    Price price(Id store_id, const QString& size) const;
    fixed targetGM(Id store_id, const QString& size) const;
    fixed allowedVariance(Id store_id, const QString& size) const;
    fixed cost(Id store_id, const QString& size, fixed qty) const;
    void setCost(Id store_id, const QString& size, Price cost);
    void setPrice(Id store_id, const QString& size, Price price);

    // Components for kit building
    const vector<Component>& items()	const { return _items; }
    vector<Component>& items()		      { return _items; }

    // Set item info from a subdepartment
    void setFromSubdept(const Subdept& subdept);

    // Operations
    bool operator==(const Item& rhs) const;
    bool operator!=(const Item& rhs) const;

private:
    QString _description;	// Main item description
    Id _dept_id;		// Department
    Id _subdept_id;		// Subdepartment
    bool _purchased;		// Can be used in order/receive
    bool _sold;			// Can be used in invoice
    bool _inventoried;		// Can be used in adjustment & has on hands
    QString _sell_size;		// Default selling size
    QString _purchase_size;	// Default purchasing size
    Id _expense_acct;		// Expense (or COGS) account
    Id _income_acct;		// Income account
    Id _asset_acct;		// Asset account (Inventory usually)
    Id _sell_tax;		// Tax when selling
    Id _purchase_tax;		// Tax when purchasing
    bool _qty_enforced;		// Quantity enforced?
    bool _qty_decimals;		// Decimals allowed in quantity
    bool _weighed;		// Weighed?
    bool _discountable;		// Can discount price?
    bool _qty_price;		// Calculate qty from price (fuel)?
    fixed _deposit;		// Individual deposit amount
    bool _cost_inc_tax;		// Cost includes tax?
    bool _cost_inc_dep;		// Cost includes deposit?
    bool _price_inc_tax;	// Price includes tax?
    bool _price_inc_dep;	// Price includes deposit?
    fixed _build_qty;		// Quantity built from components
    bool _discontinued;		// Discountinued (can't purchase)?
    bool _open_dept;		// Open department item? (cost from price)
    fixed _cost_disc;		// Special discount off cost for margin calcs
    vector<ItemDesc> _descs;	// Secondary descriptions for the item
    vector<ItemSize> _sizes;	// Sizes for item
    vector<ItemPlu> _numbers;	// Item numbers for the item
    vector<ItemVendor> _vendors;// Vendors & order numbers
    vector<Id> _groups;		// Groups (for pricing and reporting)
    vector<ItemStore> _stores;	// Store info (stocked and location)
    vector<ItemCost> _costs;	// Cost info (regular cost/price)
    vector<Component> _items;	// Components to build build_qty of item
};

#endif // ITEM_H
