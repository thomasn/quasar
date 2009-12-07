// $Id: receipt_master.cpp,v 1.58 2005/03/13 23:13:45 bpepers Exp $
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

#include "receipt_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "invoice_info.h"
#include "invoice.h"
#include "account.h"
#include "tender.h"
#include "customer.h"
#include "company.h"
#include "receipt_select.h"
#include "invoice_select.h"
#include "gltx_frame.h"
#include "table.h"
#include "date_popup.h"
#include "id_edit.h"
#include "money_edit.h"
#include "check_box.h"
#include "lookup_edit.h"
#include "account_lookup.h"
#include "tender_lookup.h"
#include "customer_lookup.h"
#include "store_lookup.h"
#include "term_lookup.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qtabwidget.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <algorithm>

ReceiptMaster::ReceiptMaster(MainWindow* main, Id receipt_id)
    : DataWindow(main, "CustomerPayment", receipt_id)
{
    _helpSource = "customer_payment.html";

    // Layouts
    QVBoxLayout* vbox1 = new QVBoxLayout(_frame);
    vbox1->setSpacing(6);
    vbox1->setMargin(6);

    _gltxFrame = new GltxFrame(main, tr("Receipt No."), _frame);
    _gltxFrame->hideMemo();
    vbox1->addWidget(_gltxFrame);

    QFrame* top = new QFrame(_frame);
    top->setFrameStyle(QFrame::Raised | QFrame::Panel);

    QLabel* custLabel = new QLabel(tr("Customer:"), top);
    _customer = new LookupEdit(new CustomerLookup(main, this), top);
    _customer->setLength(30);
    custLabel->setBuddy(_customer);
    connect(_customer, SIGNAL(validData()), SLOT(slotCustomerChanged()));

    QLabel* tenderLabel = new QLabel(tr("Tender:"), top);
    _tender = new LookupEdit(new TenderLookup(main, this), top);
    _tender->setLength(10);
    tenderLabel->setBuddy(_tender);
    connect(_tender, SIGNAL(validData()), SLOT(slotTenderChanged()));

    QLabel* amountLabel = new QLabel(tr("Amount:"), top);
    _amount = new MoneyEdit(top);
    amountLabel->setBuddy(_amount);
    connect(_amount, SIGNAL(validData()), SLOT(slotAmountChanged()));

    QLabel* balanceLabel = new QLabel(tr("Balance:"), top);
    _balance = new MoneyEdit(top);
    _balance->setFocusPolicy(NoFocus);

    QLabel* discLabel = new QLabel(tr("Discount Account:"), top);
    AccountLookup* discLook = new AccountLookup(main, this, Account::Expense);
    _discount = new LookupEdit(discLook, top);
    _discount->setLength(20);
    _discount->setFocusPolicy(ClickFocus);
    discLabel->setBuddy(_discount);

    QLabel* discAmtLabel = new QLabel(tr("Discount Amount:"), top);
    _discAmt = new MoneyEdit(top);
    _discAmt->setFocusPolicy(NoFocus);

    QGridLayout* topGrid = new QGridLayout(top);
    topGrid->setSpacing(3);
    topGrid->setMargin(3);
    topGrid->setColStretch(2, 1);
    topGrid->addWidget(custLabel, 0, 0);
    topGrid->addWidget(_customer, 0, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(balanceLabel, 0, 3);
    topGrid->addWidget(_balance, 0, 4, AlignLeft | AlignVCenter);
    topGrid->addWidget(tenderLabel, 1, 0);
    topGrid->addWidget(_tender, 1, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(discLabel, 1, 3);
    topGrid->addWidget(_discount, 1, 4, AlignLeft | AlignVCenter);
    topGrid->addWidget(amountLabel, 2, 0);
    topGrid->addWidget(_amount, 2, 1, AlignLeft | AlignVCenter);
    topGrid->addWidget(discAmtLabel, 2, 3);
    topGrid->addWidget(_discAmt, 2, 4, AlignLeft | AlignVCenter);
    vbox1->addWidget(top);

    QTabWidget* tabs = new QTabWidget(_frame);
    tabs->setFocusPolicy(ClickFocus);
    QFrame* alloc = new QFrame(tabs);
    QFrame* tenders = new QFrame(tabs);
    QFrame* refs = new QFrame(tabs);
    tabs->addTab(alloc, tr("Allocations"));
    tabs->addTab(tenders, tr("Tenders"));
    tabs->addTab(refs, tr("References"));
    vbox1->addWidget(tabs);

    _table = new Table(alloc);
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
    new MoneyColumn(_table, tr("Discount"));
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

    QPushButton* payOld = new QPushButton(tr("Oldest"), alloc);
    payOld->setFocusPolicy(NoFocus);
    connect(payOld, SIGNAL(clicked()), SLOT(slotPayOldest()));

    QPushButton* clearAll = new QPushButton(tr("Clear"), alloc);
    clearAll->setFocusPolicy(NoFocus);
    connect(clearAll, SIGNAL(clicked()), SLOT(slotClearAll()));

    QPushButton* invoiceInfo = new QPushButton(tr("Info"), alloc);
    invoiceInfo->setFocusPolicy(NoFocus);
    connect(invoiceInfo, SIGNAL(clicked()), SLOT(slotInvoiceInfo()));

    QLabel* allocLabel = new QLabel(tr("Allocated:"), alloc);
    _alloc = new MoneyEdit(alloc);
    _alloc->setLength(14);
    _alloc->setFocusPolicy(NoFocus);

    QLabel* diffLabel = new QLabel(tr("Difference:"), alloc);
    _diff = new MoneyEdit(alloc);
    _diff->setLength(14);
    _diff->setFocusPolicy(NoFocus);

    QGridLayout* allocGrid = new QGridLayout(alloc);
    allocGrid->setSpacing(3);
    allocGrid->setRowStretch(3, 1);
    allocGrid->setColStretch(2, 1);
    allocGrid->addMultiCellWidget(_table, 0, 3, 0, 3);
    allocGrid->addWidget(payOld, 0, 4);
    allocGrid->addWidget(clearAll, 1, 4);
    allocGrid->addWidget(invoiceInfo, 2, 4);
    allocGrid->addWidget(allocLabel, 4, 0, AlignLeft | AlignVCenter);
    allocGrid->addWidget(_alloc, 4, 1, AlignLeft | AlignVCenter);
    allocGrid->addWidget(diffLabel, 4, 2, AlignRight | AlignVCenter);
    allocGrid->addWidget(_diff, 4, 3, AlignLeft | AlignVCenter);

    _tenders = new Table(tenders);
    _tenders->setVScrollBarMode(QScrollView::AlwaysOn);
    _tenders->setDisplayRows(3);
    connect(_tenders, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(tenderCellChanged(int,int,Variant)));
    connect(_tenders, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(tenderFocusNext(bool&,int&,int&,int)));

    TenderLookup* tendLookup = new TenderLookup(_main, this);

    new LookupColumn(_tenders, tr("Tender"), 20, tendLookup);
    new MoneyColumn(_tenders, tr("Amount"));
    new NumberColumn(_tenders, tr("Rate"), 3);
    new MoneyColumn(_tenders, tr("Total"));

    new LookupEditor(_tenders, 0, new LookupEdit(tendLookup, _tenders));
    new NumberEditor(_tenders, 1, new MoneyEdit(_tenders));

    QGridLayout* tendGrid = new QGridLayout(tenders);
    tendGrid->setSpacing(3);
    tendGrid->setMargin(3);
    tendGrid->setRowStretch(0, 1);
    tendGrid->setColStretch(0, 1);
    tendGrid->addWidget(_tenders, 0, 0);

    _refs = new Table(refs);
    _refs->setVScrollBarMode(QScrollView::AlwaysOn);
    _refs->setDisplayRows(3);
    connect(_refs, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(referenceFocusNext(bool&,int&,int&,int)));

    new TextColumn(_refs, tr("Name"), 20);
    new TextColumn(_refs, tr("Data"), 20);
    new LineEditor(_refs, 1, new LineEdit(20, _refs));

    QGridLayout* refGrid = new QGridLayout(refs);
    refGrid->setSpacing(3);
    refGrid->setMargin(3);
    refGrid->setRowStretch(0, 1);
    refGrid->setColStretch(0, 1);
    refGrid->addWidget(_refs, 0, 0);

    _inactive->setText(tr("Voided?"));

    setCaption(tr("Customer Payment"));
    finalize();
}

ReceiptMaster::~ReceiptMaster()
{
    if (!_info.isNull())
	delete _info;
}

void
ReceiptMaster::setStoreId(Id store_id)
{
    if (store_id != INVALID_ID)
	_gltxFrame->store->setId(store_id);
}

void
ReceiptMaster::setCardId(Id card_id)
{
    if (card_id != INVALID_ID) {
	_customer->setId(card_id);
	slotCustomerChanged();
    }
}

void
ReceiptMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _customer;
}

void
ReceiptMaster::newItem()
{
    Company company;
    _quasar->db()->lookup(company);
    Id discId = company.customerTermsAcct();

    Receipt blank;
    _orig = blank;
    _gltxFrame->defaultData(_orig);
    _orig.setDiscountId(discId);

    _curr = _orig;
    _firstField = _customer;
}

void
ReceiptMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _gltxFrame->cloneData(_curr);
    dataToWidget();
}

bool
ReceiptMaster::fileItem()
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

    if (_curr.number() != "#")
	if (checkGltxUsed(_curr.number(), DataObject::RECEIPT, _curr.id()))
	    return false;

    if (_orig.id() == INVALID_ID) {
	if (!_quasar->db()->create(_curr)) return false;
    } else {
	if (!_quasar->db()->update(_orig, _curr)) return false;
    }

    Company company;
    _quasar->db()->lookup(company);

    if (company.customerTermsAcct() == INVALID_ID) {
	Company orig = company;
	company.setCustomerTermsAcct(_curr.discountId());
	_quasar->db()->update(orig, company);
    }

    _orig = _curr;
    _id = _curr.id();

    return true;
}

bool
ReceiptMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
ReceiptMaster::restoreItem()
{
    _curr = _orig;
}

void
ReceiptMaster::cloneItem()
{
    ReceiptMaster* clone = new ReceiptMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
ReceiptMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
ReceiptMaster::dataToWidget()
{
    _gltxFrame->setData(_curr);
    _customer->setId(_curr.cardId());
    _discount->setId(_curr.discountId());
    _amount->setFixed(_curr.total());
    _inactive->setChecked(!_curr.isActive());

    // Load the tenders
    _tenders->clear();
    const vector<TenderLine>& tenders = _curr.tenders();
    unsigned int i;
    for (i = 0; i < tenders.size(); ++i) {
	Id tender_id = tenders[i].tender_id;
	fixed amount = tenders[i].amount;
	fixed conv_rate = tenders[i].conv_rate;
	fixed conv_amt = tenders[i].conv_amt;
	bool voided = tenders[i].voided;
	if (voided) continue;

	VectorRow* row = new VectorRow(_tenders->columns());
	row->setValue(0, tender_id);
	row->setValue(1, conv_amt);
	row->setValue(2, conv_rate);
	row->setValue(3, amount);
	_tenders->appendRow(row);

	if (i == 0) {
	    _tender->setId(tender_id);
	} else {
	    _tender->setId(INVALID_ID);
	    _tender->setEnabled(false);
	}
    }
    _tenders->appendRow(new VectorRow(_tenders->columns()));

    // Load the references
    _refs->clear();
    for (i = 0; i < _curr.referenceName().size(); ++i) {
	VectorRow* row = new VectorRow(_refs->columns());
	row->setValue(0, _curr.referenceName()[i]);
	row->setValue(1, _curr.referenceData()[i]);
	_refs->appendRow(row);
    }

    slotRefresh();
    recalculate();
}

// Set the data object from the widgets.
void
ReceiptMaster::widgetToData()
{
    _gltxFrame->getData(_curr);
    _curr.setCardId(_customer->getId());
    _curr.setDiscountId(_discount->getId());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    // Setup account lines
    vector<AccountLine>& lines = _curr.accounts();
    lines.clear();
    Id store_id = _curr.storeId();

    // Add the tenders
    _curr.tenders().clear();
    int row;
    for (row = 0; row < _tenders->rows(); ++row) {
	Id tender_id = _tenders->cellValue(row, 0).toId();
	if (tender_id == INVALID_ID) continue;
	fixed conv_amt = _tenders->cellValue(row, 1).toFixed();
	fixed conv_rate = _tenders->cellValue(row, 2).toFixed();
	fixed amount = _tenders->cellValue(row, 3).toFixed();

	Tender tender;
	_quasar->db()->lookup(tender_id, tender);

	TenderLine line;
	line.tender_id = tender_id;
	line.amount = amount;
	line.conv_rate = conv_rate;
	line.conv_amt = conv_amt;
	line.is_change = (amount < 0.0);
	// TODO: auth, ...
	_curr.tenders().push_back(line);

	lines.push_back(AccountLine(tender.accountId(), amount));
    }

    // Add all the references
    _curr.referenceName().clear();
    _curr.referenceData().clear();
    for (row = 0; row < _refs->rows(); ++row) {
	QString name = _refs->cellValue(row, 0).toString();
	QString data = _refs->cellValue(row, 1).toString();
	if (name.isEmpty()) continue;

	_curr.referenceName().push_back(name);
	_curr.referenceData().push_back(data);
    }

    // Total up discounts and amounts
    fixed amount = _amount->getFixed();
    fixed disc_amt = 0.0;
    for (row = 0; row < _table->rows(); ++row) {
	fixed discount = _table->cellValue(row, 6).toFixed();
	if (_table->cellValue(row, 5).toFixed() != 0.0)
	    disc_amt += discount;
    }
    fixed total = amount + disc_amt;

    // Add AR line
    if (_curr.cardId() != INVALID_ID) {
	Customer customer;
	_quasar->db()->lookup(_curr.cardId(), customer);
	lines.push_back(AccountLine(customer.accountId(), -total));

	// Add card line
	_curr.cards().clear();
	_curr.cards().push_back(CardLine(_curr.cardId(), -total));
    }

    // Add discount
    if (disc_amt != 0.0)
	lines.push_back(AccountLine(_discount->getId(), disc_amt));

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
ReceiptMaster::slotCustomerChanged()
{
    _table->clear();
    _balance->setFixed(0.0);
    _refs->clear();

    if (_customer->getId() != INVALID_ID) {
	Customer customer;
	_quasar->db()->lookup(_customer->getId(), customer);

	for (unsigned int i = 0; i < customer.references().size(); ++i) {
	    QString name = customer.references()[i];

	    // Ask for reference
	    QString caption = tr("Reference Information");
	    QString label = tr("%1:").arg(name);
	    bool ok = false;
	    QString data = QInputDialog::getText(caption, label,
						 QLineEdit::Normal,
						 "", &ok, this);
	    if (!ok) {
		_customer->setId(INVALID_ID);
		slotCustomerChanged();
		return;
	    }

	    VectorRow* row = new VectorRow(_refs->columns());
	    row->setValue(0, name);
	    row->setValue(1, data);
	    _refs->appendRow(row);
	}

	slotRefresh();
    }
    recalculate();
}

void
ReceiptMaster::slotTenderChanged()
{
    _tenders->clear();

    if (_tender->getId() != INVALID_ID) {
	Tender tender;
	_quasar->db()->lookup(_tender->getId(), tender);

	fixed amount = _amount->getFixed();
	fixed convertRate = tender.convertRate();
	fixed convertAmt = amount / convertRate;

	// Fix up but may cause rounding problems
	convertAmt.moneyRound();
	amount = convertAmt * convertRate;
	amount.moneyRound();

	VectorRow* row = new VectorRow(_tenders->columns());
	row->setValue(0, _tender->getId());
	row->setValue(1, convertAmt);
	row->setValue(2, convertRate);
	row->setValue(3, amount);
	_tenders->appendRow(row);
    }

    _tenders->appendRow(new VectorRow(_tenders->columns()));
    recalculate();
}

void
ReceiptMaster::slotAmountChanged()
{
    if (_alloc->getFixed() == 0.0) {
	fixed amount = _amount->getFixed();

	slotClearAll();
	for (int row = 0; row < _table->rows(); ++row) {
	    fixed amt_due = _table->cellValue(row, 4).toFixed();
	    if (amount == amt_due) {
		_table->setCellValue(row, 5, amount);
		_table->setCellValue(row, 7, true);
		break;
	    }

	    fixed disc_amt = _suggest[row];
	    if (amount + disc_amt == amt_due && disc_amt != 0.0) {
		_table->setCellValue(row, 5, amount);
		_table->setCellValue(row, 6, disc_amt);
		_table->setCellValue(row, 7, true);
		break;
	    }
	}
    }
    recalculate();
}

void
ReceiptMaster::slotPayClicked()
{
    int row = _table->currentRow();
    if (row < 0 || row >= _table->rows())
	return;

    // Just get value which should trigger cell changed
    _table->cellValue(row, 7).toBool();
}

void
ReceiptMaster::slotPayOldest()
{
    fixed amount = _amount->getFixed();
    if (amount == 0.0) return;
    if (_table->rows() == 0) return;

    slotClearAll();
    for (int row = 0; row < _table->rows() && amount > 0.0; ++row) {
	fixed disc_amt = _suggest[row];
	fixed amt_due = _table->cellValue(row, 4).toFixed();
	fixed pay_amt = amt_due - disc_amt;

	fixed alloc = QMIN(amount, pay_amt);
	if (alloc < pay_amt) disc_amt = 0.0;

	_table->setCellValue(row, 5, alloc);
	if (disc_amt == 0.0)
	    _table->setCellValue(row, 6, "");
	else
	    _table->setCellValue(row, 6, disc_amt);
	_table->setCellValue(row, 7, true);
	amount -= alloc;
    }
    recalculate();
}

void
ReceiptMaster::slotClearAll()
{
    for (int row = 0; row < _table->rows(); ++row) {
	_table->setCellValue(row, 5, "");
	_table->setCellValue(row, 6, "");
	_table->setCellValue(row, 7, false);
    }
    recalculate();
}

void
ReceiptMaster::slotInvoiceInfo()
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
ReceiptMaster::setDiscount(fixed new_disc)
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
ReceiptMaster::slotRefresh()
{
    if (_customer->getId() == INVALID_ID) {
	_balance->setFixed(0);
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Customer customer;
    _quasar->db()->lookup(_customer->getId(), customer);
    fixed balance = _quasar->db()->cardBalance(customer.id());
    _balance->setFixed(balance);

    // Load transactions
    GltxSelect conditions;
    conditions.card_id = customer.id();
    conditions.unpaid = true;
    conditions.activeOnly = true;
    vector<Gltx> gltxs;
    _quasar->db()->select(gltxs, conditions);

    // Load ones that have been paid
    if (_curr.id() != INVALID_ID && _curr.cardId() == customer.id()) {
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
    for (unsigned int i = 0; i < gltxs.size(); ++i) {
	Gltx& gltx = gltxs[i];

	// Get terms if its a customer invoice
	Term term;
	if (gltx.dataType() == DataObject::INVOICE) {
	    Invoice invoice;
	    _quasar->db()->lookup(gltx.id(), invoice);
	    _quasar->db()->lookup(invoice.termsId(), term);
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

	QDate dueDate = gltx.postDate() + term.dueDays();
	QDate discDate = gltx.postDate() + term.discountDays();

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
ReceiptMaster::tenderCellChanged(int row, int col, Variant old)
{
    // If not blank and last row, append row
    if (row == _tenders->rows() - 1 && col == 0) {
	Variant id = _tenders->cellValue(row, col);
	if (!id.isNull() && id.toId() != INVALID_ID)
	    _tenders->appendRow(new VectorRow(_tenders->columns()));
    }

    // Check for other changes
    Id tender_id;
    switch (col) {
    case 0: // tender_id
	tender_id = _tenders->cellValue(row, 0).toId();
	if (tender_id != INVALID_ID) {
	    Tender tender;
	    _quasar->db()->lookup(tender_id, tender);

	    fixed remain = _amount->getFixed();
	    for (int i = 0; i < _tenders->rows(); ++i) {
		if (i == row) continue;
		if (_tenders->cellValue(i, 0).toId() == INVALID_ID) continue;
		remain -= _tenders->cellValue(i, 3).toFixed();
	    }

	    fixed convertRate = tender.convertRate();
	    fixed amount = remain / convertRate;
	    amount.moneyRound();

	    _tenders->setCellValue(row, 1, amount);
	    _tenders->setCellValue(row, 2, convertRate);
	    _tenders->setCellValue(row, 3, remain);
	} else {
	    _tenders->setCellValue(row, 1, "");
	    _tenders->setCellValue(row, 2, "");
	    _tenders->setCellValue(row, 3, "");
	}
	if (row != 0)
	    _tender->setId(INVALID_ID);
	else if (old.toId() == _tender->getId())
	    _tender->setId(tender_id);
	break;
    case 1: // amount
	if (!_tenders->cellValue(row, 2).toString().isEmpty()) {
	    fixed amount = _tenders->cellValue(row, 1).toFixed();
	    fixed rate = _tenders->cellValue(row, 2).toFixed();
	    _tenders->setCellValue(row, 3, amount * rate);
	}
	break;
    }
}

void
ReceiptMaster::tenderFocusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _tenders->currentRow();
    int col = _tenders->currentColumn();

    if (type == Table::MoveNext && col == 0) {
	Variant val = _tenders->cellValue(row, col);
	Id id = val.toId();
	if ((val.isNull() || id == INVALID_ID) && row == _tenders->rows()-1) {
	    leave = true;
	}
    } else if (type == Table::MoveNext && col == 1) {
	newCol = 0;
	newRow = row + 1;
    } else if (type == Table::MovePrev && col == 0) {
	if (row > 0) {
	    newCol = 1;
	    newRow = row - 1;
	}
    }
}

void
ReceiptMaster::referenceFocusNext(bool& leave, int& newRow, int& newCol, int type)
{
    int row = _refs->currentRow();
    int col = _refs->currentColumn();

    if (type == Table::MoveNext && col == 1) {
	if (row == _tenders->rows()-1) {
	    leave = true;
	} else {
	    newRow = row + 1;
	    newCol = 1;
	}
    } else if (type == Table::MovePrev && col == 1) {
	if (row > 0) {
	    newCol = 1;
	    newRow = row - 1;
	}
    }
}

void
ReceiptMaster::recalculate()
{
    static bool in_recalculate = false;
    if (in_recalculate) return;
    in_recalculate = true;

    fixed payment = 0.0;
    fixed discount = 0.0;
    for (int row = 0; row < _table->rows(); ++row) {
	payment += _table->cellValue(row, 5).toFixed();
	discount += _table->cellValue(row, 6).toFixed();
    }

    fixed amount = _amount->getFixed();
    _discAmt->setFixed(discount);
    _alloc->setFixed(payment);
    _diff->setFixed(amount - payment);

    // Update tender amount if single tender
    if (_tender->getId() != INVALID_ID)
	slotTenderChanged();

    in_recalculate = false;
}

void
ReceiptMaster::cellMoved(int row, int)
{
    if (row == -1 || _info.isNull()) return;
    fixed disc_amt = _table->cellValue(row, 6).toFixed();
    _info->setInvoice(_gltxs[row], _terms[row], disc_amt);
}

void
ReceiptMaster::cellChanged(int row, int col, Variant)
{
    fixed amt_due = _table->cellValue(row, 4).toFixed();
    fixed amt_paid = _table->cellValue(row, 5).toFixed();
    fixed disc_amt = _table->cellValue(row, 6).toFixed();
    bool pay = _table->cellValue(row, 7).toBool();

    switch (col) {
    case 5:
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
    case 6:
	if (amt_paid + disc_amt > amt_due)
	    amt_paid = amt_due - disc_amt;

	if (amt_paid == 0.0)
	    _table->setCellValue(row, 5, "");
	else
	    _table->setCellValue(row, 5, amt_paid);
	break;
    case 7:
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
ReceiptMaster::focusNext(bool& leave, int& newRow, int& newCol, int type)
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
ReceiptMaster::payInvoice(Id invoice_id)
{
    Gltx gltx;
    if (!_quasar->db()->lookup(invoice_id, gltx)) return;
    if (gltx.cardTotal() < 0.0) return;

    _customer->setId(gltx.cardId());
    slotCustomerChanged();

    for (int row = 0; row < _table->rows(); ++row) {
	if (_gltxs[row].id() != invoice_id) continue;

	fixed amt_due = _table->cellValue(row, 4).toFixed();
	fixed disc_amt = _suggest[row];
	_amount->setFixed(amt_due - disc_amt);

	slotClearAll();
	_table->setCellValue(row, 5, amt_due - disc_amt);
	_table->setCellValue(row, 6, disc_amt);
	_table->setCellValue(row, 7, true);
	break;
    }

    recalculate();
}
