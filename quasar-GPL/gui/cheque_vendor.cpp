// $Id: cheque_vendor.cpp,v 1.41 2005/03/13 23:13:45 bpepers Exp $
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

#include "cheque_vendor.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "cheque_print.h"
#include "invoice_info.h"
#include "recurring_master.h"
#include "account.h"
#include "vendor.h"
#include "term.h"
#include "company.h"
#include "cheque_select.h"
#include "receive_select.h"
#include "gltx_frame.h"
#include "table.h"
#include "money_edit.h"
#include "check_box.h"
#include "lookup_edit.h"
#include "account_lookup.h"
#include "vendor_lookup.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <algorithm>

ChequeVendor::ChequeVendor(MainWindow* main, Id cheque_id)
    : DataWindow(main, "VendorPayment", cheque_id)
{
    _helpSource = "vendor_payment.html";

    _file->insertItem(tr("Recurring"), this, SLOT(slotRecurring()));

    // Printed checkbox
    _printed = new QCheckBox(tr("Printed?"), _buttons);
    _printed->setFocusPolicy(NoFocus);
    QPushButton* print = new QPushButton(tr("Print"), _buttons);
    connect(print, SIGNAL(clicked()), SLOT(slotPrint()));

    // Create widgets
    QFrame* top = new QFrame(_frame);
    top->setFrameStyle(QFrame::Raised | QFrame::Panel);

    QLabel* accountLabel = new QLabel(tr("Bank Account:"), top);
    _account = new LookupEdit(new AccountLookup(main, this, Account::Bank),
			      top);
    _account->setLength(30);
    accountLabel->setBuddy(_account);
    connect(_account, SIGNAL(validData()), SLOT(slotAccountChanged()));

    QLabel* discLabel = new QLabel(tr("Discount Account:"), top);
    AccountLookup* discLook = new AccountLookup(main, this,
						Account::OtherIncome);
    _discount = new LookupEdit(discLook, top);
    _discount->setLength(30);
    discLabel->setBuddy(_discount);

    QLabel* currentLabel = new QLabel(tr("Current Balance:"), top);
    _current = new MoneyEdit(top);
    _current->setFocusPolicy(NoFocus);

    QLabel* endingLabel = new QLabel(tr("Ending Balance:"), top);
    _ending = new MoneyEdit(top);
    _ending->setFocusPolicy(NoFocus);

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(3);
    topGrid->setMargin(3);
    topGrid->setColStretch(2, 1);
    topGrid->addColSpacing(2, 10);
    topGrid->addWidget(accountLabel, 0, 0);
    topGrid->addWidget(_account, 0, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(currentLabel, 0, 3);
    topGrid->addWidget(_current, 0, 4, AlignLeft | AlignVCenter);
    topGrid->addWidget(discLabel, 1, 0);
    topGrid->addWidget(_discount, 1, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(endingLabel, 1, 3);
    topGrid->addWidget(_ending, 1, 4, AlignLeft | AlignVCenter);

    _gltxFrame = new GltxFrame(main, tr("Cheque No."), _frame);
    _gltxFrame->hideMemo();

    QFrame* mid = new QFrame(_frame);
    mid->setFrameStyle(QFrame::Raised | QFrame::Panel);

    QLabel* vendorLabel = new QLabel(tr("Vendor:"), mid);
    _vendor = new LookupEdit(new VendorLookup(main, this), mid);
    _vendor->setLength(30);
    vendorLabel->setBuddy(_vendor);
    connect(_vendor, SIGNAL(validData()), SLOT(slotVendorChanged()));

    QLabel* balanceLabel = new QLabel(tr("Balance:"), mid);
    _balance = new MoneyEdit(mid);
    _balance->setFocusPolicy(NoFocus);

    QLabel* memoLabel = new QLabel(tr("&Memo:"), mid);
    _memo = new LineEdit(mid);
    _memo->setMaxLength(40);
    _memo->setMinimumWidth(_memo->fontMetrics().width('x') * 40);
    memoLabel->setBuddy(_memo);

    QGridLayout* midGrid = new QGridLayout(mid);
    midGrid->setSpacing(3);
    midGrid->setMargin(3);
    midGrid->setColStretch(2, 1);
    midGrid->addWidget(vendorLabel, 0, 0);
    midGrid->addWidget(_vendor, 0, 1, AlignLeft | AlignVCenter);
    midGrid->addWidget(balanceLabel, 0, 2, AlignRight | AlignVCenter);
    midGrid->addWidget(_balance, 0, 3, AlignLeft | AlignVCenter);
    midGrid->addWidget(memoLabel, 1, 0);
    midGrid->addMultiCellWidget(_memo, 1, 1, 1, 3);

    QFrame* bot = new QFrame(_frame);
    bot->setFrameStyle(QFrame::Raised | QFrame::Panel);

    QVBoxLayout* vbox2 = new QVBoxLayout(bot);
    vbox2->setSpacing(3);
    vbox2->setMargin(3);

    _table = new Table(bot);
    _table->setVScrollBarMode(QScrollView::AlwaysOn);
    _table->setDisplayRows(5);
    _table->setLeftMargin(0);
    _table->stopUserInsertDelete();
    connect(_table, SIGNAL(cellMoved(int,int)), SLOT(cellMoved(int,int)));
    connect(_table, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(cellChanged(int,int,Variant)));
    connect(_table, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(focusNext(bool&,int&,int&,int)));

    new DateColumn(_table, tr("Due Date"));
    new TextColumn(_table, tr("Invoice"), 10);
    new MoneyColumn(_table, tr("Total"));
    new DateColumn(_table, tr("Disc Date"));
    new MoneyColumn(_table, tr("Due"));
    new MoneyColumn(_table, tr("Payment"));
    new MoneyColumn(_table, tr("Discount"), 6);
    new CheckColumn(_table, tr("Pay?"));

    NumberEdit* paidEdit = new MoneyEdit(_table);
    paidEdit->setMaxLength(14);
    new NumberEditor(_table, 5, paidEdit);

    NumberEdit* discEdit = new MoneyEdit(_table);
    discEdit->setMaxLength(14);
    new NumberEditor(_table, 6, discEdit);

    CheckBox* check = new CheckBox(_table);
    new CheckEditor(_table, 7, check);
    connect(check, SIGNAL(toggled(bool)), SLOT(slotPayClicked()));

    QFrame* totals = new QFrame(bot);

    QPushButton* payAll = new QPushButton(tr("Pay All"), totals);
    payAll->setFocusPolicy(NoFocus);
    connect(payAll, SIGNAL(clicked()), SLOT(slotPayAll()));

    QPushButton* clearAll = new QPushButton(tr("Clear Payments"), totals);
    clearAll->setFocusPolicy(NoFocus);
    connect(clearAll, SIGNAL(clicked()), SLOT(slotClearAll()));

    QPushButton* invoiceInfo = new QPushButton(tr("Invoice Info"), totals);
    invoiceInfo->setFocusPolicy(NoFocus);
    connect(invoiceInfo, SIGNAL(clicked()), SLOT(slotInvoiceInfo()));

    QLabel* paymentLabel = new QLabel(tr("Payment Total:"), totals);
    _payment = new MoneyEdit(totals);
    _payment->setFocusPolicy(NoFocus);

    QLabel* amountLabel = new QLabel(tr("Overpay Amount:"), totals);
    _amount = new MoneyEdit(totals);
    amountLabel->setBuddy(_amount);
    connect(_amount, SIGNAL(validData()), SLOT(recalculate()));

    QLabel* discAmtLabel = new QLabel(tr("Discount Amt:"), totals);
    _discAmt = new MoneyEdit(totals);
    _discAmt->setFocusPolicy(NoFocus);

    QLabel* totalLabel = new QLabel(tr("Total Amount:"), totals);
    _total = new MoneyEdit(totals);
    _total->setFocusPolicy(NoFocus);

    QGridLayout* totalGrid = new QGridLayout(totals);
    totalGrid->setSpacing(3);
    totalGrid->setColStretch(1, 1);
    totalGrid->setColStretch(4, 1);
    totalGrid->addWidget(payAll, 0, 0);
    totalGrid->addWidget(clearAll, 1, 0);
    totalGrid->addWidget(invoiceInfo, 2, 0);
    totalGrid->addWidget(paymentLabel, 0, 2, AlignRight | AlignVCenter);
    totalGrid->addWidget(_payment, 0, 3);
    totalGrid->addWidget(amountLabel, 1, 2);
    totalGrid->addWidget(_amount, 1, 3, AlignLeft | AlignVCenter);
    totalGrid->addWidget(discAmtLabel, 0, 5, AlignRight | AlignVCenter);
    totalGrid->addWidget(_discAmt, 0, 6);
    totalGrid->addWidget(totalLabel, 1, 5, AlignRight | AlignVCenter);
    totalGrid->addWidget(_total, 1, 6);

    vbox2->addWidget(_table);
    vbox2->addWidget(totals);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(0, 1);
    grid->setRowStretch(3, 1);
    grid->addWidget(top, 0, 0);
    grid->addWidget(_gltxFrame, 1, 0);
    grid->addWidget(mid, 2, 0);
    grid->addWidget(bot, 3, 0);

    _inactive->setText(tr("Voided?"));

    setCaption(tr("Vendor Payment"));
    finalize();
}

ChequeVendor::~ChequeVendor()
{
    if (!_info.isNull())
	delete _info;
}

void
ChequeVendor::setStoreId(Id store_id)
{
    if (store_id != INVALID_ID)
	_gltxFrame->store->setId(store_id);
}

void
ChequeVendor::setCardId(Id card_id)
{
    if (card_id != INVALID_ID) {
	_vendor->setId(card_id);
	slotVendorChanged();
    }
}

void
ChequeVendor::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _vendor;
}

void
ChequeVendor::newItem()
{
    Id bankId = _quasar->db()->defaultBankId();

    Company company;
    _quasar->db()->lookup(company);
    Id discId = company.vendorTermsAcct();

    Cheque blank;
    _orig = blank;
    _gltxFrame->defaultData(_orig);
    _orig.setType(Cheque::VENDOR);
    _orig.setNumber(getChequeNumber(bankId).toString());
    _orig.setAccountId(bankId);
    _orig.setDiscountId(discId);

    _curr = _orig;
    _firstField = _vendor;
}

void
ChequeVendor::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _gltxFrame->cloneData(_curr);
    _curr.setNumber(getChequeNumber(_curr.accountId()).toString());
    dataToWidget();
}

bool
ChequeVendor::fileItem()
{
    // Check that any line with a discount is fully paid
    for (int row = 0; row < _table->rows(); ++row) {
	fixed disc_amt = _table->cellValue(row, 6).toFixed();
	if (disc_amt == 0.0) continue;

	fixed amt_due = _table->cellValue(row, 4).toFixed();
	fixed payment = _table->cellValue(row, 5).toFixed();

	if (payment + disc_amt != amt_due) {
	    _table->setCurrentCell(row, 6);
	    QString message = tr("Discount on a line that wasn't fully paid");
	    error(message, _table);
	    return false;
	}
    }

    if (_curr.number() != "#") {
	vector<Cheque> cheques;
	ChequeSelect conditions;
	conditions.number = _curr.number();
	_quasar->db()->select(cheques, conditions);

	bool used = false;
	for (unsigned int i = 0; i < cheques.size(); ++i) {
	    if (cheques[i].id() == _curr.id()) continue;
	    if (cheques[i].accountId() != _curr.accountId()) continue;
	    used = true;
	}

	if (used) {
	    QString message = tr("This cheque number is already used for\n"
		"another cheque on this account. Are you\n"
		"sure you want to file this cheque?");
	    int choice = QMessageBox::warning(this, tr("Warning"), message,
					QMessageBox::No, QMessageBox::Yes);
	    if (choice != QMessageBox::Yes)
		return false;
	}
    }

    if (_orig.id() == INVALID_ID) {
	if (!_quasar->db()->create(_curr)) return false;
    } else {
	if (!_quasar->db()->update(_orig, _curr)) return false;
    }

    Company company;
    _quasar->db()->lookup(company);

    if (company.vendorTermsAcct() == INVALID_ID) {
	Company orig = company;
	company.setVendorTermsAcct(_curr.discountId());
	_quasar->db()->update(orig, company);
    }

    _orig = _curr;
    _id = _curr.id();

    return true;
}

bool
ChequeVendor::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
ChequeVendor::restoreItem()
{
    _curr = _orig;
}

void
ChequeVendor::cloneItem()
{
    ChequeVendor* clone = new ChequeVendor(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
ChequeVendor::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void
ChequeVendor::dataToWidget()
{
    _gltxFrame->setData(_curr);
    _account->setId(_curr.accountId());
    _discount->setId(_curr.discountId());
    _vendor->setId(_curr.cardId());
    _memo->setText(_curr.memo());
    _printed->setChecked(_curr.printed());
    _inactive->setChecked(!_curr.isActive());

    _amount->setFixed(_curr.total() + _curr.paymentTotal() -
		      _curr.termsDiscountTotal());

    slotVendorChanged();

    Id account_id = _account->getId();
    fixed balance = _quasar->db()->accountBalance(account_id);
    if (_orig.accountId() == account_id)
	balance += _orig.total();

    _current->setFixed(balance);
    _ending->setFixed(balance - _payment->getFixed());
}

// Set the data object from the widgets.
void
ChequeVendor::widgetToData()
{
    _gltxFrame->getData(_curr);
    _curr.setAccountId(_account->getId());
    _curr.setDiscountId(_discount->getId());
    _curr.setCardId(_vendor->getId());
    _curr.setMemo(_memo->text());
    _curr.setPrinted(_printed->isChecked());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    Id store_id = _curr.storeId();

    // Add bank line
    vector<AccountLine>& lines = _curr.accounts();
    lines.clear();

    // Total up discounts and amounts
    fixed pay_amt = _amount->getFixed();
    fixed disc_amt = 0.0;
    int row;
    for (row = 0; row < _table->rows(); ++row) {
	fixed amount = _table->cellValue(row, 5).toFixed();
	fixed discount = _table->cellValue(row, 6).toFixed();
	if (amount != 0.0) {
	    pay_amt += amount;
	    disc_amt += discount;
	}
    }
    fixed total = pay_amt + disc_amt;

    lines.push_back(AccountLine(_account->getId(), -pay_amt));

    // Add AP line
    if (_curr.cardId() != INVALID_ID) {
	Vendor vendor;
	_quasar->db()->lookup(_curr.cardId(), vendor);
	lines.push_back(AccountLine(vendor.accountId(), total));

	// Add card line
	_curr.cards().clear();
	_curr.cards().push_back(CardLine(_curr.cardId(), -total));
    }

    // Add discount
    if (disc_amt != 0.0)
	lines.push_back(AccountLine(_discount->getId(), -disc_amt));

    // Load the lines from the table
    vector<PaymentLine>& payments = _curr.payments();
    payments.clear();
    for (row = 0; row < _table->rows(); ++row) {
	Id gltx_id = _gltxs[row].id();
	fixed amount = _table->cellValue(row, 5).toFixed();
	fixed discount = _table->cellValue(row, 6).toFixed();

	if (amount != 0.0)
	    payments.push_back(PaymentLine(gltx_id, -amount, -discount));
    }
}

void
ChequeVendor::printItem(bool ask)
{
    if (ask) return;
    ChequePrint* window = new ChequePrint(_main);
    window->setCheque(_curr.id());
    window->show();
}

void
ChequeVendor::slotAccountChanged()
{
    Id account_id = _account->getId();
    fixed balance = _quasar->db()->accountBalance(account_id);
    if (account_id == _orig.accountId())
	balance += _orig.total();

    _gltxFrame->number->setText(getChequeNumber(account_id).toString());
    _current->setFixed(balance);
    _ending->setFixed(balance - _payment->getFixed());
}

void
ChequeVendor::slotVendorChanged()
{
    _table->clear();
    _balance->setFixed(0.0);
    if (_vendor->getId() != INVALID_ID)
	slotRefresh();
    recalculate();
}

void
ChequeVendor::slotPayClicked()
{
    int row = _table->currentRow();
    if (row < 0 || row >= _table->rows())
	return;

    // Just get value which should trigger cell changed
    _table->cellValue(row, 7).toBool();
}

void
ChequeVendor::slotPayAll()
{
    slotClearAll();
    for (int row = 0; row < _table->rows(); ++row) {
	fixed disc_amt = _suggest[row];
	fixed amt_due = _table->cellValue(row, 4).toFixed();
	fixed pay_amt = amt_due - disc_amt;

	_table->setCellValue(row, 5, pay_amt);
	if (disc_amt == 0.0)
	    _table->setCellValue(row, 6, "");
	else
	    _table->setCellValue(row, 6, disc_amt);
	_table->setCellValue(row, 7, true);
    }
    recalculate();
}

void
ChequeVendor::slotClearAll()
{
    for (int row = 0; row < _table->rows(); ++row) {
	_table->setCellValue(row, 5, "");
	_table->setCellValue(row, 6, "");
	_table->setCellValue(row, 7, false);
    }
    recalculate();
}

void
ChequeVendor::slotInvoiceInfo()
{
    if (!_info.isNull()) return;

    _info = new InvoiceInfo(_main, this);
    int row = _table->currentRow();
    if (row >= 0 && row < _table->rows()) {
	fixed disc_amt = _table->cellValue(row, 6).toFixed();
	_info->setInvoice(_gltxs[row], _terms[row], disc_amt);
    }
    connect(_info, SIGNAL(newDiscount(fixed)), SLOT(setDiscount(fixed)));
    _info->show();
}

void
ChequeVendor::setDiscount(fixed new_disc)
{
    int row = _table->currentRow();
    if (row < 0 || row >= _table->rows())
	return;

    fixed amt_due = _table->cellValue(row, 4).toFixed();
    fixed old_amt = _table->cellValue(row, 5).toFixed();
    fixed old_disc = _table->cellValue(row, 6).toFixed();

    fixed new_amt = old_amt;
    if (old_disc + old_amt == amt_due) {
	new_amt = amt_due - new_disc;
	if (new_amt + new_disc > amt_due)
	    new_amt = amt_due - new_disc;
    }

    if (new_amt == 0.0)
	_table->setCellValue(row, 5, "");
    else
	_table->setCellValue(row, 5, new_amt);
    if (new_disc == 0.0)
	_table->setCellValue(row, 6, "");
    else
	_table->setCellValue(row, 6, new_disc);
    _table->setCellValue(row, 7, (new_amt != 0.0 || new_disc != 0.0));

    recalculate();
}

void
ChequeVendor::slotRefresh()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Vendor vendor;
    _quasar->db()->lookup(_vendor->getId(), vendor);
    fixed balance = _quasar->db()->cardBalance(vendor.id());
    _balance->setFixed(balance);

    // Load transactions
    GltxSelect conditions;
    conditions.card_id = vendor.id();
    conditions.unpaid = true;
    conditions.activeOnly = true;
    vector<Gltx> gltxs;
    _quasar->db()->select(gltxs, conditions);

    // Load ones that have been paid
    if (_curr.id() != INVALID_ID && _curr.cardId() == vendor.id()) {
	for (unsigned int i = 0; i < _curr.payments().size(); ++i) {
	    const PaymentLine& line = _curr.payments()[i];

	    bool found = false;
	    for (unsigned int j = 0; j < gltxs.size(); ++j)
		if (gltxs[j].id() == line.gltx_id)
		    found = true;

	    if (!found) {
		Gltx gltx;
		_quasar->db()->lookup(line.gltx_id, gltx);
		gltxs.push_back(gltx);
	    }
	}
    }

    // Sort by date
    std::sort(gltxs.begin(), gltxs.end());

    // Load into table
    _gltxs.clear();
    _terms.clear();
    _suggest.clear();
    _table->clear();
    for (unsigned int i = 0; i < gltxs.size(); ++i) {
	Gltx& gltx = gltxs[i];

	// Get terms if its a vendor invoice
	Term term;
	if (gltx.dataType() == DataObject::RECEIVE) {
	    Receive receive;
	    _quasar->db()->lookup(gltx.id(), receive);
	    _quasar->db()->lookup(receive.termsId(), term);
	}

	// Skip credits (should they be added up and used for payments?)
	if (gltx.cardTotal() < 0.0) continue;

	// Remove payments that are this transaction
	for (unsigned int j = 0; j < gltx.payments().size(); ++j) {
	    const PaymentLine& line = gltx.payments()[j];
	    if (line.gltx_id == _curr.id())
		gltx.payments().erase(gltx.payments().begin() + j);
	}

	// If remain is zero, ignore it
	fixed remain = gltx.cardTotal() - gltx.paymentTotal();
	if (remain == 0.0) continue;

	QDate invoiceDate = gltx.postDate();
	if (gltx.dataType() == DataObject::RECEIVE) {
	    Receive receive;
	    _quasar->db()->lookup(gltx.id(), receive);
	    invoiceDate = receive.invoiceDate();
	}
	QDate dueDate = invoiceDate + term.dueDays();
	QDate discDate = invoiceDate + term.discountDays();

	fixed suggest = 0.0;
	if (term.discount() != 0.0 && QDate::currentDate() <= discDate)
	    suggest = gltx.cardTotal() * term.discount() / 100.0;
	suggest.moneyRound();

	VectorRow* row = new VectorRow(_table->columns());
	row->setValue(0, dueDate);
	row->setValue(1, gltx.number());
	row->setValue(2, gltx.cardTotal());
	if (term.discountDays() != 0)
	    row->setValue(3, discDate);
	row->setValue(4, gltx.cardTotal() - gltx.paymentTotal());
	_table->appendRow(row);

	_gltxs.push_back(gltx);
	_terms.push_back(term);
	_suggest.push_back(suggest);
    }

    // Adjust amounts for existing payments
    if (_curr.id() != INVALID_ID) {
	for (unsigned int i = 0; i < _curr.payments().size(); ++i) {
	    const PaymentLine& line = _curr.payments()[i];

	    for (unsigned int j = 0; j < _gltxs.size(); ++j) {
		const Gltx& gltx = _gltxs[j];
		if (gltx.id() != line.gltx_id) continue;
		_table->setCellValue(j, 5, -line.amount);
		_table->setCellValue(j, 6, -line.discount);
		_table->setCellValue(j, 7, true);
		break;
	    }
	}
    }

    if (_table->rows() > 0) _table->setCurrentCell(0, 5);
    QApplication::restoreOverrideCursor();
}

void
ChequeVendor::recalculate()
{
    fixed payment = 0.0;
    fixed discount = 0.0;
    for (int row = 0; row < _table->rows(); ++row) {
	payment += _table->cellValue(row, 5).toFixed();
	discount += _table->cellValue(row, 6).toFixed();
    }

    fixed amount = _amount->getFixed();
    fixed total = payment + discount + amount;

    _ending->setFixed(_current->getFixed() - payment);
    _payment->setFixed(payment);
    _discAmt->setFixed(discount);
    _total->setFixed(total);
}

fixed
ChequeVendor::getChequeNumber(Id bankId)
{
    Account account;
    if (_quasar->db()->lookup(bankId, account))
	return account.nextNumber();
    return 1;
}

void
ChequeVendor::cellMoved(int row, int)
{
    if (row == -1 || _info.isNull()) return;
    fixed disc_amt = _table->cellValue(row, 6).toFixed();
    _info->setInvoice(_gltxs[row], _terms[row], disc_amt);
}

void
ChequeVendor::cellChanged(int row, int col, Variant)
{
    fixed amt_due = _table->cellValue(row, 4).toFixed();
    fixed amt_paid = _table->cellValue(row, 5).toFixed();
    fixed disc_amt = _table->cellValue(row, 6).toFixed();
    bool pay = _table->cellValue(row, 7).toBool();

    switch (col) {
    case 5: // payment
	if (amt_paid + disc_amt > amt_due)
	    disc_amt = amt_due - amt_paid;
	if (amt_paid + disc_amt < amt_due)
	    disc_amt = 0.0;
	if (amt_paid + _suggest[row] == amt_due)
	    disc_amt = _suggest[row];

	if (disc_amt == 0.0)
	    _table->setCellValue(row, 6, "");
	else
	    _table->setCellValue(row, 6, disc_amt);
	break;
    case 6: // discount
	if (amt_paid + disc_amt > amt_due)
	    amt_paid = amt_due - disc_amt;

	if (amt_paid == 0.0)
	    _table->setCellValue(row, 5, "");
	else
	    _table->setCellValue(row, 5, amt_paid);
	break;
    case 7: // pay?
	if (pay && amt_paid == 0.0 && disc_amt == 0.0) {
	    amt_paid = amt_due - _suggest[row];
	    disc_amt = _suggest[row];
	    _table->setCellValue(row, 5, amt_paid);
	    _table->setCellValue(row, 6, disc_amt);
	} else if (!pay) {
	    amt_paid = 0.0;
	    disc_amt = 0.0;
	    _table->setCellValue(row, 5, "");
	    _table->setCellValue(row, 6, "");
	}
	break;
    }

    if (!_info.isNull())
	_info->setInvoice(_gltxs[row], _terms[row], disc_amt);

    bool new_pay = (amt_paid != 0.0 || disc_amt != 0.0);
    if (pay != new_pay) {
	_table->setCellValue(row, 7, new_pay);
    }

    recalculate();
}

void
ChequeVendor::focusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _table->currentRow();
    int col = _table->currentColumn();

    if (type == Table::MoveNext && col == 6) {
	if (row == _table->rows() - 1) {
	    leave = true;
	} else {
	    newRow = row + 1;
	    newCol = 5;
	}
    } else if (type == Table::MoveNext && col == 7) {
	if (row == _table->rows() - 1) {
	    leave = true;
	} else {
	    newRow = row + 1;
	    newCol = 5;
	}
    } else if (type == Table::MovePrev && col == 5) {
	if (row > 0) {
	    newRow  = row - 1;
	    newCol = 6;
	} else {
	    leave = true;
	}
    }
}

void
ChequeVendor::payReceiving(Id receive_id)
{
    Gltx gltx;
    if (!_quasar->db()->lookup(receive_id, gltx)) return;
    if (gltx.cardTotal() < 0.0) return;

    _vendor->setId(gltx.cardId());
    slotVendorChanged();

    for (int row = 0; row < _table->rows(); ++row) {
	if (_gltxs[row].id() != receive_id) continue;

	fixed amt_due = _table->cellValue(row, 4).toFixed();
	fixed disc_amt = _suggest[row];

	slotClearAll();
	_table->setCellValue(row, 5, amt_due - disc_amt);
	_table->setCellValue(row, 6, disc_amt);
	_table->setCellValue(row, 7, true);
	break;
    }

    recalculate();
}

void
ChequeVendor::slotRecurring()
{
    if (!saveItem(true)) return;

    RecurringMaster* screen = new RecurringMaster(_main);
    screen->setGltx(_curr.id());
    screen->show();
}
