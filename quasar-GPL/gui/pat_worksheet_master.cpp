// $Id: pat_worksheet_master.cpp,v 1.24 2005/03/13 23:13:45 bpepers Exp $
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

#include "pat_worksheet_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "pat_worksheet_select.h"
#include "pat_group.h"
#include "customer.h"
#include "vendor.h"
#include "dept.h"
#include "card_adjust.h"
#include "money_edit.h"
#include "double_edit.h"
#include "integer_edit.h"
#include "date_popup.h"
#include "list_view_item.h"
#include "lookup_edit.h"
#include "account_lookup.h"
#include "pat_group_lookup.h"
#include "account.h"
#include "grid.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qmessagebox.h>

PatWorksheetMaster::PatWorksheetMaster(MainWindow* main, Id worksheet_id)
    : DataWindow(main, "PatWorksheetMaster", worksheet_id),
      _cache(main->quasar()->db())
{
    _helpSource = "pat_worksheet_master.html";

    QPushButton* reload = new QPushButton(tr("Reload"), _buttons);
    connect(reload, SIGNAL(clicked()), SLOT(slotReload()));

    QPushButton* print = new QPushButton(tr("Print"), _buttons);
    connect(print, SIGNAL(clicked()), SLOT(slotPrint()));

    QPushButton* post = new QPushButton(tr("Post"), _buttons);
    connect(post, SIGNAL(clicked()), SLOT(slotPost()));

    // Create widgets
    QLabel* descLabel = new QLabel(tr("Description:"), _frame);
    _desc = new LineEdit(_frame);
    _desc->setLength(40);
    descLabel->setBuddy(_desc);

    QLabel* fromLabel = new QLabel(tr("From Date:"), _frame);
    _from = new DatePopup(_frame);
    fromLabel->setBuddy(_from);

    QLabel* toLabel = new QLabel(tr("To Date:"), _frame);
    _to = new DatePopup(_frame);
    toLabel->setBuddy(_to);

    QLabel* equityLabel = new QLabel(tr("Equity Amount:"), _frame);
    _equity_amt = new MoneyEdit(_frame);
    equityLabel->setBuddy(_equity_amt);
    connect(_equity_amt, SIGNAL(validData()), SLOT(slotRefreshTotals()));

    QLabel* equityIdLabel = new QLabel(tr("Equity Account:"), _frame);
    AccountLookup* lk1 = new AccountLookup(main, this, Account::Expense);
    _equity_id = new LookupEdit(lk1, _frame);
    _equity_id->setLength(30);
    equityIdLabel->setBuddy(_equity_id);

    QLabel* equityMemoLabel = new QLabel(tr("Equity Memo:"), _frame);
    _equity_memo = new LineEdit(_frame);
    _equity_memo->setMaxLength(40);
    _equity_memo->setMinimumWidth(_equity_memo->fontMetrics().width('x') * 40);
    equityMemoLabel->setBuddy(_equity_memo);

    QLabel* creditLabel = new QLabel(tr("Credit Amount:"), _frame);
    _credit_amt = new MoneyEdit(_frame);
    creditLabel->setBuddy(_credit_amt);
    connect(_credit_amt, SIGNAL(validData()), SLOT(slotRefreshTotals()));

    QLabel* creditIdLabel = new QLabel(tr("Credit Account:"), _frame);
    AccountLookup* lk2 = new AccountLookup(main, this, Account::Expense);
    _credit_id = new LookupEdit(lk2, _frame);
    _credit_id->setLength(30);
    creditIdLabel->setBuddy(_credit_id);

    QLabel* creditMemoLabel = new QLabel(tr("Credit Memo:"), _frame);
    _credit_memo = new LineEdit(_frame);
    _credit_memo->setMaxLength(40);
    _credit_memo->setMinimumWidth(_credit_memo->fontMetrics().width('x') * 40);
    creditMemoLabel->setBuddy(_credit_memo);

    _tabs = new QTabWidget(_frame);
    QFrame* totals = new QFrame(_tabs);
    QFrame* sales = new QFrame(_tabs);
    QFrame* purchases = new QFrame(_tabs);
    QFrame* balances = new QFrame(_tabs);
    QFrame* adjusts = new QFrame(_tabs);
    _tabs->addTab(totals, tr("Totals"));
    _tabs->addTab(sales, tr("Sales"));
    _tabs->addTab(purchases, tr("Purchases"));
    _tabs->addTab(balances, tr("Balance Change"));
    _tabs->addTab(adjusts, tr("Adjustments"));

    _totals = new ListView(totals);
    _totals->setAllColumnsShowFocus(true);
    _totals->setShowSortIndicator(true);
    _totals->addTextColumn("Patronage Group", 20);
    _totals->addNumberColumn("Sales", 10);
    _totals->addNumberColumn("Purchases", 8);
    _totals->addNumberColumn("Balances", 8);
    _totals->addNumberColumn("Adjusts", 8);
    _totals->addNumberColumn("Total Points", 12);
    _totals->addMoneyColumn("Equity", 8);
    _totals->addMoneyColumn("Credit", 8);
    _totals->addMoneyColumn("Total Alloc", 8);

    QGridLayout* totalGrid = new QGridLayout(totals);
    totalGrid->setSpacing(3);
    totalGrid->setMargin(3);
    totalGrid->addWidget(_totals, 0, 0);

    _sales = new ListView(sales);
    _sales->setAllColumnsShowFocus(true);
    _sales->setShowSortIndicator(true);
    _sales->setSorting(0);

    _sales_cust_detail = new QCheckBox(tr("Customer Detail?"), sales);
    _sales_dept_detail = new QCheckBox(tr("Department Detail?"), sales);
    connect(_sales_cust_detail, SIGNAL(toggled(bool)),
	    SLOT(slotSalesReformat()));
    connect(_sales_dept_detail, SIGNAL(toggled(bool)),
	    SLOT(slotSalesReformat()));

    QGridLayout* salesGrid = new QGridLayout(sales);
    salesGrid->setSpacing(3);
    salesGrid->setMargin(3);
    salesGrid->setRowStretch(0, 1);
    salesGrid->setColStretch(1, 1);
    salesGrid->addMultiCellWidget(_sales, 0, 0, 0, 2);
    salesGrid->addWidget(_sales_cust_detail, 1, 0);
    salesGrid->addWidget(_sales_dept_detail, 1, 2);

    _purchases = new ListView(purchases);
    _purchases->setAllColumnsShowFocus(true);
    _purchases->setShowSortIndicator(true);
    _purchases->setSorting(0);

    _purch_vend_detail = new QCheckBox(tr("Vendor Detail?"), purchases);
    connect(_purch_vend_detail, SIGNAL(toggled(bool)),
	    SLOT(slotPurchasesReformat()));

    QLabel* purchLabel = new QLabel(tr("Purchase Points:"), purchases);
    _purch_pnts = new DoubleEdit(purchases);
    _purch_pnts->setLength(6);
    purchLabel->setBuddy(_purch_pnts);
    connect(_purch_pnts, SIGNAL(validData()), SLOT(slotRefreshPurchases()));

    QGridLayout* purchasesGrid = new QGridLayout(purchases);
    purchasesGrid->setSpacing(3);
    purchasesGrid->setMargin(3);
    purchasesGrid->setRowStretch(0, 1);
    purchasesGrid->setColStretch(1, 1);
    purchasesGrid->addMultiCellWidget(_purchases, 0, 0, 0, 3);
    purchasesGrid->addWidget(_purch_vend_detail, 1, 0);
    purchasesGrid->addWidget(purchLabel, 1, 2);
    purchasesGrid->addWidget(_purch_pnts, 1, 3);

    _balances = new ListView(balances);
    _balances->setAllColumnsShowFocus(true);
    _balances->setShowSortIndicator(true);
    _balances->setSorting(0);

    _bal_cust_detail = new QCheckBox(tr("Customer Detail?"), balances);
    connect(_bal_cust_detail, SIGNAL(toggled(bool)),
	    SLOT(slotBalancesReformat()));

    QLabel* balLabel = new QLabel(tr("Balance Points:"), balances);
    _bal_pnts = new DoubleEdit(balances);
    _bal_pnts->setLength(6);
    balLabel->setBuddy(_bal_pnts);
    connect(_bal_pnts, SIGNAL(validData()), SLOT(slotRefreshBalances()));

    QGridLayout* balancesGrid = new QGridLayout(balances);
    balancesGrid->setSpacing(3);
    balancesGrid->setMargin(3);
    balancesGrid->setRowStretch(0, 1);
    balancesGrid->setColStretch(1, 1);
    balancesGrid->addMultiCellWidget(_balances, 0, 0, 0, 3);
    balancesGrid->addWidget(_bal_cust_detail, 1, 0);
    balancesGrid->addWidget(balLabel, 1, 2);
    balancesGrid->addWidget(_bal_pnts, 1, 3);

    _adjusts = new ListView(adjusts);
    _adjusts->setAllColumnsShowFocus(true);
    _adjusts->setShowSortIndicator(true);
    _adjusts->setSorting(0);
    _adjusts->addTextColumn("Group #", 8);
    _adjusts->addTextColumn("Group Name", 20);
    _adjusts->addNumberColumn("Points", 8);
    _adjusts->addTextColumn("Reason", 50);

    QPushButton* addAdj = new QPushButton(tr("Add"), adjusts);
    QPushButton* editAdj = new QPushButton(tr("Edit"), adjusts);
    QPushButton* removeAdj = new QPushButton(tr("Remove"), adjusts);

    connect(addAdj, SIGNAL(clicked()), SLOT(slotAddAdjustment()));
    connect(editAdj, SIGNAL(clicked()), SLOT(slotEditAdjustment()));
    connect(removeAdj, SIGNAL(clicked()), SLOT(slotRemoveAdjustment()));
    connect(_adjusts, SIGNAL(doubleClicked(QListViewItem*)), this,
	    SLOT(slotEditAdjustment()));

    QGridLayout* adjustsGrid = new QGridLayout(adjusts);
    adjustsGrid->setSpacing(3);
    adjustsGrid->setMargin(3);
    adjustsGrid->setRowStretch(0, 1);
    adjustsGrid->setColStretch(0, 1);
    adjustsGrid->addMultiCellWidget(_adjusts, 0, 0, 0, 3);
    adjustsGrid->addWidget(addAdj, 1, 1);
    adjustsGrid->addWidget(editAdj, 1, 2);
    adjustsGrid->addWidget(removeAdj, 1, 3);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(5, 1);
    grid->setColStretch(2, 1);
    grid->addWidget(descLabel, 0, 0);
    grid->addMultiCellWidget(_desc, 0, 0, 1, 4, AlignLeft | AlignVCenter);
    grid->addWidget(fromLabel, 1, 0);
    grid->addWidget(_from, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(toLabel, 1, 3);
    grid->addWidget(_to, 1, 4, AlignLeft | AlignVCenter);
    grid->addWidget(equityLabel, 2, 0);
    grid->addWidget(_equity_amt, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(equityIdLabel, 3, 0);
    grid->addWidget(_equity_id, 3, 1, AlignLeft | AlignVCenter);
    grid->addWidget(equityMemoLabel, 4, 0);
    grid->addWidget(_equity_memo, 4, 1, AlignLeft | AlignVCenter);
    grid->addWidget(creditLabel, 2, 3);
    grid->addWidget(_credit_amt, 2, 4, AlignLeft | AlignVCenter);
    grid->addWidget(creditIdLabel, 3, 3);
    grid->addWidget(_credit_id, 3, 4, AlignLeft | AlignVCenter);
    grid->addWidget(creditMemoLabel, 4, 3);
    grid->addWidget(_credit_memo, 4, 4, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_tabs, 5, 5, 0, 4);

    connect(_sales, SIGNAL(doubleClicked(QListViewItem*)), this,
	    SLOT(slotSalesDetail()));
    connect(_purchases, SIGNAL(doubleClicked(QListViewItem*)), this,
	    SLOT(slotPurchaseDetail()));
    connect(_balances, SIGNAL(doubleClicked(QListViewItem*)), this,
	    SLOT(slotBalanceDetail()));

    slotSalesReformat();
    slotPurchasesReformat();
    slotBalancesReformat();

    setCaption(tr("Patronage Worksheet"));
    finalize();
}

PatWorksheetMaster::~PatWorksheetMaster()
{
}

void
PatWorksheetMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _desc;
}

void
PatWorksheetMaster::newItem()
{
    PatWorksheet blank;
    _orig = blank;

    _curr = _orig;
    _firstField = _desc;
}

void
PatWorksheetMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _desc->selectAll();
}

bool
PatWorksheetMaster::fileItem()
{
    if (_orig.id() == INVALID_ID) {
	if (!_quasar->db()->create(_curr)) return false;
    } else {
	if (!_quasar->db()->update(_orig, _curr)) return false;
    }

    _orig = _curr;
    _id = _curr.id();

    return true;
}

bool
PatWorksheetMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
PatWorksheetMaster::restoreItem()
{
    _curr = _orig;
}

void
PatWorksheetMaster::cloneItem()
{
    PatWorksheetMaster* clone = new PatWorksheetMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
PatWorksheetMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
PatWorksheetMaster::dataToWidget()
{
    _desc->setText(_curr.description());
    _from->setDate(_curr.fromDate());
    _to->setDate(_curr.toDate());
    _equity_amt->setFixed(_curr.equityAmount());
    _credit_amt->setFixed(_curr.creditAmount());
    _equity_id->setId(_curr.equityId());
    _credit_id->setId(_curr.creditId());
    _equity_memo->setText(_curr.equityMemo());
    _credit_memo->setText(_curr.creditMemo());
    _purch_pnts->setFixed(_curr.purchasePoints());
    _bal_pnts->setFixed(_curr.balancePoints());
    _inactive->setChecked(!_curr.isActive());

    refresh();
}

// Set the data object from the widgets.
void
PatWorksheetMaster::widgetToData()
{
    _curr.setDescription(_desc->text());
    _curr.setFromDate(_from->getDate());
    _curr.setToDate(_to->getDate());
    _curr.setEquityAmount(_equity_amt->getFixed());
    _curr.setCreditAmount(_credit_amt->getFixed());
    _curr.setEquityId(_equity_id->getId());
    _curr.setCreditId(_credit_id->getId());
    _curr.setEquityMemo(_equity_memo->text());
    _curr.setCreditMemo(_credit_memo->text());
    _curr.setPurchasePoints(_purch_pnts->getFixed());
    _curr.setBalancePoints(_bal_pnts->getFixed());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());
}

void
PatWorksheetMaster::slotReload()
{
    if (_totals->childCount() != 0) {
	QString message = tr("Reloading will lose any changes\n"
			     "made to the worksheet.  Are you\n"
			     "sure you want to reload?");
	int choice = QMessageBox::warning(this, tr("Warning"), message,
					  QMessageBox::Yes, QMessageBox::No);
	if (choice != QMessageBox::Yes)
	    return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Clear existing information
    _cache.clear();
    _curr.sales().clear();
    _curr.purchases().clear();
    _curr.balances().clear();

    // Date range
    QDate from = _from->getDate();
    QDate to = _to->getDate();
    unsigned int i;

    // Select sales
    vector<Id> customer_ids;
    vector<Id> dept_ids;
    vector<fixed> sales_amts;
    _quasar->db()->patronageSales(from, to, customer_ids, dept_ids,
				  sales_amts);
    for (i = 0; i < customer_ids.size(); ++i) {
	Customer customer;
	if (!_cache.findCustomer(customer_ids[i], customer))
	    continue;
	if (customer.patGroupId() == INVALID_ID)
	    continue;

	Dept dept;
	if (!_cache.findDept(dept_ids[i], dept))
	    continue;

	PatSales sales;
	sales.customer_id = customer.id();
	sales.dept_id = dept.id();
	sales.patgroup_id = customer.patGroupId();
	sales.sales_amt = sales_amts[i];
	_curr.sales().push_back(sales);
    }

    // Select purchases
    vector<Id> vendor_ids;
    vector<fixed> purchase_amts;
    _quasar->db()->patronagePurchases(from, to, vendor_ids, purchase_amts);
    for (i = 0; i < vendor_ids.size(); ++i) {
	Vendor vendor;
	if (!_cache.findVendor(vendor_ids[i], vendor))
	    continue;
	if (vendor.patGroupId() == INVALID_ID)
	    continue;

	PatPurch purchase;
	purchase.vendor_id = vendor.id();
	purchase.patgroup_id = vendor.patGroupId();
	purchase.purchase_amt = purchase_amts[i];
	_curr.purchases().push_back(purchase);
    }

    // Get customer open balances
    vector<Id> open_ids;
    vector<fixed> open_amts;
    _quasar->db()->cardBalances(from - 1, INVALID_ID, open_ids, open_amts);

    // Select balance changes
    vector<fixed> change_amts;
    _quasar->db()->patronageBalances(from, to, customer_ids, change_amts);
    for (i = 0; i < customer_ids.size(); ++i) {
	Customer customer;
	if (!_cache.findCustomer(customer_ids[i], customer))
	    continue;
	if (customer.patGroupId() == INVALID_ID)
	    continue;

	fixed open_amt = 0.0;
	for (unsigned int j = 0; j < open_ids.size(); ++j) {
	    if (open_ids[j] == customer.id()) {
		open_amt = open_amts[j];
		break;
	    }
	}

	PatBalance balance;
	balance.customer_id = customer.id();
	balance.patgroup_id = customer.patGroupId();
	balance.open_amt = open_amt;
	balance.close_amt = open_amt + change_amts[i];
	_curr.balances().push_back(balance);
    }

    refresh();
    QApplication::restoreOverrideCursor();
}

void
PatWorksheetMaster::slotPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = NULL;
    QString label = _tabs->tabLabel(_tabs->currentPage());
    if (label == tr("Totals"))
	grid = Grid::buildGrid(_totals, tr("Patronage Totals"));
    else if (label == tr("Sales"))
	grid = Grid::buildGrid(_sales, tr("Patronage Sales"));
    else if (label == tr("Purchases"))
	grid = Grid::buildGrid(_purchases, tr("Patronage Purchases"));
    else if (label == tr("Balance Change"))
	grid = Grid::buildGrid(_balances, tr("Patronage Balance Change"));
    else if (label == tr("Adjustments"))
	grid = Grid::buildGrid(_adjusts, tr("Patronage Adjustments"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
PatWorksheetMaster::slotPost()
{
    if (!saveItem(true))
	return;

    Id equity_id = _equity_id->getId();
    Id credit_id = _credit_id->getId();
    QString equity_memo = _equity_memo->text();
    QString credit_memo = _credit_memo->text();
    fixed equity_amt = _equity_amt->getFixed();
    fixed credit_amt = _credit_amt->getFixed();

    if (equity_amt == 0.0 && credit_amt == 0.0) {
	QMessageBox::critical(this, tr("Error"), tr("No amount to post"));
	_equity_amt->setFocus();
	return;
    }

    if (equity_amt != 0.0 && equity_id == INVALID_ID) {
	QString message = tr("Equity account is required");
	QMessageBox::critical(this, tr("Error"), message);
	_equity_id->setFocus();
	return;
    }

    if (credit_amt != 0.0 && credit_id == INVALID_ID) {
	QString message = tr("Credit account is required");
	QMessageBox::critical(this, tr("Error"), message);
	_credit_id->setFocus();
	return;
    }

    if (_totals->childCount() == 0) {
	QMessageBox::critical(this, tr("Error"), tr("No groups to post to"));
	return;
    }

    // Change cursor
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    int equityCnt = 0;
    int creditCnt = 0;
    ListViewItem* item = _totals->firstChild();
    while (item != NULL) {
	Id patgroup_id = item->id;
	fixed equityAmt = item->value(5).toFixed();
	fixed creditAmt = item->value(6).toFixed();

	PatGroup group;
	if (!_cache.findPatGroup(patgroup_id, group))
	    continue;

	if (equityAmt != 0.0) {
	    Id customer_id = group.equityId();
	    Customer customer;
	    _cache.findCustomer(customer_id, customer);

	    CardAdjust adjust;
	    adjust.setPostDate(QDate::currentDate());
	    adjust.setPostTime(QTime::currentTime());
	    adjust.setMemo(equity_memo);
	    adjust.setStationId(_quasar->defaultStation());
	    adjust.setEmployeeId(_quasar->defaultEmployee());
	    adjust.setCardId(customer_id);
	    adjust.setStoreId(_quasar->defaultStore());

	    Id ar_id = customer.accountId();
	    adjust.accounts().push_back(AccountLine(ar_id, -equityAmt));
	    adjust.accounts().push_back(AccountLine(equity_id, equityAmt));
	    adjust.cards().push_back(CardLine(customer_id, -equityAmt));

	    if (!_db->create(adjust))
		qWarning("Create equity adjustment failed");
	    equityCnt++;
	}

	if (creditAmt != 0.0) {
	    Id customer_id = group.creditId();
	    Customer customer;
	    _cache.findCustomer(customer_id, customer);

	    CardAdjust adjust;
	    adjust.setPostDate(QDate::currentDate());
	    adjust.setPostTime(QTime::currentTime());
	    adjust.setMemo(credit_memo);
	    adjust.setStationId(_quasar->defaultStation());
	    adjust.setEmployeeId(_quasar->defaultEmployee());
	    adjust.setCardId(customer_id);
	    adjust.setStoreId(_quasar->defaultStore());

	    Id ar_id = customer.accountId();
	    adjust.accounts().push_back(AccountLine(ar_id, -creditAmt));
	    adjust.accounts().push_back(AccountLine(credit_id, creditAmt));
	    adjust.cards().push_back(CardLine(customer_id, -creditAmt));

	    if (!_db->create(adjust))
		qWarning("Create credit adjustment failed");
	    creditCnt++;
	}

	item = item->nextSibling();
    }

    // Mark worksheet as inactive and save
    _quasar->db()->setActive(_curr, false);
    if (!_quasar->db()->update(_orig, _curr))
	qWarning("Failed setting inactive");

    QApplication::restoreOverrideCursor();

    QString message;
    if (equityCnt == 0)
	message = tr("Posted %1 credit transactions").arg(creditCnt);
    else if (creditCnt == 0)
	message = tr("Posted %1 equity transactions").arg(equityCnt);
    else
	message = tr("Posted %1 equity and %2 credit transactions").arg(equityCnt).arg(creditCnt);

    QMessageBox::information(this, tr("Posting"), message);
    close();
}

void
PatWorksheetMaster::slotSalesDetail()
{
    ListViewItem* current = _sales->currentItem();
    if (current == NULL) return;

    PatGroup group;
    if (!_cache.findPatGroup(current->id, group)) return;

    QDialog* dialog = new QDialog(this, "SalesDetail", true);
    dialog->setCaption("Sales Detail");

    QLabel* nameLabel = new QLabel(tr("Group:"), dialog);
    QLabel* name = new QLabel(group.name(), dialog);
    nameLabel->setBuddy(name);

    ListView* list = new ListView(dialog);
    list->setAllColumnsShowFocus(true);
    list->setShowSortIndicator(true);
    list->addTextColumn("Customer", 30);
    list->addTextColumn("Number", 10);
    list->addTextColumn("Dept", 20);
    list->addMoneyColumn("Sales", 10);

    QFrame* buttons = new QFrame(dialog);
    QPushButton* close = new QPushButton(tr("Close"), buttons);

    close->connect(close, SIGNAL(clicked()), dialog, SLOT(reject()));
    close->setDefault(true);

    QGridLayout* grid1 = new QGridLayout(buttons);
    grid1->setSpacing(6);
    grid1->setMargin(6);
    grid1->setColStretch(0, 1);
    grid1->addWidget(close, 0, 1);

    QGridLayout* grid = new QGridLayout(dialog);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setRowStretch(1, 1);
    grid->setColStretch(1, 1);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(name, 0, 1);
    grid->addMultiCellWidget(list, 1, 1, 0, 1);
    grid->addMultiCellWidget(buttons, 2, 2, 0, 1);

    for (unsigned int i = 0; i < _curr.sales().size(); ++i) {
	const PatSales& sales = _curr.sales()[i];
	if (sales.patgroup_id != group.id()) continue;

	Customer customer;
	if (!_cache.findCustomer(sales.customer_id, customer))
	    continue;

	Dept dept;
	if (!_cache.findDept(sales.dept_id, dept))
	    continue;

	ListViewItem* item = new ListViewItem(list, customer.id());
	item->setValue(0, customer.name());
	item->setValue(1, customer.number());
	item->setValue(2, dept.name());
	item->setValue(3, sales.sales_amt);
    }

    dialog->exec();
    delete dialog;
}

void
PatWorksheetMaster::slotPurchaseDetail()
{
    ListViewItem* current = _purchases->currentItem();
    if (current == NULL) return;

    PatGroup group;
    if (!_cache.findPatGroup(current->id, group)) return;

    QDialog* dialog = new QDialog(this, "PurchaseDetail", true);
    dialog->setCaption("Purchases Detail");

    QLabel* nameLabel = new QLabel(tr("Group:"), dialog);
    QLabel* name = new QLabel(group.name(), dialog);
    nameLabel->setBuddy(name);

    ListView* list = new ListView(dialog);
    list->setAllColumnsShowFocus(true);
    list->setShowSortIndicator(true);
    list->addTextColumn("Vendor", 30);
    list->addTextColumn("Number", 10);
    list->addMoneyColumn("Purchases", 10);

    QFrame* buttons = new QFrame(dialog);
    QPushButton* close = new QPushButton(tr("Close"), buttons);

    close->connect(close, SIGNAL(clicked()), dialog, SLOT(reject()));
    close->setDefault(true);

    QGridLayout* grid1 = new QGridLayout(buttons);
    grid1->setSpacing(6);
    grid1->setMargin(6);
    grid1->setColStretch(0, 1);
    grid1->addWidget(close, 0, 1);

    QGridLayout* grid = new QGridLayout(dialog);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setRowStretch(1, 1);
    grid->setColStretch(1, 1);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(name, 0, 1);
    grid->addMultiCellWidget(list, 1, 1, 0, 1);
    grid->addMultiCellWidget(buttons, 2, 2, 0, 1);

    for (unsigned int i = 0; i < _curr.purchases().size(); ++i) {
	const PatPurch& purchase = _curr.purchases()[i];
	if (purchase.patgroup_id != group.id()) continue;

	Vendor vendor;
	if (!_cache.findVendor(purchase.vendor_id, vendor))
	    continue;

	ListViewItem* item = new ListViewItem(list, vendor.id());
	item->setValue(0, vendor.name());
	item->setValue(1, vendor.number());
	item->setValue(2, purchase.purchase_amt);
    }

    dialog->exec();
    delete dialog;
}

void
PatWorksheetMaster::slotBalanceDetail()
{
    ListViewItem* current = _balances->currentItem();
    if (current == NULL) return;

    PatGroup group;
    if (!_cache.findPatGroup(current->id, group)) return;

    QDialog* dialog = new QDialog(this, "BalanceDetail", true);
    dialog->setCaption("Balance Detail");

    QLabel* nameLabel = new QLabel(tr("Group:"), dialog);
    QLabel* name = new QLabel(group.name(), dialog);
    nameLabel->setBuddy(name);

    ListView* list = new ListView(dialog);
    list->setAllColumnsShowFocus(true);
    list->setShowSortIndicator(true);
    list->addTextColumn("Customer", 30);
    list->addTextColumn("Number", 10);
    list->addMoneyColumn("Change", 10);

    QFrame* buttons = new QFrame(dialog);
    QPushButton* close = new QPushButton(tr("Close"), buttons);

    close->connect(close, SIGNAL(clicked()), dialog, SLOT(reject()));
    close->setDefault(true);

    QGridLayout* grid1 = new QGridLayout(buttons);
    grid1->setSpacing(6);
    grid1->setMargin(6);
    grid1->setColStretch(0, 1);
    grid1->addWidget(close, 0, 1);

    QGridLayout* grid = new QGridLayout(dialog);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setRowStretch(1, 1);
    grid->setColStretch(1, 1);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(name, 0, 1);
    grid->addMultiCellWidget(list, 1, 1, 0, 1);
    grid->addMultiCellWidget(buttons, 2, 2, 0, 1);

    for (unsigned int i = 0; i < _curr.balances().size(); ++i) {
	const PatBalance& balance = _curr.balances()[i];
	if (balance.patgroup_id != group.id()) continue;

	Customer customer;
	if (!_cache.findCustomer(balance.customer_id, customer))
	    continue;

	fixed change_amt = balance.close_amt - balance.open_amt;

	ListViewItem* item = new ListViewItem(list, customer.id());
	item->setValue(0, customer.name());
	item->setValue(1, customer.number());
	item->setValue(2, change_amt);
    }

    dialog->exec();
    delete dialog;
}

void
PatWorksheetMaster::slotAddAdjustment()
{
    QDialog* dialog = new QDialog(this, "AddAdjust", true);
    dialog->setCaption("Add Adjustment");

    QLabel* groupLabel = new QLabel(tr("Group:"), dialog);
    LookupEdit* group = new LookupEdit(new PatGroupLookup(_main, this),dialog);
    groupLabel->setBuddy(group);

    QLabel* pointsLabel = new QLabel(tr("Points:"), dialog);
    IntegerEdit* points = new IntegerEdit(dialog);
    pointsLabel->setBuddy(points);

    QLabel* reasonLabel = new QLabel(tr("Reason:"), dialog);
    LineEdit* reason = new LineEdit(80, dialog);
    reasonLabel->setBuddy(reason);

    QFrame* buttons = new QFrame(dialog);
    QPushButton* ok = new QPushButton(tr("Ok"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);

    connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
    ok->setDefault(true);

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(ok, 0, 1);
    buttonGrid->addWidget(cancel, 0, 2);

    QGridLayout* grid = new QGridLayout(dialog);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setColStretch(1, 1);
    grid->addWidget(groupLabel, 0, 0);
    grid->addWidget(group, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(pointsLabel, 1, 0);
    grid->addWidget(points, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(reasonLabel, 2, 0);
    grid->addWidget(reason, 2, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(buttons, 3, 3, 0, 1);

    while (true) {
	if (!dialog->exec()) break;

	if (group->getId() == INVALID_ID) {
	    QMessageBox::critical(dialog, tr("Error"), tr("Group required"));
	    continue;
	}
	if (points->getInt() == 0) {
	    QMessageBox::critical(dialog, tr("Error"), tr("Points required"));
	    continue;
	}
	if (reason->text().isEmpty()) {
	    QMessageBox::critical(dialog, tr("Error"), tr("Reason required"));
	    continue;
	}

	PatAdjust adjust;
	adjust.patgroup_id = group->getId();
	adjust.points = points->getInt();
	adjust.reason = reason->text();
	_curr.adjusts().push_back(adjust);

	slotRefreshAdjustments();
	break;
    }

    delete dialog;
}

void
PatWorksheetMaster::slotEditAdjustment()
{
    ListViewItem* current = _adjusts->currentItem();
    if (current == NULL) return;
    if (current->id == INVALID_ID) return;

    QDialog* dialog = new QDialog(this, "EditAdjust", true);
    dialog->setCaption("Edit Adjustment");

    QLabel* groupLabel = new QLabel(tr("Group:"), dialog);
    LookupEdit* group = new LookupEdit(new PatGroupLookup(_main, this),dialog);
    groupLabel->setBuddy(group);

    QLabel* pointsLabel = new QLabel(tr("Points:"), dialog);
    IntegerEdit* points = new IntegerEdit(dialog);
    pointsLabel->setBuddy(points);

    QLabel* reasonLabel = new QLabel(tr("Reason:"), dialog);
    LineEdit* reason = new LineEdit(80, dialog);
    reasonLabel->setBuddy(reason);

    QFrame* buttons = new QFrame(dialog);
    QPushButton* ok = new QPushButton(tr("Ok"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);

    connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
    ok->setDefault(true);

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(ok, 0, 1);
    buttonGrid->addWidget(cancel, 0, 2);

    QGridLayout* grid = new QGridLayout(dialog);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setColStretch(1, 1);
    grid->addWidget(groupLabel, 0, 0);
    grid->addWidget(group, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(pointsLabel, 1, 0);
    grid->addWidget(points, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(reasonLabel, 2, 0);
    grid->addWidget(reason, 2, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(buttons, 3, 3, 0, 1);

    group->setId(current->id);
    points->setInt(current->value(2).toInt());
    reason->setText(current->value(3).toString());

    while (true) {
	if (!dialog->exec()) break;

	if (group->getId() == INVALID_ID) {
	    QMessageBox::critical(dialog, tr("Error"), tr("Group required"));
	    continue;
	}
	if (points->getInt() == 0) {
	    QMessageBox::critical(dialog, tr("Error"), tr("Points required"));
	    continue;
	}
	if (reason->text().isEmpty()) {
	    QMessageBox::critical(dialog, tr("Error"), tr("Reason required"));
	    continue;
	}

	for (unsigned int i = 0; i < _curr.adjusts().size(); ++i) {
	    PatAdjust& adjust = _curr.adjusts()[i];
	    if (adjust.patgroup_id != current->id) continue;
	    if (adjust.points != current->value(2).toFixed()) continue;
	    if (adjust.reason != current->value(3).toString()) continue;

	    adjust.patgroup_id = group->getId();
	    adjust.points = points->getInt();
	    adjust.reason = reason->text();
	    break;
	}

	slotRefreshAdjustments();
	break;
    }

    delete dialog;
}

void
PatWorksheetMaster::slotRemoveAdjustment()
{
    ListViewItem* current = _adjusts->currentItem();
    if (current == NULL) return;
    if (current->id == INVALID_ID) return;

    for (unsigned int i = 0; i < _curr.adjusts().size(); ++i) {
	const PatAdjust& adjust = _curr.adjusts()[i];
	if (adjust.patgroup_id != current->id) continue;
	if (adjust.points != current->value(2).toFixed()) continue;
	if (adjust.reason != current->value(3).toString()) continue;

	_curr.adjusts().erase(_curr.adjusts().begin() + i);
	slotRefreshAdjustments();
	return;
    }
}

void
PatWorksheetMaster::refresh()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    slotRefreshSales(false);
    slotRefreshPurchases(false);
    slotRefreshBalances(false);
    slotRefreshAdjustments(false);
    slotRefreshTotals();

    QApplication::restoreOverrideCursor();
}

void
PatWorksheetMaster::slotSalesReformat()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    bool customerDetail = _sales_cust_detail->isChecked();
    bool deptDetail = _sales_dept_detail->isChecked();

    _sales->clear();
    while (_sales->columns() > 0) _sales->removeColumn(0);

    _sales->addTextColumn("Group #", 8);
    _sales->addTextColumn("Group Name", 20);
    if (customerDetail) {
	_sales->addTextColumn("Cust #", 8);
	_sales->addTextColumn("Customer Name", 20);
    }
    if (deptDetail) {
	_sales->addTextColumn("Department", 14);
	_sales->addNumberColumn("Dept Pnts", 10);
    }
    _sales->addMoneyColumn("Net Sales", 10);
    _sales->addNumberColumn("Points", 8);

    slotRefreshSales();
    QApplication::restoreOverrideCursor();
}

void
PatWorksheetMaster::slotRefreshSales(bool totals)
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    bool customerDetail = _sales_cust_detail->isChecked();
    bool deptDetail = _sales_dept_detail->isChecked();

    vector<PatSales> sales;
    vector<fixed> points;

    for (unsigned int i = 0; i < _curr.sales().size(); ++i) {
	const PatSales& info = _curr.sales()[i];

	Dept dept;
	_cache.findDept(info.dept_id, dept);

	fixed num_points = info.sales_amt * dept.patPoints();
	num_points.roundTo(0);

	int index = -1;
	for (unsigned int j = 0; j < sales.size(); ++j) {
	    if (info.patgroup_id != sales[j].patgroup_id)
		continue;
	    if (customerDetail && info.customer_id != sales[j].customer_id)
		continue;
	    if (deptDetail && info.dept_id != sales[j].dept_id)
		continue;

	    index = j;
	    break;
	}

	if (index == -1) {
	    PatSales newInfo;
	    newInfo.patgroup_id = info.patgroup_id;
	    newInfo.sales_amt = info.sales_amt;
	    if (deptDetail) newInfo.dept_id = info.dept_id;
	    if (customerDetail) newInfo.customer_id = info.customer_id;

	    sales.push_back(newInfo);
	    points.push_back(num_points);
	} else {
	    sales[index].sales_amt += info.sales_amt;
	    points[index] += num_points;
	}
    }

    fixed totalSales, totalPoints;
    _sales->clear();
    for (unsigned int i = 0; i < sales.size(); ++i) {
	const PatSales& sale = sales[i];
	fixed num_points = points[i];

	PatGroup group;
	if (!_cache.findPatGroup(sale.patgroup_id, group))
	    continue;

	Customer customer;
	if (customerDetail && !_cache.findCustomer(sale.customer_id, customer))
	    continue;

	Dept dept;
	if (deptDetail && !_cache.findDept(sale.dept_id, dept))
	    continue;

	int col = 0;
	ListViewItem* item = new ListViewItem(_sales, group.id());
	item->setValue(col++, group.number());
	item->setValue(col++, group.name());
	if (customerDetail) {
	    item->setValue(col++, customer.number());
	    item->setValue(col++, customer.name());
	}
	if (deptDetail) {
	    item->setValue(col++, dept.name());
	    item->setValue(col++, dept.patPoints());
	}
	item->setValue(col++, sale.sales_amt);
	item->setValue(col++, num_points);

	totalSales += sale.sales_amt;
	totalPoints += num_points;
    }

    ListViewItem* item = new ListViewItem(_sales);
    item->isLast = true;
    item->setValue(0, "Total");
    int col = 2;
    if (customerDetail) col += 2;
    if (deptDetail) col += 2;
    item->setValue(col++, totalSales);
    item->setValue(col++, totalPoints);

    if (totals) slotRefreshTotals();
    QApplication::restoreOverrideCursor();
}

void
PatWorksheetMaster::slotPurchasesReformat()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    bool vendorDetail = _purch_vend_detail->isChecked();

    _purchases->clear();
    while (_purchases->columns() > 0) _purchases->removeColumn(0);

    _purchases->addTextColumn("Group #", 8);
    _purchases->addTextColumn("Group Name", 20);
    if (vendorDetail) {
	_purchases->addTextColumn("Vend #", 8);
	_purchases->addTextColumn("Vendor Name", 20);
    }
    _purchases->addMoneyColumn("Purchases", 10);
    _purchases->addNumberColumn("Points", 8);

    slotRefreshPurchases();
    QApplication::restoreOverrideCursor();
}

void
PatWorksheetMaster::slotRefreshPurchases(bool totals)
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    bool vendorDetail = _purch_vend_detail->isChecked();
    vector<PatPurch> purchaseList = _curr.purchaseSummary(vendorDetail);

    fixed totalPurchases, totalPoints;
    _purchases->clear();
    for (unsigned int i = 0; i < purchaseList.size(); ++i) {
	const PatPurch& purchase = purchaseList[i];

	PatGroup group;
	if (!_cache.findPatGroup(purchase.patgroup_id, group))
	    continue;

	Vendor vendor;
	if (vendorDetail && !_cache.findVendor(purchase.vendor_id, vendor))
	    continue;

	fixed points = purchase.purchase_amt * _purch_pnts->getFixed();
	points.roundTo(0);

	int col = 0;
	ListViewItem* item = new ListViewItem(_purchases, group.id());
	item->setValue(col++, group.number());
	item->setValue(col++, group.name());
	if (vendorDetail) {
	    item->setValue(col++, vendor.number());
	    item->setValue(col++, vendor.name());
	}
	item->setValue(col++, purchase.purchase_amt);
	item->setValue(col++, points);

	totalPurchases += purchase.purchase_amt;
	totalPoints += points;
    }

    ListViewItem* item = new ListViewItem(_purchases);
    item->isLast = true;
    item->setValue(0, "Total");
    int col = 2;
    if (vendorDetail) col += 2;
    item->setValue(col++, totalPurchases);
    item->setValue(col++, totalPoints);

    if (totals) slotRefreshTotals();
    QApplication::restoreOverrideCursor();
}

void
PatWorksheetMaster::slotBalancesReformat()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    bool customerDetail = _bal_cust_detail->isChecked();

    _balances->clear();
    while (_balances->columns() > 0) _balances->removeColumn(0);

    _balances->addTextColumn("Group #", 8);
    _balances->addTextColumn("Group Name", 20);
    if (customerDetail) {
	_balances->addTextColumn("Cust #", 8);
	_balances->addTextColumn("Customer Name", 20);
    }
    _balances->addMoneyColumn("Starting", 8);
    _balances->addMoneyColumn("Ending", 8);
    _balances->addMoneyColumn("Change", 8);
    _balances->addNumberColumn("Points", 8);

    slotRefreshBalances();
    QApplication::restoreOverrideCursor();
}

void
PatWorksheetMaster::slotRefreshBalances(bool totals)
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    bool customerDetail = _bal_cust_detail->isChecked();
    vector<PatBalance> balanceList = _curr.balanceSummary(customerDetail);

    fixed totalOpen, totalClose, totalChange, totalPoints;
    _balances->clear();
    for (unsigned int i = 0; i < balanceList.size(); ++i) {
	const PatBalance& balance = balanceList[i];

	PatGroup group;
	if (!_cache.findPatGroup(balance.patgroup_id, group))
	    continue;

	Customer customer;
	if (customerDetail && !_cache.findCustomer(balance.customer_id, customer))
	    continue;

	fixed change_amt = balance.close_amt - balance.open_amt;
	fixed points = 0.0;
	if (change_amt < 0.0)
	    points = change_amt * _bal_pnts->getFixed() * -1.0;
	points.roundTo(0);

	int col = 0;
	ListViewItem* item = new ListViewItem(_balances, group.id());
	item->setValue(col++, group.number());
	item->setValue(col++, group.name());
	if (customerDetail) {
	    item->setValue(col++, customer.number());
	    item->setValue(col++, customer.name());
	}
	item->setValue(col++, balance.open_amt);
	item->setValue(col++, balance.close_amt);
	item->setValue(col++, change_amt);
	item->setValue(col++, points);

	totalOpen += balance.open_amt;
	totalClose += balance.close_amt;
	totalChange += change_amt;
	totalPoints += points;
    }

    ListViewItem* item = new ListViewItem(_balances);
    item->isLast = true;
    item->setValue(0, "Total");
    int col = 2;
    if (customerDetail) col += 2;
    item->setValue(col++, totalOpen);
    item->setValue(col++, totalClose);
    item->setValue(col++, totalChange);
    item->setValue(col++, totalPoints);

    if (totals) slotRefreshTotals();
    QApplication::restoreOverrideCursor();
}

void
PatWorksheetMaster::slotRefreshAdjustments(bool totals)
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    fixed totalPoints;
    _adjusts->clear();
    for (unsigned int i = 0; i < _curr.adjusts().size(); ++i) {
	const PatAdjust& adjust = _curr.adjusts()[i];

	PatGroup group;
	if (!_cache.findPatGroup(adjust.patgroup_id, group))
	    continue;

	ListViewItem* item = new ListViewItem(_adjusts, group.id());
	item->setValue(0, group.number());
	item->setValue(1, group.name());
	item->setValue(2, adjust.points);
	item->setValue(3, adjust.reason);

	totalPoints += adjust.points;
    }

    ListViewItem* item = new ListViewItem(_adjusts);
    item->isLast = true;
    item->setValue(0, "Total");
    item->setValue(2, totalPoints);

    if (totals) slotRefreshTotals();
    QApplication::restoreOverrideCursor();
}

struct PatTotal {
    Id patgroup_id;
    fixed sales_pnts;
    fixed purchase_pnts;
    fixed balance_pnts;
    fixed adjust_pnts;
};

typedef QMap<Id,PatTotal> TotalMap;

void
PatWorksheetMaster::slotRefreshTotals()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    _totals->clear();
    TotalMap totals;
    fixed totalSales, totalPurch, totalBal, totalAdjust, totalPoints;

    ListViewItem* item = _sales->firstChild();
    while (item != NULL) {
	Id patgroup_id = item->id;
	if (patgroup_id != INVALID_ID) {
	    fixed points = item->value(_sales->columns() - 1).toFixed();
	    totalPoints += points;
	    totalSales += points;

	    if (totals.contains(patgroup_id)) {
		totals[patgroup_id].sales_pnts += points;
	    } else {
		PatTotal total;
		total.patgroup_id = patgroup_id;
		total.sales_pnts = points;
		totals[patgroup_id] = total;
	    }
	}

	item = item->nextSibling();
    }

    item = _purchases->firstChild();
    while (item != NULL) {
	Id patgroup_id = item->id;
	if (patgroup_id != INVALID_ID) {
	    fixed points = item->value(_purchases->columns() - 1).toFixed();
	    totalPoints += points;
	    totalPurch += points;

	    if (totals.contains(patgroup_id)) {
		totals[patgroup_id].purchase_pnts += points;
	    } else {
		PatTotal total;
		total.patgroup_id = patgroup_id;
		total.purchase_pnts = points;
		totals[patgroup_id] = total;
	    }
	}

	item = item->nextSibling();
    }

    item = _balances->firstChild();
    while (item != NULL) {
	Id patgroup_id = item->id;
	if (patgroup_id != INVALID_ID) {
	    fixed points = item->value(_balances->columns() - 1).toFixed();
	    totalPoints += points;
	    totalBal += points;

	    if (totals.contains(patgroup_id)) {
		totals[patgroup_id].balance_pnts += points;
	    } else {
		PatTotal total;
		total.patgroup_id = patgroup_id;
		total.balance_pnts = points;
		totals[patgroup_id] = total;
	    }
	}

	item = item->nextSibling();
    }

    item = _adjusts->firstChild();
    while (item != NULL) {
	Id patgroup_id = item->id;
	if (patgroup_id != INVALID_ID) {
	    fixed points = item->value(2).toFixed();
	    totalPoints += points;
	    totalAdjust += points;

	    if (totals.contains(patgroup_id)) {
		totals[patgroup_id].adjust_pnts += points;
	    } else {
		PatTotal total;
		total.patgroup_id = patgroup_id;
		total.adjust_pnts = points;
		totals[patgroup_id] = total;
	    }
	}

	item = item->nextSibling();
    }

    fixed equityTotal = _equity_amt->getFixed();
    fixed creditTotal = _credit_amt->getFixed();
    fixed equityRemain = equityTotal;
    fixed creditRemain = creditTotal;

    TotalMap::iterator it;
    for (it = totals.begin(); it != totals.end(); ++it) {
	const PatTotal& total = it.data();

	PatGroup group;
	if (!_cache.findPatGroup(total.patgroup_id, group))
	    continue;

	TotalMap::iterator next = it;
	++next;

	fixed group_pnts = total.sales_pnts + total.purchase_pnts +
	    total.balance_pnts + total.adjust_pnts;
	fixed equityAmt, creditAmt;
	if (next == totals.end()) {
	    equityAmt = equityRemain;
	    creditAmt = creditRemain;
	} else if (totalPoints != 0.0) {
	    equityAmt = equityTotal * group_pnts / totalPoints;
	    creditAmt = creditTotal * group_pnts / totalPoints;
	    equityAmt.moneyRound();
	    creditAmt.moneyRound();
	    equityRemain -= equityAmt;
	    creditRemain -= creditAmt;
	}

	ListViewItem* item = new ListViewItem(_totals, group.id());
	item->setValue(0, group.name());
	item->setValue(1, total.sales_pnts);
	item->setValue(2, total.purchase_pnts);
	item->setValue(3, total.balance_pnts);
	item->setValue(4, total.adjust_pnts);
	item->setValue(5, group_pnts);
	item->setValue(6, equityAmt);
	item->setValue(7, creditAmt);
	item->setValue(8, equityAmt + creditAmt);
    }

    item = new ListViewItem(_totals);
    item->isLast = true;
    item->setValue(0, "Total");
    item->setValue(1, totalSales);
    item->setValue(2, totalPurch);
    item->setValue(3, totalBal);
    item->setValue(4, totalAdjust);
    item->setValue(5, totalPoints);

    QApplication::restoreOverrideCursor();
}
