// $Id: report_tx.cpp,v 1.38 2005/03/13 22:20:54 bpepers Exp $
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

#include "report_tx.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "company.h"
#include "screen_decl.h"
#include "shift.h"
#include "invoice.h"
#include "station.h"
#include "employee.h"
#include "list_view_item.h"
#include "gltx_select.h"
#include "account_select.h"
#include "tender_select.h"
#include "tx_combo.h"
#include "lookup_edit.h"
#include "station_lookup.h"
#include "gltx_lookup.h"
#include "employee_lookup.h"
#include "store_lookup.h"
#include "account_lookup.h"
#include "tender_lookup.h"
#include "discount_lookup.h"
#include "tax_lookup.h"
#include "dept_lookup.h"
#include "subdept_lookup.h"
#include "date_range.h"
#include "date_popup.h"
#include "money_edit.h"
#include "percent_edit.h"
#include "combo_box.h"
#include "grid.h"
#include "text_frame.h"
#include "line_frame.h"

#include <qapplication.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qheader.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <assert.h>

ReportTx::ReportTx(MainWindow* main)
    : QuasarWindow(main, "ReportTx"), _cache(main->quasar()->db())
{
    _helpSource = "report_tx.html";

    QFrame* frame = new QFrame(this);

    QGroupBox* select = new QGroupBox(tr("Data Selection"), frame);
    QGridLayout* grid1 = new QGridLayout(select, 2, 1, select->frameWidth()*2);
    grid1->addRowSpacing(0, select->fontMetrics().height());

    QLabel* rangeLabel = new QLabel(tr("Date Range:"), select);
    _range = new DateRange(select);

    QLabel* fromLabel = new QLabel(tr("From Date:"), select);
    _from = new DatePopup(select);
    _range->setFromPopup(_from);
    fromLabel->setBuddy(_from);

    QLabel* toLabel = new QLabel(tr("To Date:"), select);
    _to = new DatePopup(select);
    _range->setToPopup(_to);
    toLabel->setBuddy(_to);

    QLabel* shiftLabel = new QLabel(tr("Shift No.:"), select);
    _shift = new LookupEdit(new GltxLookup(main, this, DataObject::SHIFT),
			       select);
    _shift->setLength(10);
    shiftLabel->setBuddy(_shift);

    QLabel* stationLabel = new QLabel(tr("Station:"), select);
    _station = new LookupEdit(new StationLookup(main, this), select);
    _station->setLength(30);
    stationLabel->setBuddy(_station);

    QLabel* employeeLabel = new QLabel(tr("Employee:"), select);
    _employee = new LookupEdit(new EmployeeLookup(main, this), select);
    _employee->setLength(30);
    employeeLabel->setBuddy(_employee);

    QLabel* cardLabel = new QLabel(tr("Card:"), select);
    _card = new LookupEdit(new CardLookup(main, this), select);
    _card->setLength(30);
    cardLabel->setBuddy(_card);

    QLabel* storeLabel = new QLabel(tr("Store:"), select);
    _store = new LookupEdit(new StoreLookup(main, this), select);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    _cashrecOnly = new QCheckBox(tr("Cashrec Only?"), select);

    grid1->setColStretch(2, 1);
    grid1->setColStretch(5, 1);
    grid1->addWidget(rangeLabel, 1, 0);
    grid1->addWidget(_range, 1, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(fromLabel, 2, 0);
    grid1->addWidget(_from, 2, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(toLabel, 3, 0);
    grid1->addWidget(_to, 3, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(shiftLabel, 4, 0);
    grid1->addWidget(_shift, 4, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(stationLabel, 1, 3);
    grid1->addWidget(_station, 1, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(employeeLabel, 2, 3);
    grid1->addWidget(_employee, 2, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(cardLabel, 3, 3);
    grid1->addWidget(_card, 3, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(storeLabel, 4, 3);
    grid1->addWidget(_store, 4, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(_cashrecOnly, 1, 6);

    _tabs = new QTabWidget(frame);
    _typeFrame = new QFrame(_tabs);
    _salesFrame = new QFrame(_tabs);
    _accountFrame = new QFrame(_tabs);
    _tenderFrame = new QFrame(_tabs);
    _recFrame = new QFrame(_tabs);
    _safeFrame = new QFrame(_tabs);
    _taxFrame = new QFrame(_tabs);
    _discountFrame = new QFrame(_tabs);
    _tabs->addTab(_typeFrame, tr("Transactions"));
    _tabs->addTab(_salesFrame, tr("Sales"));
    _tabs->addTab(_accountFrame, tr("Accounts"));
    _tabs->addTab(_tenderFrame, tr("Tenders"));
    _tabs->addTab(_recFrame, tr("Receivables"));
    _tabs->addTab(_safeFrame, tr("Safe"));
    _tabs->addTab(_taxFrame, tr("Taxes"));
    _tabs->addTab(_discountFrame, tr("Discounts"));
    connect(_tabs, SIGNAL(currentChanged(QWidget*)), SLOT(slotTabChanged()));

    QLabel* typeLabel = new QLabel(tr("Type:"), _typeFrame);
    _type = new TxCombo(_quasar, _typeFrame);
    typeLabel->setBuddy(_type);
    connect(_type, SIGNAL(activated(int)), SLOT(slotUpdateTypes()));

    QPushButton* allTypes = new QPushButton(tr("All Types"), _typeFrame);
    connect(allTypes, SIGNAL(clicked()), SLOT(slotAllTypes()));

    _types = new ListView(_typeFrame);
    _types->setAllColumnsShowFocus(true);
    _types->setShowSortIndicator(true);
    _typesFooter = new QHeader(_typeFrame);
    _typesFooter->setTracking(true);
    initializeTypes();

    connect(_types, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotPickTypes()));
    connect(_types->header(), SIGNAL(sizeChange(int,int,int)),
	    SLOT(slotTypesSizeChange(int,int,int)));
    connect(_typesFooter, SIGNAL(sizeChange(int,int,int)),
	    SLOT(slotTypesSizeChange(int,int,int)));
    connect(_types->horizontalScrollBar(), SIGNAL(sliderMoved(int)),
	    _typesFooter, SLOT(setOffset(int)));
    connect(_types->horizontalScrollBar(), SIGNAL(valueChanged(int)),
	    _typesFooter, SLOT(setOffset(int)));

    QGridLayout* typeGrid = new QGridLayout(_typeFrame);
    typeGrid->setSpacing(6);
    typeGrid->setMargin(3);
    typeGrid->setRowStretch(1, 1);
    typeGrid->setColStretch(2, 1);
    typeGrid->addWidget(typeLabel, 0, 0);
    typeGrid->addWidget(_type, 0, 1, AlignLeft | AlignVCenter);
    typeGrid->addWidget(allTypes, 0, 2, AlignRight | AlignVCenter);
    typeGrid->addMultiCellWidget(_types, 1, 1, 0, 2);
    typeGrid->addMultiCellWidget(_typesFooter, 2, 2, 0, 2);

    QLabel* deptLabel = new QLabel(tr("Department:"), _salesFrame);
    _dept = new LookupEdit(new DeptLookup(main, this), _salesFrame);
    _dept->setLength(30);
    deptLabel->setBuddy(_dept);
    connect(_dept, SIGNAL(validData()), SLOT(slotDeptChanged()));

    QPushButton* allDepts = new QPushButton(tr("All Depts"), _salesFrame);
    connect(allDepts, SIGNAL(clicked()), SLOT(slotAllDepts()));

    QLabel* subdeptLabel = new QLabel(tr("Sub-department:"), _salesFrame);
    _subdeptLookup = new SubdeptLookup(main, this);
    _subdept = new LookupEdit(_subdeptLookup, _salesFrame);
    _subdept->setLength(30);
    subdeptLabel->setBuddy(_subdept);
    connect(_subdept, SIGNAL(validData()), SLOT(slotSubdeptChanged()));

    QPushButton* allSubdepts = new QPushButton(tr("All Subdepts"),_salesFrame);
    connect(allSubdepts, SIGNAL(clicked()), SLOT(slotAllSubdepts()));
    allDepts->setMinimumSize(allSubdepts->sizeHint());

    _sales = new ListView(_salesFrame);
    _sales->setAllColumnsShowFocus(true);
    _sales->setShowSortIndicator(true);
    _salesFooter = new QHeader(_salesFrame);
    _salesFooter->setTracking(true);
    initializeSales();

    connect(_sales, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotPickSales()));
    connect(_sales->header(), SIGNAL(sizeChange(int,int,int)),
	    SLOT(slotSalesSizeChange(int,int,int)));
    connect(_salesFooter, SIGNAL(sizeChange(int,int,int)),
	    SLOT(slotSalesSizeChange(int,int,int)));
    connect(_sales->horizontalScrollBar(), SIGNAL(sliderMoved(int)),
	    _salesFooter, SLOT(setOffset(int)));
    connect(_sales->horizontalScrollBar(), SIGNAL(valueChanged(int)),
	    _salesFooter, SLOT(setOffset(int)));

    QGridLayout* salesGrid = new QGridLayout(_salesFrame);
    salesGrid->setSpacing(6);
    salesGrid->setMargin(3);
    salesGrid->setRowStretch(2, 1);
    salesGrid->setColStretch(2, 1);
    salesGrid->addWidget(deptLabel, 0, 0);
    salesGrid->addWidget(_dept, 0, 1, AlignLeft | AlignVCenter);
    salesGrid->addWidget(allDepts, 0, 2, AlignRight | AlignVCenter);
    salesGrid->addWidget(subdeptLabel, 1, 0);
    salesGrid->addWidget(_subdept, 1, 1, AlignLeft | AlignVCenter);
    salesGrid->addWidget(allSubdepts, 1, 2, AlignRight | AlignVCenter);
    salesGrid->addMultiCellWidget(_sales, 2, 2, 0, 2);
    salesGrid->addMultiCellWidget(_salesFooter, 3, 3, 0, 2);

    QLabel* accountLabel = new QLabel(tr("Account:"), _accountFrame);
    _account = new LookupEdit(new AccountLookup(main, this), _accountFrame);
    _account->setLength(20);
    accountLabel->setBuddy(_account);
    connect(_account, SIGNAL(validData()), SLOT(slotUpdateAccounts()));

    QPushButton* allAccounts = new QPushButton(tr("All Accounts"),
					       _accountFrame);
    connect(allAccounts, SIGNAL(clicked()), SLOT(slotAllAccounts()));

    _accounts = new ListView(_accountFrame);
    _accounts->setAllColumnsShowFocus(true);
    _accounts->setShowSortIndicator(true);
    _accountsFooter = new QHeader(_accountFrame);
    _accountsFooter->setTracking(true);
    initializeAccounts();

    connect(_accounts, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotPickAccounts()));
    connect(_accounts->header(), SIGNAL(sizeChange(int,int,int)),
	    SLOT(slotAccountsSizeChange(int,int,int)));
    connect(_accountsFooter, SIGNAL(sizeChange(int,int,int)),
	    SLOT(slotAccountsSizeChange(int,int,int)));
    connect(_accounts->horizontalScrollBar(), SIGNAL(sliderMoved(int)),
	    _accountsFooter, SLOT(setOffset(int)));
    connect(_accounts->horizontalScrollBar(), SIGNAL(valueChanged(int)),
	    _accountsFooter, SLOT(setOffset(int)));

    QGridLayout* accountGrid = new QGridLayout(_accountFrame);
    accountGrid->setSpacing(6);
    accountGrid->setMargin(3);
    accountGrid->setRowStretch(1, 1);
    accountGrid->setColStretch(2, 1);
    accountGrid->addWidget(accountLabel, 0, 0);
    accountGrid->addWidget(_account, 0, 1, AlignLeft | AlignVCenter);
    accountGrid->addWidget(allAccounts, 0, 2, AlignRight | AlignVCenter);
    accountGrid->addMultiCellWidget(_accounts, 1, 1, 0, 2);
    accountGrid->addMultiCellWidget(_accountsFooter, 2, 2, 0, 2);

    QLabel* tenderLabel = new QLabel(tr("Tender:"), _tenderFrame);
    _tender = new LookupEdit(new TenderLookup(main, this), _tenderFrame);
    _tender->setLength(20);
    tenderLabel->setBuddy(_tender);
    connect(_tender, SIGNAL(validData()), SLOT(slotUpdateTenders()));

    QPushButton* allTenders = new QPushButton(tr("All Tenders"), _tenderFrame);
    connect(allTenders, SIGNAL(clicked()), SLOT(slotAllTenders()));

    _tenders = new ListView(_tenderFrame);
    _tenders->setAllColumnsShowFocus(true);
    _tenders->setShowSortIndicator(true);
    _tendersFooter = new QHeader(_tenderFrame);
    _tendersFooter->setTracking(true);
    initializeTenders();

    connect(_tenders, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotPickTenders()));
    connect(_tenders->header(), SIGNAL(sizeChange(int,int,int)),
	    SLOT(slotTendersSizeChange(int,int,int)));
    connect(_tendersFooter, SIGNAL(sizeChange(int,int,int)),
	    SLOT(slotTendersSizeChange(int,int,int)));
    connect(_tenders->horizontalScrollBar(), SIGNAL(sliderMoved(int)),
	    _tendersFooter, SLOT(setOffset(int)));
    connect(_tenders->horizontalScrollBar(), SIGNAL(valueChanged(int)),
	    _tendersFooter, SLOT(setOffset(int)));

    QGridLayout* tenderGrid = new QGridLayout(_tenderFrame);
    tenderGrid->setSpacing(6);
    tenderGrid->setMargin(3);
    tenderGrid->setRowStretch(1, 1);
    tenderGrid->setColStretch(2, 1);
    tenderGrid->addWidget(tenderLabel, 0, 0);
    tenderGrid->addWidget(_tender, 0, 1, AlignLeft | AlignVCenter);
    tenderGrid->addWidget(allTenders, 0, 2, AlignRight | AlignVCenter);
    tenderGrid->addMultiCellWidget(_tenders, 1, 1, 0, 2);
    tenderGrid->addMultiCellWidget(_tendersFooter, 2, 2, 0, 2);

    QLabel* recLabel = new QLabel(tr("Account:"), _recFrame);
    _recAccount = new LookupEdit(new AccountLookup(main, this, Account::AR),
				 _recFrame);
    _recAccount->setLength(30);
    recLabel->setBuddy(_recAccount);
    connect(_recAccount, SIGNAL(validData()), SLOT(slotUpdateReceivables()));

    QPushButton* allRecs = new QPushButton(tr("All Accounts"), _recFrame);
    connect(allRecs, SIGNAL(clicked()), SLOT(slotAllReceivables()));

    _recs = new ListView(_recFrame);
    _recs->setAllColumnsShowFocus(true);
    _recs->setShowSortIndicator(true);
    _recsFooter = new QHeader(_recFrame);
    _recsFooter->setTracking(true);
    initializeReceivables();

    connect(_recs, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotPickReceivables()));
    connect(_recs->header(), SIGNAL(sizeChange(int,int,int)),
	    SLOT(slotReceivablesSizeChange(int,int,int)));
    connect(_recsFooter, SIGNAL(sizeChange(int,int,int)),
	    SLOT(slotReceivablesSizeChange(int,int,int)));
    connect(_recs->horizontalScrollBar(), SIGNAL(sliderMoved(int)),
	    _recsFooter, SLOT(setOffset(int)));
    connect(_recs->horizontalScrollBar(), SIGNAL(valueChanged(int)),
	    _recsFooter, SLOT(setOffset(int)));

    QGridLayout* recGrid = new QGridLayout(_recFrame);
    recGrid->setSpacing(6);
    recGrid->setMargin(3);
    recGrid->setRowStretch(1, 1);
    recGrid->setColStretch(2, 1);
    recGrid->addWidget(recLabel, 0, 0);
    recGrid->addWidget(_recAccount, 0, 1, AlignLeft | AlignVCenter);
    recGrid->addWidget(allRecs, 0, 2, AlignRight | AlignVCenter);
    recGrid->addMultiCellWidget(_recs, 1, 1, 0, 2);
    recGrid->addMultiCellWidget(_recsFooter, 2, 2, 0, 2);

    QLabel* safeLabel = new QLabel(tr("Tender:"), _safeFrame);
    _safeTender = new LookupEdit(new TenderLookup(main, this), _safeFrame);
    _safeTender->setLength(20);
    safeLabel->setBuddy(_safeTender);
    connect(_safeTender, SIGNAL(validData()), SLOT(slotUpdateSafe()));

    QPushButton* allSafe = new QPushButton(tr("All Tenders"), _safeFrame);
    connect(allSafe, SIGNAL(clicked()), SLOT(slotAllSafe()));

    _safe = new ListView(_safeFrame);
    _safe->setAllColumnsShowFocus(true);
    _safe->setShowSortIndicator(true);
    _safeFooter = new QHeader(_safeFrame);
    _safeFooter->setTracking(true);
    initializeSafe();

    connect(_safe, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotPickSafe()));
    connect(_safe->header(), SIGNAL(sizeChange(int,int,int)),
	    SLOT(slotSafeSizeChange(int,int,int)));
    connect(_safeFooter, SIGNAL(sizeChange(int,int,int)),
	    SLOT(slotSafeSizeChange(int,int,int)));
    connect(_safe->horizontalScrollBar(), SIGNAL(sliderMoved(int)),
	    _safeFooter, SLOT(setOffset(int)));
    connect(_safe->horizontalScrollBar(), SIGNAL(valueChanged(int)),
	    _safeFooter, SLOT(setOffset(int)));

    QGridLayout* safeGrid = new QGridLayout(_safeFrame);
    safeGrid->setSpacing(6);
    safeGrid->setMargin(3);
    safeGrid->setColStretch(2, 1);
    safeGrid->addWidget(safeLabel, 0, 0);
    safeGrid->addWidget(_safeTender, 0, 1, AlignLeft | AlignVCenter);
    safeGrid->addWidget(allSafe, 0, 2, AlignRight | AlignVCenter);
    safeGrid->addMultiCellWidget(_safe, 1, 1, 0, 2);
    safeGrid->addMultiCellWidget(_safeFooter, 2, 2, 0, 2);

    QLabel* taxLabel = new QLabel(tr("Tax:"), _taxFrame);
    _tax = new LookupEdit(new TaxLookup(main, this), _taxFrame);
    _tax->setLength(20);
    taxLabel->setBuddy(_tax);
    connect(_tax, SIGNAL(validData()), SLOT(slotUpdateTaxes()));

    QPushButton* allTaxes = new QPushButton(tr("All Taxes"), _taxFrame);
    connect(allTaxes, SIGNAL(clicked()), SLOT(slotAllTaxes()));

    _taxes = new ListView(_taxFrame);
    _taxes->setAllColumnsShowFocus(true);
    _taxes->setShowSortIndicator(true);
    _taxesFooter = new QHeader(_taxFrame);
    _taxesFooter->setTracking(true);
    initializeTaxes();

    connect(_taxes, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotPickTaxes()));
    connect(_taxes->header(), SIGNAL(sizeChange(int,int,int)),
	    SLOT(slotTaxesSizeChange(int,int,int)));
    connect(_taxesFooter, SIGNAL(sizeChange(int,int,int)),
	    SLOT(slotTaxesSizeChange(int,int,int)));
    connect(_taxes->horizontalScrollBar(), SIGNAL(sliderMoved(int)),
	    _taxesFooter, SLOT(setOffset(int)));
    connect(_taxes->horizontalScrollBar(), SIGNAL(valueChanged(int)),
	    _taxesFooter, SLOT(setOffset(int)));

    QGridLayout* taxGrid = new QGridLayout(_taxFrame);
    taxGrid->setSpacing(6);
    taxGrid->setMargin(3);
    taxGrid->setColStretch(2, 1);
    taxGrid->addWidget(taxLabel, 0, 0);
    taxGrid->addWidget(_tax, 0, 1, AlignLeft | AlignVCenter);
    taxGrid->addWidget(allTaxes, 0, 2, AlignRight | AlignVCenter);
    taxGrid->addMultiCellWidget(_taxes, 1, 1, 0, 2);
    taxGrid->addMultiCellWidget(_taxesFooter, 2, 2, 0, 2);

    QLabel* discountLabel = new QLabel(tr("Discount:"), _discountFrame);
    _discount = new LookupEdit(new DiscountLookup(main, this),_discountFrame);
    _discount->setLength(20);
    discountLabel->setBuddy(_discount);
    connect(_discount, SIGNAL(validData()), SLOT(slotUpdateDiscounts()));

    QPushButton* allDiscounts = new QPushButton(tr("All Discounts"),
						_discountFrame);
    connect(allDiscounts, SIGNAL(clicked()), SLOT(slotAllDiscounts()));

    _discounts = new ListView(_discountFrame);
    _discounts->setAllColumnsShowFocus(true);
    _discounts->setShowSortIndicator(true);
    _discountsFooter = new QHeader(_discountFrame);
    _discountsFooter->setTracking(true);
    initializeDiscounts();

    connect(_discounts, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotPickDiscounts()));
    connect(_discounts->header(), SIGNAL(sizeChange(int,int,int)),
	    SLOT(slotDiscountsSizeChange(int,int,int)));
    connect(_discountsFooter, SIGNAL(sizeChange(int,int,int)),
	    SLOT(slotDiscountsSizeChange(int,int,int)));
    connect(_discounts->horizontalScrollBar(), SIGNAL(sliderMoved(int)),
	    _discountsFooter, SLOT(setOffset(int)));
    connect(_discounts->horizontalScrollBar(), SIGNAL(valueChanged(int)),
	    _discountsFooter, SLOT(setOffset(int)));

    QGridLayout* discountGrid = new QGridLayout(_discountFrame);
    discountGrid->setSpacing(6);
    discountGrid->setMargin(3);
    discountGrid->setRowStretch(1, 1);
    discountGrid->setColStretch(2, 1);
    discountGrid->addWidget(discountLabel, 0, 0);
    discountGrid->addWidget(_discount, 0, 1, AlignLeft | AlignVCenter);
    discountGrid->addWidget(allDiscounts, 0, 2, AlignRight | AlignVCenter);
    discountGrid->addMultiCellWidget(_discounts, 1, 1, 0, 2);
    discountGrid->addMultiCellWidget(_discountsFooter, 2, 2, 0, 2);

    QFrame* buttons = new QFrame(frame);
    QPushButton* refresh = new QPushButton(tr("&Refresh"), buttons);
    connect(refresh, SIGNAL(clicked()), SLOT(slotRefresh()));
    refresh->setMinimumSize(refresh->sizeHint());

    QPushButton* print = new QPushButton(tr("&Print"), buttons);
    connect(print, SIGNAL(clicked()), SLOT(slotPrint()));
    print->setMinimumSize(refresh->sizeHint());

    QPushButton* ok = new QPushButton(tr("&Close"), buttons);
    connect(ok, SIGNAL(clicked()), SLOT(close()));
    ok->setMinimumSize(refresh->sizeHint());

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(2, 1);
    buttonGrid->addWidget(refresh, 0, 0, AlignLeft | AlignVCenter);
    buttonGrid->addWidget(print, 0, 1, AlignLeft | AlignVCenter);
    buttonGrid->addWidget(ok, 0, 2, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(3);
    grid->setRowStretch(1, 1);
    grid->setColStretch(0, 1);
    grid->addWidget(select, 0, 0);
    grid->addWidget(_tabs, 1, 0);
    grid->addWidget(buttons, 2, 0);

    _range->setFocus();
    _range->setCurrentItem(DateRange::Today);

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    _dirtyTypes = false;
    _dirtySales = false;
    _dirtyAccounts = false;
    _dirtyTenders = false;
    _dirtyReceivables = false;
    _dirtySafe = false;
    _dirtyTaxes = false;
    _dirtyDiscounts = false;

    setCentralWidget(frame);
    setCaption(tr("Transaction Summary"));
    finalize();
}

ReportTx::~ReportTx()
{
}

void
ReportTx::slotTypesSizeChange(int section, int, int newSize)
{
    if (sender() == _typesFooter)
	_types->header()->resizeSection(section, newSize);
    else
	_typesFooter->resizeSection(section, newSize);
}

void
ReportTx::slotSalesSizeChange(int section, int, int newSize)
{
    if (sender() == _salesFooter)
	_sales->header()->resizeSection(section, newSize);
    else
	_salesFooter->resizeSection(section, newSize);
}

void
ReportTx::slotAccountsSizeChange(int section, int, int newSize)
{
    if (sender() == _accountsFooter)
	_accounts->header()->resizeSection(section, newSize);
    else
	_accountsFooter->resizeSection(section, newSize);
}

void
ReportTx::slotTendersSizeChange(int section, int, int newSize)
{
    if (sender() == _tendersFooter)
	_tenders->header()->resizeSection(section, newSize);
    else
	_tendersFooter->resizeSection(section, newSize);
}

void
ReportTx::slotReceivablesSizeChange(int section, int, int newSize)
{
    if (sender() == _recsFooter)
	_recs->header()->resizeSection(section, newSize);
    else
	_recsFooter->resizeSection(section, newSize);
}

void
ReportTx::slotSafeSizeChange(int section, int, int newSize)
{
    if (sender() == _taxesFooter)
	_safe->header()->resizeSection(section, newSize);
    else
	_safeFooter->resizeSection(section, newSize);
}

void
ReportTx::slotTaxesSizeChange(int section, int, int newSize)
{
    if (sender() == _taxesFooter)
	_taxes->header()->resizeSection(section, newSize);
    else
	_taxesFooter->resizeSection(section, newSize);
}

void
ReportTx::slotDiscountsSizeChange(int section, int, int newSize)
{
    if (sender() == _discountsFooter)
	_discounts->header()->resizeSection(section, newSize);
    else
	_discountsFooter->resizeSection(section, newSize);
}

void
ReportTx::setDates(QDate from, QDate to)
{
    _from->setDate(from);
    _to->setDate(to);
}

void
ReportTx::setStoreId(Id store_id)
{
    _store->setId(store_id);
}

void
ReportTx::setStationId(Id station_id)
{
    _station->setId(station_id);
}

void
ReportTx::setEmployeeId(Id employee_id)
{
    _employee->setId(employee_id);
}

void
ReportTx::setShiftId(Id shift_id)
{
    _shift->setId(shift_id);
}

void
ReportTx::refresh()
{
    QTimer::singleShot(500, this, SLOT(slotRefresh()));
}

void
ReportTx::slotTabChanged()
{
    QWidget* current = _tabs->currentPage();
    if (current == _typeFrame && _dirtyTypes) slotUpdateTypes();
    if (current == _salesFrame && _dirtySales) slotUpdateSales();
    if (current == _accountFrame && _dirtyAccounts) slotUpdateAccounts();
    if (current == _tenderFrame && _dirtyTenders) slotUpdateTenders();
    if (current == _recFrame && _dirtyReceivables) slotUpdateReceivables();
    if (current == _safeFrame && _dirtySafe) slotUpdateSafe();
    if (current == _taxFrame && _dirtyTaxes) slotUpdateTaxes();
    if (current == _discountFrame && _dirtyDiscounts) slotUpdateDiscounts();
}

void
ReportTx::slotAllTypes()
{
    _type->setCurrentItem(0);
    slotUpdateTypes();
}

void
ReportTx::slotAllDepts()
{
    _dept->setId(INVALID_ID);
    _subdept->setId(INVALID_ID);
    slotUpdateSales();
}

void
ReportTx::slotAllSubdepts()
{
    _subdept->setId(INVALID_ID);
    slotUpdateSales();
}

void
ReportTx::slotAllAccounts()
{
    _account->setId(INVALID_ID);
    slotUpdateAccounts();
}

void
ReportTx::slotAllTenders()
{
    _tender->setId(INVALID_ID);
    slotUpdateTenders();
}

void
ReportTx::slotAllReceivables()
{
    _recAccount->setId(INVALID_ID);
    slotUpdateReceivables();
}

void
ReportTx::slotAllSafe()
{
    _safeTender->setId(INVALID_ID);
    slotUpdateSafe();
}

void
ReportTx::slotAllTaxes()
{
    _tax->setId(INVALID_ID);
    slotUpdateTaxes();
}

void
ReportTx::slotAllDiscounts()
{
    _discount->setId(INVALID_ID);
    slotUpdateDiscounts();
}

void
ReportTx::slotPickTypes()
{
    ListViewItem* item = (ListViewItem*)_types->currentItem();
    if (item == NULL) return;

    if (_type->currentItem() == 0) {
	_type->setCurrentItem(item->text(0));
	slotUpdateTypes();
    } else {
	Gltx gltx;
	Id gltx_id = item->id;
	if (!_quasar->db()->lookup(gltx_id, gltx)) return;

	QWidget* edit = editGltx(gltx, _main);
	if (edit != NULL) edit->show();
    }
}

void
ReportTx::slotPickSales()
{
    ListViewItem* item = (ListViewItem*)_sales->currentItem();
    if (item == NULL) return;

    if (_dept->getId() == INVALID_ID) {
	_dept->setId(item->id);
	slotUpdateSales();
    } else if (_subdept->getId() == INVALID_ID) {
	_subdept->setId(item->id);
	slotUpdateSales();
    } else {
	Gltx gltx;
	Id gltx_id = item->id;
	if (!_quasar->db()->lookup(gltx_id, gltx)) return;

	QWidget* edit = editGltx(gltx, _main);
	if (edit != NULL) edit->show();
    }
}

void
ReportTx::slotPickAccounts()
{
    ListViewItem* item = (ListViewItem*)_accounts->currentItem();
    if (item == NULL) return;

    if (_account->getId() == INVALID_ID) {
	_account->setId(item->id);
	slotUpdateAccounts();
    } else {
	Gltx gltx;
	Id gltx_id = item->id;
	if (!_quasar->db()->lookup(gltx_id, gltx)) return;

	QWidget* edit = editGltx(gltx, _main);
	if (edit != NULL) edit->show();
    }
}

void
ReportTx::slotPickTenders()
{
    ListViewItem* item = (ListViewItem*)_tenders->currentItem();
    if (item == NULL) return;

    if (_tender->getId() == INVALID_ID) {
	_tender->setId(item->id);
	slotUpdateTenders();
    } else {
	Gltx gltx;
	Id gltx_id = item->id;
	if (!_quasar->db()->lookup(gltx_id, gltx)) return;

	QWidget* edit = editGltx(gltx, _main);
	if (edit != NULL) edit->show();
    }
}

void
ReportTx::slotPickReceivables()
{
    ListViewItem* item = (ListViewItem*)_recs->currentItem();
    if (item == NULL) return;

    if (_recAccount->getId() == INVALID_ID) {
	_recAccount->setId(item->id);
	slotUpdateReceivables();
    } else {
	Gltx gltx;
	Id gltx_id = item->id;
	if (!_quasar->db()->lookup(gltx_id, gltx)) return;

	QWidget* edit = editGltx(gltx, _main);
	if (edit != NULL) edit->show();
    }
}

void
ReportTx::slotPickSafe()
{
    ListViewItem* item = (ListViewItem*)_safe->currentItem();
    if (item == NULL) return;

    if (_safeTender->getId() == INVALID_ID) {
	_safeTender->setId(item->id);
	slotUpdateSafe();
    } else {
	Gltx gltx;
	Id gltx_id = item->id;
	if (!_quasar->db()->lookup(gltx_id, gltx)) return;

	QWidget* edit = editGltx(gltx, _main);
	if (edit != NULL) edit->show();
    }
}

void
ReportTx::slotPickTaxes()
{
    ListViewItem* item = (ListViewItem*)_taxes->currentItem();
    if (item == NULL) return;

    if (_tax->getId() == INVALID_ID) {
	_tax->setId(item->id);
	slotUpdateTaxes();
    } else {
	Gltx gltx;
	Id gltx_id = item->id;
	if (!_quasar->db()->lookup(gltx_id, gltx)) return;

	QWidget* edit = editGltx(gltx, _main);
	if (edit != NULL) edit->show();
    }
}

void
ReportTx::slotPickDiscounts()
{
    ListViewItem* item = (ListViewItem*)_discounts->currentItem();
    if (item == NULL) return;

    if (_discount->getId() == INVALID_ID) {
	_discount->setId(item->id);
	slotUpdateDiscounts();
    } else {
	Gltx gltx;
	Id gltx_id = item->id;
	if (!_quasar->db()->lookup(gltx_id, gltx)) return;

	QWidget* edit = editGltx(gltx, _main);
	if (edit != NULL) edit->show();
    }
}

void
ReportTx::slotDeptChanged()
{
    _subdept->setId(INVALID_ID);
    _subdeptLookup->dept->setId(_dept->getId());
    slotUpdateSales();
}

void
ReportTx::slotSubdeptChanged()
{
    if (_subdept->getId() != INVALID_ID) {
	Subdept subdept;
	_cache.findSubdept(_subdept->getId(), subdept);
	_dept->setId(subdept.deptId());
    }
    slotUpdateSales();
}

void
ReportTx::slotPrint()
{
    QDialog* dialog = new QDialog(this, "PrintDialog", true);
    dialog->setCaption(tr("Print Selection"));

    QRadioButton* current = new QRadioButton(tr("Print Current Tab"), dialog);
    QRadioButton* all = new QRadioButton(tr("Print All Tabs"), dialog);
    QRadioButton* select = new QRadioButton(tr("Print Selected Tabs"), dialog);

    QButtonGroup* buttonGroup = new QButtonGroup(dialog);
    buttonGroup->hide();
    buttonGroup->insert(current);
    buttonGroup->insert(all);
    buttonGroup->insert(select);

    QCheckBox* types = new QCheckBox(tr("Types"), dialog);
    QCheckBox* sales = new QCheckBox(tr("Sales"), dialog);
    QCheckBox* accounts = new QCheckBox(tr("Accounts"), dialog);
    QCheckBox* tenders = new QCheckBox(tr("Tenders"), dialog);
    QCheckBox* receivables = new QCheckBox(tr("Receivables"), dialog);
    QCheckBox* safe = new QCheckBox(tr("Safe"), dialog);
    QCheckBox* taxes = new QCheckBox(tr("Taxes"), dialog);
    QCheckBox* discounts = new QCheckBox(tr("Discounts"), dialog);

    connect(select, SIGNAL(toggled(bool)), types, SLOT(setEnabled(bool)));
    connect(select, SIGNAL(toggled(bool)), sales, SLOT(setEnabled(bool)));
    connect(select, SIGNAL(toggled(bool)), accounts, SLOT(setEnabled(bool)));
    connect(select, SIGNAL(toggled(bool)), tenders, SLOT(setEnabled(bool)));
    connect(select, SIGNAL(toggled(bool)), receivables,SLOT(setEnabled(bool)));
    connect(select, SIGNAL(toggled(bool)), safe, SLOT(setEnabled(bool)));
    connect(select, SIGNAL(toggled(bool)), taxes, SLOT(setEnabled(bool)));
    connect(select, SIGNAL(toggled(bool)), discounts, SLOT(setEnabled(bool)));

    QFrame* buttons = new QFrame(dialog);
    QPushButton* ok = new QPushButton(tr("Ok"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);

    ok->connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    cancel->connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
    ok->setDefault(true);

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(ok, 0, 0, AlignRight | AlignVCenter);
    buttonGrid->addWidget(cancel, 0, 1, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(dialog);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->addColSpacing(0, 20);
    grid->addColSpacing(2, 10);
    grid->addMultiCellWidget(current, 0, 0, 0, 3, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(all, 1, 1, 0, 3, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(select, 2, 2, 0, 3, AlignLeft | AlignVCenter);
    grid->addWidget(types, 3, 1);
    grid->addWidget(sales, 4, 1);
    grid->addWidget(accounts, 5, 1);
    grid->addWidget(tenders, 6, 1);
    grid->addWidget(receivables, 3, 3);
    grid->addWidget(safe, 4, 3);
    grid->addWidget(taxes, 5, 3);
    grid->addWidget(discounts, 6, 3);
    grid->addMultiCellWidget(buttons, 7, 7, 0, 3);

    current->setChecked(true);
    types->setEnabled(false);
    sales->setEnabled(false);
    accounts->setEnabled(false);
    tenders->setEnabled(false);
    receivables->setEnabled(false);
    safe->setEnabled(false);
    taxes->setEnabled(false);
    discounts->setEnabled(false);

    int result = dialog->exec();
    if (result == QDialog::Accepted) {
	QValueVector<Grid*> grids;
	QApplication::setOverrideCursor(waitCursor);
	qApp->processEvents();

	if (current->isChecked()) {
	    QWidget* current = _tabs->currentPage();
	    if (current == _typeFrame)
		grids.push_back(typesGrid());
	    else if (current == _salesFrame)
		grids.push_back(salesGrid());
	    else if (current == _accountFrame)
		grids.push_back(accountsGrid());
	    else if (current == _tenderFrame)
		grids.push_back(tendersGrid());
	    else if (current == _recFrame)
		grids.push_back(receivablesGrid());
	    else if (current == _safeFrame)
		grids.push_back(safeGrid());
	    else if (current == _taxFrame)
		grids.push_back(taxesGrid());
	    else if (current == _discountFrame)
		grids.push_back(discountsGrid());
	    else
		assert(false);
	} else if (all->isChecked()) {
	    grids.push_back(typesGrid());
	    grids.push_back(salesGrid());
	    grids.push_back(accountsGrid());
	    grids.push_back(tendersGrid());
	    grids.push_back(receivablesGrid());
	    grids.push_back(safeGrid());
	    grids.push_back(taxesGrid());
	    grids.push_back(discountsGrid());
	} else {
	    if (types->isChecked())
		grids.push_back(typesGrid());
	    if (sales->isChecked())
		grids.push_back(salesGrid());
	    if (accounts->isChecked())
		grids.push_back(accountsGrid());
	    if (tenders->isChecked())
		grids.push_back(tendersGrid());
	    if (receivables->isChecked())
		grids.push_back(receivablesGrid());
	    if (safe->isChecked())
		grids.push_back(safeGrid());
	    if (taxes->isChecked())
		grids.push_back(taxesGrid());
	    if (discounts->isChecked())
		grids.push_back(discountsGrid());
	}

	QApplication::restoreOverrideCursor();
	Grid::print(grids, this);
	for (unsigned int i = 0; i < grids.size(); ++i)
	    delete grids[i];
    }

    delete dialog;
}

QString
ReportTx::buildTitle(const QString& name)
{
    QDate from = _from->getDate();
    QDate to = _to->getDate();
    DateValcon valcon;
    QStringList lines;

    // Company
    Company company;
    _quasar->db()->lookup(company);
    lines << company.name();

    // Dates
    if (from == to && !from.isNull())
	lines << valcon.format(from);
    else if (!from.isNull() && !to.isNull())
	lines << valcon.format(from) + tr("  to  ") + valcon.format(to);
    else if (!from.isNull())
	lines << tr("On/After  ") + valcon.format(from);
    else if (!to.isNull())
	lines << tr("On/Before  ") + valcon.format(to);

    // Shift
    if (_shift->getId() != INVALID_ID) {
	Shift shift;
	_cache.findShift(_shift->getId(), shift);
	lines << tr("Shift #") + shift.number();
    }

    // Station
    if (_station->getId() != INVALID_ID) {
	Station station;
	_cache.findStation(_station->getId(), station);
	lines << tr("Station: ") + station.name();
    }

    // Employee
    if (_employee->getId() != INVALID_ID) {
	Employee employee;
	_cache.findEmployee(_employee->getId(), employee);
	lines << tr("Employee: ") + employee.nameFL();
    }

    // Card
    if (_card->getId() != INVALID_ID) {
	Card* card = _cache.findCard(_card->getId());
	if (card != NULL)
	    lines << card->dataTypeName() + ": " + card->nameFL();
    }

    // Store
    if (_store->getId() != INVALID_ID) {
	Store store;
	_cache.findStore(_store->getId(), store);
	lines << tr("Store: ") + store.name();
    }

    QString title = name;
    if (lines.count() > 0) title += "\n\n" + lines.join("\n");
    return title;
}

Grid*
ReportTx::typesGrid()
{
    slotUpdateTypes();

    QString name = tr("Transactions");
    if (_type->currentItem() != 0)
	name = _type->currentText() + " " + name;

    Grid* grid = Grid::buildGrid(_types, buildTitle(name));

    int row = grid->rows();
    grid->set(row, 0, " ");
    for (int column = 0; column < _types->columns(); ++column) {
	int col = column * 2;
	grid->setBorder(row, col, Grid::Top);
	grid->set(row + 1, col, _typesFooter->label(column));
    }

    return grid;
}

Grid*
ReportTx::salesGrid()
{
    slotUpdateSales();

    QString name;
    if (_subdept->getId() != INVALID_ID) {
	Subdept subdept;
	_cache.findSubdept(_subdept->getId(), subdept);
	name = tr("Sales by Item") + "\n\n" + tr("Sub-department: ") +
	    subdept.name();
    } else if (_dept->getId() != INVALID_ID) {
	Dept dept;
	_cache.findDept(_dept->getId(), dept);
	name = tr("Sales by Sub-department") + "\n\n" + tr("Department: ") +
	    dept.name();
    } else {
	name = tr("Sales by Department");
    }

    Grid* grid = Grid::buildGrid(_sales, buildTitle(name));

    int row = grid->rows();
    grid->set(row, 0, " ");
    for (int column = 0; column < _sales->columns(); ++column) {
	int col = column * 2;
	grid->setBorder(row, col, Grid::Top);
	grid->set(row + 1, col, _salesFooter->label(column));
    }

    return grid;
}

Grid*
ReportTx::accountsGrid()
{
    slotUpdateAccounts();

    QString name = tr("Account Postings");
    if (_account->getId() != INVALID_ID) {
	Account account;
	_cache.findAccount(_account->getId(), account);
	name += "\n" + tr("Account: ") + account.name();
    }

    Grid* grid = Grid::buildGrid(_accounts, buildTitle(name));

    int row = grid->rows();
    grid->set(row, 0, " ");
    for (int column = 0; column < _accounts->columns(); ++column) {
	int col = column * 2;
	grid->setBorder(row, col, Grid::Top);
	grid->set(row + 1, col, _accountsFooter->label(column));
    }

    return grid;
}

Grid*
ReportTx::tendersGrid()
{
    slotUpdateTenders();

    QString name = tr("Tenders");
    if (_tender->getId() != INVALID_ID) {
	Tender tender;
	_cache.findTender(_tender->getId(), tender);
	name = tender.name() + " " + name;
    }

    Font::defaultFont = Font("Times", 7);
    Grid* grid = Grid::buildGrid(_tenders, buildTitle(name));

    int row = grid->rows();
    grid->set(row, 0, " ");
    for (int column = 0; column < _tenders->columns(); ++column) {
	int col = column * 2;
	grid->setBorder(row, col, Grid::Top);
	grid->set(row + 1, col, _tendersFooter->label(column));
    }

    Font::defaultFont = Font();
    return grid;
}

Grid*
ReportTx::receivablesGrid()
{
    slotUpdateReceivables();

    QString name = tr("Receivables");
    if (_recAccount->getId() != INVALID_ID) {
	Account account;
	_cache.findAccount(_recAccount->getId(), account);
	name = account.name() + " " + name;
    }

    Grid* grid = Grid::buildGrid(_recs, buildTitle(name));

    int row = grid->rows();
    grid->set(row, 0, " ");
    for (int column = 0; column < _recs->columns(); ++column) {
	int col = column * 2;
	grid->setBorder(row, col, Grid::Top);
	grid->set(row + 1, col, _recsFooter->label(column));
    }

    return grid;
}

Grid*
ReportTx::safeGrid()
{
    slotUpdateSafe();

    QString name = tr("Safe");
    if (_safeTender->getId() != INVALID_ID) {
	Tender tender;
	_cache.findTender(_safeTender->getId(), tender);
	name = tender.name() + " " + name;
    }

    Grid* grid = Grid::buildGrid(_safe, buildTitle(name));

    int row = grid->rows();
    grid->set(row, 0, " ");
    for (int column = 0; column < _safe->columns(); ++column) {
	int col = column * 2;
	grid->setBorder(row, col, Grid::Top);
	grid->set(row + 1, col, _safeFooter->label(column));
    }

    return grid;
}

Grid*
ReportTx::taxesGrid()
{
    slotUpdateTaxes();

    QString name = tr("Taxes");
    if (_tax->getId() != INVALID_ID) {
	Tax tax;
	_cache.findTax(_tax->getId(), tax);
	name += "\n" + tr("Tax: ") + tax.name();
    }

    Grid* grid = Grid::buildGrid(_taxes, buildTitle(name));

    int row = grid->rows();
    grid->set(row, 0, " ");
    for (int column = 0; column < _taxes->columns(); ++column) {
	int col = column * 2;
	grid->setBorder(row, col, Grid::Top);
	grid->set(row + 1, col, _taxesFooter->label(column));
    }

    return grid;
}

Grid*
ReportTx::discountsGrid()
{
    slotUpdateDiscounts();

    QString name = tr("Discounts");
    if (_discount->getId() != INVALID_ID) {
	Discount discount;
	_cache.findDiscount(_discount->getId(), discount);
	name += "\n" + tr("Discount: ") + discount.name();
    }

    Grid* grid = Grid::buildGrid(_discounts, buildTitle(name));

    int row = grid->rows();
    grid->set(row, 0, " ");
    for (int column = 0; column < _discounts->columns(); ++column) {
	int col = column * 2;
	grid->setBorder(row, col, Grid::Top);
	grid->set(row + 1, col, _discountsFooter->label(column));
    }

    return grid;
}

void
ReportTx::printTypes()
{
    if (_types->childCount() == 0) {
	QMessageBox::warning(this, tr("Warning"), tr("No data to print"));
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = typesGrid();

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
ReportTx::printSales()
{
    if (_sales->childCount() == 0) {
	QMessageBox::warning(this, tr("Warning"), tr("No data to print"));
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = salesGrid();

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
ReportTx::printAccounts()
{
    if (_accounts->childCount() == 0) {
	QMessageBox::warning(this, tr("Warning"), tr("No data to print"));
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = accountsGrid();

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
ReportTx::printTenders()
{
    if (_tenders->childCount() == 0) {
	QMessageBox::warning(this, tr("Warning"), tr("No data to print"));
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = tendersGrid();

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
ReportTx::printReceivables()
{
    if (_recs->childCount() == 0) {
	QMessageBox::warning(this, tr("Warning"), tr("No data to print"));
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = receivablesGrid();

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
ReportTx::printSafe()
{
    if (_safe->childCount() == 0) {
	QMessageBox::warning(this, tr("Warning"), tr("No data to print"));
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = safeGrid();

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
ReportTx::printTaxes()
{
    if (_taxes->childCount() == 0) {
	QMessageBox::warning(this, tr("Warning"), tr("No data to print"));
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = taxesGrid();

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
ReportTx::printDiscounts()
{
    if (_discounts->childCount() == 0) {
	QMessageBox::warning(this, tr("Warning"), tr("No data to print"));
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = discountsGrid();

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
ReportTx::printSummary()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    QValueVector<Grid*> grids;
    grids.push_back(accountsGrid());
    grids.push_back(tendersGrid());
    grids.push_back(receivablesGrid());
    grids.push_back(safeGrid());

    QApplication::restoreOverrideCursor();
    Grid::print(grids, this);
    for (unsigned int i = 0; i < grids.size(); ++i)
	delete grids[i];
}

void
ReportTx::slotRefresh()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Load transactions
    GltxSelect conditions;
    conditions.start_date = _from->getDate();
    conditions.end_date = _to->getDate();
    conditions.shift_id = _shift->getId();
    conditions.station_id = _station->getId();
    conditions.employee_id = _employee->getId();
    conditions.card_id = _card->getId();
    conditions.store_id = _store->getId();
    conditions.cashrecOnly = _cashrecOnly->isChecked();
    _quasar->db()->select(_gltxs, conditions);

    // Mark everything dirty
    _dirtyTypes = true;
    _dirtySales = true;
    _dirtyAccounts = true;
    _dirtyTenders = true;
    _dirtyReceivables = true;
    _dirtySafe = true;
    _dirtyTaxes = true;
    _dirtyDiscounts = true;

    // Update based on current tab
    QWidget* current = _tabs->currentPage();
    if (current == _typeFrame) slotUpdateTypes();
    if (current == _salesFrame) slotUpdateSales();
    if (current == _accountFrame) slotUpdateAccounts();
    if (current == _tenderFrame) slotUpdateTenders();
    if (current == _recFrame) slotUpdateReceivables();
    if (current == _safeFrame) slotUpdateSafe();
    if (current == _taxFrame) slotUpdateTaxes();
    if (current == _discountFrame) slotUpdateDiscounts();

    QApplication::restoreOverrideCursor();
}

void
ReportTx::initializeTypes()
{
    _types->clear();
    while (_types->columns() > 0) _types->removeColumn(0);
    while (_typesFooter->count() > 0) _typesFooter->removeLabel(0);

    if (_type->currentItem() == 0) {
	_types->addTextColumn(tr("Type"), 20);
	_types->addNumberColumn(tr("Count"), 6);
	_types->addNumberColumn(tr("Voids"), 6);
	_types->addMoneyColumn(tr("Amount"));
	_types->addMoneyColumn(tr("Base"));
	_types->addMoneyColumn(tr("Tax"));
	_types->addMoneyColumn(tr("Deposit"));
    } else {
	_types->addTextColumn(tr("Number"), 10);
	_types->addTextColumn(tr("Employee"), 18);
	_types->addTextColumn(tr("Customer"), 18);
	_types->addMoneyColumn(tr("Amount"));
	_types->addMoneyColumn(tr("Base"));
	_types->addMoneyColumn(tr("Tax"));
	_types->addMoneyColumn(tr("Deposit"));
	_types->addCheckColumn(tr("Voided?"));
    }

    QHeader* typesHeader = _types->header();
    for (int i = 0; i < typesHeader->count(); ++i)
	_typesFooter->addLabel(" ", typesHeader->sectionSize(i));
}

void
ReportTx::slotUpdateTypes()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Setup types list
    initializeTypes();

    // Totals for list
    vector<QString> tx_types;
    vector<int> tx_good_cnts;
    vector<int> tx_void_cnts;
    vector<fixed> tx_amts;
    vector<fixed> tx_taxes;
    vector<fixed> tx_deposits;
    int goodTotal = 0;
    int voidTotal = 0;
    fixed amountTotal = 0.0;
    fixed baseTotal = 0.0;
    fixed taxTotal = 0.0;
    fixed depositTotal = 0.0;

    unsigned int i;
    for (i = 0; i < _gltxs.size(); ++i) {
	const Gltx& gltx = _gltxs[i];
	DataObject::DataType type = gltx.dataType();

	Employee employee;
	_cache.findEmployee(gltx.employeeId(), employee);

	Card* card = _cache.findCard(gltx.cardId());

	fixed sign = 1.0;
	if (type == DataObject::RETURN) sign = -1.0;
	fixed gltxAmount = gltx.amount() * sign;
	fixed taxAmount = gltx.taxTotal() * sign;
	if (type == DataObject::RETURN) gltxAmount = -gltxAmount;
	if (type == DataObject::PAYOUT) taxAmount = -taxAmount;

	fixed depositAmount = 0.0;
	if (type == DataObject::INVOICE || type == DataObject::RETURN) {
	    Invoice invoice;
	    _cache.findInvoice(gltx.id(), invoice);

	    depositAmount = invoice.depositTotal() * sign;
	}
	if (type == DataObject::RECEIVE || type == DataObject::CLAIM) {
	    Receive receive;
	    _cache.findReceive(gltx.id(), receive);

	    depositAmount = receive.depositTotal() * sign;
	}
	fixed baseAmount = gltxAmount - taxAmount - depositAmount;

	// Add to types detail or total
	if (_type->currentItem() != 0) {
	    QString type = _type->currentText();
	    if (type == gltx.dataTypeName()) {
		ListViewItem* item = new ListViewItem(_types, gltx.id());
		item->setValue(0, gltx.number());
		item->setValue(1, employee.name());
		item->setValue(2, card != NULL ? card->name() : "");
		item->setValue(3, gltxAmount);
		item->setValue(4, baseAmount);
		item->setValue(5, taxAmount);
		item->setValue(6, depositAmount);
		item->setValue(7, !gltx.isActive());

		if (gltx.isActive()) {
		    goodTotal += 1;
		    amountTotal += gltxAmount;
		    baseTotal += baseAmount;
		    taxTotal += taxAmount;
		    depositTotal += depositAmount;
		} else{
		    voidTotal += 1;
		}
	    }
	} else {
	    bool found = false;
	    for (unsigned int j = 0; j < tx_types.size(); ++j) {
		if (tx_types[j] != gltx.dataTypeName()) continue;

		found = true;
		if (gltx.isActive()) {
		    tx_good_cnts[j] += 1;
		    tx_amts[j] += gltxAmount;
		    tx_taxes[j] += taxAmount;
		    tx_deposits[j] += depositAmount;
		} else {
		    tx_void_cnts[j] += 1;
		}
		break;
	    }

	    if (!found) {
		tx_types.push_back(gltx.dataTypeName());
		if (gltx.isActive()) {
		    tx_good_cnts.push_back(1);
		    tx_void_cnts.push_back(0);
		    tx_amts.push_back(gltxAmount);
		    tx_taxes.push_back(taxAmount);
		    tx_deposits.push_back(depositAmount);
		} else {
		    tx_good_cnts.push_back(0);
		    tx_void_cnts.push_back(1);
		    tx_amts.push_back(0.0);
		    tx_taxes.push_back(0.0);
		    tx_deposits.push_back(0.0);
		}
	    }
	}
    }

    // Load types list
    for (i = 0; i < tx_types.size(); ++i) {
	ListViewItem* item = new ListViewItem(_types);
	item->setValue(0, tx_types[i]);
	item->setValue(1, tx_good_cnts[i]);
	item->setValue(2, tx_void_cnts[i]);
	item->setValue(3, tx_amts[i]);
	item->setValue(4, tx_amts[i] - tx_taxes[i] - tx_deposits[i]);
	item->setValue(5, tx_taxes[i]);
	item->setValue(6, tx_deposits[i]);

	goodTotal += tx_good_cnts[i];
	voidTotal += tx_void_cnts[i];
	amountTotal += tx_amts[i];
	baseTotal += tx_amts[i] - tx_taxes[i] - tx_deposits[i];
	taxTotal += tx_taxes[i];
	depositTotal += tx_deposits[i];
    }

    QHeader* typesHeader = _types->header();
    for (int c = 0; c < typesHeader->count(); ++c)
	_typesFooter->resizeSection(c, typesHeader->sectionSize(c));

    _typesFooter->setLabel(0, tr("Totals:"));
    _typesFooter->setLabel(1, QString::number(goodTotal));
    _typesFooter->setLabel(2, QString::number(voidTotal));
    _typesFooter->setLabel(3, MoneyValcon().format(amountTotal));
    _typesFooter->setLabel(4, MoneyValcon().format(baseTotal));
    _typesFooter->setLabel(5, MoneyValcon().format(taxTotal));
    _typesFooter->setLabel(6, MoneyValcon().format(depositTotal));

    _dirtyTypes = false;

    QApplication::restoreOverrideCursor();
}

void
ReportTx::initializeSales()
{
    _sales->clear();
    while (_sales->columns() > 0) _sales->removeColumn(0);
    while (_salesFooter->count() > 0) _salesFooter->removeLabel(0);

    if (_dept->getId() == INVALID_ID) {
	_sales->addTextColumn(tr("Department"), 30);
	_sales->addNumberColumn(tr("Quantity"), 8);
	_sales->addMoneyColumn(tr("Sales"));
	_sales->addMoneyColumn(tr("Cost"));
	_sales->addMoneyColumn(tr("Profit"));
	_sales->addPercentColumn(tr("Margin"));
    } else if (_subdept->getId() == INVALID_ID) {
	_sales->addTextColumn(tr("Sub-department"), 30);
	_sales->addNumberColumn(tr("Quantity"), 8);
	_sales->addMoneyColumn(tr("Sales"));
	_sales->addMoneyColumn(tr("Cost"));
	_sales->addMoneyColumn(tr("Profit"));
	_sales->addPercentColumn(tr("Margin"));
    } else {
	_sales->addTextColumn(tr("Item"), 20);
	_sales->addTextColumn(tr("Size"), 8);
	_sales->addNumberColumn(tr("Quantity"), 8);
	_sales->addMoneyColumn(tr("Sales"));
	_sales->addMoneyColumn(tr("Cost"));
	_sales->addMoneyColumn(tr("Profit"));
	_sales->addPercentColumn(tr("Margin"));
    }

    QHeader* salesHeader = _sales->header();
    for (int i = 0; i < salesHeader->count(); ++i)
	_salesFooter->addLabel(" ", salesHeader->sectionSize(i));
}

void
ReportTx::slotUpdateSales()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Setup sales list
    initializeSales();

    // Totals for list
    vector<Id> sale_ids;
    vector<QString> sale_sizes;
    vector<fixed> sale_qtys;
    vector<fixed> sale_sales;
    vector<fixed> sale_costs;
    fixed qtyTotal = 0.0;
    fixed salesTotal = 0.0;
    fixed costTotal = 0.0;

    unsigned int i;
    for (i = 0; i < _gltxs.size(); ++i) {
	const Gltx& gltx = _gltxs[i];
	DataObject::DataType type = gltx.dataType();

	Employee employee;
	_cache.findEmployee(gltx.employeeId(), employee);

	// Done if voided
	if (!gltx.isActive()) continue;

	// Add to sales
	if (type == DataObject::INVOICE || type == DataObject::RETURN) {
	    Invoice invoice;
	    _cache.findInvoice(gltx.id(), invoice);

	    // Item sales
	    for (unsigned int j = 0; j < invoice.items().size(); ++j) {
		const InvoiceItem& line = invoice.items()[j];
		if (line.voided) continue;
		if (line.item_id == INVALID_ID) continue;

		Item item;
		_cache.findItem(line.item_id, item);

		Id id;
		QString size = "";
		fixed sale_amt = line.sale_price;
		fixed cost_amt = line.inv_cost;
		fixed qty = line.quantity;

		if (_dept->getId() == INVALID_ID)
		    id = item.deptId();
		else if (item.deptId() != _dept->getId())
		    continue;
		else if (_subdept->getId() == INVALID_ID)
		    id = item.subdeptId();
		else if (item.subdeptId() != _subdept->getId())
		    continue;
		else {
		    id = item.id();
		    size = line.size;
		}

		bool found = false;
		for (unsigned int k = 0; k < sale_ids.size(); ++k) {
		    if (sale_ids[k] != id) continue;
		    if (sale_sizes[k] != size) continue;

		    found = true;
		    sale_qtys[k] += qty;
		    sale_sales[k] += sale_amt;
		    sale_costs[k] += cost_amt;
		    break;
		}

		if (!found) {
		    sale_ids.push_back(id);
		    sale_sizes.push_back(size);
		    sale_qtys.push_back(qty);
		    sale_sales.push_back(sale_amt);
		    sale_costs.push_back(cost_amt);
		}
	    }
	}
    }

    // Load sales list
    for (i = 0; i < sale_ids.size(); ++i) {
	fixed qty = sale_qtys[i];
	fixed sales = sale_sales[i];
	fixed cost = sale_costs[i];
	fixed profit = sales - cost;
	fixed margin = 0.0;
	if (sales != 0.0) margin = profit / sales * 100.0;
	int next = 1;

	ListViewItem* item = new ListViewItem(_sales, sale_ids[i]);
	if (_subdept->getId() != INVALID_ID)
	    item->setValue(next++, sale_sizes[i]);
	item->setValue(next++, qty);
	item->setValue(next++, sales);
	item->setValue(next++, cost);
	item->setValue(next++, profit);
	item->setValue(next++, margin);

	qtyTotal += qty;
	salesTotal += sales;
	costTotal += cost;

	if (_dept->getId() == INVALID_ID) {
	    Dept dept;
	    _cache.findDept(sale_ids[i], dept);
	    item->setValue(0, dept.name());
	} else if (_subdept->getId() == INVALID_ID) {
	    Subdept subdept;
	    _cache.findSubdept(sale_ids[i], subdept);
	    item->setValue(0, subdept.name());
	} else {
	    Item itm;
	    _cache.findItem(sale_ids[i], itm);
	    item->setValue(0, itm.description());
	}
    }

    fixed profitTotal = salesTotal - costTotal;
    fixed margin = 0.0;
    if (salesTotal != 0.0) margin = profitTotal / salesTotal * 100.0;

    QHeader* salesHeader = _sales->header();
    for (int c = 0; c < salesHeader->count(); ++c)
	_salesFooter->resizeSection(c, salesHeader->sectionSize(c));

    if (_subdept->getId() == INVALID_ID) {
	_salesFooter->setLabel(0, tr("Totals:"));
	_salesFooter->setLabel(1, qtyTotal.toString());
	_salesFooter->setLabel(2, MoneyValcon().format(salesTotal));
	_salesFooter->setLabel(3, MoneyValcon().format(costTotal));
	_salesFooter->setLabel(4, MoneyValcon().format(profitTotal));
	_salesFooter->setLabel(5, PercentValcon().format(margin));
    } else {
	_salesFooter->setLabel(0, tr("Totals:"));
	_salesFooter->setLabel(2, qtyTotal.toString());
	_salesFooter->setLabel(3, MoneyValcon().format(salesTotal));
	_salesFooter->setLabel(4, MoneyValcon().format(costTotal));
	_salesFooter->setLabel(5, MoneyValcon().format(profitTotal));
	_salesFooter->setLabel(6, PercentValcon().format(margin));
    }

    _dirtySales = false;

    QApplication::restoreOverrideCursor();
}

void
ReportTx::initializeAccounts()
{
    _accounts->clear();
    while (_accounts->columns() > 0) _accounts->removeColumn(0);
    while (_accountsFooter->count() > 0) _accountsFooter->removeLabel(0);

    if (_account->getId() == INVALID_ID) {
	_accounts->addTextColumn(tr("Account"), 30);
	_accounts->addTextColumn(tr("Number"), 12);
	_accounts->addNumberColumn(tr("Count"), 6);
	_accounts->addMoneyColumn(tr("Debit"));
	_accounts->addMoneyColumn(tr("Credit"));
    } else {
	_accounts->addTextColumn(tr("Type"), 20);
	_accounts->addTextColumn(tr("Number"), 10);
	_accounts->addTextColumn(tr("Employee"), 18);
	_accounts->addTextColumn(tr("Customer"), 18);
	_accounts->addMoneyColumn(tr("Debit"));
	_accounts->addMoneyColumn(tr("Credit"));
    }

    QHeader* accountsHeader = _accounts->header();
    for (int i = 0; i < accountsHeader->count(); ++i)
	_accountsFooter->addLabel(" ", accountsHeader->sectionSize(i));
}

void
ReportTx::slotUpdateAccounts()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Setup accounts list
    initializeAccounts();

    // Totals for list
    vector<Id> account_ids;
    vector<int> account_cnts;
    vector<fixed> account_debit_amts;
    vector<fixed> account_credit_amts;
    int countTotal = 0;
    fixed debitTotal = 0.0;
    fixed creditTotal = 0.0;

    unsigned int i;
    for (i = 0; i < _gltxs.size(); ++i) {
	const Gltx& gltx = _gltxs[i];

	Employee employee;
	_cache.findEmployee(gltx.employeeId(), employee);

	Card* card = _cache.findCard(gltx.cardId());

	// Done if voided
	if (!gltx.isActive()) continue;

	// Add to accounts
	for (unsigned int j = 0; j < gltx.accounts().size(); ++j) {
	    const AccountLine& line = gltx.accounts()[j];

	    Id account_id = line.account_id;
	    fixed amount = line.amount;

	    if (_account->getId() != INVALID_ID) {
		if (account_id != _account->getId()) continue;

		Account account;
		_cache.findAccount(account_id, account);

		ListViewItem* item = new ListViewItem(_accounts, gltx.id());
		item->setValue(0, gltx.dataTypeName());
		item->setValue(1, gltx.number());
		item->setValue(2, employee.name());
		item->setValue(3, card != NULL ? card->name() : "");
		if (amount >= 0.0) {
		    item->setValue(4, amount);
		    debitTotal += amount;
		} else {
		    item->setValue(5, -amount);
		    creditTotal += amount;
		}
		countTotal += 1;
		continue;
	    }

	    bool found = false;
	    for (unsigned int k = 0; k < account_ids.size(); ++k) {
		if (account_ids[k] != account_id) continue;

		found = true;
		account_cnts[k] += 1;
		if (amount < 0.0)
		    account_credit_amts[k] += -amount;
		else
		    account_debit_amts[k] += amount;
		break;
	    }

	    if (!found) {
		account_ids.push_back(account_id);
		account_cnts.push_back(1);
		if (amount < 0.0) {
		    account_credit_amts.push_back(-amount);
		    account_debit_amts.push_back(0.0);
		} else {
		    account_credit_amts.push_back(0.0);
		    account_debit_amts.push_back(amount);
		}
	    }
	}
    }

    // Load accounts list
    for (i = 0; i < account_ids.size(); ++i) {
	Account account;
	_cache.findAccount(account_ids[i], account);

	fixed debitAmt = account_debit_amts[i];
	fixed creditAmt = account_credit_amts[i];
	fixed net = debitAmt - creditAmt;

	ListViewItem* item = new ListViewItem(_accounts, account_ids[i]);
	item->setValue(0, account.name());
	item->setValue(1, account.number());
	item->setValue(2, account_cnts[i]);
	if (net >= 0.0) {
	    item->setValue(3, net);
	    debitTotal += net;
	} else {
	    item->setValue(4, -net);
	    creditTotal += net;
	}
	countTotal += account_cnts[i];
    }

    QHeader* accountsHeader = _accounts->header();
    for (int c = 0; c < accountsHeader->count(); ++c)
	_accountsFooter->resizeSection(c, accountsHeader->sectionSize(c));

    if (_account->getId() == INVALID_ID) {
	_accountsFooter->setLabel(0, tr("Totals:"));
	_accountsFooter->setLabel(2, QString::number(countTotal));
	_accountsFooter->setLabel(3, MoneyValcon().format(debitTotal));
	_accountsFooter->setLabel(4, MoneyValcon().format(-creditTotal));
    } else {
	_accountsFooter->setLabel(0, tr("Totals:"));
	_accountsFooter->setLabel(1, QString::number(countTotal));
	_accountsFooter->setLabel(4, MoneyValcon().format(debitTotal));
	_accountsFooter->setLabel(5, MoneyValcon().format(-creditTotal));
    }

    _dirtyAccounts = false;

    QApplication::restoreOverrideCursor();
}

void
ReportTx::initializeTenders()
{
    _tenders->clear();
    while (_tenders->columns() > 0) _tenders->removeColumn(0);
    while (_tendersFooter->count() > 0) _tendersFooter->removeLabel(0);

    if (_tender->getId() == INVALID_ID) {
	_tenders->addTextColumn(tr("Tender"), 20);
	_tenders->addNumberColumn(tr("Count"), 6);
	_tenders->addMoneyColumn(tr("Sales"), 6);
	_tenders->addMoneyColumn(tr("Returns"), 6);
	_tenders->addMoneyColumn(tr("Payouts"), 6);
	_tenders->addMoneyColumn(tr("Payments"), 6);
	_tenders->addMoneyColumn(tr("Withdraws"), 6);
	_tenders->addMoneyColumn(tr("Adjusts"), 6);
	_tenders->addMoneyColumn(tr("Total"), 6);
	_tenders->addMoneyColumn(tr("Rungoff"), 6);
	_tenders->addMoneyColumn(tr("Over"), 6);
	_tenders->addMoneyColumn(tr("Deposit"), 6);
    } else {
	_tenders->addTextColumn(tr("Type"), 20);
	_tenders->addTextColumn(tr("Number"), 10);
	_tenders->addTextColumn(tr("Employee"), 18);
	_tenders->addTextColumn(tr("Customer"), 18);
	_tenders->addMoneyColumn(tr("Amount"));
    }

    QHeader* tendersHeader = _tenders->header();
    for (int i = 0; i < tendersHeader->count(); ++i)
	_tendersFooter->addLabel(" ", tendersHeader->sectionSize(i));
}

void
ReportTx::slotUpdateTenders()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Setup tenders list
    initializeTenders();

    // Totals for list
    vector<Id> tender_ids;
    vector<int> tender_cnts;
    vector<fixed> tender_sale_amts;
    vector<fixed> tender_return_amts;
    vector<fixed> tender_payout_amts;
    vector<fixed> tender_payment_amts;
    vector<fixed> tender_withdraw_amts;
    vector<fixed> tender_adjust_amts;
    vector<fixed> tender_rungoff_amts;
    vector<fixed> tender_over_amts;
    vector<fixed> tender_deposit_amts;
    int countTotal = 0;
    fixed saleTotal = 0.0;
    fixed returnTotal = 0.0;
    fixed payoutTotal = 0.0;
    fixed paymentTotal = 0.0;
    fixed withdrawTotal = 0.0;
    fixed adjustTotal = 0.0;
    fixed tenderTotal = 0.0;
    fixed rungoffTotal = 0.0;
    fixed overTotal = 0.0;
    fixed depositTotal = 0.0;

    Company company;
    _quasar->db()->lookup(company);

    unsigned int i;
    for (i = 0; i < _gltxs.size(); ++i) {
	const Gltx& gltx = _gltxs[i];
	DataObject::DataType type = gltx.dataType();

	Employee employee;
	_cache.findEmployee(gltx.employeeId(), employee);

	Card* card = _cache.findCard(gltx.cardId());

	// Done if voided or is a safe transaction
	if (!gltx.isActive()) continue;
	if (company.shiftMethod() == Company::BY_STATION) {
	    if (gltx.stationId() == company.safeStation()) continue;
	} else {
	    if (gltx.employeeId() == company.safeEmployee()) continue;
	}

	// Skip if this is the deposit to the safe
	if (gltx.dataType() == DataObject::TEND_ADJUST)
	    if (gltx.memo() == "Deposit to safe")
		continue;

	// Add to tenders
	for (unsigned int j = 0; j < gltx.tenders().size(); ++j) {
	    const TenderLine& line = gltx.tenders()[j];
	    if (line.voided) continue;

	    Id tender_id = line.tender_id;
	    fixed amount = line.conv_amt;

	    if (_tender->getId() != INVALID_ID) {
		if (tender_id != _tender->getId()) continue;

		ListViewItem* item = new ListViewItem(_tenders, gltx.id());
		item->setValue(0, gltx.dataTypeName());
		item->setValue(1, gltx.number());
		item->setValue(2, employee.name());
		item->setValue(3, card != NULL ? card->name() : "");
		item->setValue(4, amount);

		countTotal += 1;
		tenderTotal += amount;
		continue;
	    }

	    Shift shift;
	    _cache.findShift(gltx.shiftId(), shift);

	    fixed rungoff = 0.0;
	    fixed deposit = 0.0;
	    if (gltx.shiftId() != INVALID_ID)
		rungoff = amount;
	    if (shift.shiftId() != INVALID_ID)
		deposit = amount;

	    bool found = false;
	    for (unsigned int k = 0; k < tender_ids.size(); ++k) {
		if (tender_ids[k] != tender_id) continue;

		found = true;
		if (!line.is_change)
		    tender_cnts[k] += 1;

		tender_rungoff_amts[k] += rungoff;
		tender_deposit_amts[k] += deposit;

		switch (type) {
		case DataObject::INVOICE:
		    tender_sale_amts[k] += amount;
		    break;
		case DataObject::RETURN:
		    tender_return_amts[k] += amount;
		    break;
		case DataObject::PAYOUT:
		    tender_payout_amts[k] += amount;
		    break;
		case DataObject::RECEIPT:
		    tender_payment_amts[k] += amount;
		    break;
		case DataObject::WITHDRAW:
		    tender_withdraw_amts[k] += amount;
		    break;
		case DataObject::TEND_ADJUST:
		    if (gltx.memo() == "Over/Short") {
			tender_over_amts[k] += amount;
			tender_rungoff_amts[k] -= rungoff;
		    } else {
			tender_adjust_amts[k] += amount;
		    }
		    break;
		default:
		    tender_adjust_amts[k] += amount;
		    break;
		}
		break;
	    }

	    if (!found) {
		tender_ids.push_back(tender_id);
		tender_cnts.push_back(line.is_change ? 0 : 1);
		switch (type) {
		case DataObject::INVOICE:
		    tender_sale_amts.push_back(amount);
		    tender_return_amts.push_back(0.0);
		    tender_payout_amts.push_back(0.0);
		    tender_payment_amts.push_back(0.0);
		    tender_withdraw_amts.push_back(0.0);
		    tender_adjust_amts.push_back(0.0);
		    tender_rungoff_amts.push_back(rungoff);
		    tender_over_amts.push_back(0.0);
		    tender_deposit_amts.push_back(deposit);
		    break;
		case DataObject::RETURN:
		    tender_sale_amts.push_back(0.0);
		    tender_return_amts.push_back(amount);
		    tender_payout_amts.push_back(0.0);
		    tender_payment_amts.push_back(0.0);
		    tender_withdraw_amts.push_back(0.0);
		    tender_adjust_amts.push_back(0.0);
		    tender_rungoff_amts.push_back(rungoff);
		    tender_over_amts.push_back(0.0);
		    tender_deposit_amts.push_back(deposit);
		    break;
		case DataObject::PAYOUT:
		    tender_sale_amts.push_back(0.0);
		    tender_return_amts.push_back(0.0);
		    tender_payout_amts.push_back(amount);
		    tender_payment_amts.push_back(0.0);
		    tender_withdraw_amts.push_back(0.0);
		    tender_adjust_amts.push_back(0.0);
		    tender_rungoff_amts.push_back(rungoff);
		    tender_over_amts.push_back(0.0);
		    tender_deposit_amts.push_back(deposit);
		    break;
		case DataObject::RECEIPT:
		    tender_sale_amts.push_back(0.0);
		    tender_return_amts.push_back(0.0);
		    tender_payout_amts.push_back(0.0);
		    tender_payment_amts.push_back(amount);
		    tender_withdraw_amts.push_back(0.0);
		    tender_adjust_amts.push_back(0.0);
		    tender_rungoff_amts.push_back(rungoff);
		    tender_over_amts.push_back(0.0);
		    tender_deposit_amts.push_back(deposit);
		    break;
		case DataObject::WITHDRAW:
		    tender_sale_amts.push_back(0.0);
		    tender_return_amts.push_back(0.0);
		    tender_payout_amts.push_back(0.0);
		    tender_payment_amts.push_back(0.0);
		    tender_withdraw_amts.push_back(amount);
		    tender_adjust_amts.push_back(0.0);
		    tender_rungoff_amts.push_back(rungoff);
		    tender_over_amts.push_back(0.0);
		    tender_deposit_amts.push_back(deposit);
		    break;
		case DataObject::TEND_ADJUST:
		    tender_sale_amts.push_back(0.0);
		    tender_return_amts.push_back(0.0);
		    tender_payout_amts.push_back(0.0);
		    tender_payment_amts.push_back(0.0);
		    tender_withdraw_amts.push_back(0.0);
		    if (gltx.memo() == "Over/Short") {
			tender_adjust_amts.push_back(0.0);
			tender_rungoff_amts.push_back(0.0);
			tender_over_amts.push_back(amount);
			tender_deposit_amts.push_back(deposit);
		    } else {
			tender_adjust_amts.push_back(amount);
			tender_rungoff_amts.push_back(rungoff);
			tender_over_amts.push_back(0.0);
			tender_deposit_amts.push_back(deposit);
		    }
		    break;
		default:
		    tender_sale_amts.push_back(0.0);
		    tender_return_amts.push_back(0.0);
		    tender_payout_amts.push_back(0.0);
		    tender_payment_amts.push_back(0.0);
		    tender_withdraw_amts.push_back(0.0);
		    tender_adjust_amts.push_back(amount);
		    tender_rungoff_amts.push_back(rungoff);
		    tender_over_amts.push_back(0.0);
		    tender_deposit_amts.push_back(deposit);
		    break;
		}
	    }
	}
    }

    // Load tenders list
    for (i = 0; i < tender_ids.size(); ++i) {
	Tender tender;
	_cache.findTender(tender_ids[i], tender);
	fixed saleAmt = tender_sale_amts[i];
	fixed returnAmt = tender_return_amts[i];
	fixed payoutAmt = tender_payout_amts[i];
	fixed paymentAmt = tender_payment_amts[i];
	fixed withdrawAmt = tender_withdraw_amts[i];
	fixed adjustAmt = tender_adjust_amts[i];
	fixed rungoffAmt = tender_rungoff_amts[i];
	fixed overAmt = tender_over_amts[i];
	fixed depositAmt = tender_deposit_amts[i];

	fixed totalAmt = saleAmt + returnAmt + payoutAmt + paymentAmt +
	    withdrawAmt + adjustAmt;

	ListViewItem* item = new ListViewItem(_tenders, tender_ids[i]);
	item->setValue(0, tender.name());
	item->setValue(1, tender_cnts[i]);
	item->setValue(2, saleAmt);
	item->setValue(3, returnAmt);
	item->setValue(4, payoutAmt);
	item->setValue(5, paymentAmt);
	item->setValue(6, withdrawAmt);
	item->setValue(7, adjustAmt);
	item->setValue(8, totalAmt);
	item->setValue(9, rungoffAmt);
	item->setValue(10, overAmt);
	item->setValue(11, depositAmt);

	countTotal += tender_cnts[i];
	saleTotal += saleAmt;
	returnTotal += returnAmt;
	payoutTotal += payoutAmt;
	paymentTotal += paymentAmt;
	withdrawTotal += withdrawAmt;
	adjustTotal += adjustAmt;
	tenderTotal += totalAmt;
	rungoffTotal += rungoffAmt;
	overTotal += overAmt;
	depositTotal += depositAmt;
    }

    QHeader* tendersHeader = _tenders->header();
    for (int c = 0; c < tendersHeader->count(); ++c)
	_tendersFooter->resizeSection(c, tendersHeader->sectionSize(c));

    if (_tender->getId() == INVALID_ID) {
	_tendersFooter->setLabel(0, tr("Totals:"));
	_tendersFooter->setLabel(1, QString::number(countTotal));
	_tendersFooter->setLabel(2, MoneyValcon().format(saleTotal));
	_tendersFooter->setLabel(3, MoneyValcon().format(returnTotal));
	_tendersFooter->setLabel(4, MoneyValcon().format(payoutTotal));
	_tendersFooter->setLabel(5, MoneyValcon().format(paymentTotal));
	_tendersFooter->setLabel(6, MoneyValcon().format(withdrawTotal));
	_tendersFooter->setLabel(7, MoneyValcon().format(adjustTotal));
	_tendersFooter->setLabel(8, MoneyValcon().format(tenderTotal));
	_tendersFooter->setLabel(9, MoneyValcon().format(rungoffTotal));
	_tendersFooter->setLabel(10, MoneyValcon().format(overTotal));
	_tendersFooter->setLabel(11, MoneyValcon().format(depositTotal));
    } else {
	_tendersFooter->setLabel(0, tr("Totals:"));
	_tendersFooter->setLabel(1, QString::number(countTotal));
	_tendersFooter->setLabel(4, MoneyValcon().format(tenderTotal));
    }

    _dirtyTenders = false;

    QApplication::restoreOverrideCursor();
}

void
ReportTx::initializeReceivables()
{
    _recs->clear();
    while (_recs->columns() > 0) _recs->removeColumn(0);
    while (_recsFooter->count() > 0) _recsFooter->removeLabel(0);

    if (_recAccount->getId() == INVALID_ID) {
	_recs->addTextColumn(tr("Account"), 20);
	_recs->addNumberColumn(tr("Count"), 6);
	_recs->addMoneyColumn(tr("Open"));
	_recs->addMoneyColumn(tr("Sales"));
	_recs->addMoneyColumn(tr("Returns"));
	_recs->addMoneyColumn(tr("Payments"));
	_recs->addMoneyColumn(tr("Withdraws"));
	_recs->addMoneyColumn(tr("Other"));
	_recs->addMoneyColumn(tr("Total"));
	_recs->addMoneyColumn(tr("Close"));
    } else {
	_recs->addTextColumn(tr("Type"), 20);
	_recs->addTextColumn(tr("Number"), 10);
	_recs->addTextColumn(tr("Employee"), 18);
	_recs->addTextColumn(tr("Customer"), 18);
	_recs->addMoneyColumn(tr("Amount"));
    }

    QHeader* recsHeader = _recs->header();
    for (int i = 0; i < recsHeader->count(); ++i)
	_recsFooter->addLabel(" ", recsHeader->sectionSize(i));
}

void
ReportTx::slotUpdateReceivables()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Setup receivables list
    initializeReceivables();

    // Totals for list
    vector<Id> rec_ids;
    vector<int> rec_cnts;
    vector<fixed> rec_sale_amts;
    vector<fixed> rec_return_amts;
    vector<fixed> rec_payment_amts;
    vector<fixed> rec_withdraw_amts;
    vector<fixed> rec_other_amts;
    int countTotal = 0;
    fixed openTotal = 0.0;
    fixed saleTotal = 0.0;
    fixed returnTotal = 0.0;
    fixed paymentTotal = 0.0;
    fixed withdrawTotal = 0.0;
    fixed otherTotal = 0.0;
    fixed recTotal = 0.0;
    fixed closeTotal = 0.0;

    if (_recAccount->getId() == INVALID_ID) {
	AccountSelect conditions;
	conditions.type = Account::AR;
	conditions.activeOnly = true;
	conditions.postingOnly = true;
	vector<Account> accounts;
	_quasar->db()->select(accounts, conditions);

	for (unsigned int i = 0; i < accounts.size(); ++i) {
	    rec_ids.push_back(accounts[i].id());
	    rec_cnts.push_back(0);
	    rec_sale_amts.push_back(0.0);
	    rec_return_amts.push_back(0.0);
	    rec_payment_amts.push_back(0.0);
	    rec_withdraw_amts.push_back(0.0);
	    rec_other_amts.push_back(0.0);
	}
    }

    unsigned int i;
    for (i = 0; i < _gltxs.size(); ++i) {
	const Gltx& gltx = _gltxs[i];
	DataObject::DataType type = gltx.dataType();
	if (gltx.cardId() == INVALID_ID) continue;

	Employee employee;
	_cache.findEmployee(gltx.employeeId(), employee);

	Card* card = _cache.findCard(gltx.cardId());
	if (card == NULL || card->dataType() != DataObject::CUSTOMER)
	    continue;

	Customer customer;
	_cache.findCustomer(gltx.cardId(), customer);

	// Done if voided
	if (!gltx.isActive()) continue;

	// Get change to customer balance
	Id account_id = customer.accountId();
	fixed amount = gltx.cardTotal();
	if (account_id == INVALID_ID || amount == 0.0)
	    continue;

	if (_recAccount->getId() != INVALID_ID) {
	    if (account_id != _recAccount->getId()) continue;

	    ListViewItem* item = new ListViewItem(_recs, gltx.id());
	    item->setValue(0, gltx.dataTypeName());
	    item->setValue(1, gltx.number());
	    item->setValue(2, employee.name());
	    item->setValue(3, customer.name());
	    item->setValue(4, amount);

	    countTotal += 1;
	    recTotal += amount;
	    continue;
	}

	bool found = false;
	for (unsigned int k = 0; k < rec_ids.size(); ++k) {
	    if (rec_ids[k] != account_id) continue;

	    found = true;
	    rec_cnts[k] += 1;

	    switch (type) {
	    case DataObject::INVOICE:
		rec_sale_amts[k] += amount;
		break;
	    case DataObject::RETURN:
		rec_return_amts[k] += amount;
		break;
	    case DataObject::RECEIPT:
		rec_payment_amts[k] += amount;
		break;
	    case DataObject::WITHDRAW:
		rec_withdraw_amts[k] += amount;
		break;
	    default:
		rec_other_amts[k] += amount;
		break;
	    }
	    break;
	}

	if (!found) {
	    rec_ids.push_back(account_id);
	    rec_cnts.push_back(1);
	    switch (type) {
	    case DataObject::INVOICE:
		rec_sale_amts.push_back(amount);
		rec_return_amts.push_back(0.0);
		rec_payment_amts.push_back(0.0);
		rec_withdraw_amts.push_back(0.0);
		rec_other_amts.push_back(0.0);
		break;
	    case DataObject::RETURN:
		rec_sale_amts.push_back(0.0);
		rec_return_amts.push_back(amount);
		rec_payment_amts.push_back(0.0);
		rec_withdraw_amts.push_back(0.0);
		rec_other_amts.push_back(0.0);
		break;
	    case DataObject::RECEIPT:
		rec_sale_amts.push_back(0.0);
		rec_return_amts.push_back(0.0);
		rec_payment_amts.push_back(amount);
		rec_withdraw_amts.push_back(0.0);
		rec_other_amts.push_back(0.0);
		break;
	    case DataObject::WITHDRAW:
		rec_sale_amts.push_back(0.0);
		rec_return_amts.push_back(0.0);
		rec_payment_amts.push_back(0.0);
		rec_withdraw_amts.push_back(amount);
		rec_other_amts.push_back(0.0);
		break;
	    default:
		rec_sale_amts.push_back(0.0);
		rec_return_amts.push_back(0.0);
		rec_payment_amts.push_back(0.0);
		rec_withdraw_amts.push_back(0.0);
		rec_other_amts.push_back(amount);
		break;
	    }
	}
    }

    // Load receivables list
    for (i = 0; i < rec_ids.size(); ++i) {
	Account account;
	_cache.findAccount(rec_ids[i], account);
	fixed saleAmt = rec_sale_amts[i];
	fixed returnAmt = rec_return_amts[i];
	fixed paymentAmt = rec_payment_amts[i];
	fixed withdrawAmt = rec_withdraw_amts[i];
	fixed otherAmt = rec_other_amts[i];
	fixed total = saleAmt + returnAmt + paymentAmt + withdrawAmt +
	    otherAmt;

	QDate date = _from->getDate() - 1;
	Id store_id = _store->getId();
	fixed openAmt = _quasar->db()->accountBalance(account.id(), date,
						      store_id);
	fixed closeAmt = openAmt + total;

	ListViewItem* item = new ListViewItem(_recs, rec_ids[i]);
	item->setValue(0, account.name());
	item->setValue(1, rec_cnts[i]);
	item->setValue(2, openAmt);
	item->setValue(3, saleAmt);
	item->setValue(4, returnAmt);
	item->setValue(5, paymentAmt);
	item->setValue(6, withdrawAmt);
	item->setValue(7, otherAmt);
	item->setValue(8, total);
	item->setValue(9, closeAmt);

	countTotal += rec_cnts[i];
	openTotal += openAmt;
	saleTotal += saleAmt;
	returnTotal += returnAmt;
	paymentTotal += paymentAmt;
	withdrawTotal += withdrawAmt;
	otherTotal += otherAmt;
	recTotal += total;
	closeTotal += closeAmt;
    }

    QHeader* recsHeader = _recs->header();
    for (int c = 0; c < recsHeader->count(); ++c)
	_recsFooter->resizeSection(c, recsHeader->sectionSize(c));

    if (_recAccount->getId() == INVALID_ID) {
	_recsFooter->setLabel(0, tr("Totals:"));
	_recsFooter->setLabel(1, QString::number(countTotal));
	_recsFooter->setLabel(2, MoneyValcon().format(openTotal));
	_recsFooter->setLabel(3, MoneyValcon().format(saleTotal));
	_recsFooter->setLabel(4, MoneyValcon().format(returnTotal));
	_recsFooter->setLabel(5, MoneyValcon().format(paymentTotal));
	_recsFooter->setLabel(6, MoneyValcon().format(withdrawTotal));
	_recsFooter->setLabel(7, MoneyValcon().format(otherTotal));
	_recsFooter->setLabel(8, MoneyValcon().format(recTotal));
	_recsFooter->setLabel(9, MoneyValcon().format(closeTotal));
    } else {
	_recsFooter->setLabel(0, tr("Totals:"));
	_recsFooter->setLabel(1, QString::number(countTotal));
	_recsFooter->setLabel(4, MoneyValcon().format(recTotal));
    }

    _dirtyReceivables = false;

    QApplication::restoreOverrideCursor();
}

void
ReportTx::initializeSafe()
{
    _safe->clear();
    while (_safe->columns() > 0) _safe->removeColumn(0);
    while (_safeFooter->count() > 0) _safeFooter->removeLabel(0);

    if (_safeTender->getId() == INVALID_ID) {
	_safe->addTextColumn(tr("Tender"), 20);
	_safe->addNumberColumn(tr("Count"), 6);
	_safe->addMoneyColumn(tr("Open"));
	_safe->addMoneyColumn(tr("Deposit"));
	_safe->addMoneyColumn(tr("Bank"));
	_safe->addMoneyColumn(tr("Other"));
	_safe->addMoneyColumn(tr("Total"));
	_safe->addMoneyColumn(tr("Over"));
	_safe->addMoneyColumn(tr("Close"));
    } else {
	_safe->addTextColumn(tr("Type"), 20);
	_safe->addTextColumn(tr("Number"), 10);
	_safe->addTextColumn(tr("Employee"), 18);
	_safe->addTextColumn(tr("Customer"), 18);
	_safe->addMoneyColumn(tr("Amount"));
    }

    QHeader* safeHeader = _safe->header();
    for (int i = 0; i < safeHeader->count(); ++i)
	_safeFooter->addLabel(" ", safeHeader->sectionSize(i));
}

void
ReportTx::slotUpdateSafe()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Setup safe list
    initializeSafe();

    // Totals for list
    vector<Tender> tenders;
    vector<int> tender_cnts;
    vector<fixed> tender_deposit_amts;
    vector<fixed> tender_bank_amts;
    vector<fixed> tender_other_amts;
    vector<fixed> tender_over_amts;
    int countTotal = 0;
    fixed openTotal = 0.0;
    fixed depositTotal = 0.0;
    fixed bankTotal = 0.0;
    fixed otherTotal = 0.0;
    fixed tenderTotal = 0.0;
    fixed overTotal = 0.0;
    fixed closeTotal = 0.0;

    Company company;
    _quasar->db()->lookup(company);

    QString safeName;
    if (company.shiftMethod() == Company::BY_STATION) {
	Station station;
	_cache.findStation(company.safeStation(), station);
	safeName = station.name();
    } else {
	Employee employee;
	_cache.findEmployee(company.safeEmployee(), employee);
	safeName = employee.name();
    }

    // Load tenders and set initial values
    unsigned int i;
    if (_safeTender->getId() == INVALID_ID) {
	_quasar->db()->select(tenders, TenderSelect());
	for (i = 0; i < tenders.size(); ++i) {
	    tender_cnts.push_back(0);
	    tender_deposit_amts.push_back(0.0);
	    tender_bank_amts.push_back(0.0);
	    tender_other_amts.push_back(0.0);
	    tender_over_amts.push_back(0.0);
	}
    }

    for (i = 0; i < _gltxs.size(); ++i) {
	const Gltx& gltx = _gltxs[i];
	DataObject::DataType type = gltx.dataType();

	Employee employee;
	_cache.findEmployee(gltx.employeeId(), employee);

	Card* card = _cache.findCard(gltx.cardId());

	// Done if voided
	if (!gltx.isActive()) continue;
	if (company.shiftMethod() == Company::BY_STATION) {
	    if (gltx.stationId() != company.safeStation()) continue;
	} else {
	    if (gltx.employeeId() != company.safeEmployee()) continue;
	}

	// Add to tenders
	for (unsigned int j = 0; j < gltx.tenders().size(); ++j) {
	    const TenderLine& line = gltx.tenders()[j];
	    if (line.voided) continue;

	    Id tender_id = line.tender_id;
	    fixed amount = line.conv_amt;

	    if (_safeTender->getId() != INVALID_ID) {
		if (tender_id != _safeTender->getId()) continue;

		ListViewItem* item = new ListViewItem(_safe, gltx.id());
		item->setValue(0, gltx.dataTypeName());
		item->setValue(1, gltx.number());
		item->setValue(2, employee.name());
		item->setValue(3, card != NULL ? card->name() : "");
		item->setValue(4, amount);

		countTotal += 1;
		tenderTotal += amount;
		continue;
	    }

	    for (unsigned int k = 0; k < tenders.size(); ++k) {
		if (tenders[k].id() != tender_id) continue;

		if (!line.is_change)
		    tender_cnts[k] += 1;

		switch (type) {
		case DataObject::TEND_ADJUST:
		    if (gltx.memo() == "Over/Short")
			tender_over_amts[k] += amount;
		    else if (gltx.memo() == "Bank deposit")
			tender_bank_amts[k] += -amount;
		    else if (gltx.memo().left(7) == "Deposit")
			tender_deposit_amts[k] += amount;
		    else
			tender_other_amts[k] += amount;
		    break;
		default:
		    tender_other_amts[k] += amount;
		    break;
		}
		break;
	    }
	}
    }

    // Get the open balances
    vector<Id> open_ids;
    vector<fixed> open_amts;
    QDate date = _from->getDate();
    _quasar->db()->safeOpenBalance(date, open_ids, open_amts);

    // Load safe list
    for (i = 0; i < tenders.size(); ++i) {
	const Tender& tender = tenders[i];
	fixed depositAmt = tender_deposit_amts[i];
	fixed bankAmt = tender_bank_amts[i];
	fixed otherAmt = tender_other_amts[i];
	fixed overAmt = tender_over_amts[i];

	fixed openAmt = 0.0;
	for (unsigned int j = 0; j < open_ids.size(); ++j) {
	    if (open_ids[j] == tender.id()) {
		openAmt = open_amts[j];
		break;
	    }
	}

	fixed totalAmt = openAmt + depositAmt - bankAmt + otherAmt;
	fixed closeAmt = totalAmt + overAmt;

	if (openAmt == 0.0 && depositAmt == 0.0 && bankAmt == 0.0 &&
	    otherAmt == 0.0 && overAmt == 0.0) continue;

	ListViewItem* item = new ListViewItem(_safe, tender.id());
	item->setValue(0, tender.name());
	item->setValue(1, tender_cnts[i]);
	item->setValue(2, openAmt);
	item->setValue(3, depositAmt);
	item->setValue(4, bankAmt);
	item->setValue(5, otherAmt);
	item->setValue(6, totalAmt);
	item->setValue(7, overAmt);
	item->setValue(8, closeAmt);

	countTotal += tender_cnts[i];
	openTotal += openAmt;
	depositTotal += depositAmt;
	bankTotal += bankAmt;
	otherTotal += otherAmt;
	tenderTotal += totalAmt;
	overTotal += overAmt;
	closeTotal += closeAmt;
    }

    QHeader* safeHeader = _safe->header();
    for (int c = 0; c < safeHeader->count(); ++c)
	_safeFooter->resizeSection(c, safeHeader->sectionSize(c));

    if (_safeTender->getId() == INVALID_ID) {
	_safeFooter->setLabel(0, tr("Totals:"));
	_safeFooter->setLabel(1, QString::number(countTotal));
	_safeFooter->setLabel(2, MoneyValcon().format(openTotal));
	_safeFooter->setLabel(3, MoneyValcon().format(depositTotal));
	_safeFooter->setLabel(4, MoneyValcon().format(bankTotal));
	_safeFooter->setLabel(5, MoneyValcon().format(otherTotal));
	_safeFooter->setLabel(6, MoneyValcon().format(tenderTotal));
	_safeFooter->setLabel(7, MoneyValcon().format(overTotal));
	_safeFooter->setLabel(8, MoneyValcon().format(closeTotal));
    } else {
	_safeFooter->setLabel(0, tr("Totals:"));
	_safeFooter->setLabel(1, QString::number(countTotal));
	_safeFooter->setLabel(4, MoneyValcon().format(tenderTotal));
    }

    _dirtySafe = false;

    QApplication::restoreOverrideCursor();
}

void
ReportTx::initializeTaxes()
{
    _taxes->clear();
    while (_taxes->columns() > 0) _taxes->removeColumn(0);
    while (_taxesFooter->count() > 0) _taxesFooter->removeLabel(0);

    if (_tax->getId() == INVALID_ID) {
	_taxes->addTextColumn(tr("Tax"), 20);
	_taxes->addNumberColumn(tr("Count"), 6);
	_taxes->addMoneyColumn(tr("Collected"));
	_taxes->addMoneyColumn(tr("Paid"));
	_taxes->addMoneyColumn(tr("Total"));
    } else {
	_taxes->addTextColumn(tr("Type"), 20);
	_taxes->addTextColumn(tr("Number"), 10);
	_taxes->addTextColumn(tr("Employee"), 18);
	_taxes->addTextColumn(tr("Customer"), 18);
	_taxes->addMoneyColumn(tr("Amount"));
    }

    QHeader* taxesHeader = _taxes->header();
    for (int i = 0; i < taxesHeader->count(); ++i)
	_taxesFooter->addLabel(" ", taxesHeader->sectionSize(i));
}

void
ReportTx::slotUpdateTaxes()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Setup taxes list
    initializeTaxes();

    // Totals for list
    vector<Id> tax_ids;
    vector<int> tax_cnts;
    vector<fixed> tax_collect_amts;
    vector<fixed> tax_paid_amts;
    int countTotal = 0;
    fixed collectedTotal = 0.0;
    fixed paidTotal = 0.0;
    fixed taxTotal = 0.0;

    unsigned int i;
    for (i = 0; i < _gltxs.size(); ++i) {
	const Gltx& gltx = _gltxs[i];
	DataObject::DataType type = gltx.dataType();

	Employee employee;
	_cache.findEmployee(gltx.employeeId(), employee);

	Card* card = _cache.findCard(gltx.cardId());

	fixed sign = 1.0;
	if (type == DataObject::RETURN || type == DataObject::PAYOUT ||
		type == DataObject::RECEIVE)
	    sign = -1.0;

	// Done if voided
	if (!gltx.isActive()) continue;

	// Add to taxes
	unsigned int j;
	for (j = 0; j < gltx.taxes().size(); ++j) {
	    const TaxLine& line = gltx.taxes()[j];

	    Id tax_id = line.tax_id;
	    fixed amount = (line.amount + line.inc_amount) * sign;

	    if (_tax->getId() != INVALID_ID) {
		if (tax_id != _tax->getId()) continue;

		Tax tax;
		_cache.findTax(tax_id, tax);

		ListViewItem* item = new ListViewItem(_taxes, gltx.id());
		item->setValue(0, gltx.dataTypeName());
		item->setValue(1, gltx.number());
		item->setValue(2, employee.name());
		item->setValue(3, card != NULL ? card->name() : "");
		item->setValue(4, amount);

		countTotal += 1;
		taxTotal += amount;
		continue;
	    }

	    bool found = false;
	    for (unsigned int k = 0; k < tax_ids.size(); ++k) {
		if (tax_ids[k] != tax_id) continue;

		found = true;
		tax_cnts[k] += 1;
		if (amount > 0.0)
		    tax_collect_amts[k] += amount;
		else
		    tax_paid_amts[k] += amount;
	    }

	    if (!found) {
		tax_ids.push_back(tax_id);
		tax_cnts.push_back(1);
		if (amount >= 0.0) {
		    tax_collect_amts.push_back(amount);
		    tax_paid_amts.push_back(0.0);
		} else {
		    tax_collect_amts.push_back(0.0);
		    tax_paid_amts.push_back(amount);
		}
	    }
	}
    }

    // Load taxes list
    for (i = 0; i < tax_ids.size(); ++i) {
	Tax tax;
	_cache.findTax(tax_ids[i], tax);
	fixed collectAmt = tax_collect_amts[i];
	fixed paidAmt = tax_paid_amts[i];
	fixed total = collectAmt + paidAmt;

	ListViewItem* item = new ListViewItem(_taxes, tax_ids[i]);
	item->setValue(0, tax.name());
	item->setValue(1, tax_cnts[i]);
	item->setValue(2, collectAmt);
	item->setValue(3, -paidAmt);
	item->setValue(4, total);

	countTotal += tax_cnts[i];
	collectedTotal += collectAmt;
	paidTotal += paidAmt;
	taxTotal += total;
    }

    QHeader* taxesHeader = _taxes->header();
    for (int c = 0; c < taxesHeader->count(); ++c)
	_taxesFooter->resizeSection(c, taxesHeader->sectionSize(c));

    if (_tax->getId() == INVALID_ID) {
	_taxesFooter->setLabel(0, tr("Totals:"));
	_taxesFooter->setLabel(1, QString::number(countTotal));
	_taxesFooter->setLabel(2, MoneyValcon().format(collectedTotal));
	_taxesFooter->setLabel(3, MoneyValcon().format(-paidTotal));
	_taxesFooter->setLabel(4, MoneyValcon().format(taxTotal));
    } else {
	_taxesFooter->setLabel(0, tr("Totals:"));
	_taxesFooter->setLabel(1, QString::number(countTotal));
	_taxesFooter->setLabel(4, MoneyValcon().format(taxTotal));
    }

    _dirtyTaxes = false;

    QApplication::restoreOverrideCursor();
}

void
ReportTx::initializeDiscounts()
{
    _discounts->clear();
    while (_discounts->columns() > 0) _discounts->removeColumn(0);
    while (_discountsFooter->count() > 0) _discountsFooter->removeLabel(0);

    if (_discount->getId() == INVALID_ID) {
	_discounts->addTextColumn(tr("Discount"), 20);
	_discounts->addNumberColumn(tr("Count"), 6);
	_discounts->addMoneyColumn(tr("Amount"));
    } else {
	_discounts->addTextColumn(tr("Type"), 20);
	_discounts->addTextColumn(tr("Number"), 10);
	_discounts->addTextColumn(tr("Employee"), 18);
	_discounts->addTextColumn(tr("Customer"), 18);
	_discounts->addMoneyColumn(tr("Amount"));
    }

    QHeader* discountsHeader = _discounts->header();
    for (int i = 0; i < discountsHeader->count(); ++i)
	_discountsFooter->addLabel(" ", discountsHeader->sectionSize(i));
}

void
ReportTx::slotUpdateDiscounts()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Setup discounts list
    initializeDiscounts();

    // Totals for list
    vector<Id> discount_ids;
    vector<int> discount_cnts;
    vector<fixed> discount_amts;
    int countTotal = 0;
    fixed discountTotal = 0.0;

    unsigned int i;
    for (i = 0; i < _gltxs.size(); ++i) {
	const Gltx& gltx = _gltxs[i];
	DataObject::DataType type = gltx.dataType();

	Employee employee;
	_cache.findEmployee(gltx.employeeId(), employee);

	Card* card = _cache.findCard(gltx.cardId());

	fixed sign = 1.0;
	if (type == DataObject::RETURN) sign = -1.0;

	// Done if voided
	if (!gltx.isActive()) continue;

	// Add to discounts
	if (type == DataObject::INVOICE || type == DataObject::RETURN) {
	    Invoice invoice;
	    _cache.findInvoice(gltx.id(), invoice);

	    // Transaction discounts
	    unsigned int j;
	    for (j = 0; j < invoice.discounts().size(); ++j) {
		const InvoiceDisc& line = invoice.discounts()[j];
		if (line.voided) continue;

		Id discount_id = line.discount_id;
		fixed amount = line.total_amt * sign;

		if (_discount->getId() != INVALID_ID) {
		    if (discount_id != _discount->getId()) continue;
		
		    Discount discount;
		    _cache.findDiscount(discount_id, discount);

		    ListViewItem* item = new ListViewItem(_discounts,
							  invoice.id());
		    item->setValue(0, gltx.dataTypeName());
		    item->setValue(1, gltx.number());
		    item->setValue(2, employee.name());
		    item->setValue(3, card != NULL ? card->name() : "");
		    item->setValue(4, amount);

		    countTotal += 1;
		    discountTotal += amount;
		    continue;
		}

		bool found = false;
		for (unsigned int k = 0; k < discount_ids.size(); ++k) {
		    if (discount_ids[k] != discount_id) continue;

		    found = true;
		    discount_cnts[k] += 1;
		    discount_amts[k] += amount;
		    break;
		}

		if (!found) {
		    discount_ids.push_back(discount_id);
		    discount_cnts.push_back(1);
		    discount_amts.push_back(amount);
		}
	    }

	    // Line discounts
	    for (j = 0; j < invoice.items().size(); ++j) {
		if (invoice.items()[j].voided) continue;

		const InvoiceDisc& line = invoice.items()[j].line_disc;
		if (line.voided) continue;
		if (line.discount_id == INVALID_ID) continue;

		Id discount_id = line.discount_id;
		fixed amount = line.total_amt * sign;

		if (_discount->getId() != INVALID_ID) {
		    if (discount_id != _discount->getId()) continue;

		    Discount discount;
		    _cache.findDiscount(discount_id, discount);

		    ListViewItem* item = new ListViewItem(_discounts,
							  invoice.id());
		    item->setValue(0, gltx.dataTypeName());
		    item->setValue(1, gltx.number());
		    item->setValue(2, employee.name());
		    item->setValue(3, card != NULL ? card->name() : "");
		    item->setValue(4, amount);

		    countTotal += 1;
		    discountTotal += amount;
		    continue;
		}

		bool found = false;
		for (unsigned int k = 0; k < discount_ids.size(); ++k) {
		    if (discount_ids[k] != discount_id) continue;

		    found = true;
		    discount_cnts[k] += 1;
		    discount_amts[k] += amount;
		    break;
		}

		if (!found) {
		    discount_ids.push_back(discount_id);
		    discount_cnts.push_back(1);
		    discount_amts.push_back(amount);
		}
	    }
	}
    }

    // Load discounts list
    for (i = 0; i < discount_ids.size(); ++i) {
	Discount discount;
	_cache.findDiscount(discount_ids[i], discount);

	ListViewItem* item = new ListViewItem(_discounts, discount_ids[i]);
	item->setValue(0, discount.name());
	item->setValue(1, discount_cnts[i]);
	item->setValue(2, discount_amts[i]);

	countTotal += discount_cnts[i];
	discountTotal += discount_amts[i];
    }

    QHeader* discountsHeader = _discounts->header();
    for (int c = 0; c < discountsHeader->count(); ++c)
	_discountsFooter->resizeSection(c, discountsHeader->sectionSize(c));

    if (_discount->getId() == INVALID_ID) {
	_discountsFooter->setLabel(0, tr("Totals:"));
	_discountsFooter->setLabel(1, QString::number(countTotal));
	_discountsFooter->setLabel(2, MoneyValcon().format(discountTotal));
    } else {
	_discountsFooter->setLabel(0, tr("Totals:"));
	_discountsFooter->setLabel(1, QString::number(countTotal));
	_discountsFooter->setLabel(4, MoneyValcon().format(discountTotal));
    }

    _dirtyDiscounts = false;

    QApplication::restoreOverrideCursor();
}
