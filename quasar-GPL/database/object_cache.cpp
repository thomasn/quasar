// $Id: object_cache.cpp,v 1.16 2005/04/05 07:11:29 bpepers Exp $
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

#include "object_cache.h"
#include "quasar_db.h"

#include "account.h"
#include "adjust_reason.h"
#include "card_adjust.h"
#include "charge.h"
#include "cheque.h"
#include "company.h"
#include "count.h"
#include "customer.h"
#include "customer_type.h"
#include "dept.h"
#include "discount.h"
#include "employee.h"
#include "expense.h"
#include "extra.h"
#include "general.h"
#include "group.h"
#include "invoice.h"
#include "item.h"
#include "item_adjust.h"
#include "item_price.h"
#include "label_batch.h"
#include "location.h"
#include "nosale.h"
#include "order.h"
#include "order_template.h"
#include "pat_group.h"
#include "pat_worksheet.h"
#include "payout.h"
#include "personal.h"
#include "price_batch.h"
#include "promo_batch.h"
#include "quote.h"
#include "receipt.h"
#include "receive.h"
#include "reconcile.h"
#include "recurring.h"
#include "security_type.h"
#include "shift.h"
#include "slip.h"
#include "station.h"
#include "store.h"
#include "subdept.h"
#include "tax.h"
#include "tender.h"
#include "tender_adjust.h"
#include "tender_count.h"
#include "term.h"
#include "todo.h"
#include "user.h"
#include "vendor.h"
#include "withdraw.h"

#include <assert.h>

ObjectCache::ObjectCache(QuasarDB* db)
    : _db(db)
{
}

ObjectCache::~ObjectCache()
{
    clear();
}

DataObject*
ObjectCache::find(Id object_id, bool cacheOnly)
{
    if (object_id == INVALID_ID)
	return NULL;

    if (_objects.find(object_id) != _objects.end())
	return _objects[object_id];

    if (cacheOnly)
	return NULL;

    DataObject object;
    if (!_db->lookup(object_id, object))
	return NULL;

    switch (object.dataType()) {
    case DataObject::ACCOUNT:		return findAccount(object_id);
    case DataObject::ADJUST_REASON:	return findAdjustReason(object_id);
    case DataObject::CARD_ADJUST:	return findCardAdjust(object_id);
    case DataObject::CHARGE:		return findCharge(object_id);
    case DataObject::CHEQUE:		return findCheque(object_id);
    case DataObject::CLAIM:		return findReceive(object_id);
    case DataObject::COMPANY:		return findCompany(object_id);
    case DataObject::COUNT:		return findCount(object_id);
    case DataObject::CUSTOMER:		return findCustomer(object_id);
    case DataObject::CUST_TYPE:		return findCustomerType(object_id);
    case DataObject::DEPT:		return findDept(object_id);
    case DataObject::DISCOUNT:		return findDiscount(object_id);
    case DataObject::EMPLOYEE:		return findEmployee(object_id);
    case DataObject::EXPENSE:		return findExpense(object_id);
    case DataObject::EXTRA:		return findExtra(object_id);
    case DataObject::GENERAL:		return findGeneral(object_id);
    case DataObject::GROUP:		return findGroup(object_id);
    case DataObject::INVOICE:		return findInvoice(object_id);
    case DataObject::ITEM:		return findItem(object_id);
    case DataObject::ITEM_ADJUST:	return findItemAdjust(object_id);
    case DataObject::ITEM_PRICE:	return findItemPrice(object_id);
    case DataObject::LABEL_BATCH:	return findLabelBatch(object_id);
    case DataObject::LOCATION:		return findLocation(object_id);
    case DataObject::NOSALE:		return findNosale(object_id);
    case DataObject::ORDER:		return findOrder(object_id);
    case DataObject::PO_TEMPLATE:	return findOrderTemplate(object_id);
    case DataObject::PAT_GROUP:		return findPatGroup(object_id);
    case DataObject::PAT_WS:		return findPatWorksheet(object_id);
    case DataObject::PAYOUT:		return findPayout(object_id);
    case DataObject::PERSONAL:		return findPersonal(object_id);
    case DataObject::PLU:		assert("Invalid data type: PLU");
    case DataObject::PRICE_BATCH:	return findPriceBatch(object_id);
    case DataObject::PROMO_BATCH:	return findPromoBatch(object_id);
    case DataObject::QUOTE:		return findQuote(object_id);
    case DataObject::RECEIPT:		return findReceipt(object_id);
    case DataObject::RECEIVE:		return findReceive(object_id);
    case DataObject::RECONCILE:		return findReconcile(object_id);
    case DataObject::RECURRING:		return findRecurring(object_id);
    case DataObject::REPORT:		assert("Invalid data type: REPORT");
    case DataObject::RETURN:		return findInvoice(object_id);
    case DataObject::SECURITY_TYPE:	return findSecurityType(object_id);
    case DataObject::SHIFT:		return findShift(object_id);
    case DataObject::SLIP:		return findSlip(object_id);
    case DataObject::STATION:		return findStation(object_id);
    case DataObject::STORE:		return findStore(object_id);
    case DataObject::SUBDEPT:		return findSubdept(object_id);
    case DataObject::TAX:		return findTax(object_id);
    case DataObject::TENDER:		return findTender(object_id);
    case DataObject::TEND_COUNT:	return findTenderCount(object_id);
    case DataObject::TEND_ADJUST:	return findTenderAdjust(object_id);
    case DataObject::TERM:		return findTerm(object_id);
    case DataObject::TODO:		return findTodo(object_id);
    case DataObject::USER:		return findUser(object_id);
    case DataObject::VENDOR:		return findVendor(object_id);
    case DataObject::WITHDRAW:		return findWithdraw(object_id);
    }

    qWarning("Unknown object type: " + object.dataTypeName());
    return NULL;
}

void
ObjectCache::remove(Id object_id)
{
    if (_objects.find(object_id) != _objects.end()) {
	DataObject* object = _objects[object_id];
	_objects.erase(object_id);
	delete object;
    }
}

void
ObjectCache::clear()
{
    ObjectMap::iterator it;
    for (it = _objects.begin(); it != _objects.end(); ++it)
	delete it.data();
    _objects.clear();
}

Card*
ObjectCache::findCard(Id card_id)
{
    DataObject* object = find(card_id);
    if (object == NULL) return NULL;

    switch (object->dataType()) {
    case DataObject::CUSTOMER:
    case DataObject::EMPLOYEE:
    case DataObject::PERSONAL:
    case DataObject::VENDOR:
	return (Card*)object;
    default:
	break;
    }

    return NULL;
}

Gltx*
ObjectCache::findGltx(Id gltx_id)
{
    DataObject* object = find(gltx_id);
    if (object == NULL) return NULL;

    switch (object->dataType()) {
    case DataObject::CARD_ADJUST:
    case DataObject::CHEQUE:
    case DataObject::CLAIM:
    case DataObject::GENERAL:
    case DataObject::INVOICE:
    case DataObject::ITEM_ADJUST:
    case DataObject::NOSALE:
    case DataObject::PAYOUT:
    case DataObject::RECEIPT:
    case DataObject::RECEIVE:
    case DataObject::RETURN:
    case DataObject::SHIFT:
    case DataObject::TEND_ADJUST:
    case DataObject::WITHDRAW:
	return (Gltx*)object;
    default:
	break;
    }

    qWarning("Invalid gltx type: " + object->dataTypeName());
    return NULL;
}

Account*
ObjectCache::findAccount(Id account_id)
{
    DataObject* object = find(account_id, true);
    if (object != NULL) return (Account*)object;

    Account account;
    if (!_db->lookup(account_id, account))
	return NULL;

    object = new Account(account);
    assert(object != NULL);

    _objects.insert(account_id, object);
    return (Account*)object;
}

bool
ObjectCache::findAccount(Id account_id, Account& account)
{
    Account* object = findAccount(account_id);
    if (object == NULL) return false;
    account = *object;
    return true;
}

AdjustReason*
ObjectCache::findAdjustReason(Id reason_id)
{
    DataObject* object = find(reason_id, true);
    if (object != NULL) return (AdjustReason*)object;

    AdjustReason reason;
    if (!_db->lookup(reason_id, reason))
	return NULL;

    object = new AdjustReason(reason);
    assert(object != NULL);

    _objects.insert(reason_id, object);
    return (AdjustReason*)object;
}

bool
ObjectCache::findAdjustReason(Id reason_id, AdjustReason& reason)
{
    AdjustReason* object = findAdjustReason(reason_id);
    if (object == NULL) return false;
    reason = *object;
    return true;
}

CardAdjust*
ObjectCache::findCardAdjust(Id adjust_id)
{
    DataObject* object = find(adjust_id, true);
    if (object != NULL) return (CardAdjust*)object;

    CardAdjust adjust;
    if (!_db->lookup(adjust_id, adjust))
	return NULL;

    object = new CardAdjust(adjust);
    assert(object != NULL);

    _objects.insert(adjust_id, object);
    return (CardAdjust*)object;
}

bool
ObjectCache::findCardAdjust(Id adjust_id, CardAdjust& adjust)
{
    CardAdjust* object = findCardAdjust(adjust_id);
    if (object == NULL) return false;
    adjust = *object;
    return true;
}

Charge*
ObjectCache::findCharge(Id charge_id)
{
    DataObject* object = find(charge_id, true);
    if (object != NULL) return (Charge*)object;

    Charge charge;
    if (!_db->lookup(charge_id, charge))
	return NULL;

    object = new Charge(charge);
    assert(object != NULL);

    _objects.insert(charge_id, object);
    return (Charge*)object;
}

bool
ObjectCache::findCharge(Id charge_id, Charge& charge)
{
    Charge* object = findCharge(charge_id);
    if (object == NULL) return false;
    charge = *object;
    return true;
}

Cheque*
ObjectCache::findCheque(Id cheque_id)
{
    DataObject* object = find(cheque_id, true);
    if (object != NULL) return (Cheque*)object;

    Cheque cheque;
    if (!_db->lookup(cheque_id, cheque))
	return NULL;

    object = new Cheque(cheque);
    assert(object != NULL);

    _objects.insert(cheque_id, object);
    return (Cheque*)object;
}

bool
ObjectCache::findCheque(Id cheque_id, Cheque& cheque)
{
    Cheque* object = findCheque(cheque_id);
    if (object == NULL) return false;
    cheque = *object;
    return true;
}

Company*
ObjectCache::findCompany(Id company_id)
{
    DataObject* object = find(company_id, true);
    if (object != NULL) return (Company*)object;

    Company company;
    if (!_db->lookup(company_id, company))
	return NULL;

    object = new Company(company);
    assert(object != NULL);

    _objects.insert(company_id, object);
    return (Company*)object;
}

bool
ObjectCache::findCompany(Id company_id, Company& company)
{
    Company* object = findCompany(company_id);
    if (object == NULL) return false;
    company = *object;
    return true;
}

Count*
ObjectCache::findCount(Id count_id)
{
    DataObject* object = find(count_id, true);
    if (object != NULL) return (Count*)object;

    Count count;
    if (!_db->lookup(count_id, count))
	return NULL;

    object = new Count(count);
    assert(object != NULL);

    _objects.insert(count_id, object);
    return (Count*)object;
}

bool
ObjectCache::findCount(Id count_id, Count& count)
{
    Count* object = findCount(count_id);
    if (object == NULL) return false;
    count = *object;
    return true;
}

Customer*
ObjectCache::findCustomer(Id customer_id)
{
    DataObject* object = find(customer_id, true);
    if (object != NULL) return (Customer*)object;

    Customer customer;
    if (!_db->lookup(customer_id, customer))
	return NULL;

    object = new Customer(customer);
    assert(object != NULL);

    _objects.insert(customer_id, object);
    return (Customer*)object;
}

bool
ObjectCache::findCustomer(Id customer_id, Customer& customer)
{
    Customer* object = findCustomer(customer_id);
    if (object == NULL) return false;
    customer = *object;
    return true;
}

CustomerType*
ObjectCache::findCustomerType(Id type_id)
{
    DataObject* object = find(type_id, true);
    if (object != NULL) return (CustomerType*)object;

    CustomerType type;
    if (!_db->lookup(type_id, type))
	return NULL;

    object = new CustomerType(type);
    assert(object != NULL);

    _objects.insert(type_id, object);
    return (CustomerType*)object;
}

bool
ObjectCache::findCustomerType(Id type_id, CustomerType& type)
{
    CustomerType* object = findCustomerType(type_id);
    if (object == NULL) return false;
    type = *object;
    return true;
}

Dept*
ObjectCache::findDept(Id dept_id)
{
    DataObject* object = find(dept_id, true);
    if (object != NULL) return (Dept*)object;

    Dept dept;
    if (!_db->lookup(dept_id, dept))
	return NULL;

    object = new Dept(dept);
    assert(object != NULL);

    _objects.insert(dept_id, object);
    return (Dept*)object;
}

bool
ObjectCache::findDept(Id dept_id, Dept& dept)
{
    Dept* object = findDept(dept_id);
    if (object == NULL) return false;
    dept = *object;
    return true;
}

Discount*
ObjectCache::findDiscount(Id discount_id)
{
    DataObject* object = find(discount_id, true);
    if (object != NULL) return (Discount*)object;

    Discount discount;
    if (!_db->lookup(discount_id, discount))
	return NULL;

    object = new Discount(discount);
    assert(object != NULL);

    _objects.insert(discount_id, object);
    return (Discount*)object;
}

bool
ObjectCache::findDiscount(Id discount_id, Discount& discount)
{
    Discount* object = findDiscount(discount_id);
    if (object == NULL) return false;
    discount = *object;
    return true;
}

Employee*
ObjectCache::findEmployee(Id employee_id)
{
    DataObject* object = find(employee_id, true);
    if (object != NULL) return (Employee*)object;

    Employee employee;
    if (!_db->lookup(employee_id, employee))
	return NULL;

    object = new Employee(employee);
    assert(object != NULL);

    _objects.insert(employee_id, object);
    return (Employee*)object;
}

bool
ObjectCache::findEmployee(Id employee_id, Employee& employee)
{
    Employee* object = findEmployee(employee_id);
    if (object == NULL) return false;
    employee = *object;
    return true;
}

Expense*
ObjectCache::findExpense(Id expense_id)
{
    DataObject* object = find(expense_id, true);
    if (object != NULL) return (Expense*)object;

    Expense expense;
    if (!_db->lookup(expense_id, expense))
	return NULL;

    object = new Expense(expense);
    assert(object != NULL);

    _objects.insert(expense_id, object);
    return (Expense*)object;
}

bool
ObjectCache::findExpense(Id expense_id, Expense& expense)
{
    Expense* object = findExpense(expense_id);
    if (object == NULL) return false;
    expense = *object;
    return true;
}

Extra*
ObjectCache::findExtra(Id extra_id)
{
    DataObject* object = find(extra_id, true);
    if (object != NULL) return (Extra*)object;

    Extra extra;
    if (!_db->lookup(extra_id, extra))
	return NULL;

    object = new Extra(extra);
    assert(object != NULL);

    _objects.insert(extra_id, object);
    return (Extra*)object;
}

bool
ObjectCache::findExtra(Id extra_id, Extra& extra)
{
    Extra* object = findExtra(extra_id);
    if (object == NULL) return false;
    extra = *object;
    return true;
}

General*
ObjectCache::findGeneral(Id general_id)
{
    DataObject* object = find(general_id, true);
    if (object != NULL) return (General*)object;

    General general;
    if (!_db->lookup(general_id, general))
	return NULL;

    object = new General(general);
    assert(object != NULL);

    _objects.insert(general_id, object);
    return (General*)object;
}

bool
ObjectCache::findGeneral(Id general_id, General& general)
{
    General* object = findGeneral(general_id);
    if (object == NULL) return false;
    general = *object;
    return true;
}

Group*
ObjectCache::findGroup(Id group_id)
{
    DataObject* object = find(group_id, true);
    if (object != NULL) return (Group*)object;

    Group group;
    if (!_db->lookup(group_id, group))
	return NULL;

    object = new Group(group);
    assert(object != NULL);

    _objects.insert(group_id, object);
    return (Group*)object;
}

bool
ObjectCache::findGroup(Id group_id, Group& group)
{
    Group* object = findGroup(group_id);
    if (object == NULL) return false;
    group = *object;
    return true;
}

Invoice*
ObjectCache::findInvoice(Id invoice_id)
{
    DataObject* object = find(invoice_id, true);
    if (object != NULL) return (Invoice*)object;

    Invoice invoice;
    if (!_db->lookup(invoice_id, invoice))
	return NULL;

    object = new Invoice(invoice);
    assert(object != NULL);

    _objects.insert(invoice_id, object);
    return (Invoice*)object;
}

bool
ObjectCache::findInvoice(Id invoice_id, Invoice& invoice)
{
    Invoice* object = findInvoice(invoice_id);
    if (object == NULL) return false;
    invoice = *object;
    return true;
}

Item*
ObjectCache::findItem(Id item_id)
{
    DataObject* object = find(item_id, true);
    if (object != NULL) return (Item*)object;

    Item item;
    if (!_db->lookup(item_id, item))
	return NULL;

    object = new Item(item);
    assert(object != NULL);

    _objects.insert(item_id, object);
    return (Item*)object;
}

bool
ObjectCache::findItem(Id item_id, Item& item)
{
    Item* object = findItem(item_id);
    if (object == NULL) return false;
    item = *object;
    return true;
}

ItemAdjust*
ObjectCache::findItemAdjust(Id adjust_id)
{
    DataObject* object = find(adjust_id, true);
    if (object != NULL) return (ItemAdjust*)object;

    ItemAdjust adjust;
    if (!_db->lookup(adjust_id, adjust))
	return NULL;

    object = new ItemAdjust(adjust);
    assert(object != NULL);

    _objects.insert(adjust_id, object);
    return (ItemAdjust*)object;
}

bool
ObjectCache::findItemAdjust(Id adjust_id, ItemAdjust& adjust)
{
    ItemAdjust* object = findItemAdjust(adjust_id);
    if (object == NULL) return false;
    adjust = *object;
    return true;
}

ItemPrice*
ObjectCache::findItemPrice(Id price_id)
{
    DataObject* object = find(price_id, true);
    if (object != NULL) return (ItemPrice*)object;

    ItemPrice price;
    if (!_db->lookup(price_id, price))
	return NULL;

    object = new ItemPrice(price);
    assert(object != NULL);

    _objects.insert(price_id, object);
    return (ItemPrice*)object;
}

bool
ObjectCache::findItemPrice(Id price_id, ItemPrice& price)
{
    ItemPrice* object = findItemPrice(price_id);
    if (object == NULL) return false;
    price = *object;
    return true;
}

LabelBatch*
ObjectCache::findLabelBatch(Id batch_id)
{
    DataObject* object = find(batch_id, true);
    if (object != NULL) return (LabelBatch*)object;

    LabelBatch batch;
    if (!_db->lookup(batch_id, batch))
	return NULL;

    object = new LabelBatch(batch);
    assert(object != NULL);

    _objects.insert(batch_id, object);
    return (LabelBatch*)object;
}

bool
ObjectCache::findLabelBatch(Id batch_id, LabelBatch& batch)
{
    LabelBatch* object = findLabelBatch(batch_id);
    if (object == NULL) return false;
    batch = *object;
    return true;
}

Location*
ObjectCache::findLocation(Id location_id)
{
    DataObject* object = find(location_id, true);
    if (object != NULL) return (Location*)object;

    Location location;
    if (!_db->lookup(location_id, location))
	return NULL;

    object = new Location(location);
    assert(object != NULL);

    _objects.insert(location_id, object);
    return (Location*)object;
}

bool
ObjectCache::findLocation(Id location_id, Location& location)
{
    Location* object = findLocation(location_id);
    if (object == NULL) return false;
    location = *object;
    return true;
}

Nosale*
ObjectCache::findNosale(Id nosale_id)
{
    DataObject* object = find(nosale_id, true);
    if (object != NULL) return (Nosale*)object;

    Nosale nosale;
    if (!_db->lookup(nosale_id, nosale))
	return NULL;

    object = new Nosale(nosale);
    assert(object != NULL);

    _objects.insert(nosale_id, object);
    return (Nosale*)object;
}

bool
ObjectCache::findNosale(Id nosale_id, Nosale& nosale)
{
    Nosale* object = findNosale(nosale_id);
    if (object == NULL) return false;
    nosale = *object;
    return true;
}

Order*
ObjectCache::findOrder(Id order_id)
{
    DataObject* object = find(order_id, true);
    if (object != NULL) return (Order*)object;

    Order order;
    if (!_db->lookup(order_id, order))
	return NULL;

    object = new Order(order);
    assert(object != NULL);

    _objects.insert(order_id, object);
    return (Order*)object;
}

bool
ObjectCache::findOrder(Id order_id, Order& order)
{
    Order* object = findOrder(order_id);
    if (object == NULL) return false;
    order = *object;
    return true;
}

OrderTemplate*
ObjectCache::findOrderTemplate(Id template_id)
{
    DataObject* object = find(template_id, true);
    if (object != NULL) return (OrderTemplate*)object;

    OrderTemplate templ;
    if (!_db->lookup(template_id, templ))
	return NULL;

    object = new OrderTemplate(templ);
    assert(object != NULL);

    _objects.insert(template_id, object);
    return (OrderTemplate*)object;
}

bool
ObjectCache::findOrderTemplate(Id template_id, OrderTemplate& templ)
{
    OrderTemplate* object = findOrderTemplate(template_id);
    if (object == NULL) return false;
    templ = *object;
    return true;
}

PatGroup*
ObjectCache::findPatGroup(Id group_id)
{
    DataObject* object = find(group_id, true);
    if (object != NULL) return (PatGroup*)object;

    PatGroup group;
    if (!_db->lookup(group_id, group))
	return NULL;

    object = new PatGroup(group);
    assert(object != NULL);

    _objects.insert(group_id, object);
    return (PatGroup*)object;
}

bool
ObjectCache::findPatGroup(Id group_id, PatGroup& group)
{
    PatGroup* object = findPatGroup(group_id);
    if (object == NULL) return false;
    group = *object;
    return true;
}

PatWorksheet*
ObjectCache::findPatWorksheet(Id worksheet_id)
{
    DataObject* object = find(worksheet_id, true);
    if (object != NULL) return (PatWorksheet*)object;

    PatWorksheet worksheet;
    if (!_db->lookup(worksheet_id, worksheet))
	return NULL;

    object = new PatWorksheet(worksheet);
    assert(object != NULL);

    _objects.insert(worksheet_id, object);
    return (PatWorksheet*)object;
}

bool
ObjectCache::findPatWorksheet(Id worksheet_id, PatWorksheet& worksheet)
{
    PatWorksheet* object = findPatWorksheet(worksheet_id);
    if (object == NULL) return false;
    worksheet = *object;
    return true;
}

Payout*
ObjectCache::findPayout(Id payout_id)
{
    DataObject* object = find(payout_id, true);
    if (object != NULL) return (Payout*)object;

    Payout payout;
    if (!_db->lookup(payout_id, payout))
	return NULL;

    object = new Payout(payout);
    assert(object != NULL);

    _objects.insert(payout_id, object);
    return (Payout*)object;
}

bool
ObjectCache::findPayout(Id payout_id, Payout& payout)
{
    Payout* object = findPayout(payout_id);
    if (object == NULL) return false;
    payout = *object;
    return true;
}

Personal*
ObjectCache::findPersonal(Id personal_id)
{
    DataObject* object = find(personal_id, true);
    if (object != NULL) return (Personal*)object;

    Personal personal;
    if (!_db->lookup(personal_id, personal))
	return NULL;

    object = new Personal(personal);
    assert(object != NULL);

    _objects.insert(personal_id, object);
    return (Personal*)object;
}

bool
ObjectCache::findPersonal(Id personal_id, Personal& personal)
{
    Personal* object = findPersonal(personal_id);
    if (object == NULL) return false;
    personal = *object;
    return true;
}

PriceBatch*
ObjectCache::findPriceBatch(Id batch_id)
{
    DataObject* object = find(batch_id, true);
    if (object != NULL) return (PriceBatch*)object;

    PriceBatch batch;
    if (!_db->lookup(batch_id, batch))
	return NULL;

    object = new PriceBatch(batch);
    assert(object != NULL);

    _objects.insert(batch_id, object);
    return (PriceBatch*)object;
}

bool
ObjectCache::findPriceBatch(Id batch_id, PriceBatch& batch)
{
    PriceBatch* object = findPriceBatch(batch_id);
    if (object == NULL) return false;
    batch = *object;
    return true;
}

PromoBatch*
ObjectCache::findPromoBatch(Id batch_id)
{
    DataObject* object = find(batch_id, true);
    if (object != NULL) return (PromoBatch*)object;

    PromoBatch batch;
    if (!_db->lookup(batch_id, batch))
	return NULL;

    object = new PromoBatch(batch);
    assert(object != NULL);

    _objects.insert(batch_id, object);
    return (PromoBatch*)object;
}

bool
ObjectCache::findPromoBatch(Id batch_id, PromoBatch& batch)
{
    PromoBatch* object = findPromoBatch(batch_id);
    if (object == NULL) return false;
    batch = *object;
    return true;
}

Quote*
ObjectCache::findQuote(Id quote_id)
{
    DataObject* object = find(quote_id, true);
    if (object != NULL) return (Quote*)object;

    Quote quote;
    if (!_db->lookup(quote_id, quote))
	return NULL;

    object = new Quote(quote);
    assert(object != NULL);

    _objects.insert(quote_id, object);
    return (Quote*)object;
}

bool
ObjectCache::findQuote(Id quote_id, Quote& quote)
{
    Quote* object = findQuote(quote_id);
    if (object == NULL) return false;
    quote = *object;
    return true;
}

Receipt*
ObjectCache::findReceipt(Id receipt_id)
{
    DataObject* object = find(receipt_id, true);
    if (object != NULL) return (Receipt*)object;

    Receipt receipt;
    if (!_db->lookup(receipt_id, receipt))
	return NULL;

    object = new Receipt(receipt);
    assert(object != NULL);

    _objects.insert(receipt_id, object);
    return (Receipt*)object;
}

bool
ObjectCache::findReceipt(Id receipt_id, Receipt& receipt)
{
    Receipt* object = findReceipt(receipt_id);
    if (object == NULL) return false;
    receipt = *object;
    return true;
}

Receive*
ObjectCache::findReceive(Id receive_id)
{
    DataObject* object = find(receive_id, true);
    if (object != NULL) return (Receive*)object;

    Receive receive;
    if (!_db->lookup(receive_id, receive))
	return NULL;

    object = new Receive(receive);
    assert(object != NULL);

    _objects.insert(receive_id, object);
    return (Receive*)object;
}

bool
ObjectCache::findReceive(Id receive_id, Receive& receive)
{
    Receive* object = findReceive(receive_id);
    if (object == NULL) return false;
    receive = *object;
    return true;
}

Reconcile*
ObjectCache::findReconcile(Id reconcile_id)
{
    DataObject* object = find(reconcile_id, true);
    if (object != NULL) return (Reconcile*)object;

    Reconcile reconcile;
    if (!_db->lookup(reconcile_id, reconcile))
	return NULL;

    object = new Reconcile(reconcile);
    assert(object != NULL);

    _objects.insert(reconcile_id, object);
    return (Reconcile*)object;
}

bool
ObjectCache::findReconcile(Id reconcile_id, Reconcile& reconcile)
{
    Reconcile* object = findReconcile(reconcile_id);
    if (object == NULL) return false;
    reconcile = *object;
    return true;
}

Recurring*
ObjectCache::findRecurring(Id recurring_id)
{
    DataObject* object = find(recurring_id, true);
    if (object != NULL) return (Recurring*)object;

    Recurring recurring;
    if (!_db->lookup(recurring_id, recurring))
	return NULL;

    object = new Recurring(recurring);
    assert(object != NULL);

    _objects.insert(recurring_id, object);
    return (Recurring*)object;
}

bool
ObjectCache::findRecurring(Id recurring_id, Recurring& recurring)
{
    Recurring* object = findRecurring(recurring_id);
    if (object == NULL) return false;
    recurring = *object;
    return true;
}

SecurityType*
ObjectCache::findSecurityType(Id type_id)
{
    DataObject* object = find(type_id, true);
    if (object != NULL) return (SecurityType*)object;

    SecurityType type;
    if (!_db->lookup(type_id, type))
	return NULL;

    object = new SecurityType(type);
    assert(object != NULL);

    _objects.insert(type_id, object);
    return (SecurityType*)object;
}

bool
ObjectCache::findSecurityType(Id type_id, SecurityType& type)
{
    SecurityType* object = findSecurityType(type_id);
    if (object == NULL) return false;
    type = *object;
    return true;
}

Shift*
ObjectCache::findShift(Id shift_id)
{
    DataObject* object = find(shift_id, true);
    if (object != NULL) return (Shift*)object;

    Shift shift;
    if (!_db->lookup(shift_id, shift))
	return NULL;

    object = new Shift(shift);
    assert(object != NULL);

    _objects.insert(shift_id, object);
    return (Shift*)object;
}

bool
ObjectCache::findShift(Id shift_id, Shift& shift)
{
    Shift* object = findShift(shift_id);
    if (object == NULL) return false;
    shift = *object;
    return true;
}

Slip*
ObjectCache::findSlip(Id slip_id)
{
    DataObject* object = find(slip_id, true);
    if (object != NULL) return (Slip*)object;

    Slip slip;
    if (!_db->lookup(slip_id, slip))
	return NULL;

    object = new Slip(slip);
    assert(object != NULL);

    _objects.insert(slip_id, object);
    return (Slip*)object;
}

bool
ObjectCache::findSlip(Id slip_id, Slip& slip)
{
    Slip* object = findSlip(slip_id);
    if (object == NULL) return false;
    slip = *object;
    return true;
}

Station*
ObjectCache::findStation(Id station_id)
{
    DataObject* object = find(station_id, true);
    if (object != NULL) return (Station*)object;

    Station station;
    if (!_db->lookup(station_id, station))
	return NULL;

    object = new Station(station);
    assert(object != NULL);

    _objects.insert(station_id, object);
    return (Station*)object;
}

bool
ObjectCache::findStation(Id station_id, Station& station)
{
    Station* object = findStation(station_id);
    if (object == NULL) return false;
    station = *object;
    return true;
}

Store*
ObjectCache::findStore(Id store_id)
{
    DataObject* object = find(store_id, true);
    if (object != NULL) return (Store*)object;

    Store store;
    if (!_db->lookup(store_id, store))
	return NULL;

    object = new Store(store);
    assert(object != NULL);

    _objects.insert(store_id, object);
    return (Store*)object;
}

bool
ObjectCache::findStore(Id store_id, Store& store)
{
    Store* object = findStore(store_id);
    if (object == NULL) return false;
    store = *object;
    return true;
}

Subdept*
ObjectCache::findSubdept(Id subdept_id)
{
    DataObject* object = find(subdept_id, true);
    if (object != NULL) return (Subdept*)object;

    Subdept subdept;
    if (!_db->lookup(subdept_id, subdept))
	return NULL;

    object = new Subdept(subdept);
    assert(object != NULL);

    _objects.insert(subdept_id, object);
    return (Subdept*)object;
}

bool
ObjectCache::findSubdept(Id subdept_id, Subdept& subdept)
{
    Subdept* object = findSubdept(subdept_id);
    if (object == NULL) return false;
    subdept = *object;
    return true;
}

Tax*
ObjectCache::findTax(Id tax_id)
{
    DataObject* object = find(tax_id, true);
    if (object != NULL) return (Tax*)object;

    Tax tax;
    if (!_db->lookup(tax_id, tax))
	return NULL;

    object = new Tax(tax);
    assert(object != NULL);

    _objects.insert(tax_id, object);
    return (Tax*)object;
}

bool
ObjectCache::findTax(Id tax_id, Tax& tax)
{
    Tax* object = findTax(tax_id);
    if (object == NULL) return false;
    tax = *object;
    return true;
}

Tender*
ObjectCache::findTender(Id tender_id)
{
    DataObject* object = find(tender_id, true);
    if (object != NULL) return (Tender*)object;

    Tender tender;
    if (!_db->lookup(tender_id, tender))
	return NULL;

    object = new Tender(tender);
    assert(object != NULL);

    _objects.insert(tender_id, object);
    return (Tender*)object;
}

bool
ObjectCache::findTender(Id tender_id, Tender& tender)
{
    Tender* object = findTender(tender_id);
    if (object == NULL) return false;
    tender = *object;
    return true;
}

TenderAdjust*
ObjectCache::findTenderAdjust(Id adjust_id)
{
    DataObject* object = find(adjust_id, true);
    if (object != NULL) return (TenderAdjust*)object;

    TenderAdjust adjust;
    if (!_db->lookup(adjust_id, adjust))
	return NULL;

    object = new TenderAdjust(adjust);
    assert(object != NULL);

    _objects.insert(adjust_id, object);
    return (TenderAdjust*)object;
}

bool
ObjectCache::findTenderAdjust(Id adjust_id, TenderAdjust& adjust)
{
    TenderAdjust* object = findTenderAdjust(adjust_id);
    if (object == NULL) return false;
    adjust = *object;
    return true;
}

TenderCount*
ObjectCache::findTenderCount(Id count_id)
{
    DataObject* object = find(count_id, true);
    if (object != NULL) return (TenderCount*)object;

    TenderCount count;
    if (!_db->lookup(count_id, count))
	return NULL;

    object = new TenderCount(count);
    assert(object != NULL);

    _objects.insert(count_id, object);
    return (TenderCount*)object;
}

bool
ObjectCache::findTenderCount(Id count_id, TenderCount& count)
{
    TenderCount* object = findTenderCount(count_id);
    if (object == NULL) return false;
    count = *object;
    return true;
}

Term*
ObjectCache::findTerm(Id term_id)
{
    DataObject* object = find(term_id, true);
    if (object != NULL) return (Term*)object;

    Term term;
    if (!_db->lookup(term_id, term))
	return NULL;

    object = new Term(term);
    assert(object != NULL);

    _objects.insert(term_id, object);
    return (Term*)object;
}

bool
ObjectCache::findTerm(Id term_id, Term& term)
{
    Term* object = findTerm(term_id);
    if (object == NULL) return false;
    term = *object;
    return true;
}

Todo*
ObjectCache::findTodo(Id todo_id)
{
    DataObject* object = find(todo_id, true);
    if (object != NULL) return (Todo*)object;

    Todo todo;
    if (!_db->lookup(todo_id, todo))
	return NULL;

    object = new Todo(todo);
    assert(object != NULL);

    _objects.insert(todo_id, object);
    return (Todo*)object;
}

bool
ObjectCache::findTodo(Id todo_id, Todo& todo)
{
    Todo* object = findTodo(todo_id);
    if (object == NULL) return false;
    todo = *object;
    return true;
}

User*
ObjectCache::findUser(Id user_id)
{
    DataObject* object = find(user_id, true);
    if (object != NULL) return (User*)object;

    User user;
    if (!_db->lookup(user_id, user))
	return NULL;

    object = new User(user);
    assert(object != NULL);

    _objects.insert(user_id, object);
    return (User*)object;
}

bool
ObjectCache::findUser(Id user_id, User& user)
{
    User* object = findUser(user_id);
    if (object == NULL) return false;
    user = *object;
    return true;
}

Vendor*
ObjectCache::findVendor(Id vendor_id)
{
    DataObject* object = find(vendor_id, true);
    if (object != NULL) return (Vendor*)object;

    Vendor vendor;
    if (!_db->lookup(vendor_id, vendor))
	return NULL;

    object = new Vendor(vendor);
    assert(object != NULL);

    _objects.insert(vendor_id, object);
    return (Vendor*)object;
}

bool
ObjectCache::findVendor(Id vendor_id, Vendor& vendor)
{
    Vendor* object = findVendor(vendor_id);
    if (object == NULL) return false;
    vendor = *object;
    return true;
}

Withdraw*
ObjectCache::findWithdraw(Id withdraw_id)
{
    DataObject* object = find(withdraw_id, true);
    if (object != NULL) return (Withdraw*)object;

    Withdraw withdraw;
    if (!_db->lookup(withdraw_id, withdraw))
	return NULL;

    object = new Withdraw(withdraw);
    assert(object != NULL);

    _objects.insert(withdraw_id, object);
    return (Withdraw*)object;
}

bool
ObjectCache::findWithdraw(Id withdraw_id, Withdraw& withdraw)
{
    Withdraw* object = findWithdraw(withdraw_id);
    if (object == NULL) return false;
    withdraw = *object;
    return true;
}
