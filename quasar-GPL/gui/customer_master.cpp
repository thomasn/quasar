// $Id: customer_master.cpp,v 1.61 2005/02/03 08:19:28 bpepers Exp $
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

#include "customer_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "send_email.h"
#include "account.h"
#include "account_select.h"
#include "tender.h"
#include "tender_select.h"
#include "card_adjust.h"
#include "company.h"
#include "group.h"
#include "customer_type.h"
#include "lookup_edit.h"
#include "date_popup.h"
#include "pat_group_lookup.h"
#include "account_lookup.h"
#include "customer_type_lookup.h"
#include "extra_lookup.h"
#include "term_lookup.h"
#include "tax_lookup.h"
#include "group_lookup.h"
#include "discount_lookup.h"
#include "extra_select.h"
#include "money_edit.h"
#include "percent_edit.h"
#include "table.h"
#include "extra.h"

#include <qapplication.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qtabwidget.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qstatusbar.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qinputdialog.h>
#include <qmessagebox.h>

CustomerMaster::CustomerMaster(MainWindow* main, Id customer_id)
    : DataWindow(main, "CustomerMaster", customer_id)
{
    _helpSource = "customer_master.html";

    QPushButton* email = new QPushButton(tr("Email"), _buttons);
    connect(email, SIGNAL(clicked()), SLOT(slotEmail()));

    // Create widgets
    _company = new QCheckBox(tr("&Company?"), _frame);
    connect(_company, SIGNAL(toggled(bool)), SLOT(companyChanged(bool)));

    _label1 = new QLabel(_frame);
    _entry1 = new LineEdit(30, _frame);
    _label1->setBuddy(_entry1);

    _label2 = new QLabel(_frame);
    _entry2 = new LineEdit(30, _frame);
    _label2->setBuddy(_entry2);

    QLabel* numberLabel = new QLabel(tr("Number:"), _frame);
    _number = new LineEdit(12, _frame);
    numberLabel->setBuddy(_number);

    QLabel* typeLabel = new QLabel(tr("Type:"), _frame);
    _type = new LookupEdit(new CustomerTypeLookup(_main, this), _frame);
    _type->setSizeInfo(12, 'x');
    _type->setMinCharWidth(12);
    _type->setMaxLength(20);
    typeLabel->setBuddy(_type);
    connect(_type, SIGNAL(validData()), this, SLOT(typeChanged()));

    QGroupBox* addr = new QGroupBox(tr("Address"), _frame);
    QGridLayout* grid1 = new QGridLayout(addr, 7, 4,
					 addr->frameWidth() * 2);
    grid1->addRowSpacing(0, addr->fontMetrics().height());
    grid1->setColStretch(2, 1);

    QLabel* streetLabel = new QLabel(tr("Street:"), addr);
    _street = new LineEdit(addr);
    _street->setLength(60);
    streetLabel->setBuddy(_street);

    _street2 = new LineEdit(addr);
    _street2->setLength(60);

    QLabel* cityLabel = new QLabel(tr("City:"), addr);
    _city = new LineEdit(20, addr);
    cityLabel->setBuddy(_city);

    QLabel* provLabel = new QLabel(tr("Prov/State:"), addr);
    _province = new LineEdit(20, addr);
    provLabel->setBuddy(_province);

    QLabel* postalLabel = new QLabel(tr("Postal/Zip:"), addr);
    _postal = new LineEdit(20, addr);
    postalLabel->setBuddy(_postal);

    QLabel* countryLabel = new QLabel(tr("Country:"), addr);
    _country = new LineEdit(20, addr);
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
    _email = new LineEdit(60, addr);
    emailLabel->setBuddy(_email);

    QLabel* webLabel = new QLabel(tr("Web Page:"), addr);
    _web_page = new LineEdit(60,addr);
    webLabel->setBuddy(_web_page);

    grid1->addWidget(streetLabel, 1, 0);
    grid1->addMultiCellWidget(_street, 1, 1, 1, 4, AlignLeft | AlignVCenter);
    grid1->addMultiCellWidget(_street2, 2, 2, 1, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(cityLabel, 3, 0);
    grid1->addWidget(_city, 3, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(provLabel, 3, 3);
    grid1->addWidget(_province, 3, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(postalLabel, 4, 0);
    grid1->addWidget(_postal, 4, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(countryLabel, 4, 3);
    grid1->addWidget(_country, 4, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(phoneLabel, 5, 0);
    grid1->addWidget(_phone_num, 5, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(phone2Label, 5, 3);
    grid1->addWidget(_phone2_num, 5, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(faxLabel, 6, 0);
    grid1->addWidget(_fax_num, 6, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(emailLabel, 7, 0);
    grid1->addMultiCellWidget(_email, 7, 7, 1, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(webLabel, 8, 0);
    grid1->addMultiCellWidget(_web_page, 8, 8, 1, 4, AlignLeft | AlignVCenter);

    QTabWidget* tabs = new QTabWidget(_frame);
    QFrame* control = new QFrame(tabs);
    QFrame* groupTab = new QFrame(tabs);
    QFrame* trans = new QFrame(tabs);
    QFrame* refsTab = new QFrame(tabs);
    QFrame* extraTab = new QFrame(tabs);
    tabs->addTab(control, tr("Control"));
    tabs->addTab(groupTab, tr("Groups"));
    tabs->addTab(trans, tr("Transactions"));
    tabs->addTab(refsTab, tr("References"));
    tabs->addTab(extraTab, tr("Extra Info"));

    QLabel* patgroupLabel = new QLabel(tr("Patronage Group:"), control);
    _patgroup_id = new LookupEdit(new PatGroupLookup(main, this), control);
    _patgroup_id->setLength(20);
    _patgroup_id->setFocusPolicy(NoFocus);
    patgroupLabel->setBuddy(_patgroup_id);

    QLabel* accountLabel = new QLabel(tr("AR Account:"), control);
    _account_id = new LookupEdit(new AccountLookup(main, this, Account::AR),
				 control);
    _account_id->setLength(30);
    accountLabel->setBuddy(_account_id);

    QLabel* termLabel = new QLabel(tr("Terms:"), control);
    _term_id = new LookupEdit(new TermLookup(main, this), control);
    _term_id->setLength(30);
    termLabel->setBuddy(_term_id);

    QLabel* exemptLabel = new QLabel(tr("Tax Exempt:"), control);
    _exempt_id = new LookupEdit(new TaxLookup(main, this), control);
    _exempt_id->setLength(30);
    exemptLabel->setBuddy(_exempt_id);

    QLabel* creditLabel = new QLabel(tr("Credit Limit:"), control);
    _credit_limit = new MoneyEdit(control);
    _credit_limit->setLength(14);
    creditLabel->setBuddy(_credit_limit);

    _credit_hold = new QCheckBox(tr("Credit Hold?"), control);

    QLabel* scLabel = new QLabel(tr("Service Charge:"), control);
    _sc_rate = new PercentEdit(control);
    _sc_rate->setLength(8);
    scLabel->setBuddy(_sc_rate);

    _print_stmts = new QCheckBox(tr("Statements?"), control);

    QGridLayout* conGrid = new QGridLayout(control);
    conGrid->setSpacing(3);
    conGrid->setMargin(3);
    conGrid->setColStretch(3, 1);
    conGrid->addWidget(patgroupLabel, 0, 0);
    conGrid->addMultiCellWidget(_patgroup_id, 0,0,1,2,AlignLeft|AlignVCenter);
    conGrid->addWidget(accountLabel, 1, 0);
    conGrid->addMultiCellWidget(_account_id, 1,1,1,2,AlignLeft|AlignVCenter);
    conGrid->addWidget(termLabel, 2, 0);
    conGrid->addMultiCellWidget(_term_id, 2, 2, 1, 2, AlignLeft|AlignVCenter);
    conGrid->addWidget(exemptLabel, 3, 0);
    conGrid->addMultiCellWidget(_exempt_id, 3, 3, 1, 2,AlignLeft|AlignVCenter);
    conGrid->addWidget(creditLabel, 4, 0);
    conGrid->addWidget(_credit_limit, 4, 1, AlignLeft | AlignVCenter);
    conGrid->addWidget(_credit_hold, 4, 2, AlignLeft | AlignVCenter);
    conGrid->addWidget(scLabel, 5, 0);
    conGrid->addWidget(_sc_rate, 5, 1, AlignLeft | AlignVCenter);
    conGrid->addWidget(_print_stmts, 5, 2, AlignLeft | AlignVCenter);

    _groups = new Table(groupTab);
    _groups->setVScrollBarMode(QScrollView::AlwaysOn);
    _groups->setDisplayRows(2);
    _groups->setLeftMargin(fontMetrics().width("999"));
    connect(_groups, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(groupCellChanged(int,int,Variant)));

    GroupLookup* groupLookup = new GroupLookup(_main, this, Group::CUSTOMER);
    new LookupColumn(_groups, tr("Group Name"), 20, groupLookup);
    new LookupEditor(_groups, 0, new LookupEdit(groupLookup, _groups));

    QGridLayout* groupGrid = new QGridLayout(groupTab);
    groupGrid->setSpacing(6);
    groupGrid->setMargin(3);
    groupGrid->setRowStretch(0, 1);
    groupGrid->setColStretch(0, 1);
    groupGrid->addWidget(_groups, 0, 0);

    _charge = new QCheckBox(tr("Can Charge?"), trans);
    _withdraw = new QCheckBox(tr("Can Withdraw?"), trans);
    _payment = new QCheckBox(tr("Can Make Payment?"), trans);
    _check_bal = new QCheckBox(tr("Check Withdraw Balance?"), trans);
    _second_rcpt = new QCheckBox(tr("Second Receipt?"), trans);

    _discs = new Table(trans);
    _discs->setVScrollBarMode(QScrollView::AlwaysOn);
    _discs->setDisplayRows(2);
    _discs->setLeftMargin(fontMetrics().width("999"));
    connect(_discs, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(discountCellChanged(int,int,Variant)));

    DiscountLookup* discLookup = new DiscountLookup(_main, this);
    discLookup->txOnly = true;
    new LookupColumn(_discs, tr("Discount"), 20, discLookup);
    new LookupEditor(_discs, 0, new LookupEdit(discLookup, _discs));

    QGridLayout* transGrid = new QGridLayout(trans);
    transGrid->setSpacing(6);
    transGrid->setMargin(3);
    transGrid->setRowStretch(5, 1);
    transGrid->setColStretch(1, 1);
    transGrid->addWidget(_charge, 0, 0, AlignLeft | AlignVCenter);
    transGrid->addWidget(_withdraw, 1, 0, AlignLeft | AlignVCenter);
    transGrid->addWidget(_payment, 2, 0, AlignLeft | AlignVCenter);
    transGrid->addWidget(_check_bal, 3, 0, AlignLeft | AlignVCenter);
    transGrid->addWidget(_second_rcpt, 4, 0, AlignLeft | AlignVCenter);
    transGrid->addMultiCellWidget(_discs, 0, 5, 2, 2);

    _refs = new Table(refsTab);
    _refs->setVScrollBarMode(QScrollView::AlwaysOn);
    _refs->setDisplayRows(2);
    _refs->setLeftMargin(fontMetrics().width("999"));
    connect(_refs, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(refCellChanged(int,int,Variant)));

    new TextColumn(_refs, tr("Reference"), 16);
    LineEdit* refEdit = new LineEdit(_refs);
    refEdit->setMaxLength(16);
    new LineEditor(_refs, 0, refEdit);

    QGridLayout* refGrid = new QGridLayout(refsTab);
    refGrid->setSpacing(6);
    refGrid->setMargin(3);
    refGrid->setRowStretch(0, 1);
    refGrid->setColStretch(0, 1);
    refGrid->addWidget(_refs, 0, 0);

    _extra = new Table(extraTab);
    _extra->setVScrollBarMode(QScrollView::AlwaysOn);
    _extra->setDisplayRows(2);
    _extra->setLeftMargin(fontMetrics().width("999"));
    connect(_extra, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(extraFocusNext(bool&,int&,int&,int)));

    ExtraLookup* extraLookup = new ExtraLookup(_main, this, "Customer");

    new LookupColumn(_extra, tr("Data Name"), 30, extraLookup);
    new TextColumn(_extra, tr("Value"), 30);

    new LineEditor(_extra, 1, new LineEdit(_extra));

    QPushButton* createData = new QPushButton(tr("Create Data"), extraTab);
    QPushButton* deleteData = new QPushButton(tr("Delete Data"), extraTab);
    QPushButton* renameData = new QPushButton(tr("Rename Data"), extraTab);

    connect(createData, SIGNAL(clicked()), SLOT(slotCreateData()));
    connect(deleteData, SIGNAL(clicked()), SLOT(slotDeleteData()));
    connect(renameData, SIGNAL(clicked()), SLOT(slotRenameData()));

    QGridLayout* extraGrid = new QGridLayout(extraTab);
    extraGrid->setSpacing(6);
    extraGrid->setMargin(3);
    extraGrid->setRowStretch(0, 1);
    extraGrid->setColStretch(0, 1);
    extraGrid->addMultiCellWidget(_extra, 0, 0, 0, 3);
    extraGrid->addWidget(createData, 1, 1);
    extraGrid->addWidget(deleteData, 1, 2);
    extraGrid->addWidget(renameData, 1, 3);

    _open_bal = new QGroupBox(tr("Opening Balance"), _frame);
    QGridLayout* i_grid = new QGridLayout(_open_bal, 2, 2,
					  _open_bal->frameWidth() * 2);
    i_grid->addRowSpacing(0, _open_bal->fontMetrics().height());

    QLabel* asOfLabel = new QLabel(tr("As of"), _open_bal);
    _as_of = new DatePopup(_open_bal);
    asOfLabel->setBuddy(_as_of);

    QLabel* openBalLabel = new QLabel(tr("Balance:"), _open_bal);
    _open_balance = new MoneyEdit(_open_bal);
    _open_balance->setLength(20);
    openBalLabel->setBuddy(_open_balance);

    i_grid->setColStretch(2, 1);
    i_grid->addWidget(asOfLabel, 1, 0, AlignLeft);
    i_grid->addWidget(_as_of, 2, 0, AlignLeft | AlignVCenter);
    i_grid->addWidget(openBalLabel, 1, 1, AlignLeft);
    i_grid->addWidget(_open_balance, 2, 1, AlignLeft | AlignVCenter);

    Company company;
    _quasar->db()->lookup(company);
    if (_id != INVALID_ID || company.historicalBalancing() == INVALID_ID)
	_open_bal->hide();

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(4, 1);
    grid->setColStretch(2, 1);
    grid->addColSpacing(2, 10);
    grid->addWidget(_company, 0, 0);
    grid->addWidget(_label1, 1, 0, AlignLeft | AlignVCenter);
    grid->addWidget(_entry1, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(_label2, 2, 0, AlignLeft | AlignVCenter);
    grid->addWidget(_entry2, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(numberLabel, 1, 3, AlignLeft | AlignVCenter);
    grid->addWidget(_number, 1, 4, AlignLeft | AlignVCenter);
    grid->addWidget(typeLabel, 2, 3, AlignLeft | AlignVCenter);
    grid->addWidget(_type, 2, 4, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(addr, 3, 3, 0, 4);
    grid->addMultiCellWidget(tabs, 4, 4, 0, 4);
    grid->addMultiCellWidget(_open_bal, 5, 5, 0, 4);

    setCaption(tr("Customer Master"));
    finalize();
}

CustomerMaster::~CustomerMaster()
{
}

void
CustomerMaster::setName(const QString& name)
{
    // TODO: try to split up on comma?
    _entry1->setText(name);
}

void
CustomerMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _company;
}

void
CustomerMaster::newItem()
{
    Customer blank;
    _orig = blank;
    _orig.setCompany(true);

    _as_of->setDate(QDate::currentDate());
    _open_balance->setText("");

    Company company;
    _quasar->db()->lookup(company);
    if (company.historicalBalancing() != INVALID_ID)
	_open_bal->show();

    // Figure out default AR account
    if (company.customerAccount() != INVALID_ID)
	_orig.setAccountId(company.customerAccount());
    else {
	AccountSelect conditions;
	conditions.type = Account::AR;
	conditions.postingOnly = true;
	vector<Account> accounts;
	_quasar->db()->select(accounts, conditions);
	if (accounts.size() > 0)
	    _orig.setAccountId(accounts[0].id());
    }

    _curr = _orig;
    _firstField = _company;
}

void
CustomerMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _curr.setNumber("");
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
}

bool
CustomerMaster::fileItem()
{
    if (_orig.id() == INVALID_ID) {
	if (!_quasar->db()->create(_curr)) return false;

	fixed open = _open_balance->getFixed();
	if (open != 0.0) {
	    Id store_id = _quasar->defaultStore();
	    QDate date = _as_of->getDate();
	    if (date.isNull()) date = QDate::currentDate();

	    CardAdjust adjustment;
	    adjustment.setPostDate(date);
	    adjustment.setPostTime(QTime(0, 0, 0));
	    adjustment.setMemo(tr("Open balance"));
	    adjustment.setCardId(_curr.id());
	    adjustment.setStoreId(store_id);

	    CardLine cardLine(_curr.id(), open);
	    adjustment.cards().push_back(cardLine);

	    // Get historical balancing account
	    Company company;
	    _quasar->db()->lookup(company);
	    Id hist_id = company.historicalBalancing();
	    Id acct_id = _curr.accountId();

	    // Add account lines
	    adjustment.accounts().push_back(AccountLine(acct_id, open));
	    adjustment.accounts().push_back(AccountLine(hist_id, -open));

	    // Post adjustment
	    _quasar->db()->create(adjustment);
	}
    } else {
	if (!_quasar->db()->update(_orig, _curr)) return false;
    }

    Company company;
    _quasar->db()->lookup(company);

    if (company.customerAccount() == INVALID_ID) {
	Company orig = company;
	company.setCustomerAccount(_curr.accountId());
	_quasar->db()->update(orig, company);
    }

    _orig = _curr;
    _id = _curr.id();

    return true;
}

bool
CustomerMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
CustomerMaster::restoreItem()
{
    _curr = _orig;
}

void
CustomerMaster::cloneItem()
{
    CustomerMaster* clone = new CustomerMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
CustomerMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
CustomerMaster::dataToWidget()
{
    _company->setChecked(_curr.company());
    _companyName = _curr.lastName();
    _lastName = _curr.lastName();
    _firstName = _curr.firstName();
    _contact = _curr.contact();
    if (_curr.company()) {
	_entry1->setText(_companyName);
	_entry2->setText(_contact);
    } else {
	_entry1->setText(_lastName);
	_entry2->setText(_firstName);
    }
    _number->setText(_curr.number());
    _type->setId(_curr.typeId());
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
    _patgroup_id->setId(_curr.patGroupId());
    _account_id->setId(_curr.accountId());
    _term_id->setId(_curr.termsId());
    _exempt_id->setId(_curr.taxExemptId());
    _credit_limit->setFixed(_curr.creditLimit());
    _credit_hold->setChecked(_curr.creditHold());
    _sc_rate->setFixed(_curr.serviceCharge());
    _print_stmts->setChecked(_curr.printStatements());
    _charge->setChecked(_curr.canCharge());
    _withdraw->setChecked(_curr.canWithdraw());
    _payment->setChecked(_curr.canPayment());
    _check_bal->setChecked(_curr.checkWithdrawBalance());
    _second_rcpt->setChecked(_curr.secondReceipt());
    _inactive->setChecked(!_curr.isActive());

    // Set groups
    _groups->clear();
    unsigned int i;
    for (i = 0; i < _curr.groups().size(); ++i) {
	VectorRow* row = new VectorRow(_groups->columns());
	row->setValue(0, _curr.groups()[i]);
	_groups->appendRow(row);
    }
    _groups->appendRow(new VectorRow(_groups->columns()));

    // Set discounts
    _discs->clear();
    for (i = 0; i < _curr.discounts().size(); ++i) {
	VectorRow* row = new VectorRow(_discs->columns());
	row->setValue(0, _curr.discounts()[i]);
	_discs->appendRow(row);
    }
    _discs->appendRow(new VectorRow(_discs->columns()));

    // Set references
    _refs->clear();
    for (i = 0; i < _curr.references().size(); ++i) {
	VectorRow* row = new VectorRow(_refs->columns());
	row->setValue(0, _curr.references()[i]);
	_refs->appendRow(row);
    }
    _refs->appendRow(new VectorRow(_refs->columns()));

    // Extra info
    _extra->clear();
    ExtraSelect extraSelect;
    extraSelect.activeOnly = true;
    extraSelect.table = "Customer";
    vector<Extra> extras;
    _quasar->db()->select(extras, extraSelect);
    for (i = 0; i < extras.size(); ++i) {
	Id extra_id = extras[i].id();
	QString value = _curr.value(extra_id);

	VectorRow* row = new VectorRow(_extra->columns());
	row->setValue(0, extra_id);
	row->setValue(1, value);
	_extra->appendRow(row);
    }
    if (_extra->rows() > 0)
	_extra->setCurrentCell(0, 1);

    companyChanged(_company->isChecked());
}

// Set the data object from the widgets.
void
CustomerMaster::widgetToData()
{
    _curr.setCompany(_company->isChecked());
    _curr.setLastName(_entry1->text());
    if (_curr.company()) {
	_curr.setContact(_entry2->text());
	_curr.setFirstName("");
    } else {
	_curr.setContact("");
	_curr.setFirstName(_entry2->text());
    }
    _curr.setNumber(_number->text());
    _curr.setTypeId(_type->getId());
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
    _curr.setAccountId(_account_id->getId());
    _curr.setTermsId(_term_id->getId());
    _curr.setTaxExemptId(_exempt_id->getId());
    _curr.setCreditLimit(_credit_limit->getFixed());
    _curr.setCreditHold(_credit_hold->isChecked());
    _curr.setServiceCharge(_sc_rate->getFixed());
    _curr.setPrintStatements(_print_stmts->isChecked());
    _curr.setCanCharge(_charge->isChecked());
    _curr.setCanWithdraw(_withdraw->isChecked());
    _curr.setCanPayment(_payment->isChecked());
    _curr.setCheckWithdrawBalance(_check_bal->isChecked());
    _curr.setSecondReceipt(_second_rcpt->isChecked());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    int row;
    _curr.groups().clear();
    for (row = 0; row < _groups->rows(); ++row) {
	Id group_id = _groups->cellValue(row, 0).toId();
	if (group_id != INVALID_ID)
	    _curr.groups().push_back(group_id);
    }

    _curr.discounts().clear();
    for (row = 0; row < _discs->rows(); ++row) {
	Id discount_id = _discs->cellValue(row, 0).toId();
	if (discount_id != INVALID_ID)
	    _curr.discounts().push_back(discount_id);
    }

    _curr.references().clear();
    for (row = 0; row < _refs->rows(); ++row) {
	QString name = _refs->cellValue(row, 0).toString();
	if (!name.isEmpty())
	    _curr.references().push_back(name);
    }

    _curr.extra().clear();
    for (row = 0; row < _extra->rows(); ++row) {
	Id extra_id = _extra->cellValue(row, 0).toId();
	QString value = _extra->cellValue(row, 1).toString();
	_curr.setValue(extra_id, value);
    }
}

// Validate the widget values.
bool
CustomerMaster::dataValidate()
{
    if (_orig.id() == INVALID_ID) {
	fixed open = _open_balance->getFixed();
	if (open != 0.0) {
	    Company company;
	    _quasar->db()->lookup(company);
	    if (company.historicalBalancing() == INVALID_ID)
		return error(tr("No historical balance account setup"),
			     _open_balance);
	}
    }

    return DataWindow::dataValidate();
}

void
CustomerMaster::companyChanged(bool company)
{
    if (company) {
	_label1->setText(tr("Company &Name:"));
	_label2->setText(tr("Contact:"));
	_lastName = _entry1->text();
	_firstName = _entry2->text();
	_entry1->setText(_companyName);
	_entry2->setText(_contact);
    } else {
	_label1->setText(tr("Last &Name:"));
	_label2->setText(tr("&First Name:"));
	_companyName = _entry1->text();
	_contact = _entry2->text();
	_entry1->setText(_lastName);
	_entry2->setText(_firstName);
    }
}

void
CustomerMaster::typeChanged()
{
    if (_type->getId() == INVALID_ID) return;
    CustomerType type;
    if (_quasar->db()->lookup(_type->getId(), type)) {
        _account_id->setId(type.accountId());
	_term_id->setId(type.termsId());
	_exempt_id->setId(type.taxExemptId());
	_credit_limit->setFixed(type.creditLimit());
	_sc_rate->setFixed(type.serviceCharge());
	_print_stmts->setChecked(type.printStatements());
	_charge->setChecked(type.canCharge());
	_withdraw->setChecked(type.canWithdraw());
	_payment->setChecked(type.canPayment());
	_check_bal->setChecked(type.checkWithdrawBalance());
	_second_rcpt->setChecked(type.secondReceipt());
    }
}

void
CustomerMaster::groupCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _groups->rows() - 1 && col == 0) {
	Id group_id = _groups->cellValue(row, col).toId();
	if (group_id != INVALID_ID)
	    _groups->appendRow(new VectorRow(_groups->columns()));
    }
}

void
CustomerMaster::discountCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _discs->rows() - 1 && col == 0) {
	Id discount_id = _discs->cellValue(row, col).toId();
	if (discount_id != INVALID_ID)
	    _discs->appendRow(new VectorRow(_discs->columns()));
    }
}

void
CustomerMaster::refCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _refs->rows() - 1 && col == 0) {
	QString name = _refs->cellValue(row, col).toString();
	if (!name.isEmpty())
	    _refs->appendRow(new VectorRow(_refs->columns()));
    }
}

void
CustomerMaster::extraFocusNext(bool&, int& newRow, int& newCol, int type)
{
    int row = _extra->currentRow();
    int col = _extra->currentColumn();

    if (type == Table::MoveNext && col == 1) {
	newCol = 1;
    } else if (type == Table::MovePrev && col == 1) {
	newRow = row - 1;
	newCol = 1;
    }
}

void
CustomerMaster::slotCreateData()
{
    bool ok = false;
    QString message = tr("Enter the new data name:");
    QString text = QInputDialog::getText(tr("Create Data"), message,
					 QLineEdit::Normal, QString::null,
					 &ok, this);
    if (!ok || text.isEmpty()) return;

    Extra extra;
    extra.setTable("Customer");
    extra.setName(text);
    if (!_quasar->db()->create(extra)) {
	QMessageBox::critical(this, tr("Error"),
			      tr("Creating new data failed."));
	return;
    }

    VectorRow* row = new VectorRow(_extra->columns());
    row->setValue(0, extra.id());
    row->setValue(1, "");
    _extra->appendRow(row);
}

void
CustomerMaster::slotDeleteData()
{
    int row = _extra->currentRow();
    if (row == -1) return;

    Id extra_id = _extra->cellValue(row, 0).toId();
    if (extra_id == INVALID_ID) return;

    Extra extra;
    if (!_quasar->db()->lookup(extra_id, extra)) {
	QMessageBox::critical(this, tr("Error"), tr("Data lookup failed."));
	return;
    }

    QString message = tr("Are you sure you want to delete the\n"
	"%1 data?  It will be removed from\nall items.").arg(extra.name());
    int choice = QMessageBox::warning(this, tr("Delete?"), message,
				      QMessageBox::No, QMessageBox::Yes);
    if (choice != QMessageBox::Yes)
	return;

    if (!_quasar->db()->remove(extra)) {
	QMessageBox::critical(this, tr("Error"), tr("Deleting data failed."));
	return;
    }

    _extra->deleteRow(row);
}

void
CustomerMaster::slotRenameData()
{
    int row = _extra->currentRow();
    if (row == -1) return;

    Id extra_id = _extra->cellValue(row, 0).toId();
    if (extra_id == INVALID_ID) return;

    Extra extra;
    if (!_quasar->db()->lookup(extra_id, extra)) {
	QMessageBox::critical(this, tr("Error"), tr("Data lookup failed."));
	return;
    }

    bool ok = false;
    QString message = tr("Enter the new name for %1:").arg(extra.name());
    QString text = QInputDialog::getText(tr("Rename Data"), message,
					 QLineEdit::Normal, QString::null,
					 &ok, this);
    if (!ok || text.isEmpty()) return;

    Extra orig = extra;
    extra.setName(text);
    if (!_quasar->db()->update(orig, extra)) {
	QMessageBox::critical(this, tr("Error"), tr("Renaming data failed."));
	return;
    }

    _extra->setCellValue(row, 0, extra.id());
}

void
CustomerMaster::slotEmail()
{
    if (!saveItem(true)) return;

    Company company;
    _quasar->db()->lookup(company);

    SendEmail* email = new SendEmail(_main);
    email->setCard(_curr.id());
    email->show();
}
