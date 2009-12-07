// $Id: open_balances.h,v 1.6 2004/02/18 04:41:58 bpepers Exp $
//
// Copyright (C) 1998-2003 Linux Canada Inc.  All rights reserved.
//
// This file is part of Quasar Accounting
//
// This file may be distributed and/or modified under the terms of the
// GNU General Public License version 2 as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL included in the
// packaging of this file.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// See http://www.linuxcanada.com or email sales@linuxcanada.com for
// information about Quasar Accounting support and maintenance options.
//
// Contact sales@linuxcanada.com if any conditions of this licensing are
// not clear to you.

#ifndef OPEN_BALANCES_H
#define OPEN_BALANCES_H

#include "quasar_window.h"
#include "variant.h"

class LookupEdit;
class Table;
class MoneyEdit;
class ItemLookup;
class QComboBox;
class QDomNodeList;
class Account;
class Customer;
class Item;
class Vendor;

class OpenBalances: public QuasarWindow {
    Q_OBJECT
public:
    OpenBalances(MainWindow* main);
    ~OpenBalances();

protected slots:
    void customerCellChanged(int row, int col, Variant old);
    void customerFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void vendorCellChanged(int row, int col, Variant old);
    void vendorFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void itemCellMoved(int row, int col);
    void itemCellChanged(int row, int col, Variant old);
    void itemFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void accountCellChanged(int row, int col, Variant old);
    void accountFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void slotStoreChanged();
    void slotImport();
    void slotPost();
    void recalculate();

protected:
    LookupEdit* _store;
    LookupEdit* _station;
    LookupEdit* _employee;
    Table* _customers;
    Table* _vendors;
    Table* _items;
    Table* _accounts;
    LookupEdit* _account;
    MoneyEdit* _total;

    ItemLookup* _itemLookup;
    QComboBox* _size;

    int _errors;
    void warning(const QString& text);

    bool importAccount(QDomNodeList& nodes);
    bool importItem(QDomNodeList& nodes);
    bool importCustomer(QDomNodeList& nodes);
    bool importVendor(QDomNodeList& nodes);

    bool findAccount(const QString& text, Account& account);
    bool findItem(const QString& text, Item& item);
    bool findCustomer(const QString& text, Customer& customer);
    bool findVendor(const QString& text, Vendor& vendor);
};

#endif // OPEN_BALANCES_H
