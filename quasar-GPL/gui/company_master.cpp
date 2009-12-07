// $Id: company_master.cpp,v 1.15 2005/03/13 22:09:17 bpepers Exp $
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

#include "company_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "company_select.h"
#include "money_edit.h"
#include "lookup_edit.h"
#include "store_lookup.h"
#include "station_lookup.h"
#include "employee_lookup.h"
#include "account_lookup.h"
#include "table.h"
#include "combo_box.h"
#include "date_popup.h"
#include "account.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qtabwidget.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qmessagebox.h>

CompanyMaster::CompanyMaster(MainWindow* main, Id company_id)
    : DataWindow(main, "CompanyMaster", company_id)
{
    _helpSource = "company_master.html";

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("&Name:"), _frame);
    _name = new LineEdit(30, _frame);
    nameLabel->setBuddy(_name);

    QLabel* numberLabel = new QLabel(tr("Number:"), _frame);
    _number = new LineEdit(5, _frame);
    numberLabel->setBuddy(_number);

    QTabWidget* tabs = new QTabWidget(_frame);
    QFrame* addr = new QFrame(tabs);
    QFrame* ledger = new QFrame(tabs);
    QFrame* accounts = new QFrame(tabs);
    QFrame* rounding = new QFrame(tabs);
    tabs->addTab(addr, tr("Address"));
    tabs->addTab(ledger, tr("Ledger"));
    tabs->addTab(accounts, tr("Accounts"));
    tabs->addTab(rounding, tr("Rounding"));

    QLabel* streetLabel = new QLabel(tr("Street:"), addr);
    _street = new LineEdit(addr);
    _street->setLength(60);
    streetLabel->setBuddy(_street);

    _street2 = new LineEdit(addr);
    _street2->setLength(60);

    QLabel* cityLabel = new QLabel(tr("City:"), addr);
    _city = new LineEdit(addr);
    _city->setLength(20);
    cityLabel->setBuddy(_city);

    QLabel* provLabel = new QLabel(tr("Prov/State:"), addr);
    _province = new LineEdit(addr);
    _province->setLength(20);
    provLabel->setBuddy(_province);

    QLabel* postalLabel = new QLabel(tr("Postal/Zip:"), addr);
    _postal = new LineEdit(addr);
    _postal->setLength(20);
    postalLabel->setBuddy(_postal);

    QLabel* countryLabel = new QLabel(tr("Country:"), addr);
    _country = new LineEdit(addr);
    _country->setLength(20);
    countryLabel->setBuddy(_country);

    QLabel* phoneLabel = new QLabel(tr("Phone #:"), addr);
    _phone_num = new LineEdit(20, addr);
    phoneLabel->setBuddy(_phone_num);

    QLabel* phone2Label = new QLabel(tr("Phone2 #:"), addr);
    _phone2_num = new LineEdit(20, addr);
    phone2Label->setBuddy(_phone2_num);

    QLabel* faxLabel = new QLabel(tr("Fax #:"), addr);
    _fax_num = new LineEdit(20, addr);
    faxLabel->setBuddy(_fax_num);

    QLabel* emailLabel = new QLabel(tr("Email:"), addr);
    _email = new LineEdit(addr);
    _email->setLength(60);
    emailLabel->setBuddy(_email);

    QLabel* webLabel = new QLabel(tr("Web Page:"), addr);
    _web_page = new LineEdit(addr);
    _web_page->setLength(60);
    webLabel->setBuddy(_web_page);

    QGridLayout* addrGrid = new QGridLayout(addr);
    addrGrid->setSpacing(3);
    addrGrid->setMargin(3);
    addrGrid->setRowStretch(8, 1);
    addrGrid->setColStretch(2, 1);
    addrGrid->addWidget(streetLabel, 0, 0);
    addrGrid->addMultiCellWidget(_street, 0, 0, 1, 4, AlignLeft|AlignVCenter);
    addrGrid->addMultiCellWidget(_street2, 1, 1, 1, 4, AlignLeft|AlignVCenter);
    addrGrid->addWidget(cityLabel, 2, 0);
    addrGrid->addWidget(_city, 2, 1, AlignLeft | AlignVCenter);
    addrGrid->addWidget(provLabel, 2, 3);
    addrGrid->addWidget(_province, 2, 4, AlignLeft | AlignVCenter);
    addrGrid->addWidget(postalLabel, 3, 0);
    addrGrid->addWidget(_postal, 3, 1, AlignLeft | AlignVCenter);
    addrGrid->addWidget(countryLabel, 3, 3);
    addrGrid->addWidget(_country, 3, 4, AlignLeft | AlignVCenter);
    addrGrid->addWidget(phoneLabel, 4, 0);
    addrGrid->addWidget(_phone_num, 4, 1, AlignLeft | AlignVCenter);
    addrGrid->addWidget(phone2Label, 4, 3);
    addrGrid->addWidget(_phone2_num, 4, 4, AlignLeft | AlignVCenter);
    addrGrid->addWidget(faxLabel, 5, 0);
    addrGrid->addWidget(_fax_num, 5, 1, AlignLeft | AlignVCenter);
    addrGrid->addWidget(emailLabel, 6, 0);
    addrGrid->addMultiCellWidget(_email, 6, 6, 1, 4, AlignLeft | AlignVCenter);
    addrGrid->addWidget(webLabel, 7, 0);
    addrGrid->addMultiCellWidget(_web_page, 7, 7, 1, 4,AlignLeft|AlignVCenter);

    QLabel* defaultStoreLabel = new QLabel(tr("Default Store:"), ledger);
    _defaultStore = new LookupEdit(new StoreLookup(_main, this), ledger);
    _defaultStore->setLength(30);
    defaultStoreLabel->setBuddy(_defaultStore);

    QLabel* startDateLabel = new QLabel(tr("Start of Fiscal Year:"), ledger);
    _startDate = new DatePopup(ledger);
    startDateLabel->setBuddy(_startDate);

    QLabel* closeDateLabel = new QLabel(tr("Date Ledger is Closed:"), ledger);
    _closeDate = new DatePopup(ledger);
    closeDateLabel->setBuddy(_closeDate);

    QLabel* shiftLabel = new QLabel(tr("Cash Reconcile Method:"), ledger);
    _shiftMethod = new ComboBox(ledger);
    _shiftMethod->insertItem(tr("Station"));
    _shiftMethod->insertItem(tr("Employee"));
    shiftLabel->setBuddy(_shiftMethod);
    connect(_shiftMethod, SIGNAL(activated(int)), SLOT(slotSetSafeWidget()));

    QLabel* safeStoreLabel = new QLabel(tr("Safe Store:"), ledger);
    _safeStore = new LookupEdit(new StoreLookup(_main, this), ledger);
    _safeStore->setLength(30);
    safeStoreLabel->setBuddy(_safeStore);

    _safeStationLabel = new QLabel(tr("Safe Station:"), ledger);
    _safeStation = new LookupEdit(new StationLookup(_main, this), ledger);
    _safeStation->setLength(30);
    _safeStationLabel->setBuddy(_safeStation);

    _safeEmployeeLabel = new QLabel(tr("Safe Employee:"), ledger);
    _safeEmployee = new LookupEdit(new EmployeeLookup(_main, this), ledger);
    _safeEmployee->setLength(30);
    _safeEmployeeLabel->setBuddy(_safeEmployee);

    QLabel* reAccountLabel = new QLabel(tr("Retained Earnings:"), ledger);
    AccountLookup* reLookup = new AccountLookup(main, this, Account::Equity);
    _reAccount = new LookupEdit(reLookup, ledger);
    _reAccount->setLength(30);
    reAccountLabel->setBuddy(_reAccount);

    QLabel* hbAccountLabel = new QLabel(tr("Historical Balancing:"), ledger);
    AccountLookup* hbLookup = new AccountLookup(main, this, Account::Equity);
    _hbAccount = new LookupEdit(hbLookup, ledger);
    _hbAccount->setLength(30);
    hbAccountLabel->setBuddy(_hbAccount);

    QLabel* splitAccountLabel = new QLabel(tr("Ledger Transfers:"), ledger);
    AccountLookup* splitLookup= new AccountLookup(main, this,Account::Expense);
    _splitAccount = new LookupEdit(splitLookup, ledger);
    _splitAccount->setLength(30);
    splitAccountLabel->setBuddy(_splitAccount);

    QGridLayout* ledgerGrid = new QGridLayout(ledger);
    ledgerGrid->setSpacing(3);
    ledgerGrid->setMargin(3);
    ledgerGrid->setRowStretch(9, 1);
    ledgerGrid->setColStretch(2, 1);
    ledgerGrid->addWidget(defaultStoreLabel, 0, 0);
    ledgerGrid->addWidget(_defaultStore, 0, 1, AlignLeft | AlignVCenter);
    ledgerGrid->addWidget(startDateLabel, 1, 0);
    ledgerGrid->addWidget(_startDate, 1, 1, AlignLeft | AlignVCenter);
    ledgerGrid->addWidget(closeDateLabel, 2, 0);
    ledgerGrid->addWidget(_closeDate, 2, 1, AlignLeft | AlignVCenter);
    ledgerGrid->addWidget(shiftLabel, 3, 0);
    ledgerGrid->addWidget(_shiftMethod, 3, 1, AlignLeft | AlignVCenter);
    ledgerGrid->addWidget(safeStoreLabel, 4, 0);
    ledgerGrid->addWidget(_safeStore, 4, 1, AlignLeft | AlignVCenter);
    ledgerGrid->addWidget(_safeStationLabel, 5, 0);
    ledgerGrid->addWidget(_safeStation, 5, 1, AlignLeft | AlignVCenter);
    ledgerGrid->addWidget(_safeEmployeeLabel, 5, 0);
    ledgerGrid->addWidget(_safeEmployee, 5, 1, AlignLeft | AlignVCenter);
    ledgerGrid->addWidget(reAccountLabel, 6, 0);
    ledgerGrid->addWidget(_reAccount, 6, 1, AlignLeft | AlignVCenter);
    ledgerGrid->addWidget(hbAccountLabel, 7, 0);
    ledgerGrid->addWidget(_hbAccount, 7, 1, AlignLeft | AlignVCenter);
    ledgerGrid->addWidget(splitAccountLabel, 8, 0);
    ledgerGrid->addWidget(_splitAccount, 8, 1, AlignLeft | AlignVCenter);

    QLabel* bankAccountLabel = new QLabel(tr("Default Bank:"), accounts);
    AccountLookup* bankLookup = new AccountLookup(main, this, Account::Bank);
    _bankAccount = new LookupEdit(bankLookup, accounts);
    _bankAccount->setLength(30);
    bankAccountLabel->setBuddy(_bankAccount);

    QLabel* customerAcctLabel= new QLabel(tr("Default Receivables:"),accounts);
    AccountLookup* customerLookup = new AccountLookup(main, this, Account::AR);
    _customerAcct = new LookupEdit(customerLookup, accounts);
    _customerAcct->setLength(30);
    customerAcctLabel->setBuddy(_customerAcct);

    QLabel* vendorAcctLabel = new QLabel(tr("Default Payables:"), accounts);
    AccountLookup* vendorLookup = new AccountLookup(main, this, Account::AP);
    _vendorAcct = new LookupEdit(vendorLookup, accounts);
    _vendorAcct->setLength(30);
    vendorAcctLabel->setBuddy(_vendorAcct);

    QLabel* custTermsAccountLabel = new QLabel(tr("Customer Terms Disc:"),
					       accounts);
    AccountLookup* custTermsLookup = new AccountLookup(main, this,
						       Account::Expense);
    _custTermsAcct = new LookupEdit(custTermsLookup, accounts);
    _custTermsAcct->setLength(30);
    custTermsAccountLabel->setBuddy(_custTermsAcct);

    QLabel* vendTermsAccountLabel = new QLabel(tr("Vendor Terms Disc:"),
					       accounts);
    AccountLookup* vendTermsLookup = new AccountLookup(main, this,
						       Account::OtherIncome);
    _vendTermsAcct = new LookupEdit(vendTermsLookup, accounts);
    _vendTermsAcct->setLength(30);
    vendTermsAccountLabel->setBuddy(_vendTermsAcct);

    QLabel* depositAccountLabel= new QLabel(tr("Container Deposit:"),accounts);
    AccountLookup* depositLookup = new AccountLookup(main, this,
						     Account::Inventory);
    _depositAccount=new LookupEdit(depositLookup, accounts);
    _depositAccount->setLength(30);
    depositAccountLabel->setBuddy(_depositAccount);

    QLabel* transferAccountLabel = new QLabel(tr("Transfers:"), accounts);
    AccountLookup* transferLookup = new AccountLookup(main, this,
					      Account::OtherCurLiability);
    _transferAccount=new LookupEdit(transferLookup, accounts);
    _transferAccount->setLength(30);
    transferAccountLabel->setBuddy(_transferAccount);

    QLabel* physicalLabel = new QLabel(tr("Physical Inventory:"), accounts);
    AccountLookup* physicalLookup = new AccountLookup(main, this);
    _physicalAccount = new LookupEdit(physicalLookup, accounts);
    _physicalAccount->setLength(30);
    physicalLabel->setBuddy(_physicalAccount);

    QLabel* chargeAccountLabel = new QLabel(tr("Service Charges:"), accounts);
    AccountLookup* chargeLookup = new AccountLookup(main, this,
						    Account::OtherIncome);
    _chargeAccount = new LookupEdit(chargeLookup, accounts);
    _chargeAccount->setLength(30);
    chargeAccountLabel->setBuddy(_chargeAccount);

    QLabel* overShortAccountLabel = new QLabel(tr("Tender over/short:"),
					       accounts);
    AccountLookup* overShortLookup = new AccountLookup(main, this,
						       Account::Expense);
    _overShortAccount = new LookupEdit(overShortLookup, accounts);
    _overShortAccount->setLength(30);
    overShortAccountLabel->setBuddy(_overShortAccount);

    QGridLayout* accountGrid = new QGridLayout(accounts);
    accountGrid->setSpacing(3);
    accountGrid->setMargin(3);
    accountGrid->setRowStretch(10, 1);
    accountGrid->setColStretch(1, 1);
    accountGrid->addWidget(bankAccountLabel, 0, 0);
    accountGrid->addWidget(_bankAccount, 0, 1, AlignLeft | AlignVCenter);
    accountGrid->addWidget(customerAcctLabel, 1, 0);
    accountGrid->addWidget(_customerAcct, 1, 1, AlignLeft | AlignVCenter);
    accountGrid->addWidget(vendorAcctLabel, 2, 0);
    accountGrid->addWidget(_vendorAcct, 2, 1, AlignLeft | AlignVCenter);
    accountGrid->addWidget(custTermsAccountLabel, 3, 0);
    accountGrid->addWidget(_custTermsAcct, 3, 1, AlignLeft | AlignVCenter);
    accountGrid->addWidget(vendTermsAccountLabel, 4, 0);
    accountGrid->addWidget(_vendTermsAcct, 4, 1, AlignLeft | AlignVCenter);
    accountGrid->addWidget(depositAccountLabel, 5, 0);
    accountGrid->addWidget(_depositAccount, 5, 1, AlignLeft | AlignVCenter);
    accountGrid->addWidget(transferAccountLabel, 6, 0);
    accountGrid->addWidget(_transferAccount, 6, 1, AlignLeft | AlignVCenter);
    accountGrid->addWidget(physicalLabel, 7, 0);
    accountGrid->addWidget(_physicalAccount, 7, 1, AlignLeft | AlignVCenter);
    accountGrid->addWidget(chargeAccountLabel, 8, 0);
    accountGrid->addWidget(_chargeAccount, 8, 1, AlignLeft | AlignVCenter);
    accountGrid->addWidget(overShortAccountLabel, 9, 0);
    accountGrid->addWidget(_overShortAccount, 9, 1, AlignLeft | AlignVCenter);


    _rounding = new Table(rounding);
    _rounding->setVScrollBarMode(QScrollView::AlwaysOn);
    _rounding->setDisplayRows(6);
    connect(_rounding, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(slotRoundingCellChanged(int,int,Variant)));

    // Add columns
    new TextColumn(_rounding, tr("Ends In"), 10);
    new MoneyColumn(_rounding, tr("Add On"));

    // Add editors
    new LineEditor(_rounding, 0, new LineEdit(10, _rounding));
    new NumberEditor(_rounding, 1, new MoneyEdit(_rounding));

    QGridLayout* roundGrid = new QGridLayout(rounding);
    roundGrid->setSpacing(3);
    roundGrid->setMargin(3);
    roundGrid->setRowStretch(0, 1);
    roundGrid->setColStretch(1, 1);
    roundGrid->addWidget(_rounding, 0, 0);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(2, 1);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(numberLabel, 1, 0);
    grid->addWidget(_number, 1, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(tabs, 2, 2, 0, 2);

    slotSetSafeWidget();

    setCaption(tr("Company Master"));
    finalize();
}

CompanyMaster::~CompanyMaster()
{
}

void
CompanyMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _name;
}

void
CompanyMaster::newItem()
{
    Company blank;
    _orig = blank;
    _orig.setNumber("#");

    _curr = _orig;
    _firstField = _name;
}

void
CompanyMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _curr.setNumber("#");
    //_curr.setYearEndTransferId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _name->selectAll();
}

bool
CompanyMaster::fileItem()
{
    // Check close date
    if (!_closeDate->getDate().isNull()) {
	// Check its before todays date
	if (_closeDate->getDate() >= QDate::currentDate()) {
	    QString message = tr("The close date marks the point where\n"
				 "you have closed your journal and don't\n"
				 "allow any further changes.  It has to\n"
				 "be set to a date before today.");
	    QMessageBox::critical(this, tr("Error"), message);
	    return false;
	}
    }

    QString number = _curr.number();
    if (number != "" && _orig.number() != number) {
	vector<Company> companies;
	CompanySelect conditions;
	conditions.number = number;
	_quasar->db()->select(companies, conditions);
	if (companies.size() != 0) {
	    QString message = tr("This company number is already used\n"
		"for another company. Are you sure\n"
		"you want to file this company?");
	    int ch = QMessageBox::warning(this, tr("Warning"), message,
					  QMessageBox::No, QMessageBox::Yes);
	    if (ch != QMessageBox::Yes)
		return false;
	}
    }

    if (_orig.id() == INVALID_ID) {
	if (!_quasar->db()->create(_curr)) return false;
    } else {
	if (!_quasar->db()->update(_orig, _curr)) return false;
    }

    // TODO: ask to clone locations and item info from another company

    _orig = _curr;
    _id = _curr.id();

    return true;
}

bool
CompanyMaster::deleteItem()
{
    bool ok = _quasar->db()->remove(_curr);
    return ok;
}

void
CompanyMaster::restoreItem()
{
    _curr = _orig;
}

void
CompanyMaster::cloneItem()
{
    CompanyMaster* clone = new CompanyMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
CompanyMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
CompanyMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _number->setText(_curr.number());
    _street->setText(_curr.street());
    _street2->setText(_curr.street2());
    _city->setText(_curr.city());
    _province->setText(_curr.province());
    _country->setText(_curr.country());
    _postal->setText(_curr.postal());
    _phone_num->setText(_curr.phoneNumber());
    _phone2_num->setText(_curr.phone2Number());
    _fax_num->setText(_curr.faxNumber());
    _email->setText(_curr.email());
    _web_page->setText(_curr.webPage());
    _inactive->setChecked(!_curr.isActive());

    // Ledger
    _defaultStore->setId(_curr.defaultStore());
    _startDate->setDate(_curr.startOfYear());
    _closeDate->setDate(_curr.closeDate());
    _shiftMethod->setCurrentItem(_curr.shiftMethod());
    _safeStore->setId(_curr.safeStore());
    _safeStation->setId(_curr.safeStation());
    _safeEmployee->setId(_curr.safeEmployee());

    // Linked accounts
    _reAccount->setId(_curr.retainedEarnings());
    _hbAccount->setId(_curr.historicalBalancing());
    _bankAccount->setId(_curr.bankAccount());
    _customerAcct->setId(_curr.customerAccount());
    _vendorAcct->setId(_curr.vendorAccount());
    _custTermsAcct->setId(_curr.customerTermsAcct());
    _vendTermsAcct->setId(_curr.vendorTermsAcct());
    _depositAccount->setId(_curr.depositAccount());
    _transferAccount->setId(_curr.transferAccount());
    _physicalAccount->setId(_curr.physicalAccount());
    _splitAccount->setId(_curr.splitAccount());
    _chargeAccount->setId(_curr.chargeAccount());
    _overShortAccount->setId(_curr.overShortAccount());

    // Rounding rules
    _rounding->clear();
    for (unsigned int i = 0; i < _curr.rounding().size(); ++i) {
	QString endsIn = _curr.rounding()[i].endsIn;
	fixed addAmt = _curr.rounding()[i].addAmt;

	VectorRow* row = new VectorRow(_rounding->columns());
	row->setValue(0, endsIn);
	row->setValue(1, addAmt);
	_rounding->appendRow(row);
    }
    _rounding->appendRow(new VectorRow(_rounding->columns()));
}

// Set the data object from the widgets.
void
CompanyMaster::widgetToData()
{
    _curr.setName(_name->text());
    _curr.setNumber(_number->text());
    _curr.setStreet(_street->text());
    _curr.setStreet2(_street2->text());
    _curr.setCity(_city->text());
    _curr.setProvince(_province->text());
    _curr.setCountry(_country->text());
    _curr.setPostal(_postal->text());
    _curr.setPhoneNumber(_phone_num->text());
    _curr.setPhone2Number(_phone2_num->text());
    _curr.setFaxNumber(_fax_num->text());
    _curr.setEmail(_email->text());
    _curr.setWebPage(_web_page->text());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    // Ledger
    _curr.setDefaultStore(_defaultStore->getId());
    _curr.setStartOfYear(_startDate->getDate());
    _curr.setCloseDate(_closeDate->getDate());
    _curr.setShiftMethod(_shiftMethod->currentItem());
    _curr.setSafeStore(_safeStore->getId());
    _curr.setSafeStation(_safeStation->getId());
    _curr.setSafeEmployee(_safeEmployee->getId());

    // Linked accounts
    _curr.setRetainedEarnings(_reAccount->getId());
    _curr.setHistoricalBalancing(_hbAccount->getId());
    _curr.setBankAccount(_bankAccount->getId());
    _curr.setCustomerAccount(_customerAcct->getId());
    _curr.setVendorAccount(_vendorAcct->getId());
    _curr.setCustomerTermsAcct(_custTermsAcct->getId());
    _curr.setVendorTermsAcct(_vendTermsAcct->getId());
    _curr.setDepositAccount(_depositAccount->getId());
    _curr.setTransferAccount(_transferAccount->getId());
    _curr.setPhysicalAccount(_physicalAccount->getId());
    _curr.setSplitAccount(_splitAccount->getId());
    _curr.setChargeAccount(_chargeAccount->getId());
    _curr.setOverShortAccount(_overShortAccount->getId());

    // Rounding rules
    _curr.rounding().clear();
    for (int row = 0; row < _rounding->rows(); ++row) {
	QString endsIn = _rounding->cellValue(row, 0).toString();
	fixed addAmt = _rounding->cellValue(row, 1).toFixed();
	if (endsIn.isEmpty()) continue;

	RoundingRule info;
	info.endsIn = endsIn;
	info.addAmt = addAmt;
	_curr.rounding().push_back(info);
    }
}

void
CompanyMaster::slotSetSafeWidget()
{
    if (_shiftMethod->currentText() == tr("Station")) {
	_safeStationLabel->show();
	_safeStation->show();
	_safeEmployeeLabel->hide();
	_safeEmployee->hide();
    } else {
	_safeStationLabel->hide();
	_safeStation->hide();
	_safeEmployeeLabel->show();
	_safeEmployee->show();
    }
}

void
CompanyMaster::slotRoundingCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _rounding->rows() - 1 && col == 0) {
	QString endsIn = _rounding->cellValue(row, col).toString();
	if (!endsIn.isEmpty())
	    _rounding->appendRow(new VectorRow(_rounding->columns()));
    }
}
