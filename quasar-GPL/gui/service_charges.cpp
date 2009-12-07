// $Id: service_charges.cpp,v 1.17 2005/03/13 23:13:45 bpepers Exp $
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

#include "service_charges.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "card_adjust.h"
#include "customer.h"
#include "invoice.h"
#include "group.h"
#include "company.h"
#include "customer_select.h"
#include "gltx_select.h"
#include "date_popup.h"
#include "lookup_edit.h"
#include "store_lookup.h"
#include "account_lookup.h"
#include "list_view_item.h"
#include "grid.h"
#include "page_number.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include <algorithm>

ServiceCharges::ServiceCharges(MainWindow* main)
    : QuasarWindow(main, "ServiceCharges")
{
    _helpSource = "service_charges.html";

    QWidget* frame = new QWidget(this);

    QLabel* startLabel = new QLabel(tr("Start Date:"), frame);
    _start = new DatePopup(frame);
    startLabel->setBuddy(_start);

    QLabel* endLabel = new QLabel(tr("End Date:"), frame);
    _end = new DatePopup(frame);
    endLabel->setBuddy(_end);

    QLabel* storeLabel = new QLabel(tr("Store:"), frame);
    _store = new LookupEdit(new StoreLookup(_main, this), frame);
    _store->setFocusPolicy(ClickFocus);
    _store->setLength(20);
    storeLabel->setBuddy(_store);

    QLabel* accountLabel = new QLabel(tr("Account:"), frame);
    _account = new LookupEdit(new AccountLookup(_main, this), frame);
    _account->setFocusPolicy(ClickFocus);
    _account->setLength(20);
    accountLabel->setBuddy(_account);

    _charges = new ListView(frame);
    _charges->setAllColumnsShowFocus(true);
    _charges->setRootIsDecorated(false);
    _charges->setShowSortIndicator(true);
    _charges->addTextColumn(tr("Customer"), 50);
    _charges->addMoneyColumn(tr("Amount"));

    QFrame* buttons = new QFrame(frame);
    QPushButton* calc = new QPushButton(tr("Calculate"), buttons);
    QPushButton* post = new QPushButton(tr("Post"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);
    connect(calc, SIGNAL(clicked()), this, SLOT(slotCalculate()));
    connect(post, SIGNAL(clicked()), this, SLOT(slotPost()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(close()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(4);
    buttonGrid->setMargin(4);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(calc, 0, 1);
    buttonGrid->addWidget(post, 0, 2);
    buttonGrid->addWidget(cancel, 0, 3);

    QGridLayout* frameGrid = new QGridLayout(frame);
    frameGrid->setSpacing(6);
    frameGrid->setMargin(6);
    frameGrid->setRowStretch(2, 1);
    frameGrid->setColStretch(2, 1);
    frameGrid->addWidget(startLabel, 0, 0);
    frameGrid->addWidget(_start, 0, 1, AlignLeft | AlignVCenter);
    frameGrid->addWidget(storeLabel, 0, 3);
    frameGrid->addWidget(_store, 0, 4, AlignLeft | AlignVCenter);
    frameGrid->addWidget(endLabel, 1, 0);
    frameGrid->addWidget(_end, 1, 1, AlignLeft | AlignVCenter);
    frameGrid->addWidget(accountLabel, 1, 3);
    frameGrid->addWidget(_account, 1, 4, AlignLeft | AlignVCenter);
    frameGrid->addMultiCellWidget(_charges, 2, 2, 0, 4);
    frameGrid->addMultiCellWidget(buttons, 3, 3, 0, 4);

    Company company;
    _quasar->db()->lookup(company);

    if (company.lastServiceCharge().isNull()) {
	_start->setDate(QDate::currentDate() - 30);
	_end->setDate(QDate::currentDate());
    } else {
	_start->setDate(company.lastServiceCharge() + 1);
	_end->setDate(QDate::currentDate());
    }

    _store->setId(_quasar->defaultStore());
    _account->setId(company.chargeAccount());

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    if (_start->getDate().isNull()) {
	_start->setFocus();
    } else {
	_end->setFocus();
	_end->dateWidget()->selectAll();
    }

    setCentralWidget(frame);
    setCaption(tr("Service Charges"));
    finalize();
}

ServiceCharges::~ServiceCharges()
{
}

void
ServiceCharges::slotCalculate()
{
    QDate start = _start->getDate();
    QDate end = _end->getDate();
    int days = end - start + 1;

    // Validate data
    if (start.isNull()) {
	QString message = tr("A start date must be entered");
	QMessageBox::critical(this, tr("Error"), message);
	_start->setFocus();
	return;
    }
    if (end.isNull()) {
	QString message = tr("An end date must be entered");
	QMessageBox::critical(this, tr("Error"), message);
	_end->setFocus();
	return;
    }
    if (start > end) {
	QString message = tr("Invalid date range");
	QMessageBox::critical(this, tr("Error"), message);
	_start->setFocus();
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Select all customers
    vector<Customer> customers;
    CustomerSelect conditions;
    conditions.activeOnly = true;
    _quasar->db()->select(customers, conditions);

    // Get invoice terms
    _quasar->db()->invoiceTerms(_invoice_ids, _invoice_term_ids);

    // Setup progress dialog
    unsigned int custs = customers.size();
    QString message = tr("Calculating service charges...");
    QProgressDialog* progress = new QProgressDialog(message, tr("Cancel"),
						    custs, this, "foo", true);
    progress->setMinimumDuration(0);
    progress->setCaption(tr("Progress"));

    // Process each customer
    fixed totalCharges;
    _charges->clear();
    for (unsigned int i = 0; i < custs; ++i) {
	progress->setProgress(i);
	qApp->processEvents();
	if (progress->wasCancelled()) {
	    _charges->clear();
	    QApplication::restoreOverrideCursor();
	    QString message = tr("Service charge calculation cancelled");
	    QMessageBox::information(this, tr("Cancelled"), message);
	    return;
	}

	const Customer& customer = customers[i];
	if (customer.serviceCharge() == 0.0) continue;
	double dailyRate = customer.serviceCharge().toDouble() * 1000.0/365.0;

	// Get all transactions for customer
	GltxSelect conditions;
	conditions.activeOnly = true;
	conditions.card_id = customer.id();
	_quasar->db()->select(_gltxs, conditions);
	if (_gltxs.size() == 0) continue;

	// Remove all that don't effect card balance
	for (int k = _gltxs.size() - 1; k >= 0; --k) {
	    const Gltx& gltx = _gltxs[k];
	    if (gltx.cardTotal() != 0.0) continue;
	    _gltxs.erase(_gltxs.begin() + k);
	}

	// Sort by date
	std::sort(_gltxs.begin(), _gltxs.end());

	// Allocate unallocated payments
	for (unsigned int j = 0; j < _gltxs.size(); ++j) {
	    const Gltx& payment = _gltxs[j];
	    if (payment.cardTotal() >= 0.0) continue;
	    if (payment.cardTotal() == payment.paymentTotal()) continue;

	    fixed alloc = -(payment.cardTotal() - payment.paymentTotal());
	    for (unsigned int k = 0; k < _gltxs.size(); ++k) {
		Gltx& gltx = _gltxs[k];
		if (gltx.cardTotal() <= 0.0) continue;
		if (gltx.cardTotal() == gltx.paymentTotal()) continue;

		fixed pay = gltx.cardTotal() - gltx.paymentTotal();
		if (pay > alloc) pay = alloc;

		gltx.payments().push_back(PaymentLine(payment.id(), pay, 0.0));
		alloc -= pay;
		if (alloc == 0.0) break;
	    }

	    // If not all allocated then nothing left to allocate against
	    if (alloc != 0.0) break;
	}

	// Calculate service charge total
	double totalCharge = 0.0;
	for (unsigned int j = 0; j < _gltxs.size(); ++j) {
	    const Gltx& gltx = _gltxs[j];
	    if (gltx.cardTotal() < 0.0) continue;

	    for (int day = 0; day < days; ++day) {
	        fixed balance = balanceOn(gltx, start + day);
		double charge = balance.toDouble() * dailyRate;
		totalCharge += charge;
	    }
	}

	// Adjust and round
	fixed serviceCharge = totalCharge / 100000.0;
	serviceCharge.moneyRound();
	if (serviceCharge <= 0.0) continue;

	ListViewItem* item = new ListViewItem(_charges, customer.id());
	item->setValue(0, customer.name());
	item->setValue(1, serviceCharge);
	totalCharges += serviceCharge;
    }

    ListViewItem* item = new ListViewItem(_charges);
    item->isLast = true;
    item->setValue(0, "Total");
    item->setValue(1, totalCharges);

    progress->setProgress(custs);
    delete progress;

    QApplication::restoreOverrideCursor();
    message = tr("Service charge calculation is complete");
    QMessageBox::information(this, tr("Complete"), message);
}

void
ServiceCharges::slotPost()
{
    QDate end = _end->getDate();
    Id store_id = _store->getId();
    Id account_id = _account->getId();

    // Validate data
    if (store_id == INVALID_ID) {
	QString message = tr("A store must be entered");
	QMessageBox::critical(this, tr("Error"), message);
	_store->setFocus();
	return;
    }
    if (account_id == INVALID_ID) {
	QString message = tr("An account must be entered");
	QMessageBox::critical(this, tr("Error"), message);
	_account->setFocus();
	return;
    }

    QString message = tr("Are you sure you want to post the charges?");
    int ch = QMessageBox::warning(this, tr("Continue?"), message,
				  QMessageBox::No, QMessageBox::Yes);
    if (ch != QMessageBox::Yes) return;

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    QListViewItemIterator it(_charges);
    for (; it.current(); ++it) {
	ListViewItem* item = (ListViewItem*)it.current();
	Id customer_id = item->id;
	fixed amount = item->value(1).toFixed();
	if (customer_id == INVALID_ID) continue;

	Customer customer;
	_quasar->db()->lookup(customer_id, customer);

	// Generate service charge transaction
	CardAdjust adjustment;
	adjustment.setPostDate(end);
	adjustment.setPostTime(QTime(0, 0, 0));
	adjustment.setMemo(tr("Service Charges"));
	adjustment.setCardId(customer_id);
	adjustment.setStoreId(store_id);

	CardLine cardLine(customer_id, amount);
	adjustment.cards().push_back(cardLine);

	// Add account lines
	Id receivables_id = customer.accountId();
	adjustment.accounts().push_back(AccountLine(receivables_id, amount));
	adjustment.accounts().push_back(AccountLine(account_id, -amount));

	// Post adjustment
	_quasar->db()->create(adjustment);
    }

    Company orig, company;
    _quasar->db()->lookup(orig);
    company = orig;

    company.setLastServiceCharge(end);
    company.setChargeAccount(account_id);
    _quasar->db()->update(orig, company);

    QApplication::restoreOverrideCursor();
    message = tr("The service charges have been posted");
    QMessageBox::information(this, tr("Posted"), message);

    close();
}

fixed
ServiceCharges::balanceOn(const Gltx& gltx, QDate date)
{
    QDate dueDate = gltx.postDate();
    if (gltx.dataType() == DataObject::INVOICE) {
	Id term_id = invoiceTermId(gltx.id());
	if (term_id != INVALID_ID) {
	    Term term;
	    if (findTerm(term_id, term))
		dueDate = gltx.postDate() + term.dueDays();
	}
    }
    if (date < dueDate) return 0.0;

    fixed balance = gltx.cardTotal();
    for (unsigned int i = 0; i < gltx.payments().size(); ++i) {
	Id payment_id = gltx.payments()[i].gltx_id;
	fixed amount = gltx.payments()[i].amount;
	Gltx payment;
	if (!findGltx(payment_id, payment))
	    continue;
	if (payment.postDate() <= date)
	    balance -= amount;
    }

    return balance;
}

bool
ServiceCharges::findGltx(Id gltx_id, Gltx& gltx)
{
    for (unsigned int i = 0; i < _gltxs.size(); ++i) {
	if (_gltxs[i].id() == gltx_id) {
	    gltx = _gltxs[i];
	    return true;
	}
    }
    return false;
}

bool
ServiceCharges::findTerm(Id term_id, Term& term)
{
    for (unsigned int i = 0; i < _terms.size(); ++i) {
	if (_terms[i].id() == term_id) {
	    term = _terms[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(term_id, term))
	return false;

    _terms.push_back(term);
    return true;
}

Id
ServiceCharges::invoiceTermId(Id invoice_id)
{
    for (unsigned int i = 0; i < _invoice_ids.size(); ++i) {
	if (_invoice_ids[i] == invoice_id)
	    return _invoice_term_ids[i];
    }
    return INVALID_ID;
}
