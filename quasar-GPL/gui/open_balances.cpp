// $Id: open_balances.cpp,v 1.14 2005/03/16 15:37:01 bpepers Exp $
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

#include "open_balances.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "company.h"
#include "station.h"
#include "employee.h"
#include "customer.h"
#include "vendor.h"
#include "item.h"
#include "account.h"
#include "account_select.h"
#include "customer_select.h"
#include "item_select.h"
#include "vendor_select.h"
#include "card_adjust.h"
#include "item_adjust.h"
#include "general.h"
#include "item_edit.h"
#include "station_lookup.h"
#include "employee_lookup.h"
#include "store_lookup.h"
#include "employee_lookup.h"
#include "customer_lookup.h"
#include "vendor_lookup.h"
#include "item_lookup.h"
#include "account_lookup.h"
#include "table.h"
#include "date_edit.h"
#include "double_edit.h"
#include "money_edit.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <qfiledialog.h>
#include <qfile.h>
#include <qdom.h>

OpenBalances::OpenBalances(MainWindow* main)
    : QuasarWindow(main, "OpenBalances")
{
    _helpSource = "open_balances.html";

    QFrame* frame = new QFrame(this);
    QFrame* top = new QFrame(frame);

    QLabel* storeLabel = new QLabel(tr("Store:"), top);
    _store = new LookupEdit(new StoreLookup(main, this), top);
    _store->setLength(30);
    storeLabel->setBuddy(_store);
    connect(_store, SIGNAL(validData()), SLOT(slotStoreChanged()));

    QLabel* stationLabel = new QLabel(tr("Station:"), top);
    _station = new LookupEdit(new StationLookup(main, this), top);
    _station->setLength(30);
    stationLabel->setBuddy(_station);

    QLabel* employeeLabel = new QLabel(tr("Employee:"), top);
    _employee = new LookupEdit(new EmployeeLookup(main, this), top);
    _employee->setLength(30);
    employeeLabel->setBuddy(_employee);

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(6);
    topGrid->setMargin(3);
    topGrid->setColStretch(2, 1);
    topGrid->addWidget(storeLabel, 0, 0);
    topGrid->addWidget(_store, 0, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(stationLabel, 1, 0);
    topGrid->addWidget(_station, 1, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(employeeLabel, 2, 0);
    topGrid->addWidget(_employee, 2, 1, AlignLeft | AlignVCenter);

    QTabWidget* tabs = new QTabWidget(frame);
    QFrame* customers = new QFrame(tabs);
    QFrame* vendors = new QFrame(tabs);
    QFrame* items = new QFrame(tabs);
    QFrame* accounts = new QFrame(tabs);
    tabs->addTab(customers, tr("Customers"));
    tabs->addTab(vendors, tr("Vendors"));
    tabs->addTab(items, tr("Items"));
    tabs->addTab(accounts, tr("Accounts"));

    _customers = new Table(customers);
    _customers->setVScrollBarMode(QScrollView::AlwaysOn);
    _customers->setDisplayRows(10);
    _customers->setLeftMargin(fontMetrics().width("99999"));
    connect(_customers, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(customerCellChanged(int,int,Variant)));
    connect(_customers, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(customerFocusNext(bool&,int&,int&,int)));

    CustomerLookup* custLookup = new CustomerLookup(_main, this);

    new LookupColumn(_customers, tr("Customer"), 30, custLookup);
    new DateColumn(_customers, tr("Date"));
    new MoneyColumn(_customers, tr("Amount"), 10);

    new LookupEditor(_customers, 0, new LookupEdit(custLookup, _customers));
    new LineEditor(_customers, 1, new DateEdit(_customers));
    new NumberEditor(_customers, 2, new MoneyEdit(_customers));

    QGridLayout* customerGrid = new QGridLayout(customers);
    customerGrid->setSpacing(6);
    customerGrid->setMargin(3);
    customerGrid->setColStretch(0, 1);
    customerGrid->addWidget(_customers, 0, 0);

    _vendors = new Table(vendors);
    _vendors->setVScrollBarMode(QScrollView::AlwaysOn);
    _vendors->setDisplayRows(10);
    _vendors->setLeftMargin(fontMetrics().width("99999"));
    connect(_vendors, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(vendorCellChanged(int,int,Variant)));
    connect(_vendors, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(vendorFocusNext(bool&,int&,int&,int)));

    VendorLookup* vendLookup = new VendorLookup(_main, this);

    new LookupColumn(_vendors, tr("Vendor"), 30, vendLookup);
    new DateColumn(_vendors, tr("Date"));
    new MoneyColumn(_vendors, tr("Amount"), 10);

    new LookupEditor(_vendors, 0, new LookupEdit(vendLookup, _vendors));
    new LineEditor(_vendors, 1, new DateEdit(_vendors));
    new NumberEditor(_vendors, 2, new MoneyEdit(_vendors));

    QGridLayout* vendorGrid = new QGridLayout(vendors);
    vendorGrid->setSpacing(6);
    vendorGrid->setMargin(3);
    vendorGrid->setColStretch(0, 1);
    vendorGrid->addWidget(_vendors, 0, 0);

    _items = new Table(items);
    _items->setVScrollBarMode(QScrollView::AlwaysOn);
    _items->setDisplayRows(10);
    _items->setLeftMargin(fontMetrics().width("99999"));
    connect(_items, SIGNAL(cellMoved(int,int)), SLOT(itemCellMoved(int,int)));
    connect(_items, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(itemCellChanged(int,int,Variant)));
    connect(_items, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(itemFocusNext(bool&,int&,int&,int)));

    _itemLookup = new ItemLookup(_main, this);
    _itemLookup->inventoriedOnly = true;

    new LookupColumn(_items, tr("Item"), 12, _itemLookup);
    new TextColumn(_items, tr("Description"), 20);
    new TextColumn(_items, tr("Size"), 8);
    new NumberColumn(_items, tr("Quantity"), 6);
    new MoneyColumn(_items, tr("Total Cost"), 8);

    _size = new QComboBox(_items);
    new LookupEditor(_items, 0, new ItemEdit(_itemLookup, _items));
    new ComboEditor(_items, 2, _size);
    new NumberEditor(_items, 3, new DoubleEdit(_items));
    new NumberEditor(_items, 4, new MoneyEdit(_items));

    QGridLayout* itemGrid = new QGridLayout(items);
    itemGrid->setSpacing(6);
    itemGrid->setMargin(3);
    itemGrid->setColStretch(0, 1);
    itemGrid->addWidget(_items, 0, 0);

    _accounts = new Table(accounts);
    _accounts->setVScrollBarMode(QScrollView::AlwaysOn);
    _accounts->setDisplayRows(10);
    _accounts->setLeftMargin(fontMetrics().width("99999"));
    connect(_accounts, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(accountCellChanged(int,int,Variant)));
    connect(_accounts, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(accountFocusNext(bool&,int&,int&,int)));

    AccountLookup* acctLookup = new AccountLookup(_main, this);

    new LookupColumn(_accounts, tr("Account"), 30, acctLookup);
    new MoneyColumn(_accounts, tr("Debit"), 10);
    new MoneyColumn(_accounts, tr("Credit"), 10);

    new LookupEditor(_accounts, 0, new LookupEdit(acctLookup, _accounts));
    new NumberEditor(_accounts, 1, new MoneyEdit(_accounts));
    new NumberEditor(_accounts, 2, new MoneyEdit(_accounts));

    QGridLayout* accountGrid = new QGridLayout(accounts);
    accountGrid->setSpacing(6);
    accountGrid->setMargin(3);
    accountGrid->setColStretch(0, 1);
    accountGrid->addWidget(_accounts, 0, 0);

    QFrame* bot = new QFrame(frame);

    QLabel* accountLabel = new QLabel(tr("Account:"), bot);
    _account = new LookupEdit(new AccountLookup(_main, this), bot);
    _account->setLength(30);
    accountLabel->setBuddy(_account);

    QLabel* totalLabel = new QLabel(tr("Total:"), bot);
    _total = new MoneyEdit(bot);
    _total->setFocusPolicy(NoFocus);
    totalLabel->setBuddy(_total);

    QGridLayout* botGrid = new QGridLayout(bot);
    botGrid->setSpacing(6);
    botGrid->setMargin(3);
    botGrid->setColStretch(2, 1);
    botGrid->addWidget(accountLabel, 0, 0);
    botGrid->addWidget(_account, 0, 1, AlignLeft | AlignVCenter);
    botGrid->addWidget(totalLabel, 0, 3);
    botGrid->addWidget(_total, 0, 4, AlignLeft | AlignVCenter);

    QFrame* buttons = new QFrame(frame);
    QPushButton* import = new QPushButton(tr("Import"), buttons);
    QPushButton* post = new QPushButton(tr("Post"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);

    connect(import, SIGNAL(clicked()), SLOT(slotImport()));
    connect(post, SIGNAL(clicked()), SLOT(slotPost()));
    connect(cancel, SIGNAL(clicked()), SLOT(close()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(1, 1);
    buttonGrid->addWidget(import, 0, 0);
    buttonGrid->addWidget(post, 0, 2);
    buttonGrid->addWidget(cancel, 0, 3);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(3);
    grid->addWidget(top, 0, 0);
    grid->addWidget(tabs, 1, 0);
    grid->addWidget(bot, 2, 0);
    grid->addWidget(buttons, 3, 0);

    Company company;
    _quasar->db()->lookup(company);

    _store->setId(_quasar->defaultStore(true));
    _station->setId(_quasar->defaultStation());
    _employee->setId(_quasar->defaultEmployee());
    _account->setId(company.historicalBalancing());

    _customers->appendRow(new VectorRow(_customers->columns()));
    _vendors->appendRow(new VectorRow(_vendors->columns()));
    _items->appendRow(new VectorRow(_items->columns()));
    _accounts->appendRow(new VectorRow(_accounts->columns()));

#if 0
    ItemSelect conditions;
    vector<Item> v_items;
    _quasar->db()->select(v_items, conditions);
    for (unsigned int i = 0; i < v_items.size(); ++i) {
	Item& item = v_items[i];
	for (unsigned int j = 0; j < item.vendors().size(); ++j) {
	    ItemVendor& vendor = item.vendors()[j];

	    bool found = false;
	    for (unsigned int k = 0; k < item.numbers().size(); ++k) {
		if (item.numbers()[k].number == vendor.number) {
		    found = true;
		    break;
		}
	    }
	    if (found) continue;

	    ItemPlu info;
	    info.number = vendor.number;

	    Item orig = item;
	    item.numbers().push_back(info);
	    _quasar->db()->update(orig, item);
	}
    }
#endif

    setCentralWidget(frame);
    setCaption(tr("Open Balances"));
    finalize();
}

OpenBalances::~OpenBalances()
{
}

void
OpenBalances::slotStoreChanged()
{
    _itemLookup->store_id = _store->getId();
}

void
OpenBalances::slotImport()
{
    QString start = "";
    QString filter = "Import Files (*.xml)";

    QString filePath = QFileDialog::getOpenFileName(start, filter, this);
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(IO_ReadOnly)) {
	QString message = tr("Can't open import file: %1").arg(filePath);
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorCol;
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorCol)) {
	QApplication::restoreOverrideCursor();
	qApp->beep();

	QString message = tr("Error processing the import file on line\n"
			     "%1, column %2:\n\n%3").arg(errorLine)
			     .arg(errorCol).arg(errorMsg);

	QMessageBox::critical(this, tr("Error"), message);
	return;
    }
    if (doc.doctype().name() != "BALANCES") {
	QApplication::restoreOverrideCursor();
	qApp->beep();

	QString message = tr("The import file is not the right type of\n"
			     "XML file for Quasar to use.");
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }
    file.close();

    _errors = 0;
    _customers->clear();
    _vendors->clear();
    _items->clear();
    _accounts->clear();

    QDomElement root = doc.documentElement();
    unsigned int i;
    for (i = 0; i < root.childNodes().count(); ++i) {
	QDomNode node = root.childNodes().item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;

	QString tag = e.tagName();
	QString text = e.text();

	QDomNodeList nodes = e.childNodes();

	if (tag == "account") {
	    QDomNodeList nodes = e.childNodes();
	    importAccount(nodes);
	} else if (tag == "customer") {
	    QDomNodeList nodes = e.childNodes();
	    importCustomer(nodes);
	} else if (tag == "vendor") {
	    QDomNodeList nodes = e.childNodes();
	    importVendor(nodes);
	} else if (tag == "item") {
	    QDomNodeList nodes = e.childNodes();
	    importItem(nodes);
	} else {
	    warning("Unknown tag: " + tag);
	}
    }

    _customers->appendRow(new VectorRow(_customers->columns()));
    _vendors->appendRow(new VectorRow(_vendors->columns()));
    _items->appendRow(new VectorRow(_items->columns()));
    _accounts->appendRow(new VectorRow(_accounts->columns()));

    recalculate();

    QApplication::restoreOverrideCursor();
    qApp->beep();

    if (_errors == 0) {
	QString message = tr("Import completed successfully");
	QMessageBox::information(this, tr("Status"), message);
    } else {
	QString message;
	if (_errors == 1)
	    message = tr("Import completed with %1 error").arg(_errors);
	else
	    message = tr("Import completed with %1 errors").arg(_errors);
	QMessageBox::critical(this, tr("Status"), message);
    }
}

void
OpenBalances::warning(const QString& text)
{
    qWarning(text);
    ++_errors;
}

bool
OpenBalances::importAccount(QDomNodeList& nodes)
{
    Account account;
    fixed debit = 0.0;
    fixed credit = 0.0;

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "name") {
	    if (!findAccount(text, account))
		warning("Can't find account: " + text);
	} else if (tag == "number") {
	    if (!findAccount(text, account))
		warning("Can't find account: " + text);
	} else if (tag == "debit") {
	    debit = text.toDouble();
	} else if (tag == "credit") {
	    credit = text.toDouble();
	} else {
	    warning("Unknown account tag: " + tag);
	}
    }

    if (account.id() == INVALID_ID) {
	warning("Failed account import");
	return false;
    }

    VectorRow* row = new VectorRow(_accounts->columns());
    row->setValue(0, account.id());
    if (debit != 0.0) row->setValue(1, debit);
    if (credit != 0.0) row->setValue(2, credit);
    _accounts->appendRow(row);

    return true;
}

bool
OpenBalances::importCustomer(QDomNodeList& nodes)
{
    Customer customer;
    QDate date = QDate::currentDate();
    fixed amount = 0.0;

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "name") {
	    if (!findCustomer(text, customer))
		warning("Can't find customer: " + text);
	} else if (tag == "number") {
	    if (!findCustomer(text, customer))
		warning("Can't find customer: " + text);
	} else if (tag == "date") {
	    date = QDate::fromString(text, Qt::ISODate);
	} else if (tag == "amount") {
	    amount = text.toDouble();
	} else {
	    warning("Unknown customer tag: " + tag);
	}
    }

    if (customer.id() == INVALID_ID) {
	warning("Failed customer import");
	return false;
    }

    VectorRow* row = new VectorRow(_customers->columns());
    row->setValue(0, customer.id());
    row->setValue(1, date);
    row->setValue(2, amount);
    _customers->appendRow(row);

    return true;
}

bool
OpenBalances::importItem(QDomNodeList& nodes)
{
    Item item;
    QString number;
    QString size;
    fixed qty = 0.0;
    fixed cost = 0.0;

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "number") {
	    if (!findItem(text, item))
		warning("Can't find item: " + text);
	    number = text;
	} else if (tag == "size") {
	    size = text;
	} else if (tag == "qty") {
	    qty = text.toDouble();
	} else if (tag == "cost") {
	    cost = text.toDouble();
	} else {
	    warning("Unknown item tag: " + tag);
	}
    }

    if (item.id() == INVALID_ID) {
	warning("Failed item import");
	return false;
    }

    if (!item.isInventoried()) {
	warning("Item " + number + " isn't inventoried");
	return false;
    }

    if (!item.stocked(_store->getId())) {
	warning("Item " + number + " isn't stocked");
	return false;
    }

    if (size.isEmpty()) size = item.numberSize(number);
    if (size.isEmpty()) size = item.sellSize();

    VectorRow* row = new VectorRow(_items->columns());
    row->setValue(0, Plu(item.id(), number));
    row->setValue(1, item.description());
    row->setValue(2, size);
    row->setValue(3, qty);
    row->setValue(4, cost);
    _items->appendRow(row);

    return true;
}

bool
OpenBalances::importVendor(QDomNodeList& nodes)
{
    Vendor vendor;
    QDate date = QDate::currentDate();
    fixed amount = 0.0;

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "name") {
	    if (!findVendor(text, vendor))
		warning("Can't find vendor: " + text);
	} else if (tag == "number") {
	    if (!findVendor(text, vendor))
		warning("Can't find vendor: " + text);
	} else if (tag == "date") {
	    date = QDate::fromString(text, Qt::ISODate);
	} else if (tag == "amount") {
	    amount = text.toDouble();
	} else {
	    warning("Unknown vendor tag: " + tag);
	}
    }

    if (vendor.id() == INVALID_ID) {
	warning("Failed vendor import");
	return false;
    }

    VectorRow* row = new VectorRow(_vendors->columns());
    row->setValue(0, vendor.id());
    row->setValue(1, date);
    row->setValue(2, amount);
    _vendors->appendRow(row);

    return true;
}

bool
OpenBalances::findAccount(const QString& text, Account& account)
{
    AccountSelect select;
    select.activeOnly = true;
    select.name = text;

    vector<Account> accounts;
    _quasar->db()->select(accounts, select);

    if (accounts.size() == 0) {
	select.name = "";
	select.number = text;
	_quasar->db()->select(accounts, select);
    }

    if (accounts.size() != 1) return false;
    account = accounts[0];
    return true;
}

bool
OpenBalances::findCustomer(const QString& text, Customer& customer)
{
    CustomerSelect select;
    select.activeOnly = true;
    select.name = text;

    vector<Customer> customers;
    _quasar->db()->select(customers, select);

    if (customers.size() == 0) {
	select.name = "";
	select.number = text;
	_quasar->db()->select(customers, select);
    }

    if (customers.size() != 1) return false;
    customer = customers[0];
    return true;
}

bool
OpenBalances::findItem(const QString& text, Item& item)
{
    ItemSelect select;
    select.activeOnly = true;
    select.number = text;
    select.store_id = _store->getId();

    vector<Item> items;
    _quasar->db()->select(items, select);

    if (items.size() != 1) return false;
    item = items[0];
    return true;
}

bool
OpenBalances::findVendor(const QString& text, Vendor& vendor)
{
    VendorSelect select;
    select.activeOnly = true;
    select.name = text;

    vector<Vendor> vendors;
    _quasar->db()->select(vendors, select);

    if (vendors.size() == 0) {
	select.name = "";
	select.number = text;
	_quasar->db()->select(vendors, select);
    }

    if (vendors.size() != 1) return false;
    vendor = vendors[0];
    return true;
}

void
OpenBalances::slotPost()
{
    Id store_id = _store->getId();
    Id station_id = _station->getId();
    Id employee_id = _employee->getId();
    Id history_id = _account->getId();

    if (store_id == INVALID_ID) {
	QString message = tr("A store is required");
	QMessageBox::critical(this, tr("Error"), message);
	_store->setFocus();
	return;
    }

    if (history_id == INVALID_ID) {
	QString message = tr("An account is required");
	QMessageBox::critical(this, tr("Error"), message);
	_account->setFocus();
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    for (int row = 0; row < _customers->rows(); ++row) {
	Id customer_id = _customers->cellValue(row, 0).toId();
	QDate date = _customers->cellValue(row, 1).toDate();
	fixed amount = _customers->cellValue(row, 2).toFixed();
	if (customer_id == INVALID_ID || amount == 0.0) continue;

	Customer customer;
	_quasar->db()->lookup(customer_id, customer);
	Id customer_acct = customer.accountId();

	CardAdjust adjustment;
	adjustment.setPostDate(date);
	adjustment.setPostTime(QTime(0, 0, 0));
	adjustment.setMemo(tr("Open Balance"));
	adjustment.setStationId(station_id);
	adjustment.setEmployeeId(employee_id);
	adjustment.setCardId(customer_id);
	adjustment.setStoreId(store_id);

	adjustment.accounts().push_back(AccountLine(customer_acct, amount));
	adjustment.accounts().push_back(AccountLine(history_id, -amount));
	adjustment.cards().push_back(CardLine(customer_id, amount));

	if (!_quasar->db()->create(adjustment)) {
	    QString message = tr("Customer '%1' open balance failed")
		.arg(customer.name());
	    QMessageBox::critical(this, tr("Error"), message);
	}
    }

    for (int row = 0; row < _vendors->rows(); ++row) {
	Id vendor_id = _vendors->cellValue(row, 0).toId();
	QDate date = _vendors->cellValue(row, 1).toDate();
	fixed amount = _vendors->cellValue(row, 2).toFixed();
	if (vendor_id == INVALID_ID || amount == 0.0) continue;

	Vendor vendor;
	_quasar->db()->lookup(vendor_id, vendor);
	Id vendor_acct = vendor.accountId();

	CardAdjust adjustment;
	adjustment.setPostDate(date);
	adjustment.setPostTime(QTime(0, 0, 0));
	adjustment.setMemo(tr("Open Balance"));
	adjustment.setStationId(station_id);
	adjustment.setEmployeeId(employee_id);
	adjustment.setCardId(vendor_id);
	adjustment.setStoreId(store_id);

	adjustment.accounts().push_back(AccountLine(vendor_acct, -amount));
	adjustment.accounts().push_back(AccountLine(history_id, amount));
	adjustment.cards().push_back(CardLine(vendor_id, amount));

	if (!_quasar->db()->create(adjustment)) {
	    QString message = tr("Vendor '%1' open balance failed")
		.arg(vendor.name());
	    QMessageBox::critical(this, tr("Error"), message);
	}
    }

    for (int row = 0; row < _items->rows(); ++row) {
	Id item_id = _items->cellValue(row, 0).toId();
	QString number = _items->cellValue(row, 0).toPlu().number();
	QString size = _items->cellValue(row, 2).toString();
	fixed qty = _items->cellValue(row, 3).toFixed();
	fixed cost = _items->cellValue(row, 4).toFixed();
	if (item_id == INVALID_ID || (qty == 0.0 && cost == 0.0)) continue;

	Item item;
	_quasar->db()->lookup(item_id, item);
	Id item_acct = item.assetAccount();

	ItemAdjust adjustment;
	adjustment.setPostDate(QDate::currentDate());
	adjustment.setPostTime(QTime::currentTime());
	adjustment.setMemo(tr("Open Balance"));
	adjustment.setStationId(station_id);
	adjustment.setEmployeeId(employee_id);
	adjustment.setStoreId(store_id);
	adjustment.setAccountId(history_id);

	adjustment.accounts().push_back(AccountLine(item_acct, cost));
	adjustment.accounts().push_back(AccountLine(history_id, -cost));

	ItemLine line;
	line.item_id = item.id();
	line.number = number;
	line.size = size;
	line.size_qty = item.sizeQty(size);
	line.quantity = qty;
	line.inv_cost = cost;
	adjustment.items().push_back(line);

	if (!_quasar->db()->create(adjustment)) {
	    QString message = tr("Item '%1' open balance failed")
		.arg(item.number());
	    QMessageBox::critical(this, tr("Error"), message);
	}
    }

    for (int row = 0; row < _accounts->rows(); ++row) {
	Id account_id = _accounts->cellValue(row, 0).toId();
	fixed debit = _accounts->cellValue(row, 1).toFixed();
	fixed credit = _accounts->cellValue(row, 2).toFixed();
	if (account_id == INVALID_ID || (debit == 0.0 && credit == 0))
		continue;

	Account account;
	_quasar->db()->lookup(account_id, account);

	General general;
	general.setPostDate(QDate::currentDate());
	general.setPostTime(QTime::currentTime());
	general.setMemo(tr("Open Balance"));
	general.setStationId(station_id);
	general.setEmployeeId(employee_id);
	general.setStoreId(store_id);

	if (debit != 0.0) {
	    general.accounts().push_back(AccountLine(account_id, debit));
	    general.accounts().push_back(AccountLine(history_id, -debit));
	}
	if (credit != 0.0) {
	    general.accounts().push_back(AccountLine(account_id, -credit));
	    general.accounts().push_back(AccountLine(history_id, credit));
	}

	if (!_quasar->db()->create(general)) {
	    QString message = tr("Account '%1' open balance failed")
		.arg(account.name());
	    QMessageBox::critical(this, tr("Error"), message);
	}
    }

    QApplication::restoreOverrideCursor();

    close();
}

void
OpenBalances::customerCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _customers->rows() - 1 && col == 0) {
	Id customer_id = _customers->cellValue(row, col).toId();
	if (customer_id != INVALID_ID)
	    _customers->appendRow(new VectorRow(_customers->columns()));
    }

    switch (col) {
    case 0:
	Id customer_id = _customers->cellValue(row, 0).toId();
	QDate date = _customers->cellValue(row, 1).toDate();
	if (customer_id != INVALID_ID && date.isNull())
	    _customers->setCellValue(row, 1, QDate::currentDate());
	break;
    }

    recalculate();
}

void
OpenBalances::customerFocusNext(bool& leave, int&, int& newCol,int type)
{
    int row = _customers->currentRow();
    int col = _customers->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id customer_id = _customers->cellValue(row, col).toId();
	if (customer_id == INVALID_ID && row == _customers->rows() - 1)
	    leave = true;
	else
	    newCol = 2;
    }
}

void
OpenBalances::vendorCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _vendors->rows() - 1 && col == 0) {
	Id vendor_id = _vendors->cellValue(row, col).toId();
	if (vendor_id != INVALID_ID)
	    _vendors->appendRow(new VectorRow(_vendors->columns()));
    }

    switch (col) {
    case 0:
	Id vendor_id = _vendors->cellValue(row, 0).toId();
	QDate date = _vendors->cellValue(row, 1).toDate();
	if (vendor_id != INVALID_ID && date.isNull())
	    _vendors->setCellValue(row, 1, QDate::currentDate());
	break;
    }

    recalculate();
}

void
OpenBalances::vendorFocusNext(bool& leave, int&, int& newCol,int type)
{
    int row = _vendors->currentRow();
    int col = _vendors->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id vendor_id = _vendors->cellValue(row, col).toId();
	if (vendor_id == INVALID_ID && row == _vendors->rows() - 1)
	    leave = true;
	else
	    newCol = 2;
    }
}

void
OpenBalances::itemCellMoved(int row, int)
{
    if (row == _items->currentRow() && _size->count() > 0)
	return;

    _size->clear();
    if (row == -1) return;

    Id item_id = _items->cellValue(row, 0).toId();
    QString number = _items->cellValue(row, 0).toPlu().number();
    QString size=  _items->cellValue(row, 2).toString();

    Item item;
    _quasar->db()->lookup(item_id, item);

    for (unsigned int i = 0; i < item.sizes().size(); ++i) {
	_size->insertItem(item.sizes()[i].name);
	if (item.sizes()[i].name == size)
	    _size->setCurrentItem(_size->count() - 1);
    }
}

void
OpenBalances::itemCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _items->rows() - 1 && col == 0) {
	Id item_id = _items->cellValue(row, col).toId();
	if (item_id != INVALID_ID)
	    _items->appendRow(new VectorRow(_items->columns()));
    }

    switch (col) {
    case 0: // item_id
    {
	Id item_id = _items->cellValue(row, 0).toId();

	Item item;
	_quasar->db()->lookup(item_id, item);
	QString size = item.sellSize();

	_size->clear();
	for (unsigned int i = 0; i < item.sizes().size(); ++i) {
	    _size->insertItem(item.sizes()[i].name);
	    if (item.sizes()[i].name == size)
		_size->setCurrentItem(_size->count() - 1);
	}

	_items->setCellValue(row, 1, item.description());
	_items->setCellValue(row, 2, size);
	_items->setCellValue(row, 3, "");
	_items->setCellValue(row, 4, "");
	break;
    }
    }

    recalculate();
}

void
OpenBalances::itemFocusNext(bool& leave, int&, int& newCol,int type)
{
    int row = _items->currentRow();
    int col = _items->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id item_id = _items->cellValue(row, col).toId();
	if (item_id == INVALID_ID && row == _items->rows() - 1)
	    leave = true;
	else
	    newCol = 2;
    } else if (type == Table::MovePrev && col == 2) {
	newCol = 0;
    }
}

void
OpenBalances::accountCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _accounts->rows() - 1 && col == 0) {
	Id account_id = _accounts->cellValue(row, col).toId();
	if (account_id != INVALID_ID)
	    _accounts->appendRow(new VectorRow(_accounts->columns()));
    }

    recalculate();
}

void
OpenBalances::accountFocusNext(bool& leave, int& newRow, int& newCol,int type)
{
    int row = _accounts->currentRow();
    int col = _accounts->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Id account_id = _accounts->cellValue(row, col).toId();
	fixed debit = _accounts->cellValue(row, 1).toFixed();
	fixed credit = _accounts->cellValue(row, 2).toFixed();
	if (account_id == INVALID_ID && row == _accounts->rows() - 1)
	    leave = true;
	else if (debit == 0.0 && credit != 0.0)
	    newCol = 2;
    } else if (type == Table::MoveNext && col == 1) {
	fixed debit = _accounts->cellValue(row, 1).toFixed();
	if (debit != 0.0) {
	    newRow = QMIN(row + 1, _accounts->rows() - 1);
	    newCol = 0;
	}
    } else if (type == Table::MovePrev && col == 0 && row > 0) {
	fixed debit = _accounts->cellValue(row - 1, 1).toFixed();
	newRow = row - 1;
	if (debit != 0.0)
	    newCol = 1;
	else
	    newCol = 2;
    } else if (type == Table::MovePrev && col == 2) {
	fixed debit = _accounts->cellValue(row, 1).toFixed();
	fixed credit = _accounts->cellValue(row, 2).toFixed();
	newRow = row;
	if (debit != 0.0 || credit == 0.0)
	    newCol = 1;
	else
	    newCol = 0;
    }
}

void
OpenBalances::recalculate()
{
    fixed total = 0.0;
    int row;

    for (row = 0; row < _customers->rows(); ++row) {
	Id customer_id = _customers->cellValue(row, 0).toId();
	fixed amount = _customers->cellValue(row, 2).toFixed();
	if (customer_id == INVALID_ID) continue;

	total += amount;
    }

    for (row = 0; row < _vendors->rows(); ++row) {
	Id vendor_id = _vendors->cellValue(row, 0).toId();
	fixed amount = _vendors->cellValue(row, 2).toFixed();
	if (vendor_id == INVALID_ID) continue;

	total -= amount;
    }

    for (row = 0; row < _items->rows(); ++row) {
	Id item_id = _items->cellValue(row, 0).toId();
	fixed amount = _items->cellValue(row, 4).toFixed();
	if (item_id == INVALID_ID) continue;

	total += amount;
    }

    for (row = 0; row < _accounts->rows(); ++row) {
	Id account_id = _accounts->cellValue(row, 0).toId();
	fixed debit = _accounts->cellValue(row, 1).toFixed();
	fixed credit = _accounts->cellValue(row, 2).toFixed();
	if (account_id == INVALID_ID) continue;

	total += debit;
	total -= credit;
    }

    _total->setFixed(-total);
}
