// $Id: object_cache.h,v 1.14 2005/04/05 07:11:29 bpepers Exp $
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

#ifndef OBJECT_CACHE_H
#define OBJECT_CACHE_H

#include "data_object.h"
#include <qmap.h>

class QuasarDB;
class Account;
class AdjustReason;
class Card;
class CardAdjust;
class Charge;
class Cheque;
class Company;
class Count;
class Customer;
class CustomerType;
class Dept;
class Discount;
class Employee;
class Expense;
class Extra;
class General;
class Gltx;
class Group;
class Invoice;
class Item;
class ItemAdjust;
class ItemPrice;
class LabelBatch;
class Location;
class Nosale;
class Order;
class OrderTemplate;
class PatGroup;
class PatWorksheet;
class Payout;
class Personal;
class PriceBatch;
class PromoBatch;
class Quote;
class Receipt;
class Receive;
class Reconcile;
class Recurring;
class SecurityType;
class Shift;
class Slip;
class Station;
class Store;
class Subdept;
class Tax;
class Tender;
class TenderAdjust;
class TenderCount;
class Term;
class Todo;
class User;
class Vendor;
class Withdraw;

typedef QMap<Id,DataObject*> ObjectMap;

class ObjectCache {
public:
    // Constructors and Destructor
    ObjectCache(QuasarDB* db);
    virtual ~ObjectCache();

    // Generic methods
    DataObject* find(Id object_id, bool cacheOnly=false);
    void remove(Id object_id);
    void clear();

    // Grouped type methods
    Card* findCard(Id card_id);
    Gltx* findGltx(Id gltx_id);

    // General data methods
    Account* findAccount(Id account_id);
    AdjustReason* findAdjustReason(Id reason_id);
    Charge* findCharge(Id charge_id);
    Company* findCompany(Id company_id);
    Count* findCount(Id count_id);
    Customer* findCustomer(Id customer_id);
    CustomerType* findCustomerType(Id type_id);
    Dept* findDept(Id dept_id);
    Discount* findDiscount(Id discount_id);
    Employee* findEmployee(Id employee_id);
    Expense* findExpense(Id expense_id);
    Extra* findExtra(Id extra_id);
    Group* findGroup(Id group_id);
    Item* findItem(Id item_id);
    ItemPrice* findItemPrice(Id price_id);
    LabelBatch* findLabelBatch(Id batch_id);
    Location* findLocation(Id location_id);
    Order* findOrder(Id order_id);
    OrderTemplate* findOrderTemplate(Id template_id);
    PatGroup* findPatGroup(Id group_id);
    PatWorksheet* findPatWorksheet(Id worksheet_id);
    Personal* findPersonal(Id personal_id);
    PriceBatch* findPriceBatch(Id batch_id);
    PromoBatch* findPromoBatch(Id batch_id);
    Quote* findQuote(Id quote_id);
    Reconcile* findReconcile(Id reconcile_id);
    Recurring* findRecurring(Id recurring_id);
    SecurityType* findSecurityType(Id type_id);
    Slip* findSlip(Id slip_id);
    Station* findStation(Id station_id);
    Store* findStore(Id store_id);
    Subdept* findSubdept(Id subdept_id);
    Tax* findTax(Id tax_id);
    Tender* findTender(Id tender_id);
    TenderCount* findTenderCount(Id count_id);
    Term* findTerm(Id term_id);
    Todo* findTodo(Id todo_id);
    User* findUser(Id user_id);
    Vendor* findVendor(Id vendor_id);

    // Transaction methods
    CardAdjust* findCardAdjust(Id adjust_id);
    Cheque* findCheque(Id cheque_id);
    General* findGeneral(Id general_id);
    Invoice* findInvoice(Id invoice_id);
    ItemAdjust* findItemAdjust(Id adjust_id);
    Nosale* findNosale(Id nosale_id);
    Payout* findPayout(Id payout_id);
    Receipt* findReceipt(Id receipt_id);
    Receive* findReceive(Id receive_id);
    Shift* findShift(Id shift_id);
    TenderAdjust* findTenderAdjust(Id adjust_id);
    Withdraw* findWithdraw(Id withdraw_id);

    // General data methods (by reference)
    bool findAccount(Id account_id, Account& account);
    bool findAdjustReason(Id reason_id, AdjustReason& reason);
    bool findCharge(Id charge_id, Charge& charge);
    bool findCompany(Id company_id, Company& company);
    bool findCount(Id count_id, Count& count);
    bool findCustomer(Id customer_id, Customer& customer);
    bool findCustomerType(Id type_id, CustomerType& type);
    bool findDept(Id dept_id, Dept& dept);
    bool findDiscount(Id discount_id, Discount& discount);
    bool findEmployee(Id employee_id, Employee& employee);
    bool findExpense(Id expense_id, Expense& expense);
    bool findExtra(Id extra_id, Extra& extra);
    bool findGroup(Id group_id, Group& group);
    bool findItem(Id item_id, Item& item);
    bool findItemPrice(Id price_id, ItemPrice& price);
    bool findLabelBatch(Id batch_id, LabelBatch& batch);
    bool findLocation(Id location_id, Location& location);
    bool findOrder(Id order_id, Order& order);
    bool findOrderTemplate(Id template_id, OrderTemplate& templ);
    bool findPatGroup(Id group_id, PatGroup& group);
    bool findPatWorksheet(Id worksheet_id, PatWorksheet& worksheet);
    bool findPersonal(Id personal_id, Personal& personal);
    bool findPriceBatch(Id batch_id, PriceBatch& batch);
    bool findPromoBatch(Id batch_id, PromoBatch& batch);
    bool findQuote(Id quote_id, Quote& quote);
    bool findReconcile(Id reconcile_id, Reconcile& reconcile);
    bool findRecurring(Id recurring_id, Recurring& recurring);
    bool findSecurityType(Id type_id, SecurityType& type);
    bool findSlip(Id slip_id, Slip& slip);
    bool findStation(Id station_id, Station& station);
    bool findStore(Id store_id, Store& store);
    bool findSubdept(Id subdept_id, Subdept& subdept);
    bool findTax(Id tax_id, Tax& tax);
    bool findTender(Id tender_id, Tender& tender);
    bool findTenderCount(Id count_id, TenderCount& count);
    bool findTerm(Id term_id, Term& term);
    bool findTodo(Id todo_id, Todo& todo);
    bool findUser(Id user_id, User& user);
    bool findVendor(Id vendor_id, Vendor& vendor);

    // Transaction methods (by reference)
    bool findCardAdjust(Id adjust_id, CardAdjust& adjust);
    bool findCheque(Id cheque_id, Cheque& cheque);
    bool findGeneral(Id general_id, General& general);
    bool findInvoice(Id invoice_id, Invoice& invoice);
    bool findItemAdjust(Id adjust_id, ItemAdjust& adjust);
    bool findNosale(Id nosale_id, Nosale& nosale);
    bool findPayout(Id payout_id, Payout& payout);
    bool findReceipt(Id receipt_id, Receipt& receipt);
    bool findReceive(Id receive_id, Receive& receive);
    bool findShift(Id shift_id, Shift& shift);
    bool findTenderAdjust(Id adjust_id, TenderAdjust& adjust);
    bool findWithdraw(Id withdraw_id, Withdraw& withdraw);

protected:
    QuasarDB* _db;
    ObjectMap _objects;
};

#endif // OBJECT_CACHE_H
