// $Id: address_book.cpp,v 1.60 2005/01/30 04:25:31 bpepers Exp $
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

#include "address_book.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "group.h"
#include "card.h"
#include "card_select.h"
#include "customer_master.h"
#include "employee_master.h"
#include "personal_master.h"
#include "vendor_master.h"
#include "inquiry.h"
#include "find_tx.h"
#include "sales_report.h"
#include "purchase_report.h"
#include "receipt_master.h"
#include "withdraw_master.h"
#include "card_allocate.h"
#include "cheque_customer.h"
#include "cheque_vendor.h"
#include "aged_receivables.h"
#include "aged_payables.h"
#include "send_email.h"
#include "line_edit.h"
#include "lookup_edit.h"
#include "store_lookup.h"
#include "group_lookup.h"
#include "grid.h"

#include <assert.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qcheckbox.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qmessagebox.h>

AddressBook::AddressBook(MainWindow* main)
    : ActiveList(main, "AddressBook", true)
{
    _helpSource = "address_book.html";

    // Columns for all tab
    _list->addTextColumn(tr("Name"), 30);
    _list->addTextColumn(tr("Number"), 12);
    _list->addTextColumn(tr("Phone Number"), 20);
    _list->addTextColumn(tr("Type"), 14);
    _list->setSorting(0);

    // Catch changing tabs so can disable buttons
    connect(_tabs, SIGNAL(currentChanged(QWidget*)),
	    SLOT(tabChanged(QWidget*)));

    // List for customers tab
    if (_quasar->securityCheck("CustomerMaster", "View")) {
	_customer = new ListView(_tabs);
	_tabs->addTab(_customer, tr("&Customers"));
	_customer->setAllColumnsShowFocus(true);
	_customer->setRootIsDecorated(false);
	_customer->setShowSortIndicator(true);
	_customer->addTextColumn(tr("Name"), 30);
	_customer->addTextColumn(tr("Number"), 12);
	_customer->addTextColumn(tr("Phone Number"), 20);
	_customer->addMoneyColumn(tr("Balance"));
	_customer->setSorting(0);
	connectList(_customer);
    } else {
	_customer = NULL;
    }

    // List for vendors tab
    if (_quasar->securityCheck("VendorMaster", "View")) {
	_vendor = new ListView(_tabs);
	_tabs->addTab(_vendor, tr("&Vendors"));
	_vendor->setAllColumnsShowFocus(true);
	_vendor->setRootIsDecorated(false);
	_vendor->setShowSortIndicator(true);
	_vendor->addTextColumn(tr("Name"), 30);
	_vendor->addTextColumn(tr("Number"), 12);
	_vendor->addTextColumn(tr("Phone Number"), 20);
	_vendor->addMoneyColumn(tr("Balance"));
	_vendor->setSorting(0);
	connectList(_vendor);
    } else {
	_vendor = NULL;
    }

    // List for employees tab
    if (_quasar->securityCheck("EmployeeMaster", "View")) {
	_employee = new ListView(_tabs);
	_tabs->addTab(_employee, tr("&Employees"));
	_employee->setAllColumnsShowFocus(true);
	_employee->setRootIsDecorated(false);
	_employee->setShowSortIndicator(true);
	_employee->addTextColumn(tr("Name"), 30);
	_employee->addTextColumn(tr("Number"), 12);
	_employee->addTextColumn(tr("Phone Number"), 20);
	_employee->setSorting(0);
	connectList(_employee);
    } else {
	_employee = NULL;
    }

    // List for personal tab
    if (_quasar->securityCheck("PersonalMaster", "View")) {
	_personal = new ListView(_tabs);
	_tabs->addTab(_personal, tr("&Personal"));
	_personal->setAllColumnsShowFocus(true);
	_personal->setRootIsDecorated(false);
	_personal->setShowSortIndicator(true);
	_personal->addTextColumn(tr("Name"), 30);
	_personal->addTextColumn(tr("Phone Number"), 20);
	_personal->setSorting(0);
	connectList(_personal);
    } else {
	_personal = NULL;
    }

    QLabel* nameLabel = new QLabel(tr("Name:"), _search);
    _name = new LineEdit(_search);
    _name->setLength(40);
    connect(_name, SIGNAL(returnPressed()), SLOT(slotRefresh()));

    QLabel* numberLabel = new QLabel(tr("Number:"), _search);
    _number = new LineEdit(_search);
    _number->setLength(12);
    connect(_number, SIGNAL(returnPressed()), SLOT(slotRefresh()));

    QLabel* groupLabel = new QLabel(tr("Group:"), _search);
    _groupLookup = new GroupLookup(_main, this, -1);
    _group = new LookupEdit(_groupLookup, _search);
    _group->setLength(15);
    groupLabel->setBuddy(_group);

    QLabel* storeLabel = new QLabel(tr("Store:"), _search);
    _store = new LookupEdit(new StoreLookup(main, this), _search);
    _store->setLength(30);

    QGridLayout* grid = new QGridLayout(_search);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(2, 1);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(numberLabel, 1, 0);
    grid->addWidget(_number, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(groupLabel, 2, 0);
    grid->addWidget(_group, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(storeLabel, 3, 0);
    grid->addWidget(_store, 3, 1, AlignLeft | AlignVCenter);

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    setCaption(tr("Address Book"));
    finalize(false);
    _name->setFocus();
}

AddressBook::~AddressBook()
{
}

void
AddressBook::slotActivities()
{
    ActiveList::slotActivities();
    _activities->insertItem(tr("Send Email"), this, SLOT(slotEmail()));

    Card card;
    if (!_quasar->db()->lookup(currentId(), card)) return;
    if (card.dataType() == DataObject::PERSONAL) return;
    if (card.dataType() == DataObject::EMPLOYEE) return;

    _activities->insertItem(tr("Inquiry"), this, SLOT(slotInquiry()));
    _activities->insertItem(tr("Find Tx"), this, SLOT(slotFindTx()));
    if (card.dataType() == DataObject::CUSTOMER) {
	_activities->insertItem(tr("Aged List"), this, SLOT(slotAgedAR()));
	_activities->insertItem(tr("Sales"), this, SLOT(slotSales()));
	_activities->insertItem(tr("Payment"), this, SLOT(slotReceipt()));
	_activities->insertItem(tr("Withdraw"), this, SLOT(slotWithdraw()));
	_activities->insertItem(tr("Write Cheque"), this, SLOT(slotCheque()));
	_activities->insertItem(tr("Allocate"), this, SLOT(slotAllocate()));
    }
    if (card.dataType() == DataObject::VENDOR) {
	_activities->insertItem(tr("Aged List"), this, SLOT(slotAgedAP()));
	_activities->insertItem(tr("Purchases"), this, SLOT(slotPurchases()));
	_activities->insertItem(tr("Payment"), this, SLOT(slotPayment()));
	_activities->insertItem(tr("Allocate"), this, SLOT(slotAllocate()));
    }
}

void
AddressBook::addToPopup(QPopupMenu* menu)
{
    ActiveList::addToPopup(menu);
    menu->insertItem(tr("Send Email"), this, SLOT(slotEmail()));

    Card card;
    if (!_quasar->db()->lookup(currentId(), card)) return;
    if (card.dataType() == DataObject::PERSONAL) return;
    if (card.dataType() == DataObject::EMPLOYEE) return;

    menu->insertItem(tr("Inquiry"), this, SLOT(slotInquiry()));
    menu->insertItem(tr("Find Tx"), this, SLOT(slotFindTx()));
    if (card.dataType() == DataObject::CUSTOMER) {
	menu->insertItem(tr("Aged List"), this, SLOT(slotAgedAR()));
	menu->insertItem(tr("Sales"), this, SLOT(slotSales()));
	menu->insertItem(tr("Payment"), this, SLOT(slotReceipt()));
	menu->insertItem(tr("Withdraw"), this, SLOT(slotWithdraw()));
	menu->insertItem(tr("Write Cheque"), this, SLOT(slotCheque()));
	menu->insertItem(tr("Allocate"), this, SLOT(slotAllocate()));
    }
    if (card.dataType() == DataObject::VENDOR) {
	menu->insertItem(tr("Aged List"), this, SLOT(slotAgedAP()));
	menu->insertItem(tr("Purchases"), this, SLOT(slotPurchases()));
	menu->insertItem(tr("Payment"), this, SLOT(slotPayment()));
	menu->insertItem(tr("Allocate"), this, SLOT(slotAllocate()));
    }
}

void
AddressBook::slotInquiry()
{
    Id card_id = currentId();
    if (card_id == INVALID_ID) return;

    Inquiry* window = (Inquiry*)(_main->findWindow("Inquiry"));
    if (window == NULL) {
	window = new Inquiry(_main);
	window->show();
    }
    window->setStoreId(_store->getId());
    window->setCardId(card_id);
}

void
AddressBook::slotFindTx()
{
    Id card_id = currentId();
    if (card_id == INVALID_ID) return;

    FindTx* window = (FindTx*)(_main->findWindow("FindTx"));
    if (window == NULL) {
	window = new FindTx(_main);
	window->show();
    }
    window->setStoreId(_store->getId());
    window->setCardId(card_id);
}

void
AddressBook::slotAgedAR()
{
    Id card_id = currentId();
    if (card_id == INVALID_ID) return;

    QString type = "AgedReceivables";
    AgedReceivables* window = (AgedReceivables*)(_main->findWindow(type));
    if (window == NULL) {
	window = new AgedReceivables(_main);
	window->show();
    }
    window->setStoreId(_store->getId());
    window->setCustomerId(card_id);
}

void
AddressBook::slotSales()
{
    Id card_id = currentId();
    if (card_id == INVALID_ID) return;

    QString type = "SalesReport";
    SalesReport* window = (SalesReport*)(_main->findWindow(type));
    if (window == NULL) {
	window = new SalesReport(_main);
	window->show();
    }
    window->setStoreId(_store->getId());
    window->setCardId(card_id);
}

void
AddressBook::slotReceipt()
{
    QString type = "CustomerPayment";
    ReceiptMaster* window = (ReceiptMaster*)(_main->findWindow(type));
    if (window == NULL) {
	window = new ReceiptMaster(_main);
	window->show();
    }

    window->setStoreId(_store->getId());
    window->setCardId(currentId());
}

void
AddressBook::slotWithdraw()
{
    QString type = "CustomerWithdraw";
    WithdrawMaster* window = (WithdrawMaster*)(_main->findWindow(type));
    if (window == NULL) {
	window = new WithdrawMaster(_main);
	window->show();
    }

    window->setStoreId(_store->getId());
    window->setCardId(currentId());
}

void
AddressBook::slotCheque()
{
    QString type = "ChequeCustomer";
    ChequeCustomer* window = (ChequeCustomer*)(_main->findWindow(type));
    if (window == NULL) {
	window = new ChequeCustomer(_main);
	window->show();
    }

    window->setStoreId(_store->getId());
    window->setCardId(currentId());
}

void
AddressBook::slotAgedAP()
{
    Id card_id = currentId();
    if (card_id == INVALID_ID) return;

    QString type = "AgedPayables";
    AgedPayables* window = (AgedPayables*)(_main->findWindow(type));
    if (window == NULL) {
	window = new AgedPayables(_main);
	window->show();
    }
    window->setStoreId(_store->getId());
    window->setVendorId(card_id);
}

void
AddressBook::slotPurchases()
{
    Id card_id = currentId();
    if (card_id == INVALID_ID) return;

    QString type = "PurchaseReport";
    PurchaseReport* window = (PurchaseReport*)(_main->findWindow(type));
    if (window == NULL) {
	window = new PurchaseReport(_main);
	window->show();
    }
    window->setStoreId(_store->getId());
    window->setCardId(card_id);
}

void
AddressBook::slotPayment()
{
    QString type = "VendorPayment";
    ChequeVendor* window = (ChequeVendor*)(_main->findWindow(type));
    if (window == NULL) {
	window = new ChequeVendor(_main);
	window->show();
    }

    window->setStoreId(_store->getId());
    window->setCardId(currentId());
}

void
AddressBook::slotEmail()
{
    SendEmail* window = new SendEmail(_main);
    window->show();

    Id card_id = currentId();
    if (card_id != INVALID_ID)
	window->setCard(card_id);
}

void
AddressBook::slotAllocate()
{
    QString type = "CardAllocate";
    CardAllocate* window = (CardAllocate*)(_main->findWindow(type));
    if (window == NULL) {
	window = new CardAllocate(_main);
	window->show();
    }

    window->setCardId(currentId());
}

bool
AddressBook::isActive(Id card_id)
{
    Card card;
    _quasar->db()->lookup(card_id, card);
    return card.isActive();
}

void
AddressBook::setActive(Id card_id, bool active)
{
    if (card_id == INVALID_ID) return;

    if (!active) {
	fixed balance = _quasar->db()->cardBalance(card_id);
	if (balance != 0.0) {
	    QString message = tr("This card still has a balance.  It must "
				 "have\na zero balance before it can be "
				 "made inactive.");
	    QMessageBox::warning(this, tr("Error"), message, QMessageBox::Ok,
				 0);
	    return;
	}
    }

    Card card;
    _quasar->db()->lookup(card_id, card);

    switch (card.dataType()) {
    case DataObject::CUSTOMER:
    {
	Customer customer;
	_quasar->db()->lookup(card_id, customer);
	Customer orig = customer;
	_quasar->db()->setActive(customer, active);
	_quasar->db()->update(orig, customer);
	break;
    }
    case DataObject::EMPLOYEE:
    {
	Employee employee;
	_quasar->db()->lookup(card_id, employee);
	Employee orig = employee;
	_quasar->db()->setActive(employee, active);
	_quasar->db()->update(orig, employee);
	break;
    }
    case DataObject::PERSONAL:
    {
	Personal personal;
	_quasar->db()->lookup(card_id, personal);
	Personal orig = personal;
	_quasar->db()->setActive(personal, active);
	_quasar->db()->update(orig, personal);
	break;
    }
    case DataObject::VENDOR:
    {
	Vendor vendor;
	_quasar->db()->lookup(card_id, vendor);
	Vendor orig = vendor;
	_quasar->db()->setActive(vendor, active);
	_quasar->db()->update(orig, vendor);
	break;
    }
    default:
	assert(0);
    }
}

void
AddressBook::tabChanged(QWidget* widget)
{
    if (_new != NULL)
	_new->setEnabled(widget != _list);

    if (widget == _customer)
	_groupLookup->type->setCurrentItem(Group::typeName(Group::CUSTOMER));
    else if (widget == _employee)
	_groupLookup->type->setCurrentItem(Group::typeName(Group::EMPLOYEE));
    else if (widget == _vendor)
	_groupLookup->type->setCurrentItem(Group::typeName(Group::VENDOR));
    else if (widget == _personal)
	_groupLookup->type->setCurrentItem(Group::typeName(Group::PERSONAL));
    else
	_groupLookup->type->setCurrentItem(tr("All Card Types"));

    _group->setText("");
}

void
AddressBook::clearLists()
{
    _list->clear();
    if (_employee != NULL) _employee->clear();
    if (_customer != NULL) _customer->clear();
    if (_vendor != NULL) _vendor->clear();
    if (_personal != NULL) _personal->clear();
}

void
AddressBook::slotInactiveChanged()
{
    clearLists();

    int columns = _list->columns();
    if (_inactive->isChecked()) {
	_list->addCheckColumn(tr("Inactive?"));
	if (_employee != NULL)
	    _employee->addCheckColumn(tr("Inactive?"));
	if (_customer != NULL)
	    _customer->addCheckColumn(tr("Inactive?"));
	if (_personal != NULL)
	    _personal->addCheckColumn(tr("Inactive?"));
	if (_vendor != NULL)
	    _vendor->addCheckColumn(tr("Inactive?"));
	resize(width() + _list->columnWidth(columns), height());
    } else {
	int column = _list->findColumn(tr("Inactive?"));
	if (column != -1) {
	    resize(width() - _list->columnWidth(column), height());
	    _list->removeColumn(column);
	    if (_employee != NULL)
		_employee->removeColumn(_employee->findColumn(tr("Inactive?")));
	    if (_customer != NULL)
		_customer->removeColumn(_customer->findColumn(tr("Inactive?")));
	    if (_personal != NULL)
		_personal->removeColumn(_personal->findColumn(tr("Inactive?")));
	    if (_vendor != NULL)
		_vendor->removeColumn(_vendor->findColumn(tr("Inactive?")));
	}
    }

    slotRefresh();
}

void
AddressBook::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    int columns = _list->columns();
    if (showInactive && columns != 5) {
	_list->addCheckColumn(tr("Inactive?"));
	if (_customer != NULL)
	    _customer->addCheckColumn(tr("Inactive?"));
	if (_vendor != NULL)
	    _vendor->addCheckColumn(tr("Inactive?"));
	if (_employee != NULL)
	    _employee->addCheckColumn(tr("Inactive?"));
	if (_personal != NULL)
	    _personal->addCheckColumn(tr("Inactive?"));
	resize(width() + _list->columnWidth(4), height());
    }
    if (!showInactive && columns != 4) {
	resize(width() - _list->columnWidth(4), height());
	_list->removeColumn(4);
	if (_customer != NULL)
	    _customer->removeColumn(4);
	if (_vendor != NULL)
	    _vendor->removeColumn(4);
	if (_employee != NULL)
	    _employee->removeColumn(3);
	if (_personal != NULL)
	    _personal->removeColumn(3);
    }

    Id allId = currentId();
    Id customerId = currentId(_customer);
    Id employeeId = currentId(_employee);
    Id vendorId = currentId(_vendor);
    Id personalId = currentId(_personal);
    QListViewItem* allItem = NULL;
    QListViewItem* customerItem = NULL;
    QListViewItem* employeeItem = NULL;
    QListViewItem* vendorItem = NULL;
    QListViewItem* personalItem = NULL;
    clearLists();

    QString searchName = _name->text();
    QString searchNumber = _number->text();
    Id searchGroup = _group->getId();

    CardSelect conditions;
    conditions.activeOnly = !showInactive;
    if (!searchName.isEmpty())
	conditions.name = "%" + searchName + "%";
    if (!searchNumber.isEmpty())
	conditions.number = searchNumber;
    ListView* list = currentList();
    if (list == _customer)
	conditions.type = DataObject::CUSTOMER;
    else if (list == _employee)
	conditions.type = DataObject::EMPLOYEE;
    else if (list == _personal)
	conditions.type = DataObject::PERSONAL;
    else if (list == _vendor)
	conditions.type = DataObject::VENDOR;
    conditions.group_id = searchGroup;

    int count;
    _quasar->db()->count(count, conditions);
    if (count > 100) {
	setEnabled(true);
	QApplication::restoreOverrideCursor();

	QString message = tr("This will select %1 cards\n"
			     "which may be slow.  Continue?").arg(count);
	int ch = QMessageBox::warning(this, tr("Are You Sure?"), message,
				      QMessageBox::Yes, QMessageBox::No);
	if (ch != QMessageBox::Yes) return;

	setEnabled(false);
	QApplication::setOverrideCursor(waitCursor);
	qApp->processEvents();
    }

    vector<Card> cards;
    _quasar->db()->select(cards, conditions);
    _quasar->db()->cardBalances(QDate::currentDate(), _store->getId(),
				_cardIds, _balances);

    for (unsigned int i = 0; i < cards.size(); ++i) {
	Card& card = cards[i];
	fixed balance = 0.0;
	int type = card.dataType();

	if (type == DataObject::CUSTOMER || type == DataObject::VENDOR) {
	    for (unsigned int j = 0; j < _cardIds.size(); ++j) {
		if (_cardIds[j] == card.id()) {
		    balance = _balances[j];
		    break;
		}
	    }
	}

	if (type == DataObject::CUSTOMER && _customer == NULL)
	    continue;
	if (type == DataObject::VENDOR && _vendor == NULL)
	    continue;
	if (type == DataObject::EMPLOYEE && _employee == NULL)
	    continue;
	if (type == DataObject::PERSONAL && _personal == NULL)
	    continue;

	ListViewItem* lvi = new ListViewItem(_list, card.id());
	lvi->setValue(0, card.name());
	lvi->setValue(1, card.number());
	lvi->setValue(2, card.phoneNumber());
	lvi->setValue(3, card.dataTypeName());
	if (showInactive) lvi->setValue(4, !card.isActive());
	if (card.id() == allId) allItem = lvi;

	switch (type) {
	case DataObject::EMPLOYEE:
	    if (_employee != NULL) {
		lvi = new ListViewItem(_employee, card.id());
		lvi->setValue(0, card.name());
		lvi->setValue(1, card.number());
		lvi->setValue(2, card.phoneNumber());
		if (showInactive) lvi->setValue(3, !card.isActive());
		if (card.id() == employeeId) employeeItem = lvi;
	    }
	    break;
	case DataObject::CUSTOMER:
	    if (_customer != NULL) {
		lvi = new ListViewItem(_customer, card.id());
		lvi->setValue(0, card.name());
		lvi->setValue(1, card.number());
		lvi->setValue(2, card.phoneNumber());
		lvi->setValue(3, balance);
		if (showInactive) lvi->setValue(4, !card.isActive());
		if (card.id() == customerId) customerItem = lvi;
	    }
	    break;
	case DataObject::VENDOR:
	    if (_vendor != NULL) {
		lvi = new ListViewItem(_vendor, card.id());
		lvi->setValue(0, card.name());
		lvi->setValue(1, card.number());
		lvi->setValue(2, card.phoneNumber());
		lvi->setValue(3, balance);
		if (showInactive) lvi->setValue(4, !card.isActive());
		if (card.id() == vendorId) vendorItem = lvi;
	    }
	    break;
	case DataObject::PERSONAL:
	    if (_personal != NULL) {
		lvi = new ListViewItem(_personal, card.id());
		lvi->setValue(0, card.name());
		lvi->setValue(1, card.phoneNumber());
		if (showInactive) lvi->setValue(2, !card.isActive());
		if (card.id() == personalId) personalItem = lvi;
	    }
	    break;
	default:
	    assert(false);
	}
    }

    if (allItem == NULL)
	allItem = _list->firstChild();
    if (customerItem == NULL && _customer != NULL)
	customerItem = _customer->firstChild();
    if (employeeItem == NULL && _employee != NULL)
	employeeItem = _employee->firstChild();
    if (vendorItem == NULL && _vendor != NULL)
	vendorItem = _vendor->firstChild();
    if (personalItem == NULL && _personal != NULL)
	personalItem = _personal->firstChild();

    _list->setCurrentItem(allItem);
    _list->setSelected(allItem, true);
    if (_customer != NULL) {
	_customer->setCurrentItem(customerItem);
	_customer->setSelected(customerItem, true);
    }
    if (_employee != NULL) {
	_employee->setCurrentItem(employeeItem);
	_employee->setSelected(employeeItem, true);
    }
    if (_vendor != NULL) {
	_vendor->setCurrentItem(vendorItem);
	_vendor->setSelected(vendorItem, true);
    }
    if (_personal != NULL) {
	_personal->setCurrentItem(personalItem);
	_personal->setSelected(personalItem, true);
    }
}

void
AddressBook::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    ListView* list = currentList();
    QString title = tr("Address Book");
    if (list == _customer) title = tr("Customers");
    if (list == _vendor) title = tr("Vendors");
    if (list == _employee) title = tr("Employees");
    if (list == _personal) title = tr("Personal");

    Grid* grid = Grid::buildGrid(list, title);

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
AddressBook::performNew()
{
    QWidget* window = NULL;

    ListView* list = currentList();
    if (list == _customer) {
	window = new CustomerMaster(_main);
    } else if (list == _employee) {
	window = new EmployeeMaster(_main);
    } else if (list == _personal) {
	window = new PersonalMaster(_main);
    } else if (list == _vendor) {
	window = new VendorMaster(_main);
    }

    if (window) window->show();
}

void
AddressBook::performEdit()
{
    Id card_id = currentId();
    Card card;
    if (!_quasar->db()->lookup(card_id, card))
	return;

    QWidget* window = NULL;
    switch (card.dataType()) {
    case DataObject::CUSTOMER:
	window = new CustomerMaster(_main, card_id);
	break;
    case DataObject::EMPLOYEE:
	window = new EmployeeMaster(_main, card_id);
	break;
    case DataObject::PERSONAL:
	window = new PersonalMaster(_main, card_id);
	break;
    case DataObject::VENDOR:
	window = new VendorMaster(_main, card_id);
	break;
    default:
	assert(false);
    }
    window->show();
}
