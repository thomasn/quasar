// $Id: data_import.h,v 1.14 2005/06/11 02:51:31 bpepers Exp $
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

#ifndef DATA_IMPORT_H
#define DATA_IMPORT_H

#include "quasar_db.h"
#include "company_defn.h"
#include "variant.h"

#include <qobject.h>
#include <qvaluevector.h>
#include <qpair.h>
#include <qdom.h>

class Account;
class AdjustReason;
class Charge;
class Count;
class Customer;
class CustomerType;
class Dept;
class Discount;
class Employee;
class Expense;
class Extra;
class Group;
class Item;
class ItemPrice;
class Location;
class Order;
class OrderTemplate;
class PatGroup;
class Personal;
class Quote;
class Reconcile;
class Recurring;
class SecurityType;
class Slip;
class Station;
class Store;
class Subdept;
class Tax;
class Tender;
class Term;
class Todo;
class User;
class Vendor;

typedef QPair<QString, QString> AccountFixup;

class DataImport: public QObject {
    Q_OBJECT
public:
    DataImport(const CompanyDefn& company, QObject* parent=NULL);
    ~DataImport();

    bool processFile(const QString& filePath);

    QString currentType() const { return _type; }
    QString currentName() const { return _name; }
    int errorCount() const { return _errors; }

signals:
    void message(int count, QString severity, QString message);

protected:
    void logMessage(const QString& severity, const QString& message);
    void logError(const QString& message);
    void logWarning(const QString& message);
    void logInfo(const QString& message);
    void logQuasarErrors(const QString& message);

    bool importAccount(QDomNodeList& nodes, bool isUpdate);
    bool importAccountDelete(QDomNodeList& nodes);
    bool importAdjustReason(QDomNodeList& nodes, bool isUpdate);
    bool importCharge(QDomNodeList& nodes, bool isUpdate);
    bool importCompany(QDomNodeList& nodes, bool isUpdate);
    bool importCount(QDomNodeList& nodes);
    bool importCustomer(QDomNodeList& nodes, bool isUpdate);
    bool importCustomerType(QDomNodeList& nodes, bool isUpdate);
    bool importDept(QDomNodeList& nodes, bool isUpdate);
    bool importDiscount(QDomNodeList& nodes, bool isUpdate);
    bool importEmployee(QDomNodeList& nodes, bool isUpdate);
    bool importExpense(QDomNodeList& nodes, bool isUpdate);
    bool importExtra(QDomNodeList& nodes);
    bool importGroup(QDomNodeList& nodes);
    bool importItem(QDomNodeList& nodes, bool isUpdate);
    bool importItemDelete(QDomNodeList& nodes);
    bool importLabelBatch(QDomNodeList& nodes);
    bool importLocation(QDomNodeList& nodes, bool isUpdate);
    bool importOrder(QDomNodeList& nodes, bool isUpdate);
    bool importOrderDelete(QDomNodeList& nodes);
    bool importPatGroup(QDomNodeList& nodes, bool isUpdate);
    bool importPatGroupDelete(QDomNodeList& nodes);
    bool importPersonal(QDomNodeList& nodes, bool isUpdate);
    bool importPrice(QDomNodeList& nodes, bool isCost);
    bool importPriceBatch(QDomNodeList& nodes);
    bool importPriceChange(QDomNodeList& nodes);
    bool importQuote(QDomNodeList& nodes);
    bool importReconcile(QDomNodeList& nodes);
    bool importRecurring(QDomNodeList& nodes);
    bool importSecurityType(QDomNodeList& nodes, bool isUpdate);
    bool importSlip(QDomNodeList& nodes, bool isUpdate);
    bool importSlipDelete(QDomNodeList& nodes);
    bool importStation(QDomNodeList& nodes, bool isUpdate);
    bool importStore(QDomNodeList& nodes, bool isUpdate);
    bool importSubdept(QDomNodeList& nodes, bool isUpdate);
    bool importSubdeptDelete(QDomNodeList& nodes);
    bool importTax(QDomNodeList& nodes, bool isUpdate);
    bool importTemplate(QDomNodeList& nodes, bool isUpdate);
    bool importTemplateDelete(QDomNodeList& nodes);
    bool importTender(QDomNodeList& nodes, bool isUpdate);
    bool importTerm(QDomNodeList& nodes, bool isUpdate);
    bool importTodo(QDomNodeList& nodes);
    bool importUser(QDomNodeList& nodes, bool isUpdate);
    bool importVendor(QDomNodeList& nodes, bool isUpdate);

    // Transaction imports
    bool importJournalEntry(QDomNodeList& nodes);
    bool importCardAdjustment(QDomNodeList& nodes);
    bool importCustomerInvoice(QDomNodeList& nodes);

    bool iconvBoolean(const QString& text);
    int iconvInt(const QString& text);
    fixed iconvFixed(const QString& text);
    Price iconvPrice(const QString& text);
    QDate iconvDate(const QString& text);
    QTime iconvTime(const QString& text);
    Id iconvAccount(const QString& text);
    Id iconvCompany(const QString& text);
    Id iconvCustomer(const QString& text);
    Id iconvDept(const QString& text);
    Id iconvDiscount(const QString& text);
    Id iconvEmployee(const QString& text);
    Id iconvGroup(const QString& text, int type);
    Id iconvPatGroup(const QString& text);
    Id iconvSecurityType(const QString& text);
    Id iconvStation(const QString& text);
    Id iconvStore(const QString& text);
    Id iconvSubdept(const QString& text);
    Id iconvTax(const QString& text);
    Id iconvTerms(const QString& text);
    Id iconvVendor(const QString& text);

    bool findAccount(const QString& name, Account& account);
    bool findAdjustReason(const QString& name, AdjustReason& reason);
    bool findCharge(const QString& name, Charge& charge);
    bool findCompany(const QString& name, Company& company);
    bool findCount(const QString& name, Count& count);
    bool findCustomer(const QString& name, Customer& customer);
    bool findCustomerType(const QString& name, CustomerType& type);
    bool findDept(const QString& name, Dept& dept);
    bool findDiscount(const QString& name, Discount& discount);
    bool findEmployee(const QString& name, Employee& employee);
    bool findExpense(const QString& name, Expense& expense);
    bool findExtra(const QString& table, const QString& name, Extra& extra);
    bool findGroup(const QString& name, int type, Group& group);
    bool findItem(const QString& number, Item& item);
    bool findLocation(const QString& name, Id store_id, Location& location);
    bool findOrder(const QString& number, Order& order);
    bool findPatGroup(const QString& name, PatGroup& group);
    bool findPersonal(const QString& name, Personal& personal);
    bool findQuote(const QString& name, Quote& quote);
    bool findReconcile(const QString& name, Reconcile& reconcile);
    bool findRecurring(const QString& name, Recurring& recurring);
    bool findSecurityType(const QString& name, SecurityType& security);
    bool findSlip(const QString& name, Slip& slip);
    bool findStation(const QString& number, Station& station);
    bool findStore(const QString& name, Store& store);
    bool findSubdept(const QString& name, Subdept& subdept);
    bool findTax(const QString& name, Tax& tax);
    bool findTemplate(const QString& name, OrderTemplate& templ);
    bool findTender(const QString& name, Tender& tender);
    bool findTerm(const QString& name, Term& term);
    bool findTodo(const QString& name, Todo& todo);
    bool findUser(const QString& name, User& user);
    bool findVendor(const QString& name, Vendor& vendor);

    CompanyDefn _company;
    QDomDocument _doc;
    QuasarDB* _db;
    int _count;
    int _errors;
    QString _type;
    QString _name;
    QValueVector<AccountFixup> _acctFixups;
};

#endif // DATA_IMPORT_H
