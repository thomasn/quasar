// $Id: quasar_db.h,v 1.115 2005/06/11 02:52:20 bpepers Exp $
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

#ifndef QUASAR_DB_H
#define QUASAR_DB_H

#include "quasar_include.h"
#include "stmt.h"
#include "sql_stmt.h"
#include "data_event.h"
#include "date_time.h"
#include "fixed.h"
#include "item_price.h"

#include <qobject.h>
#include <qvaluestack.h>

class Sequence;
class Account;
class AdjustReason;
class CardAdjust;
class Charge;
class Cheque;
class Count;
class Company;
class Customer;
class CustomerType;
class Dept;
class Discount;
class General;
class Group;
class Employee;
class Expense;
class Extra;
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
class Plu;
class PosTx;
class PosWork;
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
struct Select;
struct AccountSelect;
struct AdjustReasonSelect;
struct CardAdjustSelect;
struct ChargeSelect;
struct ChequeSelect;
struct CompanySelect;
struct CountSelect;
struct CustomerSelect;
struct CustomerTypeSelect;
struct DeptSelect;
struct DiscountSelect;
struct EmployeeSelect;
struct ExpenseSelect;
struct ExtraSelect;
struct GeneralSelect;
struct GroupSelect;
struct InvoiceSelect;
struct ItemSelect;
struct ItemAdjustSelect;
struct ItemPriceSelect;
struct LabelBatchSelect;
struct LocationSelect;
struct NosaleSelect;
struct OrderSelect;
struct OrderTemplateSelect;
struct PatGroupSelect;
struct PatWorksheetSelect;
struct PayoutSelect;
struct PersonalSelect;
struct PluSelect;
struct PosTxSelect;
struct PosWorkSelect;
struct PriceBatchSelect;
struct PromoBatchSelect;
struct QuoteSelect;
struct ReceiptSelect;
struct ReceiveSelect;
struct ReconcileSelect;
struct RecurringSelect;
struct SecurityTypeSelect;
struct ShiftSelect;
struct SlipSelect;
struct StationSelect;
struct StoreSelect;
struct SubdeptSelect;
struct TaxSelect;
struct TenderSelect;
struct TenderAdjustSelect;
struct TenderCountSelect;
struct TermSelect;
struct TodoSelect;
struct UserSelect;
struct VendorSelect;
struct WithdrawSelect;

class QuasarClient;
class ObjectCache;
class Card;
class Gltx;
struct CardSelect;
struct GltxSelect;

struct TenderLine;
struct ExpenseLine;
struct InvoiceItem;
struct InvoiceDisc;

typedef QPair<Id,QString> DataInfo;

class QuasarDB: public QObject {
    Q_OBJECT
public:
    QuasarDB(Connection* connection, QObject* parent=NULL);
    virtual ~QuasarDB();

    void setClient(QuasarClient* client) { _client = client; }

    // Version
    QString version() const;
    int versionMajor() const;
    int versionMinor() const;
    int versionPatch() const;

    // Current connection info
    QString username() const { return _username; }
    int systemId() const { return _system_id; }
    Id defaultStore(bool selling=false);
    void setUsername(const QString& name);
    void setSystemId(int system_id);
    void setDefaultStore(Id store_id);

    // Unique numbers
    fixed uniqueNumber(const QString& table, const QString& attr);
    fixed uniqueTxNumber(DataObject::DataType type);
    bool getSequence(Sequence& sequence);
    bool setSequence(const Sequence& sequence);

    // Return a unique object id
    Id uniqueId();

    // Set/get database level configuration data
    bool setConfig(const QString& key, const QString& value);
    QString getConfig(const QString& key);

    // Account methods
    bool create(Account& account);
    bool remove(const Account& account);
    bool update(const Account& orig, Account& account);
    bool lookup(Id account_id, Account& account);
    bool count(int& count, const AccountSelect& conditions);
    bool select(vector<Account>& accounts, const AccountSelect& conditions);
    bool validate(const Account& account);
    bool lookup(const QString& number, Account& account);

    // AdjustReason methods
    bool create(AdjustReason& reason);
    bool remove(const AdjustReason& reason);
    bool update(const AdjustReason& orig, AdjustReason& reason);
    bool lookup(Id reason_id, AdjustReason& reason);
    bool select(vector<AdjustReason>& reasons, const AdjustReasonSelect& cond);
    bool validate(const AdjustReason& reason);
    bool lookup(const QString& number, AdjustReason& reason);

    // CardAdjust methods
    bool create(CardAdjust& adjust);
    bool remove(const CardAdjust& adjust);
    bool update(const CardAdjust& orig, CardAdjust& adjust);
    bool lookup(Id adjust_id, CardAdjust& adjust);
    bool select(vector<CardAdjust>& adjusts, const CardAdjustSelect& select);
    bool validate(const CardAdjust& adjust);

    // Charge methods
    bool create(Charge& charge);
    bool remove(const Charge& charge);
    bool update(const Charge& orig, Charge& charge);
    bool lookup(Id charge_id, Charge& charge);
    bool select(vector<Charge>& charges, const ChargeSelect& conditions);
    bool validate(const Charge& charge);

    // Cheque methods
    bool create(Cheque& cheque);
    bool remove(const Cheque& cheque);
    bool update(const Cheque& orig, Cheque& cheque);
    bool lookup(Id cheque_id, Cheque& cheque);
    bool select(vector<Cheque>& cheques, const ChequeSelect& conditions);
    bool validate(const Cheque& cheque);

    // Company methods
    bool create(Company& company);
    bool remove(const Company& company);
    bool update(const Company& orig, Company& company);
    bool lookup(Id company_id, Company& company);
    bool select(vector<Company>& companies, const CompanySelect& conditions);
    bool validate(const Company& company);
    bool lookup(Company& company);
    bool lookup(const QString& number, Company& company);

    // Count methods
    bool create(Count& count);
    bool remove(const Count& count);
    bool update(const Count& orig, Count& count);
    bool lookup(Id count_id, Count& count);
    bool select(vector<Count>& counts, const CountSelect& conditions);
    bool validate(const Count& count);

    // Customer methods
    bool create(Customer& customer);
    bool remove(const Customer& customer);
    bool update(const Customer& orig, Customer& customer);
    bool lookup(Id customer_id, Customer& customer);
    bool select(vector<Customer>& customers, const CustomerSelect& conditions);
    bool validate(const Customer& customer);
    bool lookup(const QString& id, Customer& customer);

    // CustomerType methods
    bool create(CustomerType& type);
    bool remove(const CustomerType& type);
    bool update(const CustomerType& orig, CustomerType& type);
    bool lookup(Id type_id, CustomerType& type);
    bool select(vector<CustomerType>& types, const CustomerTypeSelect& conds);
    bool validate(const CustomerType& type);

    // Dept methods
    bool create(Dept& dept);
    bool remove(const Dept& dept);
    bool update(const Dept& orig, Dept& dept);
    bool lookup(Id dept_id, Dept& dept);
    bool select(vector<Dept>& depts, const DeptSelect& conditions);
    bool validate(const Dept& dept);
    bool lookup(const QString& number, Dept& dept);

    // Discount methods
    bool create(Discount& discount);
    bool remove(const Discount& discount);
    bool update(const Discount& orig, Discount& discount);
    bool lookup(Id discount_id, Discount& discount);
    bool select(vector<Discount>& discounts, const DiscountSelect& conditions);
    bool validate(const Discount& discount);

    // Employee methods
    bool create(Employee& employee);
    bool remove(const Employee& employee);
    bool update(const Employee& orig, Employee& employee);
    bool lookup(Id employee_id, Employee& employee);
    bool select(vector<Employee>& employees, const EmployeeSelect& conditions);
    bool validate(const Employee& employee);
    bool lookup(const QString& id, Employee& employee);

    // Expense methods
    bool create(Expense& expense);
    bool remove(const Expense& expense);
    bool update(const Expense& orig, Expense& expense);
    bool lookup(Id expense_id, Expense& expense);
    bool select(vector<Expense>& expenses, const ExpenseSelect& conditions);
    bool validate(const Expense& expense);
    bool lookup(const QString& number, Expense& expense);

    // Extra methods
    bool create(Extra& extra);
    bool remove(const Extra& extra);
    bool update(const Extra& orig, Extra& extra);
    bool lookup(Id extra_id, Extra& extra);
    bool select(vector<Extra>& extras, const ExtraSelect& conditions);
    bool validate(const Extra& extra);

    // General methods
    bool create(General& general);
    bool remove(const General& general);
    bool update(const General& orig, General& general);
    bool lookup(Id general_id, General& general);
    bool select(vector<General>& generals, const GeneralSelect& conditions);
    bool validate(const General& general);

    // Group methods
    bool create(Group& group);
    bool remove(const Group& group);
    bool update(const Group& orig, Group& group);
    bool lookup(Id group_id, Group& group);
    bool select(vector<Group>& groups, const GroupSelect& conditions);
    bool validate(const Group& group);
    bool lookup(const QString& name, Group& group);

    // Invoice methods
    bool create(Invoice& invoice);
    bool remove(const Invoice& invoice);
    bool update(const Invoice& orig, Invoice& invoice);
    bool lookup(Id invoice_id, Invoice& invoice);
    bool count(int& count, const InvoiceSelect& conditions);
    bool select(vector<Invoice>& invoices, const InvoiceSelect& conditions);
    bool validate(const Invoice& invoice);

    // Item methods
    bool create(Item& item);
    bool remove(const Item& item);
    bool update(const Item& orig, Item& item);
    bool lookup(Id item_id, Item& item);
    bool count(int& count, const ItemSelect& conditions);
    bool select(vector<Item>& items, const ItemSelect& conditions);
    bool validate(const Item& item);
    bool lookup(ItemSelect& conditions, const QString& number,
		vector<Item>& items);
    bool select(vector<Id>& item_ids, const ItemSelect& conditions);
    bool lookup(ItemSelect& conditions, const QString& number,
		vector<Id>& item_ids);

    // ItemAdjust methods
    bool create(ItemAdjust& adjustment);
    bool remove(const ItemAdjust& adjustment);
    bool update(const ItemAdjust& orig, ItemAdjust& adjustment);
    bool lookup(Id adjustment_id, ItemAdjust& adjustment);
    bool select(vector<ItemAdjust>& adjustments, const ItemAdjustSelect& cond);
    bool validate(const ItemAdjust& adjustment);

    // ItemPrice methods
    bool create(ItemPrice& price);
    bool remove(const ItemPrice& price);
    bool update(const ItemPrice& orig, ItemPrice& price);
    bool lookup(Id price_id, ItemPrice& price);
    bool select(vector<ItemPrice>& prices, const ItemPriceSelect& conditions);
    bool validate(const ItemPrice& price);

    // LabelBatch methods
    bool create(LabelBatch& batch);
    bool remove(const LabelBatch& batch);
    bool update(const LabelBatch& orig, LabelBatch& batch);
    bool lookup(Id batch_id, LabelBatch& batch);
    bool select(vector<LabelBatch>& batches, const LabelBatchSelect& cond);
    bool validate(const LabelBatch& batch);

    // Location methods
    bool create(Location& location);
    bool remove(const Location& location);
    bool update(const Location& orig, Location& location);
    bool lookup(Id location_id, Location& location);
    bool select(vector<Location>& locations, const LocationSelect& conditions);
    bool validate(const Location& location);

    // Nosale methods
    bool create(Nosale& nosale);
    bool remove(const Nosale& nosale);
    bool update(const Nosale& orig, Nosale& nosale);
    bool lookup(Id nosale_id, Nosale& nosale);
    bool select(vector<Nosale>& nosales, const NosaleSelect& conditions);
    bool validate(const Nosale& nosale);

    // Order methods
    bool create(Order& order);
    bool remove(const Order& order);
    bool update(const Order& orig, Order& order);
    bool lookup(Id order_id, Order& order);
    bool select(vector<Order>& orders, const OrderSelect& conditions);
    bool validate(const Order& order);
    bool lookup(const QString& number, Order& order);

    // OrderTemplate methods
    bool create(OrderTemplate& temp);
    bool remove(const OrderTemplate& temp);
    bool update(const OrderTemplate& orig, OrderTemplate& temp);
    bool lookup(Id template_id, OrderTemplate& temp);
    bool select(vector<OrderTemplate>& temps, const OrderTemplateSelect& cond);
    bool validate(const OrderTemplate& temp);

    // PatGroup methods
    bool create(PatGroup& group);
    bool remove(const PatGroup& group);
    bool update(const PatGroup& orig, PatGroup& group);
    bool lookup(Id group_id, PatGroup& group);
    bool select(vector<PatGroup>& groups, const PatGroupSelect& conditions);
    bool validate(const PatGroup& group);

    // PatWorksheet methods
    bool create(PatWorksheet& sheet);
    bool remove(const PatWorksheet& sheet);
    bool update(const PatWorksheet& orig, PatWorksheet& sheet);
    bool lookup(Id sheet_id, PatWorksheet& sheet);
    bool select(vector<PatWorksheet>& sheets, const PatWorksheetSelect& cond);
    bool validate(const PatWorksheet& sheet);

    // Payout methods
    bool create(Payout& payout);
    bool remove(const Payout& payout);
    bool update(const Payout& orig, Payout& payout);
    bool lookup(Id payout_id, Payout& payout);
    bool select(vector<Payout>& payouts, const PayoutSelect& conditions);
    bool validate(const Payout& payout);

    // Personal methods
    bool create(Personal& personal);
    bool remove(const Personal& personal);
    bool update(const Personal& orig, Personal& personal);
    bool lookup(Id personal_id, Personal& personal);
    bool select(vector<Personal>& personals, const PersonalSelect& conditions);
    bool validate(const Personal& personal);

    // Plu methods
    bool create(Plu& plu);
    bool remove(const Plu& plu);
    bool update(const Plu& orig, Plu& plu);
    bool lookup(Id plu_id, Plu& plu);
    bool select(vector<Plu>& plus, const PluSelect& conditions);
    bool validate(const Plu& plu);
    bool lookup(Id plu_id, Plu& plu, Item& item);
    bool lookup(const QString& number, Id store_id, Plu& plu);
    bool lookup(Id item_id, const QString& size, Plu& plu);

    // PriceBatch methods
    bool create(PriceBatch& batch);
    bool remove(const PriceBatch& batch);
    bool update(const PriceBatch& orig, PriceBatch& batch);
    bool lookup(Id batch_id, PriceBatch& batch);
    bool select(vector<PriceBatch>& batches, const PriceBatchSelect& cond);
    bool validate(const PriceBatch& batch);

    // PromoBatch methods
    bool create(PromoBatch& batch);
    bool remove(const PromoBatch& batch);
    bool update(const PromoBatch& orig, PromoBatch& batch);
    bool lookup(Id batch_id, PromoBatch& batch);
    bool select(vector<PromoBatch>& batches, const PromoBatchSelect& cond);
    bool validate(const PromoBatch& batch);

    // Quote methods
    bool create(Quote& quote);
    bool remove(const Quote& quote);
    bool update(const Quote& orig, Quote& quote);
    bool lookup(Id quote_id, Quote& quote);
    bool select(vector<Quote>& quotes, const QuoteSelect& conditions);
    bool validate(const Quote& quote);

    // Receipt methods
    bool create(Receipt& receipt);
    bool remove(const Receipt& receipt);
    bool update(const Receipt& orig, Receipt& receipt);
    bool lookup(Id receipt_id, Receipt& receipt);
    bool select(vector<Receipt>& receipts, const ReceiptSelect& conditions);
    bool validate(const Receipt& receipt);

    // Receive methods
    bool create(Receive& receive);
    bool remove(const Receive& receive);
    bool update(const Receive& orig, Receive& receive);
    bool lookup(Id receive_id, Receive& receive);
    bool select(vector<Receive>& receives, const ReceiveSelect& conditions);
    bool validate(const Receive& receive);

    // Reconcile methods
    bool create(Reconcile& reconcile);
    bool remove(const Reconcile& reconcile);
    bool update(const Reconcile& orig, Reconcile& reconcile);
    bool lookup(Id reconcile_id, Reconcile& reconcile);
    bool select(vector<Reconcile>& reconciles, const ReconcileSelect& cond);
    bool validate(const Reconcile& reconcile);

    // Recurring methods
    bool create(Recurring& recurring);
    bool remove(const Recurring& recurring);
    bool update(const Recurring& orig, Recurring& recurring);
    bool lookup(Id recurring_id, Recurring& recurring);
    bool select(vector<Recurring>& recurrings, const RecurringSelect& conds);
    bool validate(const Recurring& recurring);

    // SecurityType methods
    bool create(SecurityType& type);
    bool remove(const SecurityType& type);
    bool update(const SecurityType& orig, SecurityType& type);
    bool lookup(Id type_id, SecurityType& type);
    bool select(vector<SecurityType>& types, const SecurityTypeSelect& conds);
    bool validate(const SecurityType& type);
    bool lookup(const QString& name, SecurityType& type);

    // Shift methods
    bool create(Shift& shift);
    bool remove(const Shift& shift);
    bool update(const Shift& orig, Shift& shift);
    bool lookup(Id shift_id, Shift& shift);
    bool select(vector<Shift>& shifts, const ShiftSelect& conditions);
    bool validate(const Shift& shift);

    // Slip methods
    bool create(Slip& slip);
    bool remove(const Slip& slip);
    bool update(const Slip& orig, Slip& slip);
    bool lookup(Id slip_id, Slip& slip);
    bool select(vector<Slip>& slips, const SlipSelect& conditions);
    bool validate(const Slip& slip);

    // Station methods
    bool create(Station& station);
    bool remove(const Station& station);
    bool update(const Station& orig, Station& station);
    bool lookup(Id station_id, Station& station);
    bool lookup(const QString& number, Station& station);
    bool select(vector<Station>& stations, const StationSelect& conditions);
    bool validate(const Station& station);

    // Store methods
    bool create(Store& store);
    bool remove(const Store& store);
    bool update(const Store& orig, Store& store);
    bool lookup(Id store_id, Store& store);
    bool lookup(const QString& number, Store& store);
    bool select(vector<Store>& stores, const StoreSelect& conditions);
    bool validate(const Store& store);

    // Subdept methods
    bool create(Subdept& subdept);
    bool remove(const Subdept& subdept);
    bool update(const Subdept& orig, Subdept& subdept);
    bool lookup(Id subdept_id, Subdept& subdept);
    bool select(vector<Subdept>& subdepts, const SubdeptSelect& conditions);
    bool validate(const Subdept& subdept);
    bool lookup(const QString& number, Subdept& subdept);

    // Tax methods
    bool create(Tax& tax);
    bool remove(const Tax& tax);
    bool update(const Tax& orig, Tax& tax);
    bool lookup(Id tax_id, Tax& tax);
    bool select(vector<Tax>& taxes, const TaxSelect& conditions);
    bool validate(const Tax& tax);
    bool lookup(const QString& number, Tax& tax);

    // Tender methods
    bool create(Tender& tender);
    bool remove(const Tender& tender);
    bool update(const Tender& orig, Tender& tender);
    bool lookup(Id tender_id, Tender& tender);
    bool select(vector<Tender>& tenders, const TenderSelect& conditions);
    bool validate(const Tender& tender);
    bool lookup(const QString& name, Tender& tender);

    // TenderAdjust methods
    bool create(TenderAdjust& adjust);
    bool remove(const TenderAdjust& adjust);
    bool update(const TenderAdjust& orig, TenderAdjust& adjust);
    bool lookup(Id adjust_id, TenderAdjust& adjust);
    bool select(vector<TenderAdjust>& adjusts, const TenderAdjustSelect& c);
    bool validate(const TenderAdjust& adjust);

    // TenderCount methods
    bool create(TenderCount& count);
    bool remove(const TenderCount& count);
    bool update(const TenderCount& orig, TenderCount& count);
    bool lookup(Id count_id, TenderCount& count);
    bool select(vector<TenderCount>& counts, const TenderCountSelect& c);
    bool validate(const TenderCount& count);

    // Term methods
    bool create(Term& term);
    bool remove(const Term& term);
    bool update(const Term& orig, Term& term);
    bool lookup(Id term_id, Term& term);
    bool select(vector<Term>& terms, const TermSelect& conditions);
    bool validate(const Term& term);

    // Todo methods
    bool create(Todo& todo);
    bool remove(const Todo& todo);
    bool update(const Todo& orig, Todo& todo);
    bool lookup(Id todo_id, Todo& todo);
    bool select(vector<Todo>& todos, const TodoSelect& conditions);
    bool validate(const Todo& todo);

    // User methods
    bool create(User& user);
    bool remove(const User& user);
    bool update(const User& orig, User& user);
    bool lookup(Id user_id, User& user);
    bool lookup(const QString& name, User& user);
    bool select(vector<User>& users, const UserSelect& conditions);
    bool validate(const User& user);
    bool users(vector<QString>& users);

    // Vendor methods
    bool create(Vendor& vendor);
    bool remove(const Vendor& vendor);
    bool update(const Vendor& orig, Vendor& vendor);
    bool lookup(Id vendor_id, Vendor& vendor);
    bool select(vector<Vendor>& vendors, const VendorSelect& conditions);
    bool validate(const Vendor& vendor);
    bool lookup(const QString& number, Vendor& vendor);

    // Withdraw methods
    bool create(Withdraw& withdraw);
    bool remove(const Withdraw& withdraw);
    bool update(const Withdraw& orig, Withdraw& withdraw);
    bool lookup(Id withdraw_id, Withdraw& withdraw);
    bool select(vector<Withdraw>& withdraws, const WithdrawSelect& conditions);
    bool validate(const Withdraw& withdraw);

    // Card methods
    bool lookup(Id card_id, Card& card);
    bool count(int& count, const CardSelect& conditions);
    bool select(vector<Card>& cards, const CardSelect& conditions);

    // Gltx methods
    bool lookup(Id gltx_id, Gltx& gltx);
    bool count(int& count, const GltxSelect& conditions);
    bool select(vector<Gltx>& gltxs, const GltxSelect& conditions);

    // Special pos_tx methods
    bool create(const PosTx& tx);
    bool remove(const PosTx& tx);
    bool update(const PosTx& tx);
    bool lookup(const QString& type, const QString& id, PosTx& tx);
    bool select(vector<PosTx>& txs, const PosTxSelect& conditions);

    // Special pos_work methods
    bool create(const PosWork& work);
    bool remove(const PosWork& work);
    bool lookup(const QString& type, const QString& id, PosWork& work);
    bool select(vector<PosWork>& works, const PosWorkSelect& conditions);
    bool setStatus(PosWork& work, const QString& status);

    // Prepare transactions for posting
    bool prepare(Receive& receive, ObjectCache& cache);
    bool prepare(Invoice& invoice, ObjectCache& cache);

    // Special account methods
    fixed accountBalance(Id account_id, QDate date=QDate(),
			 Id store_id=INVALID_ID);
    void accountBalances(QDate date, vector<Id>& account_ids,
			 vector<fixed>& balances);
    fixed netProfitLoss(QDate date, Id store_id=INVALID_ID);
    bool selectChart(vector<Account>& accounts, const AccountSelect& conds);

    // Reconcilation
    void selectReconcile(Id account_id, QDate stmt_date, QDate end_date,
			 fixed& open_balance, vector<Id>& gltx_ids,
			 vector<int>& seq_nums);

    // Special card methods
    fixed cardBalance(Id card_id, QDate date=QDate(),
		      Id store_id=INVALID_ID);
    void cardBalances(QDate date, Id store_id, vector<Id>& card_ids,
		      vector<fixed>& balances);

    // Special item methods
    void itemBalances(QDate date, ItemSelect& conditions, vector<Id>& item_ids,
		      vector<fixed>& on_hands, vector<fixed>& total_costs);
    void itemSold(Id item_id, const QString& size, Id store_id, QDate start,
		  QDate end, fixed& qty, fixed& cost, fixed& price);
    void itemLastRecv(Id item_id, const QString& size, Id store_id, QDate date,
		      fixed& recvQty, fixed& recvCost);

    // Totals on item_change table.  The first three arguments are the
    // selection criteria (item, size, store).  Any of the selection
    // criteria can be blank which just means to return all values for
    // that criteria.  The itemTotals routine is the mother routine and
    // all the other methods just call it and are provided simply to make
    // calling easier.  First you have a set of routines that only return
    // information on certain areas (general, sold, received, and adjusted)
    // and then you have routines that are by area and also for just one
    // item_id.
    void itemTotals(Id item_id, const QString& size, Id store_id, QDate date,
		    vector<Id>& item_ids, vector<QString>& sizes,
		    vector<Id>& stores, vector<fixed>& on_hands,
		    vector<fixed>& total_costs, vector<fixed>& on_orders,
		    vector<fixed>& sold_qtys, vector<fixed>& sold_costs,
		    vector<fixed>& sold_prices, vector<fixed>& recv_qtys,
		    vector<fixed>& recv_costs, vector<fixed>& adj_qtys,
		    vector<fixed>& adj_costs);
    void itemGeneral(Id item_id, const QString& size, Id store_id, QDate date,
		     vector<Id>& item_ids, vector<QString>& sizes,
		     vector<Id>& stores, vector<fixed>& on_hands,
		     vector<fixed>& total_costs, vector<fixed>& on_orders);
    void itemSold(Id item_id, const QString& size, Id store_id, QDate date,
		  vector<Id>& item_ids, vector<QString>& sizes,
		  vector<Id>& stores, vector<fixed>& sold_qtys,
		  vector<fixed>& sold_costs, vector<fixed>& sold_prices);
    void itemReceived(Id item_id, const QString& size, Id store_id, QDate date,
		      vector<Id>& item_ids, vector<QString>& sizes,
		      vector<Id>& stores, vector<fixed>& recv_qtys,
		      vector<fixed>& recv_costs);
    void itemAdjusted(Id item_id, const QString& size, Id store_id, QDate date,
		      vector<Id>& item_ids, vector<QString>& sizes,
		      vector<Id>& stores, vector<fixed>& adj_qtys,
		      vector<fixed>& adj_costs);
    void itemGeneral(Id item_id, const QString& size, Id store_id, QDate date,
		     fixed& on_hand, fixed& total_cost, fixed& on_order);
    void itemSold(Id item_id, const QString& size, Id store_id, QDate date,
		  fixed& qty, fixed& cost, fixed& price);
    void itemReceived(Id item_id, const QString& size, Id store_id, QDate date,
		      fixed& qty, fixed& cost);
    void itemAdjusted(Id item_id, const QString& size, Id store_id, QDate date,
		      fixed& qty, fixed& cost);

    // Another version of itemTotals but for item history purposes.  The
    // item_id must be supplied but the rest are optional.
    void itemTotals(Id item_id, const QString& size, Id store_id, QDate from,
		    QDate to, vector<QDate>& dates, vector<fixed>& qtys,
		    vector<fixed>& costs, vector<fixed>& sold_qtys,
		    vector<fixed>& sold_costs, vector<fixed>& sold_prices,
		    vector<fixed>& recv_qtys, vector<fixed>& recv_costs,
		    vector<fixed>& adj_qtys, vector<fixed>& adj_costs);

    // Item pricing
    void itemPrices(const Item& item, vector<Id>& price_ids);
    void itemRegularPrice(Id item_id, const QString& size, ItemPrice& price);
    void itemRegularCost(Id item_id, const QString& size, ItemPrice& cost);
    void itemBestPrice(const Item& item, vector<ItemPrice>& prices,
		       const QString& size, const Card& card, Id store_id,
		       QDate date, fixed qty, bool includePromo, bool getCost,
		       ItemPrice& best, fixed& ext_price, fixed& sell_qty,
		       vector<Id>& price_ids, vector<fixed>& sold_qtys);
    void itemSellingCost(const Item& item, const QString& size, Id store_id,
			 fixed qty, fixed sell_price, fixed& inv_cost);

    // Tax calculations
    void taxList(const Tax& tax, vector<Id>& taxes);
    fixed taxPercent(ObjectCache& cache, Id tax_id, vector<Id>& taxes,
		     vector<Id>& exempts);
    fixed taxRate(const Tax& tax);
    fixed calculateTaxOn(ObjectCache& cache, const Tax& tax, fixed taxable,
			 const Tax& exempt, vector<Id>& tax_ids,
			 vector<fixed>& tax_amts);
    fixed calculateTaxOn(ObjectCache& cache, const Tax& tax, fixed taxable,
			 const Tax& exempt);
    fixed calculateTaxOn(ObjectCache& cache, const Tax& tax, fixed taxable);
    fixed calculateTaxOff(ObjectCache& cache, const Tax& tax, fixed total,
			  vector<Id>& tax_ids, vector<fixed>& tax_amts);
    fixed calculateTaxOff(ObjectCache& cache, const Tax& tax, fixed total);

    // Patronage
    void patronageSales(QDate from, QDate to, vector<Id>& customer_ids,
			vector<Id>& dept_ids, vector<fixed>& sales_amts);
    void patronagePurchases(QDate from, QDate to, vector<Id>& vendor_ids,
			    vector<fixed>& purchase_amts);
    void patronageBalances(QDate from, QDate to, vector<Id>& customer_ids,
			   vector<fixed>& change_amts);

    // Terms
    void invoiceTerms(vector<Id>& invoice_ids, vector<Id>& term_ids);
    void receiveTerms(vector<Id>& receive_ids, vector<Id>& term_ids);

    // Cash reconciliation methods
    bool shiftClose(const Shift& shift);
    bool shiftOpen(const Shift& shift);
    bool safeOpenBalance(QDate date, vector<Id>& tenders, vector<fixed>& amts);

    // Special selects for inquiries
    void selectAccount(Id account_id, Id store_id, QDate start, QDate end,
		       vector<Gltx>& gltxs, vector<fixed>& amounts);
    void selectCard(Id card_id, Id store_id, QDate start, QDate end,
		    vector<Gltx>& gltxs, vector<fixed>& amounts);
    void selectItem(Id item_id, Id store_id, QDate start, QDate end,
		    vector<Gltx>& gltxs, vector<fixed>& qtys,
		    vector<fixed>& costs, vector<fixed>& prices,
		    vector<bool>& void_flags);

    // Clone store information
    void cloneLocations(Id from_id, Id to_id);
    void cloneItems(Id from_id, Id to_id);

    // Default accounts
    Id defaultBankId();

    // Generic data handling
    void setActive(DataObject& data, bool active);
    bool lookup(Id id, DataObject& data);
    bool select(vector<DataObject>& objects, Select conditions);

    // Link two transactions
    bool linkTx(Id tx1_id, Id tx2_id);

    // Allocate payment between two transactions
    bool allocPayment(Id from_id, Id to_id, fixed amount);

    // Give some control over commit/rollback
    bool execute(Stmt& stmt);
    void commit(bool force=false);
    void rollback(bool force=false);

    // If auto commit not set, above commit and rollback calls are null ops
    bool autoCommit() const		{ return _auto_commit; }
    void setAutoCommit(bool autoCommit)	{ _auto_commit = autoCommit; }

    // Error handling
    QString* nextError();
    void clearErrors();

signals:
    void dataEvent(DataEvent* e);

protected:
    // Emit signals for data changes
    void dataSignal(DataEvent::Type type, const DataObject& data);

    bool sequenceLock(const Sequence& seq);
    bool sequenceGet(Sequence& seq);
    bool sequenceSet(const Sequence& seq);

    // DataObject methods (these don't do any exception trapping)
    //bool create(DataObject& data);
    //bool remove(const DataObject& data);
    //bool update(const DataObject& orig, DataObject& data);
    bool validate(const DataObject& data);

    // Card methods (internal)
    bool create(Card& card);
    bool remove(const Card& card);
    bool update(const Card& orig, Card& card);
    bool validate(const Card& card);

    // Gltx methods (internal)
    bool create(Gltx& gltx);
    bool remove(const Gltx& gltx);
    bool update(const Gltx& orig, Gltx& gltx);
    bool validate(const Gltx& gltx);

    // Item internal methods
    bool sqlCreate(Plu& plu);
    bool sqlRemove(const Plu& plu);

    // Return text of SQL commands
    QString insertText(const QString& table, const QString& id,
		       const QString& fields);
    QString updateText(const QString& table, const QString& id,
		       const QString& fields);

    // Methods to return SQL commands
    QString insertCmd(const QString& table, const QString& id,
		      const QString& fields);
    QString updateCmd(const QString& table, const QString& id,
		      const QString& fields);
    QString removeCmd(const QString& table, const QString& id);
    QString selectCmd(const QString& table, const QString& id,
		      const QString& fields, const Select& where,
		      const QString& order="");

    // Methods to set SQL parameters
    void insertData(DataObject& data, Stmt& stmt);
    void updateData(const DataObject& orig, DataObject& data,
		    Stmt& stmt);
    bool removeData(const DataObject& data, const QString& table,
		    const QString& id_field);
    void selectData(DataObject& data, Stmt& stmt, int& next);

    // Remove from table without data_object removal
    bool remove(const DataObject& data, const QString& table,
		const QString& id_field);

    // Select functions
    void selectCard(Card& card, Stmt& stmt, int& next);
    void selectGltx(Gltx& gltx, Stmt& stmt, int& next);

    QString cardCmd(const QString& table, const QString& id,
		    const QString& fields, const CardSelect& where,
		    const QString& order="");
    QString gltxCmd(const QString& table, const QString& id,
		    const QString& fields, const GltxSelect& where,
		    const QString& order="");

    // Check data exists in change tables
    void checkAccount(const Gltx& gltx);
    void checkCard(const Gltx& gltx);
    void checkItem(const Gltx& gltx);

    // Sub-table handling
    bool sqlCreateLines(const Gltx& gltx);
    bool sqlDeleteLines(const Gltx& gltx);
    bool sqlCreateLines(const ItemAdjust& adjustment);
    bool sqlDeleteLines(const ItemAdjust& adjustment);
    bool sqlCreateLines(const Cheque& cheque);
    bool sqlDeleteLines(const Cheque& cheque);
    bool sqlCreateLines(const Invoice& invoice);
    bool sqlDeleteLines(const Invoice& invoice);
    bool sqlCreateLines(const Payout& payout);
    bool sqlDeleteLines(const Payout& payout);
    bool sqlCreateLines(const Quote& quote);
    bool sqlDeleteLines(const Quote& quote);
    bool sqlCreateLines(const Receipt& receipt);
    bool sqlDeleteLines(const Receipt& receipt);
    bool sqlCreateLines(const Receive& receive);
    bool sqlDeleteLines(const Receive& receive);

    bool sqlCreateLines(const Account& account);
    bool sqlDeleteLines(const Account& account);
    bool sqlCreateLines(const Card& card);
    bool sqlDeleteLines(const Card& card);
    bool sqlCreateLines(const Customer& customer);
    bool sqlDeleteLines(const Customer& customer);
    bool sqlCreateLines(const Company& company);
    bool sqlDeleteLines(const Company& company);
    bool sqlCreateLines(const Count& count);
    bool sqlDeleteLines(const Count& count);
    bool sqlCreateLines(const Item& item);
    bool sqlDeleteLines(const Item& item);
    bool sqlCreateLines(const LabelBatch& batch);
    bool sqlDeleteLines(const LabelBatch& batch);
    bool sqlCreateLines(const Order& order);
    bool sqlDeleteLines(const Order& order);
    bool sqlCreateLines(const OrderTemplate& temp);
    bool sqlDeleteLines(const OrderTemplate& temp);
    bool sqlCreateLines(const PatGroup& group);
    bool sqlDeleteLines(const PatGroup& group);
    bool sqlCreateLines(const PatWorksheet& sheet);
    bool sqlDeleteLines(const PatWorksheet& sheet);
    bool sqlCreateLines(const PriceBatch& batch);
    bool sqlDeleteLines(const PriceBatch& batch);
    bool sqlCreateLines(const PromoBatch& batch);
    bool sqlDeleteLines(const PromoBatch& batch);
    bool sqlCreateLines(const Reconcile& reconcile);
    bool sqlDeleteLines(const Reconcile& reconcile);
    bool sqlCreateLines(const Slip& slip);
    bool sqlDeleteLines(const Slip& slip);
    bool sqlCreateLines(const Tender& tender);
    bool sqlDeleteLines(const Tender& tender);
    bool sqlCreateLines(const TenderCount& count);
    bool sqlDeleteLines(const TenderCount& count);

    // Methods to update _change tables
    bool accountAdjust(Id account_id, Id store_id, QDate date, fixed amount);
    bool cardAdjust(Id card_id, Id store_id, QDate date, fixed amount);
    bool itemAdjust(Id item_id, const QString& size, Id store_id, QDate date,
		    fixed qty, fixed cost);
    bool itemSale(Id item_id, const QString& size, Id store_id, QDate date,
		  fixed qty, fixed cost, fixed price);
    bool itemReceive(Id item_id, const QString& size, Id store_id, QDate date,
		     fixed qty, fixed cost);
    bool itemAdjustment(Id item_id, const QString& size, Id store_id,
			QDate date,fixed qty, fixed cost);
    bool itemOrder(Id item_id, const QString& size, Id store_id, QDate date,
		   fixed qty);

    bool itemSale(vector<Id>& item_ids, vector<QString>& sizes, Id store_id,
		  QDate date, vector<fixed>& qtys, vector<fixed>& costs,
		  vector<fixed>& prices);
    bool itemReceive(vector<Id>& item_ids, vector<QString>& sizes, Id store_id,
		     QDate date, vector<fixed>& qtys, vector<fixed>& costs);
    bool itemOrder(vector<Id>& item_ids, vector<QString>& sizes, Id store_id,
		   QDate date, vector<fixed>& qtys);

    // Payment allocation between gltx transactions
    void checkPaid(Id gltx_id);
    void deletePayments(const Gltx& invoice);

    bool error(const QString& msg);

    QuasarClient* _client;
    Connection* _connection;
    QString _username;
    int _system_id;
    Id _default_store;
    bool _auto_commit;
    static QValueStack<QString*> _errors;
};

// These defines should be procedures but in C++ I can't treat vectors
// as a generic data type!
#define GLTX_ACCOUNTS(gltxs) \
	cmd = "select account_id,amount,memo,cleared from " \
		"gltx_account where gltx_id = ? order by seq_num"; \
	stmt.setCommand(cmd); \
	stmtSetId(stmt, gltx_id); \
	if (!execute(stmt)) return false; \
	while (stmt.next()) { \
	    AccountLine line; \
	    line.account_id = stmtGetId(stmt, 1); \
	    line.amount = stmtGetFixed(stmt, 2); \
	    line.memo = stmtGetString(stmt, 3); \
	    line.cleared = stmtGetDate(stmt, 4); \
	    gltxs[i].accounts().push_back(line); \
	} \

#define GLTX_CARDS(gltxs) \
	cmd = "select card_id,amount from gltx_card " \
		"where gltx_id = ? order by seq_num"; \
	stmt.setCommand(cmd); \
	stmtSetId(stmt, gltx_id); \
	if (!execute(stmt)) return false; \
	while (stmt.next()) { \
	    CardLine line; \
	    line.card_id = stmtGetId(stmt, 1); \
	    line.amount = stmtGetFixed(stmt, 2); \
	    gltxs[i].cards().push_back(line); \
	} \

#define GLTX_PAYMENTS(gltxs) \
	cmd = "select pay_id,amount,discount from gltx_paid where " \
		"gltx_id = ? order by seq_num"; \
	stmt.setCommand(cmd); \
	stmtSetId(stmt, gltx_id); \
	if (!execute(stmt)) return false; \
	while (stmt.next()) { \
	    PaymentLine line; \
	    line.gltx_id = stmtGetId(stmt, 1); \
	    line.amount = stmtGetFixed(stmt, 2); \
	    line.discount = stmtGetFixed(stmt, 3); \
	    gltxs[i].payments().push_back(line); \
	} \
	cmd = "select gltx_id,amount,discount from gltx_paid where " \
		"pay_id = ? order by seq_num"; \
	stmt.setCommand(cmd); \
	stmtSetId(stmt, gltx_id); \
	if (!execute(stmt)) return false; \
	while (stmt.next()) { \
	    PaymentLine line; \
	    line.gltx_id = stmtGetId(stmt, 1); \
	    line.amount = -stmtGetFixed(stmt, 2); \
	    line.discount = -stmtGetFixed(stmt, 3); \
	    gltxs[i].payments().push_back(line); \
	} \

#define GLTX_TAXES(gltxs) \
	cmd = "select tax_id,taxable,amount,inc_taxable,inc_amount from " \
		"gltx_tax where gltx_id = ? order by seq_num"; \
	stmt.setCommand(cmd); \
	stmtSetId(stmt, gltx_id); \
	if (!execute(stmt)) return false; \
	while (stmt.next()) { \
	    TaxLine line; \
	    line.tax_id = stmtGetId(stmt, 1); \
	    line.taxable = stmtGetFixed(stmt, 2); \
	    line.amount = stmtGetFixed(stmt, 3); \
	    line.inc_taxable = stmtGetFixed(stmt, 4); \
	    line.inc_amount = stmtGetFixed(stmt, 5); \
	    gltxs[i].taxes().push_back(line); \
	} \

#define GLTX_TENDERS(gltxs) \
	cmd = "select tender_id,amount,convert_rate,convert_amt," \
		"card_num,expiry_date,auth_num,is_change,voided " \
		"from gltx_tender where gltx_id = ? order by seq_num"; \
	stmt.setCommand(cmd); \
	stmtSetId(stmt, gltx_id); \
	if (!execute(stmt)) return false; \
	while (stmt.next()) { \
	    TenderLine line; \
	    line.tender_id = stmtGetId(stmt, 1); \
	    line.amount = stmtGetFixed(stmt, 2); \
	    line.conv_rate = stmtGetFixed(stmt, 3); \
	    line.conv_amt = stmtGetFixed(stmt, 4); \
	    line.card_num = stmtGetString(stmt, 5); \
	    line.expiry_date = stmtGetDate(stmt, 6); \
	    line.auth_num = stmtGetString(stmt, 7); \
	    line.is_change = stmtGetBool(stmt, 8); \
	    line.voided = stmtGetBool(stmt, 9); \
	    gltxs[i].tenders().push_back(line); \
	} \

#define GLTX_REFERENCES(gltxs) \
	cmd = "select ref_name,ref_data from gltx_refs where gltx_id = ? " \
		"order by seq_num"; \
	stmt.setCommand(cmd); \
	stmtSetId(stmt, gltx_id); \
	if (!execute(stmt)) return false; \
	while (stmt.next()) { \
	    gltxs[i].referenceName().push_back(stmtGetString(stmt, 1)); \
	    gltxs[i].referenceData().push_back(stmtGetString(stmt, 2)); \
	} \

#endif // QUASAR_DB_H
